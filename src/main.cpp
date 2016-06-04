#include "sansation.hpp"

#include <SFML/Graphics.hpp>

#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>

class Plot{
	public:
		static float scaleX(float xi, float xf){ return 800/(xf-xi); }
		static float scaleY(float yi, float yf){ return 600/(yf-yi); }

		void add(std::string type, std::string fileName){
			_subplots.push_back({type, fileName});
		}

		void plot(){
			for(auto& s: _subplots){
				//clear
				s.x.clear();
				s.y.clear();
				//open
				std::ifstream file(s.fileName);
				if(!file.is_open()) continue;
				//read
				if(s.type=="line"){
					float value;
					int i=0;
					while(file>>value){
						s.x.push_back(float(i++));
						s.y.push_back(value);
					}
				}
				else if(s.type=="heat"){
					float x, y;
					while(file>>x&&file>>y){
						s.x.push_back(x);
						s.y.push_back(y);
					}
				}
				//range
				if(s.x.size()){
					s.xi=*std::min_element(s.x.begin(), s.x.end());
					s.xf=*std::max_element(s.x.begin(), s.x.end());
				}
				if(s.y.size()){
					s.yi=*std::min_element(s.y.begin(), s.y.end());
					s.yf=*std::max_element(s.y.begin(), s.y.end());
				}
			}
		}

		void draw(sf::RenderTarget& target, const sf::Font& font, float xi, float xf, float yi, float yf){
			unsigned x=0, y=0;
			for(const auto& s: _subplots){
				const unsigned
					PLOT_WIDTH =unsigned(190*scaleX(xi, xf)),
					PLOT_HEIGHT=unsigned(100*scaleY(yi, yf)),
					PLOT_SPACE_WIDTH =PLOT_WIDTH +10,
					PLOT_SPACE_HEIGHT=PLOT_HEIGHT+10,
					TEXT_HEIGHT=12,
					TEXT_SPACE=16;
				float originX=1.0f*x*PLOT_SPACE_WIDTH -xi*scaleX(xi, xf);
				float originY=1.0f*y*PLOT_SPACE_HEIGHT-yi*scaleY(yi, yf);
				//name
				std::string fileName=s.fileName.substr(s.fileName.find_last_of("/\\")+1);
				sf::Text name(fileName.c_str(), font, TEXT_HEIGHT);
				name.setPosition(originX, originY);
				target.draw(name);
				//plot
				if(s.type=="line"){
					//line
					sf::VertexArray va(sf::LinesStrip);
					for(unsigned i=0; i<s.x.size(); ++i) va.append(sf::Vertex(sf::Vector2f(
						originX                       +1.0f*(PLOT_WIDTH              )*(s.x[i]-s.xi)/(s.xf-s.xi),
						originY+PLOT_HEIGHT-TEXT_SPACE-1.0f*(PLOT_HEIGHT-2*TEXT_SPACE)*(s.y[i]-s.yi)/(s.yf-s.yi)
					)));
					target.draw(va);
					//range
					std::stringstream ss;
					ss<<s.xi<<".."<<s.xf<<", "<<s.yi<<".."<<s.yf;
					sf::Text range(ss.str().c_str(), font, TEXT_HEIGHT);
					range.setPosition(originX, originY+PLOT_HEIGHT-TEXT_HEIGHT);
					target.draw(range);
				}
				else if(s.type=="heat"){
					//heat
					sf::VertexArray va(sf::Triangles);
					for(unsigned i=0; i<s.x.size(); ++i){
						float rxi=originX                       +1.0f*(PLOT_WIDTH              )*(s.x[i]-s.xi)/(s.xf-s.xi+1);
						float rxf=rxi                           +1.0f*(PLOT_WIDTH              )*(          1)/(s.xf-s.xi+1);
						float ryi=originY+PLOT_HEIGHT-TEXT_SPACE-1.0f*(PLOT_HEIGHT-2*TEXT_SPACE)*(s.y[i]-s.yi)/(s.yf-s.yi+1);
						float ryf=ryi                           -1.0f*(PLOT_HEIGHT-2*TEXT_SPACE)*(          1)/(s.yf-s.yi+1);
						va.append(sf::Vertex(sf::Vector2f(rxi, ryi)));
						va.append(sf::Vertex(sf::Vector2f(rxf, ryf)));
						va.append(sf::Vertex(sf::Vector2f(rxf, ryi)));
						va.append(sf::Vertex(sf::Vector2f(rxf, ryf)));
						va.append(sf::Vertex(sf::Vector2f(rxi, ryi)));
						va.append(sf::Vertex(sf::Vector2f(rxi, ryf)));
					}
					target.draw(va);
				}
				//next
				++x;
				if(x>3){ x=0; ++y; }
			}
		}

	private:
		struct Subplot{
			std::string type, fileName;
			std::vector<float> x, y;
			float xi, xf, yi, yf;
		};

		std::vector<Subplot> _subplots;
};

int main(int argc, char** argv){
	Plot plot;
	//args
	std::cout<<"received args:\n";
	for(int i=1; i+1<argc; i+=2){
		std::cout<<argv[i]<<" "<<argv[i+1]<<"\n";
		plot.add(argv[i], argv[i+1]);
	}
	//plot
	std::cout<<"plotting\n";
	plot.plot();
	std::cout<<"plot finished, press space to replot\n";
	//geometry
	float xi=0.0f, xf=800.0f, yi=0.0f, yf=600.0f;
	//sfml
	sf::RenderWindow window(sf::VideoMode(int(xf-xi), int(yf-yi)), "plot stuff");
	window.setFramerateLimit(60);
	sf::Font font;
	if(!font.loadFromMemory(sansation, sansationSize)) exit(-1);
	int draws=2;
	//loop
	while(window.isOpen()){
		sf::Event event;
		while(window.pollEvent(event)){
			switch(event.type){
				case sf::Event::KeyPressed:
					if(event.key.code==sf::Keyboard::Space){
						plot.plot();
						draws=2;
					}
					break;
				case sf::Event::MouseMoved:{
					static sf::Event::MouseMoveEvent previous;
					if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
						auto dx=(previous.x-event.mouseMove.x)/Plot::scaleX(xi, xf); xi+=dx; xf+=dx;
						auto dy=(previous.y-event.mouseMove.y)/Plot::scaleY(yi, yf); yi+=dy; yf+=dy;
						draws=2;
					}
					previous=event.mouseMove;
					break;
				}
				case sf::Event::MouseWheelScrolled:{
					auto zoom=[](float& i, float& f, float delta, float center){
						auto m=1+delta/10;
						if(m<0.5f) m=0.5f;
						if(m>2.0f) m=2.0f;
						auto d=m*(f-i);
						auto c=i+center*(f-i);
						i=(1-center)*i+(  center)*(f-d);//if center is 0, then i remains untouched
						f=(  center)*f+(1-center)*(i+d);//if center is 1, then f remains untouched
					};
					if(!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
						zoom(xi, xf, event.mouseWheelScroll.delta, 1.0f*event.mouseWheelScroll.x/window.getSize().x);
					if(!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
						zoom(yi, yf, event.mouseWheelScroll.delta, 1.0f*event.mouseWheelScroll.y/window.getSize().y);
					draws=2;
					break;
				}
				case sf::Event::Resized:
					window.setView(sf::View(sf::FloatRect(0.0f, 0.0f, float(event.size.width), float(event.size.height))));
					draws=2;
					break;
				case sf::Event::Closed:
					window.close();
					break;
				default: break;
			}
		}
		if(draws){
			window.clear();
			plot.draw(window, font, xi, xf, yi, yf);
			--draws;
		}
		window.display();
	}
	//done
	return 0;
}

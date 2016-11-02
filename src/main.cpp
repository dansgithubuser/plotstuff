#include "sansation.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class Plot{
	public:
		static double scaleX(double xi, double xf){ return 800/(xf-xi); }
		static double scaleY(double yi, double yf){ return 600/(yf-yi); }

		static void subplotAttributes(
			unsigned i,
			double xi, double xf, double yi, double yf,
			unsigned& width, unsigned& height,
			unsigned& spaceWidth, unsigned& spaceHeight,
			unsigned& textHeight, unsigned& textSpace,
			double& originX, double& originY
		){
			unsigned x=i%4, y=i/4;
			width =unsigned(190*scaleX(xi, xf));
			height=unsigned(100*scaleY(yi, yf));
			spaceWidth =width +10;
			spaceHeight=height+10;
			textHeight=12,
			textSpace=16;
			originX=x*spaceWidth -xi*scaleX(xi, xf);
			originY=y*spaceHeight-yi*scaleY(yi, yf);
		}

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
				//commands
				std::string c;
				if(file>>c&&c=="begin"){
					while(file>>c){
						if(c=="end") break;
						else if(c=="ylabel"){
							double y;
							if(!(file>>y)) break;
							std::string label;
							std::getline(file, label);
							s.yLabels.push_back(std::make_pair(y, label));
						}
						else if(c=="ycolor"){
							int y, r, g, b, a;
							if(!(file>>y&&file>>r&&file>>g&&file>>b&&file>>a)) break;
							s.yColors[y]=sf::Color(r, g, b, a);
						}
						else if(c=="hover"){
							int x, y;
							if(!(file>>x&&file>>y)) break;
							std::string text;
							while(true){
								std::string t;
								std::getline(file, t);
								if(t=="") break;
								text+=t+"\n";
							}
							s.hovers[x][y]=text;
						}
					}
				}
				else file.seekg(0);
				//read
				if(s.type=="line"){
					double value;
					int i=0;
					while(file>>value){
						s.x.push_back(double(i++));
						s.y.push_back(value);
					}
				}
				else if(s.type=="scatter"){
					double x, y;
					while(file>>x&&file>>y){
						s.x.push_back(x);
						s.y.push_back(y);
					}
				}
				else if(s.type=="heat"){
					double x, y;
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

		void draw(sf::RenderTarget& target, const sf::Font& font, double xi, double xf, double yi, double yf){
			for(unsigned j=0; j<_subplots.size(); ++j){
				unsigned plotWidth, plotHeight, plotSpaceWidth, plotSpaceHeight, textHeight, textSpace;
				double originX, originY;
				subplotAttributes(j, xi, xf, yi, yf,
					plotWidth, plotHeight, plotSpaceWidth, plotSpaceHeight, textHeight, textSpace, originX, originY);
				const auto& s=_subplots[j];
				//name
				std::string fileName=s.fileName.substr(s.fileName.find_last_of("/\\")+1);
				sf::Text name(fileName.c_str(), font, textHeight);
				name.setPosition(originX, originY);
				target.draw(name);
				//plot
				auto cartesian2d=[&](std::string type){
					sf::VertexArray va(std::map<std::string, sf::PrimitiveType>({
						{"line", sf::LinesStrip},
						{"scatter", sf::Points},
					})[type]);
					for(unsigned i=0; i<s.x.size(); ++i) va.append(sf::Vertex(sf::Vector2f(
						originX                     +(plotWidth             )*(s.x[i]-s.xi)/(s.xf-s.xi),
						originY+plotHeight-textSpace-(plotHeight-2*textSpace)*(s.y[i]-s.yi)/(s.yf-s.yi)
					)));
					target.draw(va);
					std::stringstream ss;
					ss<<s.xi<<".."<<s.xf<<", "<<s.yi<<".."<<s.yf;
					sf::Text range(ss.str().c_str(), font, textHeight);
					range.setPosition(originX, originY+plotHeight-textHeight);
					target.draw(range);
				};
				if(s.type=="line") cartesian2d("line");
				else if(s.type=="scatter") cartesian2d("scatter");
				else if(s.type=="heat"){
					//heat
					sf::VertexArray va(sf::Triangles);
					for(unsigned i=0; i<s.x.size(); ++i){
						double rxi=originX                     +(plotWidth             )*(s.x[i]-s.xi)/(s.xf-s.xi+1);
						double rxf=rxi                         +(plotWidth             )*(          1)/(s.xf-s.xi+1);
						double ryi=originY+plotHeight-textSpace-(plotHeight-2*textSpace)*(s.y[i]-s.yi)/(s.yf-s.yi+1);
						double ryf=ryi                         -(plotHeight-2*textSpace)*(          1)/(s.yf-s.yi+1);
						sf::Color color(255, 0, 0);
						if(s.yColors.count(int(s.y[i]))) color=s.yColors.at(int(s.y[i]));
						va.append(sf::Vertex(sf::Vector2f(rxi, ryi), color));
						va.append(sf::Vertex(sf::Vector2f(rxf, ryf), color));
						va.append(sf::Vertex(sf::Vector2f(rxf, ryi), color));
						va.append(sf::Vertex(sf::Vector2f(rxf, ryf), color));
						va.append(sf::Vertex(sf::Vector2f(rxi, ryi), color));
						va.append(sf::Vertex(sf::Vector2f(rxi, ryf), color));
					}
					target.draw(va);
				}
				//labels
				for(auto i: s.yLabels){
					sf::Text label(i.second.c_str(), font, textHeight);
					label.setPosition(originX,
						originY+plotHeight-textSpace-(plotHeight-2*textSpace)*(i.first+1-s.yi)/(s.yf-s.yi+1));
					target.draw(label);
				}
				//hover
				sf::Text text(_hover.c_str(), font, textHeight);
				text.setFillColor(sf::Color(0, 255, 0));
				text.setPosition(0, 0);
				target.draw(text);
			}
		}

		bool hover(int x, int y, double xi, double xf, double yi, double yf){
			std::string newHover="";
			for(unsigned i=0; i<_subplots.size(); ++i){
				unsigned plotWidth, plotHeight, plotSpaceWidth, plotSpaceHeight, textHeight, textSpace;
				double originX, originY;
				subplotAttributes(i, xi, xf, yi, yf,
					plotWidth, plotHeight, plotSpaceWidth, plotSpaceHeight, textHeight, textSpace, originX, originY);
				const auto& s=_subplots[i];
				int px=int( (x-(originX                     ))/(plotWidth             )*(s.xf-s.xi+1)+s.xi);
				int py=int(-(y-(originY+plotHeight-textSpace))/(plotHeight-2*textSpace)*(s.yf-s.yi+1)+s.yi);
				if(s.hovers.count(px)&&s.hovers.at(px).count(py)){
					newHover=s.hovers.at(px).at(py);
					break;
				}
			}
			if(newHover!=_hover){
				_hover=newHover;
				return true;
			}
			return false;
		}

	private:
		struct Subplot{
			std::string type, fileName;
			std::vector<double> x, y;
			double xi, xf, yi, yf;
			std::vector<std::pair<double, std::string>> yLabels;
			std::map<int, sf::Color> yColors;
			std::map<int, std::map<int, std::string>> hovers;
		};

		std::vector<Subplot> _subplots;
		std::string _hover;
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
	double xi=0.0, xf=800.0, yi=0.0, yf=600.0;
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
					if(plot.hover(event.mouseMove.x, event.mouseMove.y, xi, xf, yi, yf)) draws=2;
					break;
				}
				case sf::Event::MouseWheelScrolled:{
					auto zoom=[](double& i, double& f, double delta, double center){
						auto m=1+delta/10;
						if(m<0.5) m=0.5;
						if(m>2.0) m=2.0;
						auto d=m*(f-i);
						auto c=i+center*(f-i);
						i=(1-center)*i+(  center)*(f-d);//if center is 0, then i remains untouched
						f=(  center)*f+(1-center)*(i+d);//if center is 1, then f remains untouched
					};
					if(!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
						zoom(xi, xf, event.mouseWheelScroll.delta, 1.0*event.mouseWheelScroll.x/window.getSize().x);
					if(!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
						zoom(yi, yf, event.mouseWheelScroll.delta, 1.0*event.mouseWheelScroll.y/window.getSize().y);
					draws=2;
					break;
				}
				case sf::Event::Resized:
					window.setView(sf::View(sf::FloatRect(0.0, 0.0, double(event.size.width), double(event.size.height))));
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

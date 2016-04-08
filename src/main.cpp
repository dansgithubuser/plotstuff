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

void punch(sf::RenderWindow& window){
	sf::Texture t;
	t.create(window.getSize().x, window.getSize().y);
	t.update(window);
	window.display();
	sf::Sprite s(t, sf::IntRect(0, 0, window.getSize().x, window.getSize().y));
	window.draw(s);
	window.display();
}

void draw(const std::vector<std::string>& fileNames, sf::RenderWindow& window){
	const unsigned PLOT_WIDTH=190, PLOT_HEIGHT=100, PLOT_SPACE_WIDTH=200, PLOT_SPACE_HEIGHT=110, TEXT_HEIGHT=12, TEXT_SPACE=16;
	unsigned x=0, y=0;
	sf::Font font;
	if(!font.loadFromMemory(sansation, sansationSize)) exit(-1);
	window.clear();
	for(auto fileName: fileNames){
		std::ifstream file(fileName);
		if(!file.is_open()) continue;
		float originX=1.0f*x*PLOT_SPACE_WIDTH;
		float originY=1.0f*y*PLOT_SPACE_HEIGHT;
		//name
		fileName=fileName.substr(fileName.find_last_of("/\\")+1);
		sf::Text name(fileName.c_str(), font, TEXT_HEIGHT);
		name.setPosition(originX, originY);
		window.draw(name);
		//plot
		std::vector<float> v;
		float valueMin=0;
		float valueMax=0;
		{ float value; while(file>>value) v.push_back(value); }
		if(v.size()){
			valueMin=*std::min_element(v.begin(), v.end());
			valueMax=*std::max_element(v.begin(), v.end());
			sf::VertexArray va(sf::LinesStrip);
			for(unsigned j=0; j<v.size(); ++j) va.append(sf::Vertex(sf::Vector2f(
				originX                       +1.0f*PLOT_WIDTH                *j              /v.size(),
				originY+PLOT_HEIGHT-TEXT_SPACE-1.0f*(PLOT_HEIGHT-2*TEXT_SPACE)*(v[j]-valueMin)/(valueMax-valueMin)
			)));
			window.draw(va);
		}
		//range
		std::stringstream ss;
		ss<<"0.."<<v.size()<<", "<<valueMin<<".."<<valueMax;
		sf::Text range(ss.str().c_str(), font, TEXT_HEIGHT);
		range.setPosition(originX, originY+PLOT_HEIGHT-TEXT_HEIGHT);
		window.draw(range);
		//next
		++x;
		if(x*PLOT_SPACE_WIDTH>=window.getSize().x){ x=0; ++y; }
	}
	punch(window);
}

int main(int argc, char** argv){
	//get file names from args
	std::vector<std::string> fileNames;
	std::cout<<"received files as args:\n";
	for(int i=1; i<argc; ++i){
		fileNames.push_back(argv[i]);
		std::cout<<argv[i]<<"\n";
	}
	//initial setup
	sf::RenderWindow window(sf::VideoMode(800, 600), "plot stuff");
	std::cout<<"drawing\n";
	draw(fileNames, window);
	std::cout<<"draw finished, press space to redraw\n";
	//loop
	while(window.isOpen()){
		sf::Event event;
		while(window.pollEvent(event)){
			switch(event.type){
				case sf::Event::KeyPressed:
					if(event.key.code==sf::Keyboard::Space) draw(fileNames, window);
					break;
				case sf::Event::Closed:
					window.close();
					break;
				default: break;
			}
		}
		window.display();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	//done
	return 0;
}

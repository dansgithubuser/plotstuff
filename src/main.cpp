#include "sansation.hpp"

#include <SFML/Graphics.hpp>

#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <string>

void punch(sf::RenderWindow& window){
	sf::Texture t;
	t.create(window.getSize().x, window.getSize().y);
	t.update(window);
	window.display();
	sf::Sprite s(t, sf::IntRect(0, 0, window.getSize().x, window.getSize().y));
	window.draw(s);
	window.display();
}

void draw(int argc, char** argv, sf::RenderWindow& window){
	const unsigned PLOT_WIDTH=200, PLOT_HEIGHT=100;
	unsigned x=0, y=0;
	sf::Font font;
	if(!font.loadFromMemory(sansation, sansationSize)) exit(-1);
	window.clear();
	for(int i=1; i<argc; ++i){
		//name
		sf::Text name(argv[i], font, 12);
		name.setPosition(1.0f*x*PLOT_WIDTH, 1.0f*y*PLOT_HEIGHT);
		window.draw(name);
		//plot
		std::ifstream file(argv[i]);
		std::vector<int> v;
		{ int value; while(file>>value) v.push_back(value); }
		int valueMin=*std::min_element(v.begin(), v.end());
		int valueMax=*std::max_element(v.begin(), v.end());
		sf::VertexArray va(sf::LinesStrip);
		for(unsigned j=0; j<v.size(); ++j) va.append(sf::Vertex(sf::Vector2f(
			1.0f*PLOT_WIDTH *x    +1.0f*PLOT_WIDTH *j/v.size(),
			1.0f*PLOT_HEIGHT*(y+1)-1.0f*PLOT_HEIGHT*(v[j]-valueMin)/(valueMax-valueMin)
		)));
		window.draw(va);
		//next
		++x;
		if(x*PLOT_WIDTH>=window.getSize().x){ x=0; ++y; }
	}
	punch(window);
}

int main(int argc, char** argv){
	sf::RenderWindow window(sf::VideoMode(800, 600), "plot stuff");
	draw(argc, argv, window);
	while(window.isOpen()){
		sf::Event event;
		while(window.pollEvent(event)){
			switch(event.type){
				case sf::Event::KeyPressed:
					if(event.key.code==sf::Keyboard::Space) draw(argc, argv, window);
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
	return 0;
}

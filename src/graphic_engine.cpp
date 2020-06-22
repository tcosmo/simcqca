#include "graphic_engine.h"

GraphicEngine::GraphicEngine(int screen_w, int screen_h)
{
    window.create(sf::VideoMode(screen_w, screen_h), simcqca_PROG_NAME);

    camera = window.getDefaultView();
    window.setView(camera);
}

GraphicEngine::~GraphicEngine()
{
}

void GraphicEngine::run()
{
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Escape:
                    window.close();
                    break;

                default:
                    break;
                }
            }
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(BACKGROUND_COLOR);

        renderOrigin();

        window.display();
    }
}
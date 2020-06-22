#include "../graphic_engine.h"

void GraphicEngine::renderOrigin()
{
    /*** 
     * Outline the border of the (0,0) cell (world coordinates) in black.
    */
    sf::RectangleShape carre(sf::Vector2f(CELL_W, CELL_H));
    carre.setOutlineColor(sf::Color::Black);
    carre.setFillColor(sf::Color::Transparent);
    carre.setOutlineThickness(2);
    window.draw(carre);
}
#include "../graphic_engine.h"

void GraphicEngine::outlineCell(const sf::Vector2i& cellPos, sf::Color outlineColor)
{
    sf::RectangleShape carre(sf::Vector2f(CELL_W, CELL_H));
    carre.setOutlineColor(outlineColor);
    carre.setFillColor(sf::Color::Transparent);
    carre.setOutlineThickness(2);
    carre.setPosition(mapWorldPosToCoords(cellPos));
    window.draw(carre);
}

void GraphicEngine::renderOrigin()
{
    /*** 
     * Outline the border of the (0,0) cell (world coordinates) in black.
    */
    outlineCell({ 0, 0 }, sf::Color::Blue);
}

void GraphicEngine::renderEdge()
{
    /*** 
     * Outline each cells on the edge of the world.
    */
    for (const auto& cellPos : world.cellsOnEdge)
        outlineCell(cellPos, COLOR_DARKER_GREEN);
}
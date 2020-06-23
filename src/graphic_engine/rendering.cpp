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

void GraphicEngine::renderCellText(const sf::Vector2i& cellPos, const Cell& cell)
{
    sf::Vector2f graphicCellCoords = mapWorldPosToCoords(cellPos);
    if( cell.bit != UNDEF ) {
        sf::Text text;
        text.setFont(defaultFont);
        text.setString(std::string({(char)(cell.bit + '0')}));
        text.setCharacterSize(15);

        text.setFillColor(sf::Color::White);
        
        float textX = (CELL_W-text.getLocalBounds().width)/2;
        float textY = (CELL_H-text.getLocalBounds().height)/2;

        text.setPosition(graphicCellCoords + sf::Vector2f({textX-(float)1, textY-1}));//Tweaking text pos
        window.draw(text);
    }

    if( cell.carry == ONE ) {
        sf::Text text;
        text.setFont(defaultFont);
        text.setString("_");
        text.setCharacterSize(15);

        text.setFillColor(sf::Color::White);
        
        float textX = (CELL_W-text.getLocalBounds().width)/2;
        float textY = (CELL_H-text.getLocalBounds().height)/2-20;

        text.setPosition(graphicCellCoords + sf::Vector2f({textX-(float)1, textY-1}));//Tweaking text pos
        window.draw(text);
    }
}

void GraphicEngine::renderCellsText()
{
    /***
     * Renders the textual information inside a cell (bit and carry).
     * Warning: this is costful and should be disabled by pressing `T` when
     *          too many cells are drawn. 
    */
   for(auto& cellPosAndCell: world.cells)
    if(isCellInView(cellPosAndCell.first))
        renderCellText(cellPosAndCell.first, cellPosAndCell.second);
}
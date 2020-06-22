#include "world.h"

void World::setInputCellsLine()
{
    for (int y = 0; y <= 300; y += 1)
        for (int x = -1; x >= -1 * inputStr.length(); x -= 1) {
            char current = inputStr[abs(x) - 1];
            if (current != '0' && current != '1') {
                printf("Input mode LINE expects binary inputs only. Character `%c` is invalid. Abort.\n", current);
                exit(0);
            }
            sf::Vector2i posToAdd = { x, y };
            cells[posToAdd] = Cell(static_cast<AtomicInfo>(current - '0'), UNDEF);
            cellBuffer.push_back(posToAdd);
        }
}

std::vector<sf::Vector2i> World::getAndFlushCellBuffer()
{
    std::vector<sf::Vector2i> toRet = cellBuffer;
    cellBuffer.clear();
    return toRet;
}

void World::setInputCells()
{
    switch (inputType) {
    case NONE:
        printf("Cannot input nothing to the world. Abort.\n");
        exit(0);
        break;

    case LINE:
        setInputCellsLine();
        break;

    default:
        printf("Not implemented. Abort.\n");
        exit(0);
        break;
    }
}
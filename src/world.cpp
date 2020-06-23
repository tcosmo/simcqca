#include "world.h"

void World::setInputCellsLine()
{
    for (int x = -1; x >= -1 * inputStr.length(); x -= 1) {
        char current = inputStr[inputStr.length() - abs(x)];
        if (current != '0' && current != '1') {
            printf("Input mode LINE expects binary inputs only. Character `%c` is invalid. Abort.\n", current);
            exit(0);
        }
        sf::Vector2i posToAdd = { x, 0 };
        cells[posToAdd] = Cell(static_cast<AtomicInfo>(current - '0'), UNDEF);
        cellGraphicBuffer.push_back(posToAdd);
        halfDefinedCells.insert(posToAdd);
    }
}

std::vector<CellPosAndCell> World::findNonLocalUpdates()
{
    /**
     * Finding candidate cells for applying the non-local rule of the 2D CQCA.
    */
    std::vector<CellPosAndCell> toRet;
    for (const sf::Vector2i& cellPos : halfDefinedCells) {
        assert(doesCellExists(cellPos) && cells[cellPos].getStatus() == HALF_DEFINED);
        if (cells[cellPos].bit == ONE) {
            bool lastOneOnLine = true;
            sf::Vector2i currPos = cellPos + EAST;
            while (doesCellExists(currPos)) {
                if (cells[currPos].bit == ONE)
                    lastOneOnLine = false;
                currPos += EAST;
            }
            if (lastOneOnLine) {
                if (!doesCellExists(cellPos + EAST) || cells[cellPos + EAST].getStatus() == HALF_DEFINED) {
                    //pritnf("Non local update: %d %d\n", (cellPos + EAST).x, (cellPos + EAST).y);
                    if (!doesCellExists(cellPos + EAST))
                        toRet.push_back(std::make_pair(cellPos + EAST, Cell(ZERO, UNDEF))); // For soundess of assert line 96
                    toRet.push_back(std::make_pair(cellPos + EAST, Cell(ZERO, ONE)));
                }
            }
        }
    }

    return toRet;
}

void World::nextNonLocal()
{
    auto nonLocalUpdates = findNonLocalUpdates();
    applyUpdates(nonLocalUpdates);

    // Although they are half defined, for performance, we remove
    // trailing zero's from the half defined buffer
    for (const auto& update : nonLocalUpdates) {
        const auto& cellPos = update.first;
        sf::Vector2i currPos = cellPos + EAST;
        while (doesCellExists(currPos)) {
            assert(halfDefinedCells.find(currPos) != halfDefinedCells.end());
            halfDefinedCells.erase(currPos);
            currPos += EAST;
        }
    }
}

std::vector<CellPosAndCell> World::findCarryPropUpdates()
{
    std::vector<CellPosAndCell> toRet;
    for (const sf::Vector2i& cellPos : halfDefinedCells) {
        assert(doesCellExists(cellPos) && cells[cellPos].getStatus() == HALF_DEFINED);
        if (doesCellExists(cellPos + EAST) && cells[cellPos + EAST].getStatus() == DEFINED) {
            AtomicInfo bit = cells[cellPos].bit;
            AtomicInfo newCarry = static_cast<AtomicInfo>((bit + cells[cellPos + EAST].sum()) >= 2);
            Cell updatedCell = Cell(bit, newCarry);
            toRet.push_back(std::make_pair(cellPos, updatedCell));
            //printf("Carry update: %d %d\n", cellPos.x, cellPos.y);
            // Manage edge case of left most cell on a line
            if (!doesCellExists(cellPos + WEST) && !doesCellExists(cellPos + WEST + NORTH) && updatedCell.sum()>=1) {
                //printf("Finite edge case update: %d %d\n", (cellPos + WEST).x, (cellPos + WEST).y);
                toRet.push_back(std::make_pair(cellPos + WEST, Cell(ZERO, UNDEF)));
            }
        }
    }
    return toRet;
}

std::vector<CellPosAndCell> World::findForwardDeductionUpdates()
{
    std::vector<CellPosAndCell> toRet;
    for (const sf::Vector2i& cellPos : halfDefinedCells) {
        assert(doesCellExists(cellPos) && cells[cellPos].getStatus() == HALF_DEFINED);
        if (doesCellExists(cellPos + EAST) && cells[cellPos + EAST].getStatus() == DEFINED) {
            AtomicInfo southBit = static_cast<AtomicInfo>((static_cast<int>(cells[cellPos].bit) + cells[cellPos + EAST].sum()) % 2);
            toRet.push_back(std::make_pair(cellPos + SOUTH, Cell(southBit, UNDEF)));
            //printf("Forward update: %d %d\n", (cellPos + SOUTH).x, (cellPos + SOUTH).y);
        }
    }
    return toRet;
}

void World::applyUpdates(const std::vector<CellPosAndCell>& updates)
{
    for (const auto& info : updates) {
        const sf::Vector2i& cellPos = info.first;
        const Cell& cell = info.second;
        cells[cellPos] = cell;
        cellGraphicBuffer.push_back(cellPos);
        if (cell.getStatus() == DEFINED) {
            assert(halfDefinedCells.find(cellPos) != halfDefinedCells.end());
            halfDefinedCells.erase(cellPos);
        }
        if (cell.getStatus() == HALF_DEFINED) {
            //printf("%d %d\n", cellPos.x, cellPos.y);
            assert(halfDefinedCells.find(cellPos) == halfDefinedCells.end());
            halfDefinedCells.insert(cellPos);
        }
    }
}

void World::nextLocal()
{
    auto carryPropUpdates = findCarryPropUpdates();
    auto forwardDeductionUpdates = findForwardDeductionUpdates();
    //printf("\n");
    applyUpdates(carryPropUpdates);
    applyUpdates(forwardDeductionUpdates);
}

void World::next()
{
    nextNonLocal();
    nextLocal();
}

bool World::doesCellExists(const sf::Vector2i& cellPos)
{
    return cells.find(cellPos) != cells.end();
}

std::vector<sf::Vector2i> World::getAndFlushGraphicBuffer()
{
    std::vector<sf::Vector2i> toRet = cellGraphicBuffer;
    cellGraphicBuffer.clear();
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
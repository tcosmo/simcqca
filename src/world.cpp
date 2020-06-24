#include "world.h"

void World::setInputCellsLine()
{
    /**
     * Set up the initial configuration in Line mode. 
    */
    std::vector<CellPosAndCell> updates;
    for (int x = -1; x >= -1 * inputStr.length(); x -= 1) {
        char current = inputStr[inputStr.length() - abs(x)];
        if (current != '0' && current != '1') {
            printf("Input mode LINE expects binary inputs only. Character `%c` is invalid. Abort.\n", current);
            exit(0);
        }
        sf::Vector2i posToAdd = { x, 0 };
        Cell cellToAdd = Cell(static_cast<AtomicInfo>(current - '0'), UNDEF);
        updates.push_back(std::make_pair(posToAdd, cellToAdd));
    }
    applyUpdates(updates);
}

void World::setInputCellsCol()
{
    /**
     * Set up the initial configuration in Col mode. 
    */
    std::vector<CellPosAndCell> updates;
    std::vector<int> base3p = base3To3p(inputStr);
    for (int y = 0; y < base3p.size(); y += 1) {
        int current = base3p[y];
        sf::Vector2i posToAdd = { 0, y };
        halfDefinedCells.insert(posToAdd); // For compliance with first assert in applyUpdates
        Cell cellToAdd = Cell(static_cast<AtomicInfo>(current / 2), static_cast<AtomicInfo>(current % 2));
        updates.push_back(std::make_pair(posToAdd, cellToAdd));
    }

    // Bootstrapping col mode, this is the first half defined cell
    sf::Vector2i posToAdd = { -1, 0 };
    Cell cellToAdd = { ZERO, UNDEF };
    updates.push_back(std::make_pair(posToAdd, cellToAdd));

    applyUpdates(updates);
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
                    if (!doesCellExists(cellPos + EAST))
                        toRet.push_back(std::make_pair(cellPos + EAST, Cell(ZERO, UNDEF))); // For soundess of assert in applyUpdates
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

void World::manageEdgeCases(std::vector<CellPosAndCell>& toRet, const sf::Vector2i& cellPos, const Cell& updatedCell)
{
    if (inputType == LINE) {
        // Edge case at end of a line which is in theory (0)^\infty
        if (!doesCellExists(cellPos + WEST) && !doesCellExists(cellPos + WEST + NORTH) && updatedCell.sum() >= 1) {
            toRet.push_back(std::make_pair(cellPos + WEST, Cell(ZERO, UNDEF)));
        }
    }

    if (inputType == COL) {
        // Edge case at beginning of a column which is in theory (0)^\infty
        if (!doesCellExists(cellPos + WEST) && updatedCell.sum() != 0) {
            bool onlyZero = true;
            sf::Vector2i currentPos = cellPos + NORTH;
            while (doesCellExists(currentPos)) {
                assert(cells[currentPos].getStatus() == DEFINED);
                if (cells[currentPos].sum() != 0) {
                    onlyZero = false;
                    break;
                }
                currentPos += NORTH;
            }
            if (onlyZero) {
                toRet.push_back(std::make_pair(cellPos + WEST, Cell(ZERO, UNDEF)));
            }
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

            // Because we do a finite simulation of an infinite process
            // we have some edge cases to deal with.
            manageEdgeCases(toRet, cellPos, updatedCell);
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
            assert(halfDefinedCells.find(cellPos) == halfDefinedCells.end());
            halfDefinedCells.insert(cellPos);
        }
    }
}

void World::nextLocal()
{
    auto carryPropUpdates = findCarryPropUpdates();
    auto forwardDeductionUpdates = findForwardDeductionUpdates();
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

std::vector<int> World::base3To3p(std::string base3)
{
    /***
     * Base 3 to base 3' conversion. See paper for more details.
    */
    std::reverse(base3.begin(), base3.end());
    std::vector<int> toReturn;

    bool lastSeenZero = true;
    int i = 0;
    for (char c : base3) {
        switch (c) {
        case '0':
            toReturn.push_back(0);
            lastSeenZero = true;
            break;
        case '1':
            if (lastSeenZero)
                toReturn.push_back(1);
            else
                toReturn.push_back(2);
            break;
        case '2':
            toReturn.push_back(3);
            lastSeenZero = false;
            break;
        default:
            printf("Character `%c` is invalid base 3 digit. Abort.\n", c);
            exit(1);
            break;
        }
        i += 1;
    }

    std::reverse(toReturn.begin(), toReturn.end());
    return toReturn;
}

void World::setInputCells()
{
    switch (inputType) {
    case NONE:
        printf("Cannot input nothing to the world, you probably used a wrong command line argument. Abort. Please run `./simcqca --help` for help.\n");
        exit(0);
        break;

    case LINE:
        setInputCellsLine();
        break;

    case COL:
        setInputCellsCol();
        break;

    default:
        printf("Not implemented yet. Abort.\n");
        exit(0);
        break;
    }
}
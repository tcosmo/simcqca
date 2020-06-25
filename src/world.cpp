#include "world.h"

void World::setInputCellsLine()
{
    /**
     * Set up the initial configuration in Line mode. 
    */
    assert(inputType == LINE);
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

    // Tweak to bootstrap the process when input has trailing 0s
    int x = 0;
    while (x < inputStr.length() && inputStr[inputStr.length() - x - 1] == '0')
        x += 1;
    if (x == inputStr.length())
        return;
    cellsOnEdge.insert({ -x - 1, 0 });
}

void World::setInputCellsCol()
{
    /**
     * Set up the initial configuration in Col mode. 
    */
    assert(inputType == COL);
    std::vector<CellPosAndCell> updates;
    std::vector<int> base3p = base3To3p(inputStr);
    for (int y = 1; y <= base3p.size(); y += 1) {
        int current = base3p[y - 1];
        sf::Vector2i posToAdd = { 0, y };
        Cell cellToAdd = Cell(static_cast<AtomicInfo>(current / 2), static_cast<AtomicInfo>(current % 2));
        updates.push_back(std::make_pair(posToAdd, cellToAdd));
    }

    // Bootstrapping col mode, this is the first half defined cell
    sf::Vector2i posToAdd = { -1, 1 };
    Cell cellToAdd = { ZERO, UNDEF };
    updates.push_back(std::make_pair(posToAdd, cellToAdd));

    applyUpdates(updates);
}

void World::setInputCellsBorder()
{
    /**
     * Set up the initial configuration in Border mode. 
    */
    assert(inputType == BORDER || inputType == CYCLE);

    std::vector<CellPosAndCell> updates;
    sf::Vector2i currPos = ORIGIN_BORDER_MODE;
    for (char parityBit : inputStr) {
        if (parityBit != '0' && parityBit != '1') {
            printf("A parity vector contains only `0`s and `1`s, symbol %c is invalid. Abort.\n", parityBit);
            exit(0);
        }
        if (parityBit == '0') {
            Cell cellToAdd = { ZERO, UNDEF };
            updates.push_back(std::make_pair(currPos, cellToAdd));
            currPos += WEST;
        } else {
            Cell cellToAdd = { ZERO, ONE };
            updates.push_back(std::make_pair(currPos, cellToAdd));
            currPos += SOUTH + WEST;
        }
    }
    applyUpdates(updates);
}

void World::computeParityVectorSpan()
{
    parityVectorSpan = 0;
    for (const auto& c : inputStr)
        parityVectorSpan += (c == '1');
}

void World::setInputCellsCycle()
{
    /**
     * Set up the initial configuration in Cycle mode. 
    */
    assert(inputType == CYCLE);
    setInputCellsBorder();

    computeParityVectorSpan();
    cyclicForwardVector = ORIGIN_BORDER_MODE;
    for (int iWest = 0; iWest < inputStr.length(); iWest += 1)
        cyclicForwardVector += WEST;
    for (int iSouth = 0; iSouth < parityVectorSpan; iSouth += 1)
        cyclicForwardVector += SOUTH;

    applyUpdates({ std::make_pair(cyclicForwardVector, cells[ORIGIN_BORDER_MODE]) });
}

std::vector<CellPosAndCell> World::findNonLocalUpdates()
{
    /**
     * Finding candidate cells for applying the non-local rule of the 2D CQCA.
    */
    std::vector<CellPosAndCell> toRet;
    for (const sf::Vector2i& cellPos : cellsOnEdge) {
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
    for (const sf::Vector2i& cellPos : cellsOnEdge) {
        assert(doesCellExists(cellPos));
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
    for (const sf::Vector2i& cellPos : cellsOnEdge) {
        assert(doesCellExists(cellPos) && cells[cellPos].getStatus() == HALF_DEFINED);
        if (doesCellExists(cellPos + EAST) && cells[cellPos + EAST].getStatus() == DEFINED) {
            AtomicInfo southBit = static_cast<AtomicInfo>((static_cast<int>(cells[cellPos].bit) + cells[cellPos + EAST].sum()) % 2);
            toRet.push_back(std::make_pair(cellPos + SOUTH, Cell(southBit, UNDEF)));
        }
    }
    return toRet;
}

std::vector<CellPosAndCell> World::findBackwardDeductionUpdates()
{
    std::vector<CellPosAndCell> toRet;
    for (const sf::Vector2i& cellPos : cellsOnEdge) {
        assert(doesCellExists(cellPos));
        if (!doesCellExists(cellPos + NORTH) && doesCellExists(cellPos + NORTH + EAST) && cells[cellPos + NORTH + EAST].getStatus() == DEFINED) {
            AtomicInfo northBit = static_cast<AtomicInfo>((cells[cellPos + NORTH + EAST].sum()) % 2 != static_cast<int>(cells[cellPos].bit));
            toRet.push_back(std::make_pair(cellPos + NORTH, Cell(northBit, UNDEF)));
        }
    }
    return toRet;
}

void World::cleanCellsOnEdge()
{
    /**
     * Remove the cells which are not anymore on edge from the edge.
    */
    std::vector<sf::Vector2i> toRemove;
    for (const auto& cellPos : cellsOnEdge)
        if (!isCellOnEdge(cellPos))
            toRemove.push_back(cellPos);
    for (const auto& cellPos : toRemove)
        cellsOnEdge.erase(cellPos);
}

bool World::isComputationDone()
{
    /**
     * In border mode, the computation space is finite.
    */
    assert(inputType == BORDER);
    sf::Vector2i topLeftCellPos = { -1 * static_cast<int>(inputStr.length()) + 1, 0 };
    return doesCellExists(topLeftCellPos) && cells[topLeftCellPos].getStatus() == DEFINED;
}

bool World::isCellOnEdge(const sf::Vector2i& cellPos)
{
    /**
     *  Determines whether a cell is on the edge of the computing region or not.
    */

    if (!doesCellExists(cellPos))
        return false;

    if (inputType == LINE || inputType == COL) {

        if (cells[cellPos].getStatus() == DEFINED)
            return false;

        if (cellPos.y == 0 && doesCellExists(cellPos + EAST) && cells[cellPos + EAST].getStatus() == HALF_DEFINED)
            return false;

        // Remove trailing 0s from edge
        bool isTrailingZero = true;
        if (cells[cellPos].bit == ZERO && doesCellExists(cellPos + WEST) && cells[cellPos + WEST].getStatus() == HALF_DEFINED) {
            sf::Vector2i pos = cellPos + EAST;
            while (doesCellExists(pos)) {
                if (cells[pos].bit == ONE)
                    isTrailingZero = false;
                pos += EAST;
            }
        } else {
            isTrailingZero = false;
        }

        return cells[cellPos].getStatus() == HALF_DEFINED && !isTrailingZero;
    }

    if (inputType == BORDER || inputType == CYCLE) {
        if (inputType == BORDER && cellPos.y == ORIGIN_BORDER_MODE.y)
            return cells[cellPos].getStatus() == HALF_DEFINED;
        return !doesCellExists(cellPos + NORTH);
    }

    return false;
}

void World::applyUpdates(const std::vector<CellPosAndCell>& updates)
{
    for (const auto& info : updates) {
        const sf::Vector2i& cellPos = info.first;
        const Cell& cell = info.second;
        cells[cellPos] = cell;
        cellGraphicBuffer.push_back(cellPos);
        if (isCellOnEdge(cellPos))
            cellsOnEdge.insert(cellPos);
        if (inputType == LINE || inputType == COL)
            if (isCellOnEdge(cellPos + WEST))
                cellsOnEdge.insert(cellPos + WEST);
    }
    cleanCellsOnEdge();
}

std::vector<CellPosAndCell> World::findCyclicUpdates(const std::vector<CellPosAndCell>& updates)
{
    /**
     * Find cells on which we can apply the cyclic equivalence relation. 
    */

    std::vector<CellPosAndCell> toRet;

    for (const auto& update : updates) {
        const sf::Vector2i& pos = update.first;
        const Cell& cell = update.second;

        if ((pos - cyclicForwardVector).x == ORIGIN_BORDER_MODE.x) {
            sf::Vector2i eqPos = { 0, pos.y - parityVectorSpan };
            toRet.push_back(std::make_pair(eqPos, cell));
        }
    }

    return toRet;
}

void World::nextLocal()
{
    /**
     * Applies one pass of the local rule. 
    */

    // In LINE and COL mode, the local rule is
    // carry propagation followed by forward deduction
    if (inputType == LINE || inputType == COL) {
        // First find the update then apply them
        // Do not apply some updates before they were all found
        // That would break the CA logic
        auto carryPropUpdates = findCarryPropUpdates();
        auto forwardDeductionUpdates = findForwardDeductionUpdates();
        applyUpdates(carryPropUpdates);
        applyUpdates(forwardDeductionUpdates);
    }

    // In BORDER/CYCLE mode, the local rule is
    // carry propagation followed by backward deduction
    // note that in this mode the non local rule will never be applied
    if (inputType == BORDER || inputType == CYCLE) {
        auto carryPropUpdates = findCarryPropUpdates();
        auto backwardDeductionUpdates = findBackwardDeductionUpdates();

        applyUpdates(carryPropUpdates);
        applyUpdates(backwardDeductionUpdates);

        // In cycle mode we need to enforce the equivalence relation on
        // cells of the world in order to compute
        if (inputType == CYCLE) {
            auto cyclicUpdates = findCyclicUpdates(carryPropUpdates);
            applyUpdates(cyclicUpdates);
            cyclicUpdates = findCyclicUpdates(backwardDeductionUpdates);
            applyUpdates(cyclicUpdates);
        }
    }
}

void World::next()
{
    // In BORDER or CYCLE mode non local steps are useless
    // As bootstrapping events are given by the parity vector
    if (inputType == LINE || inputType == COL) {
        nextNonLocal();
    }
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

    case BORDER:
        setInputCellsBorder();
        break;

    case CYCLE:
        setInputCellsCycle();
        break;

    default:
        printf("Not implemented yet. Abort.\n");
        exit(0);
        break;
    }
}

void World::reset()
{
    cells.clear();
    cellsOnEdge.clear();
    cellGraphicBuffer.clear();
    setInputCells();
}
#include "world.h"

std::vector<CellPosAndCell> World::findCarryPropUpdates() {
  std::vector<CellPosAndCell> toRet;
  for (const sf::Vector2i& cellPos : cellsOnEdge) {
    assert(doesCellExists(cellPos));
    if (cells[cellPos].getStatus() != HALF_DEFINED) continue;
    if (doesCellExists(cellPos + EAST) &&
        cells[cellPos + EAST].getStatus() == DEFINED) {
      AtomicInfo bit = cells[cellPos].bit;
      AtomicInfo newCarry =
          static_cast<AtomicInfo>((bit + cells[cellPos + EAST].sum()) >= 2);
      Cell updatedCell = Cell(bit, newCarry);
      toRet.push_back(std::make_pair(cellPos, updatedCell));

      // Because we do a finite simulation of an infinite process
      // we have some edge cases to deal with.
      manageEdgeCases(toRet, cellPos, updatedCell);
    }
  }
  return toRet;
}

std::vector<CellPosAndCell> World::findForwardDeductionUpdates() {
  std::vector<CellPosAndCell> toRet;
  for (const sf::Vector2i& cellPos : cellsOnEdge) {
    assert(doesCellExists(cellPos) &&
           cells[cellPos].getStatus() == HALF_DEFINED);
    if (doesCellExists(cellPos + EAST) &&
        cells[cellPos + EAST].getStatus() == DEFINED) {
      AtomicInfo southBit = static_cast<AtomicInfo>(
          (static_cast<int>(cells[cellPos].bit) + cells[cellPos + EAST].sum()) %
          2);
      toRet.push_back(std::make_pair(cellPos + SOUTH, Cell(southBit, UNDEF)));
    }
  }
  return toRet;
}

std::vector<CellPosAndCell> World::findBackwardDeductionUpdates() {
  std::vector<CellPosAndCell> toRet;
  for (const sf::Vector2i& cellPos : cellsOnEdge) {
    assert(doesCellExists(cellPos));
    if (!doesCellExists(cellPos + NORTH) &&
        doesCellExists(cellPos + NORTH + EAST) &&
        cells[cellPos + NORTH + EAST].getStatus() == DEFINED) {
      AtomicInfo northBit =
          static_cast<AtomicInfo>((cells[cellPos + NORTH + EAST].sum()) % 2 !=
                                  static_cast<int>(cells[cellPos].bit));
      toRet.push_back(std::make_pair(cellPos + NORTH, Cell(northBit, UNDEF)));
    }
  }
  return toRet;
}

void World::cleanCellsOnEdge() {
  /**
   * Remove the cells which are not anymore on edge from the edge.
   */
  std::vector<sf::Vector2i> toRemove;
  for (const auto& cellPos : cellsOnEdge)
    if (!isCellOnEdge(cellPos)) toRemove.push_back(cellPos);
  for (const auto& cellPos : toRemove) cellsOnEdge.erase(cellPos);
}

bool World::isCellOnEdge(const sf::Vector2i& cellPos) {
  /**
   *  Determines whether a cell is on the edge of the computing region or not.
   */

  if (!doesCellExists(cellPos)) return false;

  if (inputType == LINE || inputType == COL) {
    if (cells[cellPos].getStatus() == DEFINED) return false;

    if (cellPos.y == 0 && doesCellExists(cellPos + EAST) &&
        cells[cellPos + EAST].getStatus() == HALF_DEFINED)
      return false;

    // Remove trailing 0s from edge
    bool isTrailingZero = true;
    if (cells[cellPos].bit == ZERO && doesCellExists(cellPos + WEST) &&
        cells[cellPos + WEST].getStatus() == HALF_DEFINED) {
      sf::Vector2i pos = cellPos + EAST;
      while (doesCellExists(pos)) {
        if (cells[pos].bit == ONE) isTrailingZero = false;
        pos += EAST;
      }
    } else {
      isTrailingZero = false;
    }

    return cells[cellPos].getStatus() == HALF_DEFINED && !isTrailingZero;
  }

  if (inputType == BORDER || inputType == CYCLE) {
    if ((inputType == BORDER || (inputType == CYCLE && constructCycleInLine)) &&
        cellPos.y == ORIGIN_BORDER_MODE.y)
      return cells[cellPos].getStatus() == HALF_DEFINED;
    return !doesCellExists(cellPos + NORTH);
  }

  return false;
}

void World::applyUpdates(const std::vector<CellPosAndCell>& updates) {
  for (const auto& info : updates) {
    const sf::Vector2i& cellPos = info.first;
    const Cell& cell = info.second;
    cells[cellPos] = cell;
    cellGraphicBuffer.push_back(cellPos);
    if (isCellOnEdge(cellPos)) cellsOnEdge.insert(cellPos);
    if (inputType == LINE || inputType == COL)
      if (isCellOnEdge(cellPos + WEST)) cellsOnEdge.insert(cellPos + WEST);
  }
  cleanCellsOnEdge();
}

void World::nextLocal() {
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

void World::next() {
  // In BORDER or CYCLE mode non local steps are useless
  // As bootstrapping events are given by the parity vector
  if (inputType == LINE || inputType == COL) {
    nextNonLocal();
  }
  nextLocal();
}

bool World::doesCellExists(const sf::Vector2i& cellPos) {
  return cells.find(cellPos) != cells.end();
}

std::vector<sf::Vector2i> World::getAndFlushGraphicBuffer() {
  std::vector<sf::Vector2i> toRet = cellGraphicBuffer;
  cellGraphicBuffer.clear();
  return toRet;
}

std::vector<int> World::base3To3p(std::string base3) {
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

void World::setInputCells() {
  switch (inputType) {
    case NONE:
      printf(
          "Cannot input nothing to the world, you probably used a wrong "
          "command line argument. Abort. Please run `./simcqca --help` for "
          "help.\n");
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

void World::reset() {
  cells.clear();
  cellsOnEdge.clear();
  cellGraphicBuffer.clear();
  setInputCells();
}

std::string rotateStr(std::string toRotate, int offset) {
  std::string toRet = toRotate;
  for (int i = 0; i < toRotate.size(); i += 1)
    toRet[i] = toRotate[(toRotate.size() + offset + i) % (toRotate.size())];
  return toRet;
}

void World::rotate(int direction) {
  /**
   * In cycle mode rotates the input parity vector.
   */
  assert(inputType == CYCLE);
  inputStr = rotateStr(inputStr, direction);
  reset();
}
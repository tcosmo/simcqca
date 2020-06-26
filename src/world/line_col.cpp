/**
 * All the routines that deals uniquely with the simulation of the world in
 * LINE or COL mode.
 */

#include "../world.h"

void World::setInputCellsLine() {
  /**
   * Set up the initial configuration in Line mode.
   */
  assert(inputType == LINE);
  std::vector<CellPosAndCell> updates;
  for (int x = -1; x >= -1 * inputStr.length(); x -= 1) {
    char current = inputStr[inputStr.length() - abs(x)];
    if (current != '0' && current != '1') {
      printf(
          "Input mode LINE expects binary inputs only. Character `%c` is "
          "invalid. Abort.\n",
          current);
      exit(0);
    }
    sf::Vector2i posToAdd = {x, 0};
    Cell cellToAdd = Cell(static_cast<AtomicInfo>(current - '0'), UNDEF);
    updates.push_back(std::make_pair(posToAdd, cellToAdd));
  }
  applyUpdates(updates);

  // Tweak to bootstrap the process when input has trailing 0s
  int x = 0;
  while (x < inputStr.length() && inputStr[inputStr.length() - x - 1] == '0')
    x += 1;
  if (x == inputStr.length()) return;
  cellsOnEdge.insert({-x - 1, 0});
}

void World::setInputCellsCol() {
  /**
   * Set up the initial configuration in Col mode.
   */
  assert(inputType == COL);
  std::vector<CellPosAndCell> updates;
  std::vector<int> base3p = base3To3p(inputStr);
  for (int y = 1; y <= base3p.size(); y += 1) {
    int current = base3p[y - 1];
    sf::Vector2i posToAdd = {0, y};
    Cell cellToAdd = Cell(static_cast<AtomicInfo>(current / 2),
                          static_cast<AtomicInfo>(current % 2));
    updates.push_back(std::make_pair(posToAdd, cellToAdd));
  }

  // Bootstrapping col mode, this is the first half defined cell
  sf::Vector2i posToAdd = {-1, 1};
  Cell cellToAdd = {ZERO, UNDEF};
  updates.push_back(std::make_pair(posToAdd, cellToAdd));

  applyUpdates(updates);
}

std::vector<CellPosAndCell> World::findNonLocalUpdates() {
  /**
   * Finding candidate cells for applying the non-local rule of the 2D CQCA.
   */
  std::vector<CellPosAndCell> toRet;
  for (const sf::Vector2i& cellPos : cellsOnEdge) {
    assert(doesCellExists(cellPos) &&
           cells[cellPos].getStatus() == HALF_DEFINED);
    if (cells[cellPos].bit == ONE) {
      bool lastOneOnLine = true;
      sf::Vector2i currPos = cellPos + EAST;
      while (doesCellExists(currPos)) {
        if (cells[currPos].bit == ONE) lastOneOnLine = false;
        currPos += EAST;
      }
      if (lastOneOnLine) {
        if (!doesCellExists(cellPos + EAST) ||
            cells[cellPos + EAST].getStatus() == HALF_DEFINED) {
          toRet.push_back(
              std::make_pair(cellPos + EAST, Cell(ZERO, ONE, true)));
          sf::Vector2i newPos = cellPos + EAST + EAST;
          while (doesCellExists(newPos)) {
            toRet.push_back(std::make_pair(newPos, Cell(ZERO, ZERO)));
            newPos += EAST;
          }
        }
      }
    }
  }

  return toRet;
}

void World::nextNonLocal() {
  auto nonLocalUpdates = findNonLocalUpdates();
  applyUpdates(nonLocalUpdates);
}

void World::manageEdgeCases(std::vector<CellPosAndCell>& toRet,
                            const sf::Vector2i& cellPos,
                            const Cell& updatedCell) {
  if (inputType == LINE) {
    // Edge case at end of a line which is in theory (0)^\infty
    if (!doesCellExists(cellPos + WEST) &&
        !doesCellExists(cellPos + WEST + NORTH) && updatedCell.sum() >= 1) {
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
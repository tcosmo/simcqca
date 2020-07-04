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
      printf("Input mode LINE expects binary inputs only. Character `%c` is "
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
  if (x == inputStr.length())
    return;
  cellsOnEdge.insert({-x - 1, 0});
}

void World::setInputCellsCol() {
  /**
   * Set up the initial configuration in Col mode.
   */
  assert(inputType == COL);
  std::vector<CellPosAndCell> updates;
  std::vector<int> base3p = base3To3p(inputStr);
  for (int y = -1; y >= -1 * base3p.size(); y -= 1) {
    int current = base3p[base3p.size() + y];
    sf::Vector2i posToAdd = {0, y};
    Cell cellToAdd = Cell(static_cast<AtomicInfo>(current / 2),
                          static_cast<AtomicInfo>(current % 2));
    updates.push_back(std::make_pair(posToAdd, cellToAdd));
  }

  // Bootstrapping col mode, this is the first half defined cell
  sf::Vector2i posToAdd = {-1, -1 * static_cast<int>(base3p.size())};
  Cell cellToAdd = {ZERO, UNDEF};
  updates.push_back(std::make_pair(posToAdd, cellToAdd));

  // // Need this for coherence in definitions
  // posToAdd = {0,0};
  // cellToAdd = {ZERO,UNDEF};
  // updates.push_back(std::make_pair(posToAdd, cellToAdd));

  applyUpdates(updates);
}

void World::manageEdgeCases(std::vector<CellPosAndCell> &toRet,
                            const sf::Vector2i &cellPos,
                            const Cell &updatedCell) {
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
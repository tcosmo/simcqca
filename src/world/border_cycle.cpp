/**
 * All the routines that deals uniquely with the simulation of the world in
 * BORDER or CYCLE mode.
 */

#include "../world.h"

void World::setInputCellsBorder() {
  /**
   * Set up the initial configuration in Border mode.
   */
  assert(inputType == BORDER || inputType == CYCLE);
  alreadyTweaked = false;

  std::vector<CellPosAndCell> updates;
  sf::Vector2i currPos = ORIGIN_BORDER_MODE;
  for (char parityBit : inputStr) {
    if (parityBit != '0' && parityBit != '1') {
      printf(
          "A parity vector contains only `0`s and `1`s, symbol %c is invalid. "
          "Abort.\n",
          parityBit);
      exit(0);
    }
    if (parityBit == '0') {
      Cell cellToAdd = {ZERO, UNDEF};
      updates.push_back(std::make_pair(currPos, cellToAdd));
      parityVectorCells.push_back(currPos);
      currPos += WEST;
    } else {
      Cell cellToAdd = {ONE, UNDEF};
      updates.push_back(std::make_pair(currPos, cellToAdd));
      parityVectorCells.push_back(currPos);
      currPos += SOUTH + WEST;
    }
  }
  applyUpdates(updates);
}

bool World::isComputationDone() {
  /**
   * In border mode, the computation space is finite.
   */
  assert(inputType == BORDER);
  sf::Vector2i topLeftCellPos = {-1 * static_cast<int>(inputStr.length()) + 1,
                                 0};
  return doesCellExists(topLeftCellPos) &&
         cells[topLeftCellPos].getStatus() == DEFINED;
}

void World::computeParityVectorSpan() {
  /**
   * Returns the span (i.e. number of '1's) in the input parity vector.
   */
  parityVectorSpan = 0;
  for (const auto &c : inputStr)
    parityVectorSpan += (c == '1');
}

void World::setInputCellsCycle() {
  /**
   * Set up the initial configuration in Cycle mode.
   */
  assert(inputType == CYCLE);
  setInputCellsBorder();

  cycleDetectionMap.clear();
  indexesDetectedCycle = std::make_pair(-1, -1);
  computeParityVectorSpan();
  cyclicForwardVector = ORIGIN_BORDER_MODE;
  for (int iWest = 0; iWest < inputStr.length(); iWest += 1)
    cyclicForwardVector += WEST;
  for (int iSouth = 0; iSouth < parityVectorSpan; iSouth += 1)
    cyclicForwardVector += SOUTH;

  applyUpdates(
      {std::make_pair(cyclicForwardVector, cells[ORIGIN_BORDER_MODE])});
}

std::vector<CellPosAndCell>
World::findCyclicUpdates(const std::vector<CellPosAndCell> &updates) {
  /**
   * Find cells on which we can apply the cyclic equivalence relation.
   */

  std::vector<CellPosAndCell> toRet;

  for (const auto &update : updates) {
    const sf::Vector2i &pos = update.first;
    const Cell &cell = update.second;

    if (!constructCycleInLine || cycleBoth) {
      if ((pos - cyclicForwardVector).x == ORIGIN_BORDER_MODE.x) {
        sf::Vector2i posEquivalent = pos - cyclicForwardVector;
        toRet.push_back(std::make_pair(posEquivalent, cell));
      }
    }

    if (constructCycleInLine || cycleBoth) {
      if ((pos + cyclicForwardVector).y ==
          ORIGIN_BORDER_MODE.y + parityVectorSpan) {
        sf::Vector2i posEquivalent = pos + cyclicForwardVector;
        toRet.push_back(std::make_pair(posEquivalent, cell));
      }
    }
  }
  return toRet;
}

std::vector<sf::Vector2i> World::cellPosOnCyclicCut(int layerToCompute) {
  /**
   * Returns the cells sitting the translation of the parity vector
   * starting at (0,yStart).
   */
  assert(inputType == CYCLE);
  std::vector<sf::Vector2i> toRet;
  sf::Vector2i currentPos;
  if (!constructCycleInLine)
    currentPos = {ORIGIN_BORDER_MODE.x, ORIGIN_BORDER_MODE.y - layerToCompute};
  else
    currentPos = {ORIGIN_BORDER_MODE.x - layerToCompute, ORIGIN_BORDER_MODE.y};
  for (auto c : inputStr) {
    toRet.push_back(currentPos);
    if (c == '1')
      currentPos += SOUTH;
    currentPos += WEST;
  }
  return toRet;
}

std::string
World::stringOfCyclicCut(const std::vector<sf::Vector2i> &cellPosOnCut) {
  /**
   * Transforms a cyclic cut into the string of its trit or "" if not all
   * defined.
   */
  assert(inputType == CYCLE);
  std::string toRet = "";
  for (const auto &pos : cellPosOnCut) {
    if (!doesCellExists(pos) || cells[pos].getStatus() != DEFINED)
      return "";
    toRet += '0' + cells[pos].index();
  }
  return toRet;
}

bool World::isCycleDetected() {
  assert(inputType == CYCLE);
  if (indexesDetectedCycle.first != -1)
    return true;

  int layerToCompute = cycleDetectionMap.size();
  std::string stringOfLayer =
      stringOfCyclicCut(cellPosOnCyclicCut(layerToCompute));

  // Current layer not finishing computing yet
  if (stringOfLayer.size() == 0)
    return false;

  if (cycleDetectionMap.find(stringOfLayer) != cycleDetectionMap.end()) {
    indexesDetectedCycle =
        std::make_pair(cycleDetectionMap[stringOfLayer], layerToCompute);
    return true;
  }

  cycleDetectionMap[stringOfLayer] = layerToCompute;
  return false;
}

void World::printCycleInformation() {
  /**
   * Prints the 3-adic/2-adic representation of the cycle the supported by the
   * input parity vector.
   */
  assert(inputType == CYCLE && indexesDetectedCycle.first != -1);
  printf(
      "The cycle supported by the parity vector %s has the following rational ",
      inputStr.c_str());
  if (!constructCycleInLine)
    printf("3-adic ");
  else
    printf("2-adic ");
  printf("expansion:\n");

  std::string initSeg = "";
  std::string period = "";
  int parityVectorNorm = static_cast<int>(inputStr.size());
  int which = static_cast<int>(cells[ORIGIN_BORDER_MODE].bit);
  if (!constructCycleInLine) {
    for (int y = ORIGIN_BORDER_MODE.y + parityVectorSpan -1;
         y >= -1 * indexesDetectedCycle.first + parityVectorSpan -1; y -= 1) {
      sf::Vector2i cellPos = {ORIGIN_BORDER_MODE.x-parityVectorNorm+1, y};
      assert(doesCellExists(cellPos) && cells[cellPos].getStatus() == DEFINED);
      initSeg += cells[cellPos].sum() + '0';
    }
    for (int y = -1 * indexesDetectedCycle.first - 1 + parityVectorSpan -1;
         y >= -1 * indexesDetectedCycle.second+ parityVectorSpan -1; y -= 1) {
      sf::Vector2i cellPos = {ORIGIN_BORDER_MODE.x-parityVectorNorm+1, y};
      assert(doesCellExists(cellPos) && cells[cellPos].getStatus() == DEFINED);
      period += cells[cellPos].sum() + '0';
    }
  } else {
    for (int x = ORIGIN_BORDER_MODE.x; x >= -1 * indexesDetectedCycle.first;
         x -= 1) {
      sf::Vector2i cellPos = {x, ORIGIN_BORDER_MODE.y};
      if (!doesCellExists(cellPos))
        continue;
      // assert(doesCellExists(cellPos) && cells[cellPos].getStatus() ==
      // DEFINED);
      initSeg += static_cast<int>(cells[cellPos].bit) + '0';
    }
    for (int x = -1 * indexesDetectedCycle.first - 1;
         x >= -1 * indexesDetectedCycle.second; x -= 1) {
      sf::Vector2i cellPos = {x, ORIGIN_BORDER_MODE.y};
      assert(doesCellExists(cellPos) && cells[cellPos].getStatus() == DEFINED);
      period += static_cast<int>(cells[cellPos].bit) + '0';
    }
  }

  printf("\nBig endian convention\n");
  printf("=====================\n");

  printf("Initial segment: %s\n", initSeg.c_str());
  printf("Period: %s\n", period.c_str());
  printf("Number: %s (%s)^inf\n\n", initSeg.c_str(), period.c_str());

  printf("\nLittle endian convention\n");
  printf("========================\n");

  // To enforce little endian
  std::reverse(initSeg.begin(), initSeg.end());
  std::reverse(period.begin(), period.end());

  printf("Initial segment: %s\n", initSeg.c_str());
  printf("Period: %s\n", period.c_str());
  printf("Number: (%s)^inf %s\n\n", period.c_str(), initSeg.c_str());
  printf("\nNote that: due to the way it is detected, the initial segment can "
         "contain part of the period and that the period is not necessarily "
         "minimal.\n");
}

std::vector<CellPosAndCell> World::findTweakSouthBorderUpdates() {
  /*
   * Because there is no forward deduction in cycle/border mode
   * this step is needed to add some 0s that might be missing
   * south of the border.
   * In the paper that will be included in the non local rule.
   */

  std::vector<CellPosAndCell> toRet;

  if (alreadyTweaked)
    return toRet;

  for (const auto &cellPos : cellsOnEdge) {
    assert(doesCellExists(cellPos));
    if (cells[cellPos].bit == 0 && !doesCellExists(cellPos + EAST)) {
      if (doesCellExists(cellPos + NORTH + EAST) &&
          cells[cellPos + NORTH + EAST].bit == ONE) {
        alreadyTweaked = true;
        if (cells[cellPos].getStatus() == DEFINED)
          toRet.push_back(std::make_pair(cellPos + EAST, Cell({ZERO, ZERO})));
        else
          toRet.push_back(std::make_pair(cellPos + EAST, Cell({ZERO, UNDEF})));
      }
    }
  }

  return toRet;
}
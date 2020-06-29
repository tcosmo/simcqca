#include "graphic_engine.h"

GraphicEngine::GraphicEngine(World &world, int screen_w, int screen_h)
    : world(world) {
  window.create(sf::VideoMode(screen_w, screen_h), simcqca_PROG_NAME);
  window.setFramerateLimit(TARGET_FPS);

  isOriginRendered = false;
  isEdgeRendered = false;

  assert(defaultFont.loadFromFile(DEFAULT_FONT_PATH));
  assert(fontTexture.loadFromFile(DEFAULT_FONT_TEXTURE_PATH));

  isTextRendered = true;
  isTextForcedDisabled = false;

  isColorRendered = false;

  isParityVectorRendered = false;

  camera = window.getDefaultView();
  window.setView(camera);
  moveCameraMode = false;
  cameraMouseLeft = false;
  currentZoom = 1.0;

  currentSelectedColor = 0;

  initGraphicBuffers();
}

GraphicEngine::~GraphicEngine() {}

sf::Vector2f GraphicEngine::mapWorldPosToCoords(const sf::Vector2i &cellPos) {
  /**
   * Transform cell position to graphic coordinates.
   */
  return sf::Vector2f({static_cast<float>(cellPos.x * CELL_W),
                       static_cast<float>(cellPos.y * CELL_H)});
}

sf::Vector2i GraphicEngine::mapCoordsToWorldPos(const sf::Vector2f &coords) {
  /**
   * Transforms graphic coordinates to cell position.
   */
  int signX = (coords.x < 0) ? -1 * CELL_W : 0;
  int signY = (coords.y < 0) ? -1 * CELL_H : 0;
  return sf::Vector2i({static_cast<int>((coords.x + signX) / CELL_W),
                       static_cast<int>((coords.y + signY) / CELL_H)});
}

bool GraphicEngine::isControlPressed() {
  return sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
         sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
}

bool GraphicEngine::isShiftPressed() {
  return sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
         sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
}

bool GraphicEngine::isAltPressed() {
  return sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) ||
         sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt);
}

void GraphicEngine::reset() {
  selectedCells.clear();
  selectedBorder.clear();
  for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1) {
    graphicCells[iLayer].clear();
    vertexArrayCell[iLayer].clear();
  }
  initGraphicBuffers();
}

bool GraphicEngine::isSimulationInView() {
  /**
   * Checks whether the simulation is strictly contained in the view or not.
   */
  auto boundaries = getExtremalVisibleCellsPos();
  bool inView = false;
  if (world.inputType == LINE || world.inputType == COL)
    for (const auto &cellPos : world.cellsOnEdge)
      if (cellPos.x >= boundaries.first.x)
        inView = true;
  if (world.inputType == BORDER || world.inputType == CYCLE) {
    if (world.inputType == BORDER && world.isComputationDone())
      return false;

    if (!world.constructCycleInLine) {
      for (const auto &cellPos : world.cellsOnEdge)
        if (cellPos.y >= boundaries.first.y)
          inView = true;
    } else {
      for (const auto &cellPos : world.cellsOnEdge)
        if (cellPos.x >= boundaries.first.x)
          inView = true;
    }
  }

  return inView;
}

void GraphicEngine::toggleSelectedCell(const sf::Vector2i &cellPos,
                                       bool onlyAdd, bool toggleParityVector) {
  /**
   * Select the cell if not selected and unselect otherwise. If `onlyAdd` is on
   * the toggling will be applyied only if the cell was not already selected.
   */
  if (selectedCells.find(cellPos) == selectedCells.end()) {
    selectedCells[cellPos] = SELECTED_CELLS_WHEEL[currentSelectedColor];
    if (toggleParityVector) {
      assert(world.inputType == CYCLE);
      selectedBorder[cellPos] = SELECTED_CELLS_WHEEL[currentSelectedColor];
      selectedCells[cellPos - world.cyclicForwardVector] =
          SELECTED_CELLS_WHEEL[currentSelectedColor];
    }
  } else if (!onlyAdd) {
    selectedCells.erase(cellPos);
    if (world.inputType == CYCLE) {
      if (selectedBorder.find(cellPos) != selectedBorder.end())
        selectedBorder.erase(cellPos);
      if (selectedCells.find(cellPos - world.cyclicForwardVector) !=
          selectedCells.end())
        selectedCells.erase(cellPos - world.cyclicForwardVector);
    }
  }
}

void GraphicEngine::clearSelectedColor(const sf::Vector2i &cellPos) {
  /**
   * If cell is selected, clears all cells with that color.
   */

  if (selectedCells.find(cellPos) == selectedCells.end())
    return;

  sf::Color color = selectedCells[cellPos];
  std::vector<sf::Vector2i> toErase;
  for (const auto &posAndColor : selectedCells)
    if (posAndColor.second == color)
      toErase.push_back(posAndColor.first);
  for (const auto &pos : toErase)
    selectedCells.erase(pos);
}

void GraphicEngine::handleSelectorsEvents(const sf::Event &event) {
  if (event.type == sf::Event::MouseButtonPressed) {
    sf::Vector2i clickedCellPos = mapCoordsToWorldPos(
        window.mapPixelToCoords(sf::Mouse::getPosition(window)));
    if ((event.mouseButton.button == sf::Mouse::Left)) {
      bool toggleParityVec = false;
      if (world.inputType == CYCLE && isAltPressed())
        toggleParityVec = true;
      toggleSelectedCell(clickedCellPos, false, toggleParityVec);
    }
    if ((event.mouseButton.button == sf::Mouse::Right)) {
      clearSelectedColor(clickedCellPos);
    }
  }

  if (event.type == sf::Event::MouseMoved) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && isControlPressed()) {
      sf::Vector2i hoveredCellPos = mapCoordsToWorldPos(
          window.mapPixelToCoords(sf::Mouse::getPosition(window)));
      toggleSelectedCell(hoveredCellPos, true);
    }
  }

  if (event.type == sf::Event::KeyPressed) {
    switch (event.key.code) {
    case sf::Keyboard::Right:
      currentSelectedColor += 1;
      currentSelectedColor %= COLORED_SELECTORS_WHEEL_SIZE;
      break;

    case sf::Keyboard::Left:
      currentSelectedColor -= 1;
      currentSelectedColor %= COLORED_SELECTORS_WHEEL_SIZE;
      break;

    default:
      break;
    }
  }
}

void GraphicEngine::outlineResult() {
  /**
   * Visually outlines base 3 -> base 2 conversion by selecting
   * the corresponding base 3 column and base 2 line.
   */
  assert(world.inputType == LINE || world.inputType == COL);

  sf::Vector2i targetCellPos;

  // Heuristic bound to have all the necessary cells on the screen
  for (int iStep = 0; iStep < 4 * world.inputStr.size(); iStep += 1)
    world.next();

  sf::Vector2i currPos;
  std::string base3;

  if (world.inputType == LINE) {
    currPos = {-1 * static_cast<int>(world.inputStr.size()), 0};
    while (world.doesCellExists(currPos) &&
           world.doesCellExists(currPos + SOUTH)) {
      assert(world.cells[currPos].getStatus() == DEFINED);
      base3.push_back('0' + world.cells[currPos].sum());
      selectedCells[currPos] = SELECTED_CELLS_WHEEL[0];
      currPos += SOUTH;
    }
  } else {
    sf::Vector2i currPos = {0, -1 * static_cast<int>(world.inputStr.size())};
    while (world.doesCellExists(currPos) &&
           world.doesCellExists(currPos + SOUTH)) {
      assert(world.cells[currPos].getStatus() == DEFINED);
      base3.push_back('0' + world.cells[currPos].sum());
      selectedCells[currPos] = SELECTED_CELLS_WHEEL[0];
      currPos += SOUTH;
    }
  }

  std::string base2 = World::base32(base3);
  int xCenter = -1 * static_cast<int>(world.inputStr.size());
  int yCenter = base3.size() / 2;
  if (world.inputType == COL) {
    xCenter = 0;
    yCenter = -1 * yCenter;
  }
  cameraCenter(mapWorldPosToCoords({xCenter, yCenter}));

  int visibilityOffset = 4;
  sf::Vector2i startOfCol;
  sf::Vector2i endOfLine;

  if (world.inputType == LINE) {
    startOfCol = {-1 * static_cast<int>(world.inputStr.size()),
                  0 - visibilityOffset};
    endOfLine = {-1 * static_cast<int>(world.inputStr.size() + base2.size()) -
                     visibilityOffset,
                 static_cast<int>(base3.size())};
    currPos = {-1 * static_cast<int>(world.inputStr.size()) - 1,
               static_cast<int>(base3.size())};
  } else {
    startOfCol = {0, -1 * static_cast<int>(world.inputStr.size()) -
                         visibilityOffset};
    endOfLine = {-1 * static_cast<int>(base2.size()) - visibilityOffset, 0};
    currPos = {-1, 0};
  }

  int i = 0;
  std::string constructedBase2;
  while (world.doesCellExists(currPos) && i < base2.size()) {
    selectedCells[currPos] = SELECTED_CELLS_WHEEL[0];
    constructedBase2.push_back('0' +
                               static_cast<char>(world.cells[currPos].bit));
    currPos += WEST;
    i += 1;
  }

  std::reverse(constructedBase2.begin(), constructedBase2.end());

  while (!isCellInView(endOfLine) || !isCellInView(startOfCol))
    cameraZoom(1 / DEFAULT_CAM_ZOOM_STEP);

  printf("\nBase conversion result\n");
  printf("======================\n");

  int z3 = 0;
  for (const auto &c : base3)
    z3 = 3 * z3 + (c - '0');
  int z2 = 0;
  for (const auto &c : constructedBase2)
    z2 = 2 * z2 + (c - '0');

  printf("The outlined column is a base 3' encoding of: %s = %d\n",
         base3.c_str(), z3);
  printf("The outlined line is a base 2 encoding of: %s = %d\n",
         constructedBase2.c_str(), z2);
  printf("Both represent the number: %d\n\n", z3);
  // while (isSimulationInView())
  //   world.next();
}

void GraphicEngine::run() {
  cameraZoom(3);
  cameraCenter({-5 * CELL_W, 0});

  // For FPS computation
  sf::Clock clock;
  int currentFPS = 1.0f;
  int framePassed = 0;

  updateGraphicCells();

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      handleCameraEvents(event);

      if (isShiftPressed() || isControlPressed())
        handleSelectorsEvents(event);

      if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::Escape:
          window.close();
          break;

        case sf::Keyboard::A:
          printf("FPS: %d\n", currentFPS);
          printf("Vertex array (Background): %ld x O(%d)\n",
                 graphicCells[CELL_BACKGROUND].size(), VERTEX_ARRAY_MAX_SIZE);
          printf("Number of graphic cells (quads): %d\n",
                 totalGraphicBufferSize());
          printf("Number of cells on edge: %ld\n", world.cellsOnEdge.size());
          printf("Current zoom factor: %lf\n", currentZoom);
          break;

        case sf::Keyboard::O:
          isOriginRendered = !isOriginRendered;
          break;

        case sf::Keyboard::E:
          isEdgeRendered = !isEdgeRendered;
          break;

        case sf::Keyboard::F:
          isParityVectorRendered = !isParityVectorRendered;
          break;

        case sf::Keyboard::T:
          isTextRendered = !isTextRendered;
          break;

        case sf::Keyboard::K:
          isColorRendered = !isColorRendered;
          break;

        case sf::Keyboard::N:
          world.next();
          break;

        case sf::Keyboard::M:
          while (isSimulationInView())
            world.next();
          break;

        case sf::Keyboard::Right:
          if (isAltPressed() && world.inputType == CYCLE) {
            reset();
            world.rotate(1);
            while (isSimulationInView())
              world.next();
          }
          break;

        case sf::Keyboard::Left:
          if (isAltPressed() && world.inputType == CYCLE) {
            reset();
            world.rotate(-1);
            while (isSimulationInView())
              world.next();
          }
          break;

        case sf::Keyboard::P:
          if (world.inputType == CYCLE) {
            while (!world.isCycleDetected())
              world.next();
            world.printCycleInformation();
          } else if (world.inputType == LINE || world.inputType == COL) {
            outlineResult();
          }
          break;

        case sf::Keyboard::R:
          reset();
          world.reset();
          break;

        default:
          break;
        }
      }
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear(BACKGROUND_COLOR);

    updateGraphicCells();

    for (const auto &graphicBuffer : graphicCells[CELL_BACKGROUND])
      window.draw(graphicBuffer);

    if (isColorRendered)
      for (const auto &graphicBuffer : graphicCells[CELL_COLOR])
        window.draw(graphicBuffer);

    if (isTextRendered)
      for (const auto &graphicBuffer : graphicCells[CELL_TEXT])
        window.draw(graphicBuffer, &fontTexture);

    if (isEdgeRendered)
      renderEdge();

    if (isParityVectorRendered &&
        (world.inputType == BORDER || world.inputType == CYCLE))
      renderParityVector();

    renderSelectedCells();
    if (world.inputType == CYCLE)
      renderSelectedBorder();

    if (isOriginRendered)
      renderOrigin();

    window.display();

    if (clock.getElapsedTime().asSeconds() >= 1.0) {
      currentFPS = framePassed;
      framePassed = 0;
      clock.restart();
    }
    framePassed += 1;
  }
}
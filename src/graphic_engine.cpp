#include "graphic_engine.h"

GraphicEngine::GraphicEngine(World &world, int screen_w, int screen_h,
                             bool isTikzEnabled)
    : world(world), isTikzEnabled(isTikzEnabled) {
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

  tikzMode = isTikzEnabled;
  isTikzGridEnabled = false;
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

  if (isTikzEnabled) {
    tikzMode = isTikzEnabled;
    tikzSelection.clear();
    isTikzGridEnabled = false;
  }
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
    selectedCells[cellPos] = currentSelectedColor;
    if (toggleParityVector) {
      assert(world.inputType == CYCLE);
      selectedBorder[cellPos] = SELECTED_CELLS_WHEEL[currentSelectedColor];
      selectedCells[cellPos - world.cyclicForwardVector] = currentSelectedColor;
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

  int colorId = selectedCells[cellPos];
  std::vector<sf::Vector2i> toErase;
  for (const auto &posAndColor : selectedCells)
    if (posAndColor.second == colorId)
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
      if (currentSelectedColor == -1)
        currentSelectedColor = COLORED_SELECTORS_WHEEL_SIZE -1;
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

  // Heuristic bound to have all the necessary cells on the screen
  for (int iStep = 0; iStep < 4 * world.inputStr.size(); iStep += 1)
    world.next();

  sf::Vector2i targetCell = {0, 0};
  if (world.inputType == LINE) {
    targetCell = {-1 * static_cast<int>(world.inputStr.size()), 0};
    while (world.doesCellExists(targetCell) &&
           world.doesCellExists(targetCell + SOUTH))
      targetCell += SOUTH;
  }

  std::string base2Row, base3Col;
  sf::Vector2i currentPos = targetCell + NORTH;

  while (world.doesCellExists(currentPos)) {
    base3Col += '0' + world.cells[currentPos].sum();
    selectedCells[currentPos] = 0;
    currentPos += NORTH;
  }
  std::reverse(base3Col.begin(), base3Col.end());
  unsigned long long int z3 = 0;
  for (const auto &c : base3Col)
    z3 = 3 * z3 + (c - '0');

  // std::string computedBase2 = World::base32(base3Col);

  currentPos = targetCell + WEST;
  while (world.doesCellExists(currentPos)) {
    base2Row += '0' + static_cast<char>(world.cells[currentPos].bit);
    selectedCells[currentPos] = 0;
    currentPos += WEST;
  }

  // Remove head 0s
  currentPos += EAST;
  while (world.doesCellExists(currentPos) &&
         world.cells[currentPos].bit == ZERO) {

    selectedCells.erase(currentPos);
    base2Row.erase(base2Row.size() - 1);
    currentPos += EAST;
  }

  std::reverse(base2Row.begin(), base2Row.end());
  unsigned long long int z2 = 0;
  for (const auto &c : base2Row)
    z2 = 2 * z2 + (c - '0');

  if (base2Row.size() > 64) {
    printf("\nYou are considering > 64 bit strings which exceeds the precision "
           "the numbers were computed with.\n");
    printf("Hence the following numbers interpretations are false "
           "(truncated).\n\n");
  } else
    printf("\n");
  printf("The outlined vertical column is a base 3' encoding of: `%s` = %lld\n",
         base3Col.c_str(), z3);
  printf("The outlined horizontal row is a base 2 encoding of: `%s` = %lld\n",
         base2Row.c_str(), z2);
  if (base2Row.size() <= 64) {
    printf("Both represent the number: %lld\n\n", z3);
  }
  cameraCenter(mapWorldPosToCoords(targetCell));

  // Push a bit to the right
  while (isCellInView(2 * EAST))
    cameraTranslate(mapWorldPosToCoords(WEST));

  while (isCellInView(2 * SOUTH))
    cameraTranslate(mapWorldPosToCoords(NORTH));

  int visibilityOffset = 2;
  while (
      !isCellInView(targetCell +
                    (base3Col.size() + visibilityOffset) * NORTH) ||
      !isCellInView(targetCell + (base2Row.size() + visibilityOffset) * WEST))
    cameraZoom(1 / DEFAULT_CAM_ZOOM_STEP);

  while (isSimulationInView())
    world.next();
}

void GraphicEngine::handleTikzEvents(const sf::Event &event) {
  /**
   * In tikz mode, enables the user to click on cells to be selected.
   */
  assert(isTikzEnabled);
  if (event.type == sf::Event::MouseButtonPressed)
    if ((event.mouseButton.button == sf::Mouse::Left)) {
      if (std::find(tikzSelection.begin(), tikzSelection.end(),
                    tikzCursorPos) == tikzSelection.end())
        tikzSelection.push_back(tikzCursorPos);
    }
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

      if (isTikzEnabled)
        handleTikzEvents(event);

      if (isShiftPressed() || isControlPressed())
        handleSelectorsEvents(event);

      if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::Escape:
          window.close();
          break;

        case sf::Keyboard::A:
          // Not to clash with ctrl + A
          if (!isControlPressed()) {
            printf("FPS: %d\n", currentFPS);
            printf("Vertex array (Background): %ld x O(%d)\n",
                   graphicCells[CELL_BACKGROUND].size(), VERTEX_ARRAY_MAX_SIZE);
            printf("Number of graphic cells (quads): %d\n",
                   totalGraphicBufferSize());
            printf("Number of cells on edge: %ld\n", world.cellsOnEdge.size());
            printf("Current zoom factor: %lf\n", currentZoom);
          }
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

        case sf::Keyboard::G:
          if (isTikzEnabled) {
            isTikzGridEnabled = !isTikzGridEnabled;
            printf("Tikz grid %s.\n", (isTikzEnabled) ? "enabled" : "disabled");
          }
          break;

        case sf::Keyboard::T:
          if (!isTikzEnabled || !isControlPressed()) {
            isTextRendered = !isTextRendered;
          } else if (isTikzEnabled && isControlPressed()) {
            tikzMode = !tikzMode;
            tikzSelection.clear();
          }
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

    if (isTikzEnabled) {
      if (tikzSelection.size() == 2) {
        generateTikzFromSelection();
        tikzSelection.clear();
        tikzMode = false;
      }

      if (tikzMode)
        renderTikzSelection();
    }

    window.display();

    if (clock.getElapsedTime().asSeconds() >= 1.0) {
      currentFPS = framePassed;
      framePassed = 0;
      clock.restart();
    }
    framePassed += 1;
  }
}
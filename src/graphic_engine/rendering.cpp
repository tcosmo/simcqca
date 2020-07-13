#include "../graphic_engine.h"

void GraphicEngine::outlineCell(const sf::Vector2i &cellPos,
                                sf::Color outlineColor) {
  /**
   * Outlines the border of a cell with the given `outlineColor`.
   */
  sf::RectangleShape carre(sf::Vector2f(CELL_W, CELL_H));
  carre.setOutlineColor(outlineColor);
  carre.setFillColor(sf::Color::Transparent);
  carre.setOutlineThickness(DEFAULT_OUTLINE_THICKNESS);
  carre.setPosition(mapWorldPosToCoords(cellPos));
  window.draw(carre);
}

void GraphicEngine::outlineCell(const sf::Vector2i &cellPos,
                                sf::Color outlineColor,
                                const sf::Vector2i &side) {
  /**
   * Outlines the border of a cell with the given `outlineColor`.
   */
  if (side == SOUTH) {
    sf::RectangleShape rec(sf::Vector2f(CELL_W + DEFAULT_OUTLINE_THICKNESS,
                                        DEFAULT_OUTLINE_THICKNESS));
    rec.setOutlineThickness(0);
    rec.setFillColor(outlineColor);
    rec.setPosition(mapWorldPosToCoords(cellPos + SOUTH));
    window.draw(rec);
  } else if (side == EAST) {
    sf::RectangleShape rec(sf::Vector2f(DEFAULT_OUTLINE_THICKNESS, CELL_H));
    rec.setOutlineThickness(0);
    rec.setFillColor(outlineColor);
    rec.setPosition(mapWorldPosToCoords(cellPos + EAST));
    window.draw(rec);
  }
}

void GraphicEngine::renderOrigin() {
  /**
   * Outline the border of the (0,0) cell (world coordinates) in black.
   */
  outlineCell({0, 0}, sf::Color::Blue);
}

void GraphicEngine::renderEdge() {
  /**
   * Outline each cells on the edge of the world.
   */
  for (const auto &cellPos : world.cellsOnEdge)
    outlineCell(cellPos, COLOR_DARKER_GREEN);
}

void GraphicEngine::renderSelectedCells() {
  /**
   * Renders all cells which are being selected at the moment.
   */

  for (const auto &posAndColor : selectedCells)
    outlineCell(posAndColor.first, SELECTED_CELLS_WHEEL[posAndColor.second]);
}

void GraphicEngine::renderSelectedBorder() {
  /**
   * In cyclic mode renders the parity vector beneath selected cells.
   */
  for (const auto &posAndColor : selectedBorder) {
    sf::Vector2i currentPos = posAndColor.first - world.cyclicForwardVector;
    // FIXME: should not have access to world inputStr, world should give
    // a public access to the step to take.
    for (int i = 0; i < world.inputStr.length(); i++) {
      const char &c = world.inputStr[i];
      outlineCell(currentPos,

                  SELECTED_CELLS_WHEEL[posAndColor.second], SOUTH);
      currentPos += WEST;
      if (c == '1') {
        currentPos += SOUTH;
        outlineCell(currentPos, SELECTED_CELLS_WHEEL[posAndColor.second], EAST);
      }
    }
  }
}

void GraphicEngine::initGraphicBuffers() {
  /**
   * Creates the first graphic buffer (i.e. vertex array) for each layer.
   */

  for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1)
    newGraphicBuffer(iLayer);
}

void GraphicEngine::newGraphicBuffer(int iLayer) {
  /**
   * Creates a new graphic buffer for layer `iLayer`.
   */
  graphicCells[iLayer].push_back(sf::VertexArray());
  graphicCells[iLayer][graphicCells[iLayer].size() - 1].setPrimitiveType(
      LAYER_PRIMITIVE_TYPE[iLayer]);
}

int GraphicEngine::totalGraphicBufferSize() {
  /**
   * Counts all cell drawn by the graphic engine.
   */

  int toRet = 0;
  for (int iLayer = 1; iLayer < NB_LAYERS; iLayer += 1)
    for (const auto &buffer : graphicCells[CELL_BACKGROUND])
      toRet += buffer.getVertexCount();

  return toRet;
}

std::vector<sf::Vertex>
GraphicEngine::getCellBackgroundVertices(const sf::Vector2i &cellPos,
                                         const Cell &cell) {
  /**
   * Constructs the vertices for the background of a cell (layer
   * `CELL_BACKGROUND`).
   */
  sf::Color color = BACKGROUND_COLOR_HALF_DEFINED;
  if (cell.getStatus() == DEFINED)
    color = BACKGROUND_COLOR_DEFINED;

  std::vector<sf::Vertex> toRet;
  for (int i = 0; i < 4; i += 1) {
    sf::Vertex v;
    v.color = color;
    toRet.push_back(v);
  }

  toRet[0].position = mapWorldPosToCoords(cellPos);
  toRet[1].position = mapWorldPosToCoords(cellPos + EAST);
  toRet[2].position = mapWorldPosToCoords(cellPos + SOUTH + EAST);
  toRet[3].position = mapWorldPosToCoords(cellPos + SOUTH);

  return toRet;
}

std::vector<sf::Vertex>
GraphicEngine::getCellColorVertices(const sf::Vector2i &cellPos,
                                    const Cell &cell) {
  /**
   * Constructs the vertices for the color of a cell corresponding to the symbol
   * it holds when it is DEFINED (layer `CELL_COLOR`).
   */
  sf::Color color = BACKGROUND_COLOR_HALF_DEFINED;
  if (cell.getStatus() == DEFINED)
    color = CELL_DEFINED_COLORS[cell.index()];

  std::vector<sf::Vertex> toRet;
  for (int i = 0; i < 4; i += 1) {
    sf::Vertex v;
    v.color = color;
    toRet.push_back(v);
  }

  toRet[0].position = mapWorldPosToCoords(cellPos);
  toRet[1].position = mapWorldPosToCoords(cellPos + EAST);
  toRet[2].position = mapWorldPosToCoords(cellPos + SOUTH + EAST);
  toRet[3].position = mapWorldPosToCoords(cellPos + SOUTH);

  return toRet;
}

sf::Vector2f GraphicEngine::getFontTextureCharCoords(char c, int i) {
  /**
   * Returns the i^th coordinate for the character `c` in the default font
   * texture.
   */
  assert(i < 4);
  static sf::Vector2f textureVec[4] = {
      {0, 0},
      {DEFAULT_FONT_TEXTURE_CHAR_W, 0},
      {DEFAULT_FONT_TEXTURE_CHAR_W, DEFAULT_FONT_TEXTURE_CHAR_H},
      {0, DEFAULT_FONT_TEXTURE_CHAR_H}};
  c -= 1;
  sf::Vector2f topLeft = {static_cast<float>((c % DEFAULT_FONT_TEXTURE_W) *
                                             DEFAULT_FONT_TEXTURE_CHAR_W),
                          static_cast<float>((c / DEFAULT_FONT_TEXTURE_W) *
                                             DEFAULT_FONT_TEXTURE_CHAR_H)};

  return topLeft + textureVec[i];
}

std::vector<sf::Vertex>
GraphicEngine::getCellTextVertices(const sf::Vector2i &cellPos,
                                   const Cell &cell) {
  /**
   * Constructs the vertices for the text inside a cell (layer `CELL_TEXT`).
   */
  assert(cell.bit != UNDEF);
  std::vector<sf::Vertex> toRet;
  for (int iVertex = 0; iVertex < NB_TEXT_QUADS; iVertex += 1)
    toRet.push_back(sf::Vertex());

  // Bit
  toRet[0].position = mapWorldPosToCoords(cellPos);
  toRet[1].position = mapWorldPosToCoords(cellPos + EAST);
  toRet[2].position = mapWorldPosToCoords(cellPos + SOUTH + EAST);
  toRet[3].position = mapWorldPosToCoords(cellPos + SOUTH);

  // Setting color
  for (int i = 0; i < 4; i += 1)
    toRet[i].color = sf::Color::White;

  // Tweaking position
  for (int i = 0; i < 4; i += 1)
    toRet[i].position.y += 4;
  // Tweaking scale
  for (int i = 0; i < 2; i += 1)
    toRet[i].position.y += 3;

  for (int i = 0; i < 4; i += 1)
    if (!cell.bit)
      toRet[i].texCoords =
          getFontTextureCharCoords('O', i); // That's a O not a 0
    else
      toRet[i].texCoords = getFontTextureCharCoords('1', i);
  // Carry
  if (cell.carry == UNDEF || cell.carry == ZERO)
    return toRet;

  toRet[4].position = mapWorldPosToCoords(cellPos);
  toRet[5].position = mapWorldPosToCoords(cellPos + EAST);
  toRet[6].position = mapWorldPosToCoords(cellPos + SOUTH + EAST);
  toRet[7].position = mapWorldPosToCoords(cellPos + SOUTH);

  // Setting color
  sf::Color carryColor = sf::Color::White;
  if (cell.isBootstrappingCarry)
    carryColor = COLOR_SPECIAL_CARRY;
  for (int i = 4; i < 8; i += 1)
    toRet[i].color = carryColor;

  // Tweaking position
  for (int i = 4; i < 8; i += 1) {
    toRet[i].position.y -= 4.4;
    toRet[i].position.x += 1;
  }

  for (int i = 4; i < 8; i += 1)
    toRet[i].texCoords = getFontTextureCharCoords('-', i - 4);

  return toRet;
}

void GraphicEngine::appendOrUpdateCell(const sf::Vector2i &cellPos,
                                       const Cell &cell) {
  /**
   * Adding or modifying a cell in the graphic buffer.
   */
  assert(cell.getStatus() >= HALF_DEFINED);

  auto verticesBackground = getCellBackgroundVertices(cellPos, cell);
  auto verticesColor = getCellColorVertices(cellPos, cell);
  auto verticesText = getCellTextVertices(cellPos, cell);

  std::vector<sf::Vertex> allVertices[NB_LAYERS] = {
      verticesBackground, verticesColor, verticesText};

  bool append = false;
  if (vertexArrayCell[CELL_BACKGROUND].find(cellPos) ==
      vertexArrayCell[CELL_BACKGROUND].end()) {
    append = true;
    for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1) {
      vertexArrayCell[iLayer][cellPos] =
          std::make_pair(graphicCells[iLayer].size() - 1,
                         currentBuffer(iLayer).getVertexCount());
    }
  }
  // Fill background, color and text
  for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1) {
    const auto &arrayAndPos = vertexArrayCell[iLayer][cellPos];
    for (int iVertex = 0; iVertex < allVertices[iLayer].size(); iVertex += 1) {
      if (!append)
        graphicCells[iLayer][arrayAndPos.first][arrayAndPos.second + iVertex] =
            allVertices[iLayer][iVertex];
      else
        graphicCells[iLayer][arrayAndPos.first].append(
            allVertices[iLayer][iVertex]);
    }
  }
}

sf::VertexArray &GraphicEngine::currentBuffer(int iLayer) {
  /**
   * Returns the graphic buffer currently in use on layer `iLayer`.
   */
  assert(graphicCells[iLayer].size() != 0);
  return graphicCells[iLayer][graphicCells[iLayer].size() - 1];
}

bool GraphicEngine::hasBufferLimitExceeded(int iLayer) {
  /**
   * Check if we have exceeded the maximal capacity of the current graphic
   * buffer.
   */
  return currentBuffer(iLayer).getVertexCount() >= VERTEX_ARRAY_MAX_SIZE;
}

void GraphicEngine::updateGraphicCells() {
  /**
   * Updates the graphic buffers with the information sent by the world.
   */
  std::vector<sf::Vector2i> cellBuffer = world.getAndFlushGraphicBuffer();
  for (auto &cellPos : cellBuffer) {
    for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1)
      if (hasBufferLimitExceeded(iLayer))
        newGraphicBuffer(iLayer);
    appendOrUpdateCell(cellPos, world.cells[cellPos]);
  }
}

void GraphicEngine::renderParityVector() {
  /**
   * Renders all cells on the input parity vector
   */

  for (const auto &pos : world.parityVectorCells)
    outlineCell(pos, COLOR_PARITY_VECTOR);
}

void GraphicEngine::renderTikzSelection() {
  /**
   * Renders the cursor and selected cell for
   * generating tikz image of the rectangle defined by those two cells.
   */
  assert(isTikzEnabled && tikzSelection.size() < 2);

  auto mousePosition = sf::Mouse::getPosition(window);
  auto coordMouse = window.mapPixelToCoords(mousePosition);
  tikzCursorPos = mapCoordsToWorldPos(coordMouse);

  outlineCell(tikzCursorPos, sf::Color::Blue);

  for (const auto &cellPos : tikzSelection)
    outlineCell(cellPos, sf::Color::Blue);
}
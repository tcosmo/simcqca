#include "graphic_engine.h"

GraphicEngine::GraphicEngine(World& world, int screen_w, int screen_h)
    : world(world)
{
    window.create(sf::VideoMode(screen_w, screen_h), simcqca_PROG_NAME);
    window.setFramerateLimit(TARGET_FPS);

    isOriginRendered = false;
    isEdgeRendered = false;

    assert(defaultFont.loadFromFile(DEFAULT_FONT_PATH));
    assert(fontTexture.loadFromFile(DEFAULT_FONT_TEXTURE_PATH));

    isTextRendered = true;
    isTextForcedDisabled = false;

    isColorRendered = false;

    camera = window.getDefaultView();
    window.setView(camera);
    moveCameraMode = false;
    cameraMouseLeft = false;
    currentZoom = 1.0;

    for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1)
        newGraphicBuffer(iLayer);
}

GraphicEngine::~GraphicEngine()
{
}

void GraphicEngine::newGraphicBuffer(int iLayer)
{

    graphicCells[iLayer].push_back(sf::VertexArray());
    graphicCells[iLayer][graphicCells[iLayer].size() - 1].setPrimitiveType(LAYER_PRIMITIVE_TYPE[iLayer]);
}

int GraphicEngine::totalGraphicBufferSize()
{
    /***
     * Counts all cell drawn by the graphic engine. 
    */

    int toRet = 0;
    for (int iLayer = 1; iLayer < NB_LAYERS; iLayer += 1)
        for (const auto& buffer : graphicCells[CELL_BACKGROUND])
            toRet += buffer.getVertexCount();

    return toRet;
}

sf::Vector2f GraphicEngine::mapWorldPosToCoords(const sf::Vector2i& world_coords)
{
    return sf::Vector2f({ static_cast<float>(world_coords.x * CELL_W),
        static_cast<float>(world_coords.y * CELL_H) });
}

sf::Vector2i GraphicEngine::mapCoordsToWorldPos(const sf::Vector2f& coords)
{
    int sign_x = (coords.x < 0) ? -1 * CELL_W : 0;
    int sign_y = (coords.y < 0) ? -1 * CELL_H : 0;
    return sf::Vector2i({ static_cast<int>((coords.x + sign_x) / CELL_W),
        static_cast<int>((coords.y + sign_y) / CELL_H) });
}

bool GraphicEngine::isControlPressed()
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
}

bool GraphicEngine::isShiftPressed()
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
}

std::vector<sf::Vertex> GraphicEngine::getCellBackgroundVertices(const sf::Vector2i& cellPos, const Cell& cell)
{
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

std::vector<sf::Vertex> GraphicEngine::getCellColorVertices(const sf::Vector2i& cellPos, const Cell& cell)
{
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

sf::Vector2f GraphicEngine::getFontTextureCharCoords(char c, int i)
{
    /**
     * Returns the i^th coordinate for the character `c` in the default font texture.
    */
    assert(i < 4);
    static sf::Vector2f textureVec[4] = { { 0, 0 }, { DEFAULT_FONT_TEXTURE_CHAR_W, 0 },
        { DEFAULT_FONT_TEXTURE_CHAR_W, DEFAULT_FONT_TEXTURE_CHAR_H },
        { 0, DEFAULT_FONT_TEXTURE_CHAR_H } };
    c -= 1;
    sf::Vector2f topLeft = { static_cast<float>((c % DEFAULT_FONT_TEXTURE_W) * DEFAULT_FONT_TEXTURE_CHAR_W),
        static_cast<float>((c / DEFAULT_FONT_TEXTURE_W) * DEFAULT_FONT_TEXTURE_CHAR_H) };

    return topLeft + textureVec[i];
}

std::vector<sf::Vertex> GraphicEngine::getCellTextVertices(const sf::Vector2i& cellPos, const Cell& cell)
{
    assert(cell.bit != UNDEF);
    std::vector<sf::Vertex> toRet;
    for (int iVertex = 0; iVertex < NB_TEXT_QUADS; iVertex += 1)
        toRet.push_back(sf::Vertex());

    // Bit
    toRet[0].position = mapWorldPosToCoords(cellPos);
    toRet[1].position = mapWorldPosToCoords(cellPos + EAST);
    toRet[2].position = mapWorldPosToCoords(cellPos + SOUTH + EAST);
    toRet[3].position = mapWorldPosToCoords(cellPos + SOUTH);

    // Tweaking position
    for (int i = 0; i < 4; i += 1)
        toRet[i].position.y += 4;
    // Tweaking scale
    for (int i = 0; i < 2; i += 1)
        toRet[i].position.y += 3;

    for (int i = 0; i < 4; i += 1)
        if (!cell.bit)
            toRet[i].texCoords = getFontTextureCharCoords('O', i); // That's a O not a 0
        else
            toRet[i].texCoords = getFontTextureCharCoords('1', i);
    // Carry
    if (cell.carry == UNDEF || cell.carry == ZERO)
        return toRet;

    toRet[4].position = mapWorldPosToCoords(cellPos);
    toRet[5].position = mapWorldPosToCoords(cellPos + EAST);
    toRet[6].position = mapWorldPosToCoords(cellPos + SOUTH + EAST);
    toRet[7].position = mapWorldPosToCoords(cellPos + SOUTH);

    // Tweaking position
    for (int i = 4; i < 8; i += 1) {
        toRet[i].position.y -= 4.4;
        toRet[i].position.x += 1;
    }

    for (int i = 4; i < 8; i += 1)
        toRet[i].texCoords = getFontTextureCharCoords('-', i - 4);

    return toRet;
}

void GraphicEngine::appendOrUpdateCell(const sf::Vector2i& cellPos, const Cell& cell)
{
    assert(cell.getStatus() >= HALF_DEFINED);

    auto verticesBackground = getCellBackgroundVertices(cellPos, cell);
    auto verticesColor = getCellColorVertices(cellPos, cell);
    auto verticesText = getCellTextVertices(cellPos, cell);

    std::vector<sf::Vertex> allVertices[NB_LAYERS] = { verticesBackground, verticesColor, verticesText };

    bool append = false;
    if (vertexArrayCell[CELL_BACKGROUND].find(cellPos) == vertexArrayCell[CELL_BACKGROUND].end()) {
        append = true;
        for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1) {
            vertexArrayCell[iLayer][cellPos] = std::make_pair(graphicCells[iLayer].size() - 1, currentBuffer(iLayer).getVertexCount());
        }
    }
    // Fill background, color and text
    for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1) {
        const auto& arrayAndPos = vertexArrayCell[iLayer][cellPos];
        for (int iVertex = 0; iVertex < allVertices[iLayer].size(); iVertex += 1) {
            if (!append)
                graphicCells[iLayer][arrayAndPos.first][arrayAndPos.second + iVertex] = allVertices[iLayer][iVertex];
            else
                graphicCells[iLayer][arrayAndPos.first].append(allVertices[iLayer][iVertex]);
        }
    }
}

sf::VertexArray& GraphicEngine::currentBuffer(int iLayer)
{
    assert(graphicCells[iLayer].size() != 0);
    return graphicCells[iLayer][graphicCells[iLayer].size() - 1];
}

bool GraphicEngine::hasBufferLimitExceeded(int iLayer)
{
    return currentBuffer(iLayer).getVertexCount() >= VERTEX_ARRAY_MAX_SIZE;
}

void GraphicEngine::updateGraphicCells()
{
    std::vector<sf::Vector2i> cellBuffer = world.getAndFlushGraphicBuffer();
    for (auto& cellPos : cellBuffer) {
        for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1)
            if (hasBufferLimitExceeded(iLayer))
                newGraphicBuffer(iLayer);
        appendOrUpdateCell(cellPos, world.cells[cellPos]);
    }
}

bool GraphicEngine::isSimulationInView()
{
    auto boundaries = getExtremalVisibleCellsPos();
    bool inView = false;
    if (world.inputType == LINE || world.inputType == COL)
        for (const auto& cellPos : world.cellsOnEdge)
            if (cellPos.x >= boundaries.first.x)
                inView = true;
    if (world.inputType == BORDER) {
        if (world.isComputationDone())
            return false;
        sf::Vector2i topLeftCell = { boundaries.first.x, 0 };
        return !world.doesCellExists(topLeftCell) || world.cells[topLeftCell].getStatus() == HALF_DEFINED;
    }
    return inView;
}

void GraphicEngine::run()
{
    cameraZoom(3);
    cameraCenter({ -5 * CELL_W, 0 });

    // For FPS computation
    sf::Clock clock;
    int currentFPS = 1.0f;
    int framePassed = 0;

    updateGraphicCells();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            handleCameraEvents(event);
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Escape:
                    window.close();
                    break;

                case sf::Keyboard::F:
                    printf("FPS: %d\n", currentFPS);
                    printf("Vertex array (Background): %ld x O(%d)\n", graphicCells[CELL_BACKGROUND].size(), VERTEX_ARRAY_MAX_SIZE);
                    printf("Number of graphic cells (quads): %d\n", totalGraphicBufferSize());
                    printf("Number of cells on edge: %ld\n", world.cellsOnEdge.size());
                    printf("Current zoom factor: %lf\n", currentZoom);
                    break;

                case sf::Keyboard::O:
                    isOriginRendered = !isOriginRendered;
                    break;

                case sf::Keyboard::E:
                    isEdgeRendered = !isEdgeRendered;
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

                default:
                    break;
                }
            }
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(BACKGROUND_COLOR);

        updateGraphicCells();

        for (const auto& graphicBuffer : graphicCells[CELL_BACKGROUND])
            window.draw(graphicBuffer);

        if (isColorRendered)
            for (const auto& graphicBuffer : graphicCells[CELL_COLOR])
                window.draw(graphicBuffer);

        if (isTextRendered)
            for (const auto& graphicBuffer : graphicCells[CELL_TEXT])
                window.draw(graphicBuffer, &fontTexture);

        if (isEdgeRendered)
            renderEdge();

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
#include "graphic_engine.h"

GraphicEngine::GraphicEngine(World& world, int screen_w, int screen_h)
    : world(world)
{
    window.create(sf::VideoMode(screen_w, screen_h), simcqca_PROG_NAME);
    window.setFramerateLimit(TARGET_FPS);

    assert(defaultFont.loadFromFile(DEFAULT_FONT_PATH));
    isTextRendered = true;
    isTextForcedDisabled = false;

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

std::vector<sf::Vertex> GraphicEngine::getCellTextVertices(const sf::Vector2i& cellPos, const Cell& cell)
{
    std::vector<sf::Vertex> toRet;
    for(int iVertex = 0 ; iVertex < NB_TEXT_VERTICES ; iVertex += 1)
        toRet.push_back(sf::Vertex());

    if(cell.bit != UNDEF) {
        if(cell.bit == ONE) {
            sf::Vertex up, down;
            sf::Vector2f upCoords = mapWorldPosToCoords(cellPos);
            upCoords.x += CELL_W/2;
            upCoords.y += 5; // Tweaking
            up.position = upCoords;
            sf::Vector2f downCoords = upCoords;
            upCoords.y += CELL_H-10;
            down.position = downCoords;

            up.color = sf::Color::White;
            down.color = sf::Color::White;
            toRet[0] = up;
            toRet[1] = down;
        }
    }

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

bool GraphicEngine::canRenderText()
{
    return currentZoom >= ZOOM_FACTOR_TEXT_THRESH;
}

void GraphicEngine::run()
{
    cameraZoom(3);
    cameraCenter({ -5 * CELL_W, 0 });

    // For FPS computation
    sf::Clock clock;
    int currentFPS = 1.0f;
    int framePassed = 0;

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
                    printf("Number of half defined cells: %ld\n", world.halfDefinedCells.size());
                    printf("Current zoom factor: %lf\n", currentZoom);
                    break;

                case sf::Keyboard::T:
                    isTextRendered = !isTextRendered;
                    break;

                case sf::Keyboard::N:
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
        for (int iLayer = 0; iLayer < NB_LAYERS; iLayer += 1)
            for (const auto& graphicBuffer : graphicCells[iLayer])
                window.draw(graphicBuffer);
        renderOrigin();

        if (canRenderText() && isTextForcedDisabled) {
            isTextForcedDisabled = false;
            isTextRendered = true;
        }
        if (!canRenderText() && isTextRendered && !isTextForcedDisabled) {
            isTextForcedDisabled = true;
            isTextRendered = false;
        }

        if (isTextRendered)
            renderCellsText();

        window.display();

        if (clock.getElapsedTime().asSeconds() >= 1.0) {
            currentFPS = framePassed;
            framePassed = 0;
            clock.restart();
        }
        framePassed += 1;
    }
}
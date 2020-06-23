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

    newGraphicBuffer();
}

GraphicEngine::~GraphicEngine()
{
}

void GraphicEngine::newGraphicBuffer()
{
    graphicCells.push_back(sf::VertexArray());
    graphicCells[graphicCells.size() - 1].setPrimitiveType(sf::Quads);
}

int GraphicEngine::totalGraphicBufferSize()
{
    /***
     * Counts all cell drawn by the graphic engine. 
    */

    int toRet = 0;
    for (const auto& buffer : graphicCells)
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

void GraphicEngine::appendOrUpdateQuadForCell(const sf::Vector2i& cellPos, const Cell& cell)
{
    /**
     * Vertices a,b,c,d will do a cell a  b
     *                                 d  c
    */
    assert(cell.getStatus() >= HALF_DEFINED);

    sf::Vertex a, b, c, d;
    a.position = mapWorldPosToCoords(cellPos);
    b.position = mapWorldPosToCoords(cellPos + EAST);
    c.position = mapWorldPosToCoords(cellPos + SOUTH + EAST);
    d.position = mapWorldPosToCoords(cellPos + SOUTH);

    sf::Color color = BACKGROUND_COLOR_HALF_DEFINED;
    if (cell.getStatus() == DEFINED)
        color = BACKGROUND_COLOR_DEFINED;

    a.color = color;
    b.color = color;
    c.color = color;
    d.color = color;

    if (vertexArrayCell.find(cellPos) == vertexArrayCell.end()) {
        vertexArrayCell[cellPos] = std::make_pair(graphicCells.size() - 1, currentBuffer().getVertexCount());
        currentBuffer().append(a);
        currentBuffer().append(b);
        currentBuffer().append(c);
        currentBuffer().append(d);
    } else {
        const auto& arrayAndPos = vertexArrayCell[cellPos];
        graphicCells[arrayAndPos.first][arrayAndPos.second] = a;
        graphicCells[arrayAndPos.first][arrayAndPos.second + 1] = b;
        graphicCells[arrayAndPos.first][arrayAndPos.second + 2] = c;
        graphicCells[arrayAndPos.first][arrayAndPos.second + 3] = d;
    }
}

sf::VertexArray& GraphicEngine::currentBuffer()
{
    assert(graphicCells.size() != 0);
    return graphicCells[graphicCells.size() - 1];
}

bool GraphicEngine::hasBufferLimitExceeded()
{
    return currentBuffer().getVertexCount() >= VERTEX_ARRAY_MAX_SIZE;
}

void GraphicEngine::updateGraphicCells()
{
    std::vector<sf::Vector2i> cellBuffer = world.getAndFlushGraphicBuffer();
    for (auto& cellPos : cellBuffer) {
        if (hasBufferLimitExceeded())
            newGraphicBuffer();
        appendOrUpdateQuadForCell(cellPos, world.cells[cellPos]);
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
                    printf("Vertex array: %ld x O(%d)\n", graphicCells.size(), VERTEX_ARRAY_MAX_SIZE);
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
        for (const auto& graphicBuffer : graphicCells)
            window.draw(graphicBuffer);
        renderOrigin();

        if( canRenderText() && isTextForcedDisabled) {
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
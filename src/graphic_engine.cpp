#include "graphic_engine.h"

GraphicEngine::GraphicEngine(World& world, int screen_w, int screen_h)
    : world(world)
{
    window.create(sf::VideoMode(screen_w, screen_h), simcqca_PROG_NAME);
    window.setFramerateLimit(TARGET_FPS);

    camera = window.getDefaultView();
    window.setView(camera);
    moveCameraMode = false;
    cameraMouseLeft = false;

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

sf::Vector2f GraphicEngine::mapWorldCoordsToCoords(const sf::Vector2i& world_coords)
{
    return sf::Vector2f({ static_cast<float>(world_coords.x * CELL_W),
        static_cast<float>(world_coords.y * CELL_H) });
}

sf::Vector2i GraphicEngine::mapCoordsToWorldCoords(const sf::Vector2f& coords)
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

void GraphicEngine::appendQuadForCell(const sf::Vector2i& cellPos, const Cell& cell)
{
    /**
     * Vertices a,b,c,d will do a cell a  b
     *                                 d  c
    */
    sf::Vertex a, b, c, d;
    a.position = mapWorldCoordsToCoords(cellPos);
    b.position = mapWorldCoordsToCoords(cellPos + EAST);
    c.position = mapWorldCoordsToCoords(cellPos + SOUTH + EAST);
    d.position = mapWorldCoordsToCoords(cellPos + SOUTH);

    a.color = BACKGROUND_COLOR_HALF_DEFINED;
    b.color = BACKGROUND_COLOR_HALF_DEFINED;
    c.color = BACKGROUND_COLOR_HALF_DEFINED;
    d.color = BACKGROUND_COLOR_HALF_DEFINED;

    currentBuffer().append(a);
    currentBuffer().append(b);
    currentBuffer().append(c);
    currentBuffer().append(d);
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
    std::vector<sf::Vector2i> cellBuffer = world.getAndFlushCellBuffer();
    for (auto& cellPos : cellBuffer) {
        if (hasBufferLimitExceeded())
            newGraphicBuffer();
        appendQuadForCell(cellPos, world.cells[cellPos]);
    }
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

        window.display();

        if (clock.getElapsedTime().asSeconds() >= 1.0) {
            currentFPS = framePassed;
            framePassed = 0;
            clock.restart();
        }
        framePassed += 1;
    }
}
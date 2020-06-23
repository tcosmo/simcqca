#pragma once

#include "config.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "global.h"
#include "world.h"

#define CELL_W 20
#define CELL_H 20

#define DEFAULT_CAM_TRANSLATION 20
#define DEFAULT_CAM_ZOOM_STEP 1.5

// Defining some colors!!
#define BACKGROUND_COLOR sf::Color(0.3 * 255, 0.3 * 255, 0.3 * 255)
#define GRID_COLOR sf::Color(0.5 * 255, 0.5 * 255, 0.5 * 255)
#define DEFAULT_CURSOR_COLOR sf::Color(34, 220, 34)
#define BACKGROUND_COLOR_HALF_DEFINED sf::Color(0.4 * 255, 0.4 * 255, 0.4 * 255)
#define BACKGROUND_COLOR_DEFINED sf::Color(0.6 * 255, 0.6 * 255, 0.6 * 255)
#define COLOR_SPECIAL_CARRY sf::Color(0.4 * 255, 0.1 * 255, 0.1 * 255)
#define COLOR_DARKER_GREEN sf::Color(0.2 * 255, 0.9 * 255, 0.2 * 255)

class GraphicEngine {
    /***
     * Class which renders the world and its evolution.
    */

public:
    GraphicEngine(World& world, int screen_w, int screen_h);
    ~GraphicEngine();

    void run();

private:
    World& world;

    sf::RenderWindow window;

    // General routines
    sf::Vector2f mapWorldPosToCoords(const sf::Vector2i& world_coords);
    sf::Vector2i mapCoordsToWorldPos(const sf::Vector2f& coords);

    // Text attribute and routines
    sf::Font defaultFont;
    bool isTextRendered;
    bool canRenderText(); // Render only if zoom factor not too small


    // Rendering routines
    void renderOrigin();
    void renderCellsText();
    void renderCellText(const sf::Vector2i& cellPos, const Cell& cell);

    // Camera attributes and routines
    sf::View camera;
    bool moveCameraMode, cameraMouseLeft;
    double currentZoom;
    sf::Vector2i cameraMousePosition;
    void cameraTranslate(float dx, float dy);
    void cameraTranslate(const sf::Vector2f& vec);
    void cameraZoom(float zoom_factor);
    void cameraCenter(const sf::Vector2f& where);
    bool isCellInView(const sf::Vector2i& cellPos); // Cell pos is expressed in world positions
    std::pair<sf::Vector2i,sf::Vector2i> getExtremalVisibleCellsPos(); // In world positions

    // Event routines and handlers
    bool isControlPressed();
    bool isShiftPressed();
    void handleCameraEvents(const sf::Event& event);

    // Graphic cells
    std::vector<sf::VertexArray> graphicCells;
    std::map<sf::Vector2i, bool> isCellDrawn; // Mapping world pos to bool
    void updateGraphicCells();
    void newGraphicBuffer();
    void appendQuadForCell(const sf::Vector2i& cellPos, const Cell& cell);
    int totalGraphicBufferSize();
    sf::VertexArray& currentBuffer();
    bool hasBufferLimitExceeded();
};
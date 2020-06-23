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

// Vertex Array layer indexing
#define NB_LAYERS 3
#define CELL_BACKGROUND 0 // Background of cell (defined/half defined)
#define CELL_COLOR 1 // Cell color when cell defined
#define CELL_TEXT 2 // Cell "text" for symbols (more efficient than rendering a font)
static sf::PrimitiveType LAYER_PRIMITIVE_TYPE[3] = { sf::Quads, sf::Quads, sf::Quads };
#define NB_TEXT_VERTICES 8 // Max number of vertices needed to draw  0, 1, \bar 0 or \bar 1

static sf::Color CELL_DEFINED_COLORS[4] = { COLOR_DARKER_GREEN, sf::Color::Black, sf::Color::Magenta, sf::Quads };

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
    bool isTextRendered, isTextForcedDisabled;
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
    std::pair<sf::Vector2i, sf::Vector2i> getExtremalVisibleCellsPos(); // In world positions

    // Event routines and handlers
    bool isControlPressed();
    bool isShiftPressed();
    void handleCameraEvents(const sf::Event& event);

    // Graphic cells
    std::vector<sf::VertexArray> graphicCells[NB_LAYERS];
    std::map<sf::Vector2i, std::pair<int, int>, compareWorldPositions> vertexArrayCell[NB_LAYERS]; // Mapping world pos to where are the cell's quad in `graphicCells`
    void updateGraphicCells();
    void newGraphicBuffer(int iLayer);
    void appendOrUpdateCell(const sf::Vector2i& cellPos, const Cell& cell);
    int totalGraphicBufferSize();
    sf::VertexArray& currentBuffer(int iLayer);
    bool hasBufferLimitExceeded(int iLayer);
    std::vector<sf::Vertex> getCellBackgroundVertices(const sf::Vector2i& cellPos, const Cell& cell);
    std::vector<sf::Vertex> getCellColorVertices(const sf::Vector2i& cellPos, const Cell& cell);
    std::vector<sf::Vertex> getCellTextVertices(const sf::Vector2i& cellPos, const Cell& cell);
};
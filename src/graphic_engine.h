#pragma once

#include "config.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "global.h"

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

public:
    GraphicEngine(int screen_w, int screen_h);
    ~GraphicEngine();

    void run();

private:
    sf::RenderWindow window;
    sf::View camera;

    // Rendering routines
    renderOrigin();
};
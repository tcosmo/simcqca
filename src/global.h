#pragma once

#include <assert.h>

#define pritnf printf // My dyslexia

#include <SFML/Graphics.hpp> // for sf::Vector2i

#define EAST sf::Vector2i({ 1, 0 })
#define WEST sf::Vector2i({ -1, 0 })
#define SOUTH sf::Vector2i({ 0, 1 })
#define NORTH sf::Vector2i({ 0, -1 })
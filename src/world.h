#pragma once

#include "config.h"

#include <map>
#include <string>

#include "arguments.h"
#include "global.h"

enum AtomicInfo {
    /***
     * Bits and carrys can be either 0, 1 or undef (\bot in latex).
    */
    UNDEF = -1,
    ZERO = 0,
    ONE = 1
};

struct Cell {
    /***
     * A cell contains a bit and a carry.
    */
    AtomicInfo bit, carry;
    Cell(AtomicInfo bit = UNDEF, AtomicInfo carry = UNDEF)
        : bit(bit)
        , carry(carry)
    {
    }
};

class World {
    /***
     * The world is a map of cells. A 2D Quasi CA rule dictates its evolution.
     * It can also be simulated in a sequential manner.
    */
public:
    World(bool isSequentialSim, InputType inputType, std::string inputStr)
        : isSequentialSim(isSequentialSim)
        , inputType(inputType)
        , inputStr(inputStr)
    {
        if (isSequentialSim) {
            printf("Sequential simulation not implemented yet. Abort.\n");
            exit(0);
        }

        setInputCells();
    }

    std::map<sf::Vector2i, Cell, compareWorldPositions> cells;
    std::vector<sf::Vector2i> getAndFlushCellBuffer();

private:
    bool isSequentialSim; // Run in sequential mode or CA-style mode?

    // Input
    InputType inputType;
    std::string inputStr;
    void setInputCells();

    // Line mode
    void setInputCellsLine();

    // For rendering
    std::vector<sf::Vector2i> cellBuffer; // Cells that are not drawn yet
};

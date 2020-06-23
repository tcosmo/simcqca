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

enum CellStatus {
    UNDEFINED = -1, // bot bit and carry undefined
    HALF_DEFINED = 1, // bit defined and carry undefined
    DEFINED = 2 // bit-carry defined
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
    CellStatus getStatus() const
    {
        if (bit == UNDEF && carry == UNDEF)
            return UNDEFINED;
        if (bit != UNDEF && carry == UNDEF)
            return HALF_DEFINED;
        if (bit != UNDEF && carry != UNDEF)
            return DEFINED;
        assert(false); // We should never meet the case where only the carry is defined
    }
    int sum() const
    {
        assert(getStatus() == DEFINED);
        return static_cast<int>(bit) + static_cast<int>(carry);
    }
    int index() const
    {
        /**
         * Indexing in order {(0,0),(0,1),(1,0),(1,1)}.
        */
        assert(getStatus() == DEFINED);
        return static_cast<int>(2 * bit) + static_cast<int>(carry);
    }
};

typedef std::pair<sf::Vector2i, Cell> CellPosAndCell;

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

    void next(); // Next simulation step

    std::map<sf::Vector2i, Cell, compareWorldPositions> cells; // Contains only not undefined cells
    Poset halfDefinedCells; // Buffer containing position of all half defined cells
    std::vector<sf::Vector2i> getAndFlushGraphicBuffer();

private:
    bool isSequentialSim; // Run in sequential mode or CA-style mode?

    // Simulation
    std::vector<CellPosAndCell> findNonLocalUpdates();
    void nextNonLocal();
    void nextLocal();
    std::vector<CellPosAndCell> findCarryPropUpdates();
    std::vector<CellPosAndCell> findForwardDeductionUpdates();
    void applyUpdates(const std::vector<CellPosAndCell>& updates);

    // Routines
    bool doesCellExists(const sf::Vector2i& cellPos);

    // Input
    InputType inputType;
    std::string inputStr;
    void setInputCells();

    // Line mode
    void setInputCellsLine();

    // For rendering
    std::vector<sf::Vector2i> cellGraphicBuffer; // Cells that are not drawn yet
};

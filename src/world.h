#pragma once

#include "config.h"

#include <map>
#include <string>

#include "arguments.h"
#include "global.h"

static const sf::Vector2i ORIGIN_BORDER_MODE = sf::Vector2i(0, 0);

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
    bool isComputationDone(); // For border mode

    std::map<sf::Vector2i, Cell, compareWorldPositions> cells; // Contains only not undefined cells
    Poset cellsOnEdge; // Buffer containing position of all cells on the edge of the computed world
    std::vector<sf::Vector2i> getAndFlushGraphicBuffer();
    InputType inputType;

private:
    bool isSequentialSim; // Run in sequential mode or CA-style mode?

    // Simulation
    std::vector<CellPosAndCell> findNonLocalUpdates();
    void nextNonLocal();
    void nextLocal();
    std::vector<CellPosAndCell> findCarryPropUpdates();
    std::vector<CellPosAndCell> findForwardDeductionUpdates();
    std::vector<CellPosAndCell> findBackwardDeductionUpdates();
    void applyUpdates(const std::vector<CellPosAndCell>& updates);
    // Because we simulate an infinite process with finite means we have some edge cases to deal with
    void manageEdgeCases(std::vector<CellPosAndCell>& toRet, const sf::Vector2i& cellPos, const Cell& updatedCell);

    bool isCellOnEdge(const sf::Vector2i& cellPos);
    void cleanCellsOnEdge();

    // Routines
    bool doesCellExists(const sf::Vector2i& cellPos);

    // Input
    std::string inputStr;
    void setInputCells();

    // Line mode
    void setInputCellsLine();

    // Col mode
    std::vector<int> base3To3p(std::string base3);
    void setInputCellsCol();

    // Border mode
    void setInputCellsBorder();

    // For rendering
    std::vector<sf::Vector2i> cellGraphicBuffer; // Cells that are not drawn yet
};

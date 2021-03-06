#include "../graphic_engine.h"
#include <sstream>

#define TIKZ_UNDEFINED_BG_NAME "clrBackgroundUndefined"
#define TIKZ_HALF_DEFINED_BG_NAME "clrBackgroundHalfDefined"
#define TIKZ_FULLY_DEFINED_BG_NAME "clrBackgroundDefined"
#define TIKZ_BOOT_CARRY_NAME "clrBootCarry"

// This is the opposite color scheme of the simulator
#define TIKZ_UNDEFINED_BG                                                      \
  "\\definecolor{" TIKZ_UNDEFINED_BG_NAME "}{RGB}{255,255,255}\n"
#define TIKZ_HALF_DEFINED_BG                                                   \
  "\\definecolor{" TIKZ_HALF_DEFINED_BG_NAME "}{RGB}{143,143,143}\n"
#define TIKZ_FULLY_DEFINED_BG                                                  \
  "\\definecolor{" TIKZ_FULLY_DEFINED_BG_NAME "}{RGB}{102,102,102}\n"
#define TIKZ_BOOT_CARRY                                                        \
  "\\definecolor{" TIKZ_BOOT_CARRY_NAME "}{RGB}{255,69,69}\n"

#define TIKZ_TEXT_COLOR "white"

sf::Vector2f toTikzCoordinates(const sf::Vector2i &worldPos) {
  /**
   * Tikz uses the mathematical convention of NORTH = increasing y.
   */
  return {static_cast<float>(worldPos.x), static_cast<float>(-1 * worldPos.y)};
}

std::string tikzPreamble() {
  /**
   * Fills the tikz file with routine latex/tikz preamble
   * and color definitions.
   */
  std::string preamble =
      "\\documentclass{standalone}\n"
      "\\usepackage{standalone}\n"
      "\\usepackage{amsmath,amsfonts,amssymb,amsthm}\n"
      "\\usepackage{bm}\n"
      "\\standaloneconfig{mode=image|tex}\n"
      "\\usepackage{tikz}\n" TIKZ_UNDEFINED_BG TIKZ_HALF_DEFINED_BG
          TIKZ_FULLY_DEFINED_BG TIKZ_BOOT_CARRY "%\\usetikzlibrary{...}\n"
      "\\begin{document}\n"
      "\\begin{tikzpicture}[scale=0.5]\n";
  return preamble;
}

std::string GraphicEngine::getTikzCell(const sf::Vector2i &cellPos, int maxX) {
  /**
   * Get the tikz expression for the cell at coordinates x,y;
   * MaxX is given so the function can figure out if a state (\bot,\bot)
   * is real or induced by finite precision.
   */
  assert(isTikzEnabled);
  std::ostringstream stringStream;
  stringStream << "\\filldraw[";
  sf::Vector2f tikzCoord = toTikzCoordinates(cellPos);

  const char *symbols[2] = {"\\boldsymbol{0}", "\\boldsymbol{1}"};

  std::string fillColor = TIKZ_UNDEFINED_BG_NAME;
  std::string strokeColor = TIKZ_UNDEFINED_BG_NAME;
  std::string text = "";
  bool drawSpecialStroke = false;
  // Sometimes the simulator says something is not defined
  // but in an ideal math world it is.
  if (world.inputType == COL || world.inputType == LINE) {
    if (!world.doesCellExists(cellPos)) {
      sf::Vector2i pos = cellPos;
      bool ideallyDefined = false;
      while (pos.x <= maxX) {
        if (world.doesCellExists(pos)) {
          if (world.cells[pos].getStatus() == HALF_DEFINED) {
            ideallyDefined = false;
            break;
          }
          ideallyDefined = true;
          break;
        }
        pos += EAST;
      }
      if (ideallyDefined) {
        fillColor = TIKZ_FULLY_DEFINED_BG_NAME;
        strokeColor = TIKZ_FULLY_DEFINED_BG_NAME;
      }
    }
  }

  if (world.doesCellExists(cellPos)) {
    if (world.cells[cellPos].getStatus() == DEFINED) {
      fillColor = TIKZ_FULLY_DEFINED_BG_NAME;
      strokeColor = TIKZ_FULLY_DEFINED_BG_NAME;
      text = symbols[world.cells[cellPos].index() / 2];

      // if (world.cells[cellPos].index() == 0) {
      //   sf::Vector2i pos = cellPos + WEST;
      //   bool onlyNothingness = true;
      //   while (world.doesCellExists(pos)) {
      //     assert(world.cells[pos].getStatus() == DEFINED);
      //     if (world.cells[pos].index() != 0)
      //       onlyNothingness = false;
      //     pos += WEST;
      //   }
      //   // if (onlyNothingness)
      //   //   text = "";
      // }

    } else {
      fillColor = TIKZ_HALF_DEFINED_BG_NAME;
      strokeColor = TIKZ_HALF_DEFINED_BG_NAME;
      int symbolIndex = static_cast<int>(world.cells[cellPos].bit);
      text = symbols[symbolIndex];
    }

    if (selectedCells.find(cellPos) != selectedCells.end()) {
      strokeColor = TIKZ_SELECTED_CELLS_WHEEL[selectedCells[cellPos]];
      drawSpecialStroke = true;
    }
  }
  // Color

  if (isColorRendered) {
    if (world.cells[cellPos].getStatus() == DEFINED) {
      int i = world.cells[cellPos].index();
      std::string colorTikzArray[] = {"green", "black", "violet", "blue"};
      strokeColor = colorTikzArray[i];
      fillColor = colorTikzArray[i];
    }
  }

  if (drawSpecialStroke || !isTikzGridEnabled)
    stringStream << "draw=" << strokeColor << ",fill=" << fillColor
                 << ",ultra thick] ";
  else
    stringStream << "fill=" << fillColor << "] ";

  // Position
  stringStream << "(" << tikzCoord.x << "," << tikzCoord.y << ") rectangle ";
  stringStream << "(" << tikzCoord.x + 1 << "," << tikzCoord.y - 1 << ");\n";

  // Carry
  float tweaky = -0.15;
  if (world.doesCellExists(cellPos) && world.cells[cellPos].carry == ONE) {
    std::string carryColor = TIKZ_TEXT_COLOR;
    if (world.cells[cellPos].isBootstrappingCarry)
      carryColor = TIKZ_BOOT_CARRY_NAME;
    stringStream << "\\draw [" << carryColor << ", ultra thick] "
                 << "(" << tikzCoord.x + 0.27;
    stringStream << "," << tikzCoord.y - 0.11 + tweaky << ") -- ";
    stringStream << "(" << tikzCoord.x + 1 - 0.27 << ","
                 << tikzCoord.y - 0.11 + tweaky << ");\n";
  }

  // Text
  if (text.size() != 0) {
    float textX = static_cast<float>(tikzCoord.x) + 0.5;
    float textY = static_cast<float>(tikzCoord.y) - 0.5 + tweaky;

    stringStream << "\\node[text=" << TIKZ_TEXT_COLOR << "] at (" << textX;
    stringStream << "," << textY << ") "
                 << "{$" << text << "$};\n";
  }

  return stringStream.str();
}

void GraphicEngine::generateTikzFromSelection() {
  /**
   * Generates the tikz representation of the screen in the rectangle
   * delimited by the two selected cells.
   */
  assert(isTikzEnabled && tikzSelection.size() == 2);

  static int nbRun = 0;

  std::string tikzFileString = tikzPreamble();

  sf::Vector2i posA = tikzSelection[0];
  sf::Vector2i posB = tikzSelection[1];

  int minX, maxX, minY, maxY;
  minX = MIN(posA.x, posB.x);
  maxX = MAX(posA.x, posB.x);
  minY = MIN(posA.y, posB.y);
  maxY = MAX(posA.y, posB.y);

  for (int x = minX; x <= maxX; x += 1)
    for (int y = minY; y <= maxY; y += 1) {
      sf::Vector2i cellPos = {x, y};
      if (selectedCells.find(cellPos) == selectedCells.end())
        tikzFileString += getTikzCell({x, y}, maxX);
    }

  // Imposing an order on selected cells
  for (int iColor = 0; iColor < COLORED_SELECTORS_WHEEL_SIZE; iColor += 1)
    for (const auto &posAndColor : selectedCells)
      if (posAndColor.second == iColor)
        tikzFileString += getTikzCell(posAndColor.first, maxX);

  for (const auto &posAndColor : selectedBorder) {
    sf::Vector2i currentPos = posAndColor.first - world.cyclicForwardVector;
    // FIXME: should not have access to world inputStr, world should give
    // a public access to the step to take.
    for (int i = 0; i < world.inputStr.length(); i++) {
      const char &c = world.inputStr[i];

      std::string colorTikzName = TIKZ_SELECTED_CELLS_WHEEL[posAndColor.second];
      auto tikzCoord = toTikzCoordinates(currentPos);
      std::ostringstream stringStream;

      stringStream << "\n\\draw[" << colorTikzName << ",ultra thick] ("
                   << tikzCoord.x << "," << tikzCoord.y - 1 << ") -- ("
                   << tikzCoord.x + 1 << "," << tikzCoord.y - 1 << ");";
      tikzFileString += stringStream.str();

      currentPos += WEST;
      if (c == '1') {
        currentPos += SOUTH;
        tikzCoord = toTikzCoordinates(currentPos);
        stringStream << "\n\\draw[" << colorTikzName << ",ultra thick] ("
                     << tikzCoord.x + 1 << "," << tikzCoord.y << ") -- ("
                     << tikzCoord.x + 1 << "," << tikzCoord.y - 1 << ");";
        tikzFileString += stringStream.str();
      }
    }
  }

  tikzFileString += "\n\\end{tikzpicture}\n"
                    "\\end{document}";

  std::string outputFileName = "output";
  outputFileName.push_back(nbRun + '0');
  outputFileName += ".tex";
  std::string outputFilePath =
      std::string(DEFAULT_TIKZ_OUTPUT_FOLDER) + outputFileName;

  saveFile(outputFilePath, tikzFileString);

  // nbRun += 1; // More annoying than useful
}
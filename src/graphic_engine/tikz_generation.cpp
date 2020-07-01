#include "../graphic_engine.h"
#include <sstream> 

#define TIKZ_UNDEFINED_BG_NAME "clrBackgroundUndefined"
#define TIKZ_HALF_DEFINED_BG_NAME "clrBackgroundHalfDefined"
#define TIKZ_FULLY_DEFINED_BG_NAME "clrBackgroundDefined"

#define TIKZ_UNDEFINED_BG                                                      \
  "\\definecolor{clrBackgroundUndefined}{RGB}{51,51,51}\n"
#define TIKZ_HALF_DEFINED_BG                                                   \
  "\\definecolor{clrBackgroundHalfDefined}{RGB}{102,102,102}\n"
#define TIKZ_FULLY_DEFINED_BG                                                  \
  "\\definecolor{clrBackgroundDefined}{RGB}{153,153,153}\n"

sf::Vector2i toTikzCoordinates(const sf::Vector2i &worldPos) {
  /**
   * Tikz uses the mathematical convention of NORTH = increasing y.
   */
  return {worldPos.x, -1 * worldPos.y};
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
          TIKZ_FULLY_DEFINED_BG "%\\usetikzlibrary{...}\n"
      "\\begin{document}\n"
      "\\begin{tikzpicture}[scale=0.5]\n";
  return preamble;
}

std::string GraphicEngine::getTikzCell(const sf::Vector2i& cellPos, int maxX) {
  /**
   * Get the tikz expression for the cell at coordinates x,y;
   * MaxX is given so the function can figure out if a state (\bot,\bot)
   * is real or induced by finite precision.
   */
  assert(isTikzEnabled);
  std::ostringstream stringStream;
  stringStream << "\\filldraw[";
  sf::Vector2i tikzCoord = toTikzCoordinates(cellPos);
  
  const char* symbols[4] = {"\\boldsymbol{0}", "\\boldsymbol{\\overline{0}}", 
                            "\\boldsymbol{1}", "\\boldsymbol{\\overline{1}}"};

  std::string fillColor = TIKZ_UNDEFINED_BG_NAME;
  std::string strokeColor = TIKZ_UNDEFINED_BG_NAME;
  std::string text = "";
  if (world.doesCellExists(cellPos)) {
    if (world.cells[cellPos].getStatus() == DEFINED){
      fillColor = TIKZ_FULLY_DEFINED_BG_NAME;
      strokeColor = TIKZ_FULLY_DEFINED_BG_NAME;
      text = symbols[world.cells[cellPos].index()];
    } else {
      fillColor = TIKZ_HALF_DEFINED_BG_NAME;
      strokeColor = TIKZ_HALF_DEFINED_BG_NAME;
      int symbolIndex = 2*static_cast<int>(world.cells[cellPos].bit);
      text = symbols[symbolIndex];
    }
  }

  // Color
  stringStream << "draw=" << strokeColor << ",fill=" << fillColor << ",thick] ";
  
  // Position
  stringStream << "(" << tikzCoord.x << "," << tikzCoord.y << ") rectangle ";
  stringStream << "(" << tikzCoord.x+1 << "," << tikzCoord.y-1 << ");\n";

  if (text.size() != 0) {
    float textX = static_cast<float>(tikzCoord.x) + 0.5;

    float tweak = 0;
    if (world.doesCellExists(cellPos) && world.cells[cellPos].carry == ZERO) {
      tweak = 0.07; // Needed to put bit at the same level when no carry
    }
    float textY = static_cast<float>(tikzCoord.y) - 0.5 - tweak;



    stringStream << "\\node[text=white] at (" << textX << "," << textY << ") ";
    stringStream << "{$" << text << "$};\n"; 
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
  minX = MIN(posA.x,posB.x);
  maxX = MAX(posA.x,posB.x);
  minY = MIN(posA.y,posB.y);
  maxY = MAX(posA.y,posB.y);

  for (int x = minX; x <= maxX; x += 1)
    for (int y = minY; y <= maxY; y += 1) // inverted y axis
      tikzFileString += getTikzCell({x, y}, maxX);

  tikzFileString += "\n\\end{tikzpicture}\n"
                    "\\end{document}";

  std::string outputFileName = "output";
  outputFileName.push_back(nbRun + '0');
  outputFileName += ".tex";
  std::string outputFilePath =
      std::string(DEFAULT_TIKZ_OUTPUT_FOLDER) + outputFileName;

  saveFile(outputFilePath, tikzFileString);
}
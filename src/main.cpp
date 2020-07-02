#include "config.h"

#include "arguments.h"
#include "graphic_engine.h"
#include "world.h"

#include <cstdio>

int main(int argc, char *argv[]) {
  Arguments arguments;
  parseArguments(argc, argv, arguments);

  World world(arguments.isSequential, arguments.inputType, arguments.inputStr,
              arguments.constructCycleInLine, arguments.cycleBoth);
  GraphicEngine graphicEngine(world, 800 * 1.5, 600 * 1.5,
                              arguments.isTikzEnabled);
  graphicEngine.run();
}
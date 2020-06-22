#include "config.h"

#include "arguments.h"
#include "graphic_engine.h"
#include <cstdio>

int main(int argc, char* argv[])
{
    Arguments arguments;
    parseArguments(argc, argv, &arguments);

    GraphicEngine graphicEngine(800 * 1.5, 600 * 1.5);
    graphicEngine.run();
}
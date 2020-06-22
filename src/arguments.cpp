#include "arguments.h"

const char doc[] = "Welcome to the simulator for the 2D Colatz Quasi Cellular Automaton";
const char* argp_program_bug_address = "tristan.sterin@mu.ie";
const char* argp_program_version = VERSION_LITERAL;

struct argp argp = { options, parseOption, 0, doc };

void setInputType(char* arg, Arguments* arguments, InputType inputType)
{
    if (arg == NULL)
        return;
    if (arguments->inputType != UNDEF) {
        printf("Only one input mode (line/col/border/cycle) should be chosen. Abort.");
        exit(0);
    }
    arguments->inputType = inputType;
    arguments->inputStr = std::string(arg);
}

static error_t parseOption(int key, char* arg, struct argp_state* state)
{
    Arguments* arguments = (Arguments*)state->input;
    switch (key) {
    case 's':
        arguments->isSequential = true;
        break;
    case 'l':
        if (arg == NULL)
            break;
        setInputType(arg, arguments, LINE);
        break;
    case 'c':
        if (arg == NULL)
            break;
        setInputType(arg, arguments, COL);
        break;
    case 'b':
        if (arg == NULL)
            break;
        setInputType(arg, arguments, BORDER);
        break;
    case 'y':
        if (arg == NULL)
            break;
        setInputType(arg, arguments, CYCLE);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

void parseArguments(int argc, char* argv[], Arguments* arguments)
{
    argp_parse(&argp, argc, argv, 0, 0, arguments);
}

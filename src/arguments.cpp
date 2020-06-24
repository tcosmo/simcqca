#include "arguments.h"

const char doc[] = "Welcome to the simulator for the 2D Colatz Quasi Cellular Automaton";
const char* argp_program_bug_address = "tristan.sterin@mu.ie";
const char* argp_program_version = VERSION_LITERAL;

void setInputType(const std::string& arg, Arguments& arguments, InputType inputType)
{
    static const char* modeName[5] = { "None", "line", "col", "border", "cycle" };
    if (arg.size() == 0) {
        printf("Input for mode `%s` should not be empty. Abort.", modeName[inputType]);
        return;
    }
    if (arguments.inputType != NONE) {
        printf("Only one input mode (line/col/border/cycle) should be chosen. Abort.");
        exit(0);
    }
    arguments.inputType = inputType;
    arguments.inputStr = arg;
}

const std::string& orStr(const std::string& one, const std::string& two)
{
    if (one.empty())
        return two;
    return one;
}

std::string getShortOptionStr(char c)
{
    std::string s = "-";
    s.push_back(c);
    return s;
}

std::string getLongOptionStr(const char* l)
{
    std::string s = "--";
    s.append(l);
    return s;
}

void parseArguments(int argc, char* argv[], Arguments& arguments)
{
    InputParser input(argc, argv);
    if (input.cmdOptionExists("-s") || input.cmdOptionExists("--sequential")) {
        arguments.isSequential = true;
    }

    for (int iOption = 1; iOption < 5; iOption += 1) {
        std::string shortStr = getShortOptionStr(options[iOption].shortOption);
        std::string longStr = getLongOptionStr(options[iOption].longOption);
        if (input.cmdOptionExists(shortStr) || input.cmdOptionExists(longStr)) {
            setInputType(orStr(input.getCmdOption(shortStr), input.getCmdOption(longStr)), arguments, static_cast<InputType>(iOption));
        }
    }
}

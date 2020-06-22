#pragma once

#include "config.h"
#include <argp.h>
#include <string>

enum InputType {
    UNDEF = 0,
    LINE,
    COL,
    BORDER,
    CYCLE
};

static struct argp_option options[] = {
    { "sequential", 's', 0, 0, "Runs sequential simulation instead of CA-style simulation." },
    { "line", 'l', "INPUT BASE 2", 0, "Inputs a binary line to the process." },
    { "col", 'c', "INPUT BASE 3", 0, "Inputs a ternary column to the process.\n Base 3 -> base3' conversion is done internally." },
    { "border", 'b', "INPUT PARITY VECTOR", 0, "Inputs a parity vector to the process." },
    { "cycle", 'y', "INPUT PARITY VECTOR", 0, "Inputs a parity vector to the process with cyclic edges conditions." },
    { 0 }
};

struct Arguments {
    bool isSequential;
    InputType inputType;
    std::string inputStr;

    Arguments()
        : isSequential(false)
        , inputType(UNDEF)
    {
    }
};

void parseArguments(int argc, char* argv[], Arguments* arguments);
static error_t parseOption(int key, char* arg, struct argp_state* state);
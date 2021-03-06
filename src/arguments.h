#pragma once

#include "config.h"
#include <algorithm>
#include <string>
#include <vector>

enum InputType { NONE = 0, LINE, COL, BORDER, CYCLE };

// https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
class InputParser {
public:
  InputParser(int &argc, char **argv) {
    for (int i = 1; i < argc; ++i)
      this->tokens.push_back(std::string(argv[i]));
  }
  /// @author iain
  const std::string &getCmdOption(const std::string &option) const {
    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
      return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
  }
  /// @author iain
  bool cmdOptionExists(const std::string &option) const {
    return std::find(this->tokens.begin(), this->tokens.end(), option) !=
           this->tokens.end();
  }

private:
  std::vector<std::string> tokens;
};

struct InputOption {
  const char *longOption;
  char shortOption;
  const char *argumentHelper;
  const char *helpString;
};

static std::vector<InputOption> options = {
    {"seq", 's', NULL,
     "Not implemented yet: runs sequential simulation instead of CA-style "
     "simulation"},
    {"row", 'r', "INPUT BASE 2", "Inputs a binary row to the process"},
    {"col", 'c', "INPUT BASE 3",
     "Inputs a ternary column to the process. Base 3 -> base3' conversion is "
     "done internally"},
    {"border", 'b', "INPUT PARITY VECTOR",
     "Inputs a parity vector to the process"},
    {"cycle", 'y', "INPUT PARITY VECTOR",
     "Inputs a parity vector to the process with cyclic edges conditions"},
    {"cycle-row", 'j', NULL,
     "Combine this option with cycle mode to run the construction per row and "
     "not per column"},

    {"tikz", 't', NULL,
     "Use this option to generate `tikz` figures from the"
     " simulator."},
    {"cycle-both", 'j', NULL,
     "Combine this option with cycle mode to run the construction per row and "
     "per column at the same time."},

    // Default options
    {"help", 'h', NULL, "Give this help list"},
    {"usage", 'u', NULL, "Give a short usage message"},
    {"version", 'V', NULL, "Print program version"}};

struct Arguments {
  bool isSequential;
  InputType inputType;
  std::string inputStr;
  bool constructCycleInLine;
  bool isTikzEnabled;
  bool cycleBoth;

  Arguments()
      : isSequential(false), inputType(NONE), constructCycleInLine(false),
        isTikzEnabled(false), cycleBoth(false) {}
};

void parseArguments(int argc, char *argv[], Arguments &arguments);
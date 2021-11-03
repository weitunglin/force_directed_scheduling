#include "Blif.h"
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> parseProgramArgument(int argc, char* argv[]);
bool isNumber(const std::string& str);

int main(int argc, char* argv[]) {
  std::vector<std::string> args = parseProgramArgument(argc, argv);

  // check if program argument is valid
  if (argc != 3 || !isNumber(args[2])) {
    std::cerr
      << "Synopsis:\n"
      << "./lcs BLIF_FILE LATENCY_CONSTRAINT\n"
      << "Example:\n"
      << "./lcs sample01.blif 5\n";
    return 1;
  }

  // run Force-Directed Latency Constrained Scheduling
  Blif* blif = new Blif(args[1], std::stoi(args[2]));
  blif->FD_LCS();
  std::cout << *blif;
  delete blif;

  return 0;
}

// convert program argument from char* to std::string
std::vector<std::string> parseProgramArgument(int argc, char* argv[]) {
  std::vector<std::string> args;
  for (int i = 0; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  return args;
}

// return true is string is a number string
bool isNumber(const std::string& str) {
  for (char const &c : str) {
    if (std::isdigit(c) == 0) return false;
  }

  return true;
};
#include <iostream>
#include <string>
#include <vector>

#include "parser.hh"
#include "sa.hh"

int main(int argc, char *argv[]) {
  std::vector<std::string> filenames;
  for (int i = 1; i < argc; i++) {
    filenames.emplace_back(argv[i]);
  }

  auto parser = cool::Parser(filenames);
  parser.parse();
  parser.program->print(std::cout);

  auto sa = cool::SemanticAnalyser(parser.program);
  sa.analyse();

  return 0;
}

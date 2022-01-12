#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "cg.hh"
#include "parser.hh"
#include "sa.hh"

int build(const std::string &exe_filename, const std::string &asm_filename) {
  std::cout.flush();
  std::cerr.flush();

  auto cmd = "gcc -ggdb -no-pie -o \"" + exe_filename + "\" \"" + asm_filename + "\"";
  int ret = ::system(cmd.c_str());
  if (ret == -1 || ret == 127) {
    ::perror("build");
  }
  return ret ? -1 : 0;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " EXE_FILE SRC_FILE..."
              << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string exe_filename(argv[1]);
  std::string asm_filename = exe_filename + ".s";
  std::ofstream o(asm_filename);

  std::vector<std::string> src_filenames;
  for (int i = 2; i < argc; i++) {
    src_filenames.emplace_back(argv[i]);
  }

  auto parser = cool::Parser(src_filenames);
  parser.parse();
  // parser.program->print(std::cout);

  auto sa = cool::SemanticAnalyser(parser.program.get());
  sa.analyse();
  // sa.objectClass->print_hierarchy(std::cout);

  auto cg = cool::CodeGenerator(parser.program.get(), &sa);
  cg.generate(o);

  o.close(); // !!!

  return build(exe_filename, asm_filename);
}

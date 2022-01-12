#ifndef _CG_HH
#define _CG_HH

#include <iostream>
#include <map>

#include "ast.hh"
#include "sa.hh"

namespace cool {

const std::string init_method_name = "__init__";

class CodeGenerator {
public:
  CodeGenerator(ast::Program *program, cool::SemanticAnalyser *sa)
      : program(program), sa(sa), label_no(1) {}

  std::string next_label();

  void generate(std::ostream &o);

  int get_string_constant_no(std::string s);
  int get_int_constant_no(int i);

  ast::Class *selfClass;
  Scope<std::string> scope;
  int offset_rbp;

  ast::Program *program;
  cool::SemanticAnalyser *sa;

private:
  int label_no;

  void arrange_classes();

  void generate_prototypes(std::ostream &o);

  void generate_methods(std::ostream &o);

  void generate_object_methods(std::ostream &o);
  void generate_string_methods(std::ostream &o);
  void generate_int_methods(std::ostream &o);
  void generate_bool_methods(std::ostream &o);
  void generate_io_methods(std::ostream &o);
  void generate_system_methods(std::ostream &o);
  void generate_builtin_methods(std::ostream &o);

  void generate_constants(std::ostream &o);

  std::map<std::string, int> string_constants_numbered;
  std::list<std::string> string_constants;

  std::map<int, int> int_constants_numbered;
  std::list<int> int_constants;
};

} // namespace cool

#endif /* _CG_HH */

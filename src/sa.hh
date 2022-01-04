#ifndef _SA_HH
#define _SA_HH

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "ast.hh"

namespace cool {

class Scope {
public:
  Scope &enter();
  Scope &exit();
  Scope &add(std::string k, ast::Class *v);
  ast::Class *find(std::string k);

private:
  std::list<std::shared_ptr<std::map<std::string, ast::Class *>>> chain;
};

class SemanticError : public std::runtime_error {
public:
  SemanticError(const std::string &what_arg) : std::runtime_error(what_arg) {}
};

class SemanticAnalyser {
public:
  SemanticAnalyser(std::shared_ptr<ast::Program> program);

  void analyse();

  void error(const yy::location &loc, const std::string &msg);

  ast::Class *common_ancestor(ast::Class *a, ast::Class *b);
  ast::Class *common_ancestor(std::list<ast::Class *> classes);

  bool assignable(ast::Class *left, ast::Class *right);

  Scope scope;

  std::map<std::string, std::shared_ptr<ast::Class>> name2Class;

  ast::Class *selfClass;

  std::shared_ptr<ast::Class> mainClass;

  // built-in classes
  std::vector<std::shared_ptr<ast::Class>> builtin;
  std::shared_ptr<ast::Class> nullClass, errorClass;
  std::shared_ptr<ast::Class> objectClass, stringClass, intClass, boolClass,
      ioClass;

private:
  int nerrs;
  void check_errors();

  void build_and_check_class_hierarchy();
  void check_type();

  std::shared_ptr<ast::Program> program;
};

} // namespace cool

#endif /* _SA_HH */

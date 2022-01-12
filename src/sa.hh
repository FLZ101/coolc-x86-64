#ifndef _SA_HH
#define _SA_HH

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "ast.hh"

namespace cool {

template <typename T> class Scope {
public:
  Scope &enter();
  Scope &exit();
  Scope &add(std::string k, T v);
  T find(std::string k);

private:
  std::list<std::shared_ptr<std::map<std::string, T>>> chain;
};

template <typename T> Scope<T> &Scope<T>::enter() {
  chain.push_back(std::make_shared<std::map<std::string, T>>());
  return *this;
}

template <typename T> Scope<T> &Scope<T>::exit() {
  chain.pop_back();
  return *this;
}

template <typename T> Scope<T> &Scope<T>::add(std::string k, T v) {
  chain.back()->operator[](k) = v;
  return *this;
}

template <typename T> T Scope<T>::find(std::string k) {
  for (auto it = chain.crbegin(); it != chain.crend(); ++it) {
    auto m = it->get();
    if (m->find(k) != m->end()) {
      return m->operator[](k);
    }
  }
  return nullptr;
}

class SemanticError : public std::runtime_error {
public:
  SemanticError(const std::string &what_arg) : std::runtime_error(what_arg) {}
};

class SemanticAnalyser {
public:
  SemanticAnalyser(ast::Program *program);

  void analyse();

  void error(const yy::location &loc, const std::string &msg);

  ast::Class *common_ancestor(ast::Class *a, ast::Class *b);
  ast::Class *common_ancestor(std::list<ast::Class *> classes);

  bool assignable(ast::Class *left, ast::Class *right);

  Scope<ast::Class *> scope;

  std::map<std::string, std::shared_ptr<ast::Class>> name2Class;

  ast::Class *selfClass;

  std::shared_ptr<ast::Class> mainClass;

  // built-in classes
  std::vector<std::shared_ptr<ast::Class>> builtin;
  std::shared_ptr<ast::Class> errorClass;
  std::shared_ptr<ast::Class> objectClass, stringClass, intClass, boolClass,
      ioClass;

  std::vector<std::shared_ptr<ast::Class>> classes;

private:
  int nerrs;
  void check_errors();

  void build_and_check_class_hierarchy();
  void check_type();

  ast::Program *program;
};

} // namespace cool

#endif /* _SA_HH */

#include "sa.hh"

#include <iostream>

namespace cool {

SemanticAnalyser::SemanticAnalyser(ast::Program *program)
    : program(program), nerrs(0) {

  errorClass = std::make_shared<ast::Class>(
      "error", "", std::list<std::shared_ptr<ast::Feature>>{});

  objectClass = std::make_shared<ast::Class>(
      "Object", "",
      std::list<std::shared_ptr<ast::Feature>>{
          std::make_shared<ast::Method>(
              "copy", std::list<std::shared_ptr<ast::Formal>>{}, "SELF_TYPE",
              std::shared_ptr<ast::Void>()),
          std::make_shared<ast::Method>(
              "abort", std::list<std::shared_ptr<ast::Formal>>{}, "Object",
              std::shared_ptr<ast::Void>()),
          std::make_shared<ast::Method>(
              "type_name", std::list<std::shared_ptr<ast::Formal>>{}, "String",
              std::shared_ptr<ast::Void>())});

  stringClass = std::make_shared<ast::Class>(
      "String", "",
      std::list<std::shared_ptr<ast::Feature>>{
          std::make_shared<ast::Method>(
              "length", std::list<std::shared_ptr<ast::Formal>>{}, "Int",
              std::shared_ptr<ast::Void>()),
          std::make_shared<ast::Method>(
              "concat",
              std::list<std::shared_ptr<ast::Formal>>{
                  std::make_shared<ast::Formal>("other", "String")},
              "String", std::shared_ptr<ast::Void>()),
          std::make_shared<ast::Method>(
              "substr",
              std::list<std::shared_ptr<ast::Formal>>{
                  std::make_shared<ast::Formal>("begin", "Int"),
                  std::make_shared<ast::Formal>("end", "Int")},
              "String", std::shared_ptr<ast::Void>()),
          std::make_shared<ast::Method>(
              "to_int", std::list<std::shared_ptr<ast::Formal>>{}, "Int",
              std::shared_ptr<ast::Void>())});

  intClass = std::make_shared<ast::Class>(
      "Int", "",
      std::list<std::shared_ptr<ast::Feature>>{std::make_shared<ast::Method>(
          "to_string", std::list<std::shared_ptr<ast::Formal>>{}, "String",
          std::shared_ptr<ast::Void>())});

  boolClass = std::make_shared<ast::Class>(
      "Bool", "", std::list<std::shared_ptr<ast::Feature>>{});

  ioClass = std::make_shared<ast::Class>(
      "IO", "",
      std::list<std::shared_ptr<ast::Feature>>{
          std::make_shared<ast::Method>(
              "in_string", std::list<std::shared_ptr<ast::Formal>>{}, "String",
              std::shared_ptr<ast::Void>()),
          std::make_shared<ast::Method>(
              "out_string",
              std::list<std::shared_ptr<ast::Formal>>{
                  std::make_shared<ast::Formal>("x", "String")},
              "SELF_TYPE", std::shared_ptr<ast::Void>())});

  builtin = std::vector<std::shared_ptr<ast::Class>>{
      objectClass, stringClass, intClass, boolClass, ioClass};
}

void SemanticAnalyser::error(const yy::location &loc, const std::string &msg) {
  nerrs += 1;
  if (loc.begin.filename) {
    std::cerr << loc << ": ";
  }
  std::cerr << msg << std::endl;
}

void SemanticAnalyser::check_errors() {
  if (nerrs > 0) {
    throw SemanticError(std::to_string(nerrs) + " error(s)");
  }
}

void SemanticAnalyser::build_and_check_class_hierarchy() {
  for (auto &cls : builtin) {
    name2Class[cls->name] = cls;
  }

  for (auto &cls : program->classes) {
    if (name2Class.find(cls->name) != name2Class.end()) {
      error(cls->get_loc(), "redefined class \"" + cls->name + "\"");
      continue;
    }
    if (cls->name == "SELF_TYPE") {
      error(cls->get_loc(), "invalid class name \"" + cls->name + "\"");
      continue;
    }
    name2Class[cls->name] = cls;
  }
  check_errors();

  auto add_class = [this](ast::Class *cls) {
    auto parent_name = cls->parent_name;
    if (parent_name.empty()) {
      parent_name = "Object";
    }

    auto m = this->name2Class;
    if (m.find(parent_name) == m.end()) {
      error(cls->get_loc(), "undefined class \"" + cls->name + "\"");
      return;
    }
    auto parent = m.find(parent_name)->second;
    if (parent == this->stringClass || parent == this->intClass ||
        parent == this->boolClass) {
      error(cls->get_loc(), "invalid parent class \"" + parent_name + "\"");
      return;
    }
    parent->append_child(cls);
  };

  for (auto &cls : std::vector<std::shared_ptr<ast::Class>>(builtin.begin() + 1,
                                                            builtin.end())) {
    add_class(cls.get());
  }
  for (auto &cls : program->classes) {
    add_class(cls.get());
  }
  check_errors();

  classes = builtin;
  classes.insert(classes.end(), program->classes.begin(),
                 program->classes.end());

  for (auto &cls : classes) {
    for (auto &feature : cls->features) {
      auto method = std::dynamic_pointer_cast<ast::Method>(feature);
      auto field = std::dynamic_pointer_cast<ast::Field>(feature);

      if (method) {
        if (cls->name2Method.find(method->name) != cls->name2Method.end()) {
          error(method->get_loc(),
                "redefined method \"" + cls->name + "." + method->name + "\"");
          continue;
        }
        cls->name2Method[method->name] = method.get();

        for (auto &formal : method->formals) {
          if (name2Class.find(formal->type_name) == name2Class.end()) {
            error(formal->get_loc(),
                  "unknown type \"" + formal->type_name + "\"");
          }
        }

        if (method->ret_type_name != "SELF_TYPE" &&
            name2Class.find(method->ret_type_name) == name2Class.end()) {
          error(method->get_loc(),
                "unknown type \"" + method->ret_type_name + "\"");
        }

      } else if (field) {
        if (cls->name2Field.find(field->name) != cls->name2Field.end()) {
          error(field->get_loc(),
                "redefined field \"" + cls->name + "." + field->name + "\"");
          continue;
        }
        cls->name2Field[field->name] = field.get();

        if (name2Class.find(field->type_name) == name2Class.end()) {
          error(method->get_loc(), "unknown type \"" + field->type_name + "\"");
        } else {
          field->type = name2Class[field->type_name].get();
        }
      }
    }
  }
  check_errors();

  for (auto &cls : program->classes) {
    auto parent = cls->parent;
    if (!parent) {
      continue;
    }
    for (auto it = cls->name2Method.begin(); it != cls->name2Method.end();
         ++it) {
      auto name = it->first;
      auto method = it->second;
      if (parent->get_method(name) &&
          !parent->get_method(name)->same_signature(method)) {
        error(method->get_loc(), "invalid overriding");
      }
    }
  }
  check_errors();

  if (name2Class.find("Main") == name2Class.end()) {
    throw SemanticError("class \"Main\" is not defined");
  }
  mainClass = name2Class["Main"];

  if (mainClass->name2Method.find("main") == mainClass->name2Method.end()) {
    throw SemanticError("method \"Main.main\" is not defined");
  }

  auto main_method = mainClass->name2Method["main"];
  if (main_method->formals.size() > 0 || main_method->ret_type_name != "Int") {
    throw SemanticError("method \"Main.main\" is invalid");
  }
}

bool SemanticAnalyser::assignable(ast::Class *left, ast::Class *right) {
  auto p = right;
  while (p) {
    if (p == left) {
      return true;
    }
    p = p->parent;
  }
  return false;
}

ast::Class *SemanticAnalyser::common_ancestor(ast::Class *a, ast::Class *b) {
  auto p1 = a, p2 = b;
  while (p1 != p2) {
    p1 = (p1 == objectClass.get()) ? b : (p1->parent);
    p2 = (p2 == objectClass.get()) ? a : (p2->parent);
  }
  return p1;
}

ast::Class *SemanticAnalyser::common_ancestor(std::list<ast::Class *> classes) {
  auto res = classes.back();
  for (auto cls : classes) {
    res = common_ancestor(res, cls);
  }
  return res;
}

void SemanticAnalyser::check_type() {
  for (auto &cls : program->classes) {
    selfClass = cls.get();
    cls->check_type(this);
  }
  check_errors();
}

void SemanticAnalyser::analyse() {
  build_and_check_class_hierarchy();
  check_type();
}

} // namespace cool

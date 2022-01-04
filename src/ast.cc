#include "ast.hh"

#include <sstream>

#include "sa.hh"

namespace ast {

void Assign::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " <-"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

Class *Assign::type(cool::SemanticAnalyser *sa) {
  auto right = expr->type(sa);
  if (right == sa->errorClass.get()) {
    return right;
  }

  auto left = sa->scope.find(name);
  if (!left) {
    sa->error(get_loc(), "undefined variable \"" + name + "\"");
    return sa->errorClass.get();
  }

  if (!sa->assignable(left, right)) {
    sa->error(get_loc(), "type error");
    return sa->errorClass.get();
  }
  return right;
}

void Invoke::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "INVOKE"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
  if (!type_name.empty()) {
    o << std::string(indent + 2, ' ') << type_name << std::endl;
  }
  o << std::string(indent + 2, ' ') << name << std::endl;
  for (auto &arg : arguments) {
    arg->print(o, indent + 2);
  }
}

Class *Invoke::type(cool::SemanticAnalyser *sa) {
  expr2 = expr;
  if (std::dynamic_pointer_cast<Void>(expr2)) {
    expr2 = std::make_shared<Var>("self");
  }

  auto expr2_type = expr2->type(sa);
  if (expr2_type == sa->errorClass.get()) {
    return expr2_type;
  }

  type2 = sa->selfClass;
  if (!type_name.empty()) {
    if (sa->name2Class.find(type_name) == sa->name2Class.end()) {
      sa->error(get_loc(), "undefined class \"" + type_name + "\"");
      return sa->errorClass.get();
    }
    type2 = sa->name2Class[type_name].get();
  }

  if (!sa->assignable(type2, expr2_type)) {
    sa->error(get_loc(), "type error");
    return sa->errorClass.get();
  }

  method = type2->get_method(name);
  if (!method) {
    sa->error(get_loc(), "undefined method \"" + name + "\"");
    return sa->errorClass.get();
  }

  bool err = false;
  auto it1 = method->formals.begin();
  auto it2 = arguments.begin();
  for (; it1 != method->formals.end() && it2 != arguments.end(); ++it1, ++it2) {
    auto left = sa->name2Class[it1->get()->type_name].get();
    auto right = it2->get()->type(sa);
    if (right == sa->errorClass.get()) {
      err = true;
      continue;
    }
    if (!sa->assignable(left, right)) {
      sa->error(it2->get()->get_loc(), "wrong type argument");
      err = true;
    }
  }
  if (it1 != method->formals.end() || it2 != arguments.end()) {
    sa->error(get_loc(), "wrong number of arguments");
    err = true;
  }

  if (err) {
    return sa->errorClass.get();
  }

  if (method->ret_type_name == "SELF_TYPE") {
    return sa->selfClass;
  }
  return sa->name2Class[method->ret_type_name].get();
}

void If::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "IF"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
  c->print(o, indent + 2);
}

Class *If::type(cool::SemanticAnalyser *sa) {
  auto a_type = a->type(sa);
  if (a_type == sa->errorClass.get()) {
    return a_type;
  }

  if (a_type != sa->boolClass.get()) {
    sa->error(get_loc(), "type error");
    return sa->errorClass.get();
  }

  auto b_type = b->type(sa);
  if (b_type == sa->errorClass.get()) {
    return b_type;
  }

  auto c_type = c->type(sa);
  if (c_type == sa->errorClass.get()) {
    return c_type;
  }

  return sa->common_ancestor(b_type, c_type);
}

void While::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "WHILE"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *While::type(cool::SemanticAnalyser *sa) {
  auto a_type = a->type(sa);
  if (a_type == sa->errorClass.get()) {
    return a_type;
  }

  if (a_type != sa->boolClass.get()) {
    sa->error(get_loc(), "type error");
    return sa->errorClass.get();
  }

  auto b_type = b->type(sa);
  if (b_type == sa->errorClass.get()) {
    return b_type;
  }

  return b_type;
}

void Block::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "BLOCK"
    << " @" << loc << std::endl;
  for (auto &expr : expressions) {
    expr->print(o, indent + 2);
  }
}

Class *Block::type(cool::SemanticAnalyser *sa) {
  bool err = false;
  for (auto &expr : expressions) {
    auto t = expr->type(sa);
    if (t == sa->errorClass.get()) {
      err = true;
      continue;
    }
  }
  if (err) {
    return sa->errorClass.get();
  }
  return expressions.back()->type(sa);
}

void Let::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "let " << name << " : " << type_name << " @"
    << loc << std::endl;
  expr->print(o, indent + 2);
  body->print(o, indent + 2);
}

Class *Let::type(cool::SemanticAnalyser *sa) {
  if (sa->name2Class.find(type_name) == sa->name2Class.end()) {
    sa->error(get_loc(), "unknown type \"" + type_name + "\"");
    return sa->errorClass.get();
  }
  auto left = sa->name2Class[type_name].get();
  if (!std::dynamic_pointer_cast<Void>(expr)) {
    auto right = expr->type(sa);
    if (right == sa->errorClass.get()) {
      return right;
    }
    if (!sa->assignable(left, right)) {
      sa->error(get_loc(), "type error");
      return sa->errorClass.get();
    }
  }

  sa->scope.enter().add(name, left);
  auto res = body->type(sa);
  sa->scope.exit();

  return res;
}

void CaseBranch::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " : " << type_name << " @" << loc
    << std::endl;
  expr->print(o, indent + 2);
}

void Case::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "CASE"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
  for (auto &branch : branches) {
    branch->print(o, indent + 2);
  }
}

Class *Case::type(cool::SemanticAnalyser *sa) {
  auto expr_type = expr->type(sa);
  if (expr_type == sa->errorClass.get()) {
    return expr_type;
  }

  bool err = false;
  std::list<Class *> branch_expr_types{};
  for (auto &branch : branches) {
    if (sa->name2Class.find(branch->type_name) == sa->name2Class.end()) {
      sa->error(branch->get_loc(),
                "unknown type \"" + branch->type_name + "\"");
      err = true;
      continue;
    }
    auto branch_type = sa->name2Class[branch->type_name].get();
    if (!sa->assignable(expr_type, branch_type) &&
        !sa->assignable(branch_type, expr_type)) {
      sa->error(branch->get_loc(), "type error");
      err = true;
      continue;
    }

    sa->scope.enter().add(branch->name, branch_type);
    auto branch_expr_type = branch->expr->type(sa);
    if (branch_expr_type == sa->errorClass.get()) {
      err = true;
    }
    sa->scope.exit();
  }

  if (err) {
    return sa->errorClass.get();
  }

  return sa->common_ancestor(branch_expr_types);
}

void New::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "new " << type_name << " @" << loc
    << std::endl;
}

Class *New::type(cool::SemanticAnalyser *sa) {
  if (sa->name2Class.find(type_name) == sa->name2Class.end()) {
    sa->error(get_loc(), "unknown type \"" + type_name + "\"");
    return sa->errorClass.get();
  }
  return sa->name2Class[type_name].get();
}

void IsVoid::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "isvoid"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

Class *IsVoid::type(cool::SemanticAnalyser *sa) {
  if (expr->type(sa) == sa->errorClass.get()) {
    return sa->errorClass.get();
  }
  return sa->boolClass.get();
}

void Add::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "+"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *int_op_type(cool::SemanticAnalyser *sa,
                   std::list<std::shared_ptr<Expression>> expressions,
                   Class *res) {
  bool err = false;
  for (auto &expr : expressions) {
    auto type = expr->type(sa);
    if (type == sa->errorClass.get()) {
      err = true;
      continue;
    }
    if (type != sa->intClass.get()) {
      sa->error(expr->get_loc(), "type error");
      err = true;
    }
  }
  if (err) {
    return sa->errorClass.get();
  }
  return res;
}

Class *int_op_type(cool::SemanticAnalyser *sa,
                   std::list<std::shared_ptr<Expression>> expressions) {
  return int_op_type(sa, expressions, sa->intClass.get());
}

Class *Add::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{a, b});
}

void Sub::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "-"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *Sub::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{a, b});
}

void Mul::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "*"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *Mul::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{a, b});
}

void Div::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "/"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *Div::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{a, b});
}

void Neg::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "not"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

Class *Neg::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{expr});
}

void LessThan::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "<"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *LessThan::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{a, b},
                     sa->boolClass.get());
}

void Equal::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "="
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *Equal::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{a, b},
                     sa->boolClass.get());
}

void LessOrEqual::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "<="
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

Class *LessOrEqual::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{a, b},
                     sa->boolClass.get());
}

void Not::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "~"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

Class *Not::type(cool::SemanticAnalyser *sa) {
  auto expr_type = expr->type(sa);
  if (expr_type == sa->errorClass.get()) {
    return expr_type;
  }
  if (expr_type != sa->boolClass.get()) {
    sa->error(get_loc(), "type error");
    return sa->errorClass.get();
  }
  return sa->boolClass.get();
}

void Var::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " @" << loc << std::endl;
}

Class *Var::type(cool::SemanticAnalyser *sa) {
  if (name == "self") {
    return sa->selfClass;
  }
  auto res = sa->scope.find(name);
  if (!res) {
    sa->error(get_loc(), "undefined variable \"" + name + "\"");
    return sa->errorClass.get();
  }
  return res;
}

void IntConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << value << " @" << loc << std::endl;
}

Class *IntConst::type(cool::SemanticAnalyser *sa) { return sa->intClass.get(); }

void StrConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "\"" << escaped() << "\""
    << " @" << loc << std::endl;
}

Class *StrConst::type(cool::SemanticAnalyser *sa) {
  return sa->stringClass.get();
}

std::string StrConst::escaped() {
  std::ostringstream o;
  for (auto ch : value) {
    switch (ch) {
    case '\n':
      o << "\\n";
      break;
    case '\t':
      o << "\\t";
      break;
    case '\f':
      o << "\\f";
      break;
    case '\b':
      o << "\\b";
      break;
    default:
      o << ch;
    }
  }
  return o.str();
}

void BoolConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << value << " @" << loc << std::endl;
}

Class *BoolConst::type(cool::SemanticAnalyser *sa) {
  return sa->boolClass.get();
}

void Field::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " : " << type_name << " @" << loc
    << std::endl;
  expr->print(o, indent + 2);
}

bool Field::check_type(cool::SemanticAnalyser *sa) {
  if (!std::dynamic_pointer_cast<Void>(expr)) {
    auto expr_type = expr->type(sa);
    if (expr_type == sa->errorClass.get()) {
      return false;
    }

    if (!sa->assignable(sa->name2Class[type_name].get(), expr_type)) {
      sa->error(get_loc(), "type error");
      return false;
    }
  }
  return true;
}

void Formal::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " : " << type_name << " @" << loc
    << std::endl;
}

void Method::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " -> " << ret_type_name << " @"
    << loc << std::endl;
  for (auto &formal : formals) {
    formal->print(o, indent + 2);
  }
  expr->print(o, indent + 2);
}

bool Method::same_signature(Method *other) {
  if (name != other->name || ret_type_name != other->ret_type_name) {
    return false;
  }

  auto it1 = formals.begin(), it2 = other->formals.begin();
  for (; it1 != formals.end() && it2 != other->formals.end(); it1++, it2++) {
    if (it1->get()->type_name != it2->get()->type_name) {
      return false;
    }
  }
  return it1 == formals.end() && it2 == other->formals.end();
}

bool Method::check_type(cool::SemanticAnalyser *sa) {
  sa->scope.enter();
  for (auto &formal : formals) {
    sa->scope.add(formal->name, sa->name2Class[formal->type_name].get());
  }

  auto expr_type = expr->type(sa);

  sa->scope.exit();

  if (expr_type == sa->errorClass.get()) {
    return false;
  }

  auto ret_type = sa->selfClass;
  if (ret_type_name != "SELF_TYPE") {
    ret_type = sa->name2Class[ret_type_name].get();
  }

  if (!sa->assignable(ret_type, expr_type)) {
    sa->error(get_loc(), "type error");
    return false;
  }

  return true;
}

void Class::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name;
  if (!parent_name.empty()) {
    o << " inherits " << parent_name;
  }
  o << " @" << loc << std::endl;

  for (auto &feature : features) {
    feature->print(o, indent + 2);
  }
}

void Class::append_child(Class *child) {
  child->parent = this;
  children.push_back(child);
}

void Class::print_hierarchy(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << std::endl;
  for (auto &child : children) {
    child->print_hierarchy(o, indent + 2);
  }
}

Class *Class::get_method_class(std::string name) {
  if (name2Method.find(name) != name2Method.end()) {
    return this;
  }
  if (parent) {
    return parent->get_method_class(name);
  }
  return nullptr;
}

Method *Class::get_method(std::string name) {
  auto cls = get_method_class(name);
  if (!cls) {
    return nullptr;
  }
  return cls->name2Method[name];
}

void Class::check_type(cool::SemanticAnalyser *sa) {
  bool err = false;
  for (auto &feature : features) {
    auto field = std::dynamic_pointer_cast<ast::Field>(feature);
    if (field && !field->check_type(sa)) {
      err = true;
    }
  }
  if (err) {
    return;
  }

  sa->scope.enter();
  for (auto &feature : features) {
    auto field = std::dynamic_pointer_cast<ast::Field>(feature);
    if (field) {
      sa->scope.add(field->name, sa->name2Class[field->type_name].get());
    }
  }

  for (auto &feature : features) {
    auto method = std::dynamic_pointer_cast<ast::Method>(feature);
    if (method) {
      method->check_type(sa);
    }
  }

  sa->scope.exit();
}

void Program::print(std::ostream &o) {
  for (auto &cls : classes) {
    cls->print(o, 0);
  }
}

} /* namespace ast */

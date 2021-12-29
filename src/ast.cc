#include "ast.hh"

#include <sstream>

namespace ast {

void Assign::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " <-"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
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

void If::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "IF"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
  c->print(o, indent + 2);
}

void While::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "WHILE"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void Block::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "BLOCK"
    << " @" << loc << std::endl;
  for (auto &expr : expressions) {
    expr->print(o, indent + 2);
  }
}

void Let::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "let " << name << " : " << type_name << " @"
    << loc << std::endl;
  expr->print(o, indent + 2);
  body->print(o, indent + 2);
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

void New::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "new " << type_name << " @" << loc
    << std::endl;
}

void IsVoid::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "isvoid"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

void Add::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "+"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void Sub::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "-"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void Mul::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "*"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void Div::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "/"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void Neg::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "not"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

void LessThan::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "<"
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void Equal::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "="
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void LessOrEqual::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "<="
    << " @" << loc << std::endl;
  a->print(o, indent + 2);
  b->print(o, indent + 2);
}

void Not::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "~"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

void Var::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " @" << loc << std::endl;
}

void IntConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << value << " @" << loc << std::endl;
}

void StrConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "\"" << escaped() << "\""
    << " @" << loc << std::endl;
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

void Field::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << name << " : " << type_name << " @" << loc
    << std::endl;
  expr->print(o, indent + 2);
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

void Program::print(std::ostream &o) {
  for (auto &cls : classes) {
    cls->print(o, 0);
  }
}

} /* namespace ast */

#ifndef _AST_HH
#define _AST_HH

#include <list>
#include <memory>
#include <string>

#include "location.hh"

namespace ast {

class Node {
public:
  void set_loc(const yy::location &l) { loc = l; }

  virtual void print(std::ostream &o, int indent) {}

protected:
  yy::location loc;
};

class Expression : public Node {};

class Void : public Expression {};

class Assign : public Expression {
public:
  Assign(std::string name, std::shared_ptr<Expression> expr)
      : name(name), expr(expr) {}

  void print(std::ostream &o, int indent) override;

  std::string name;
  std::shared_ptr<Expression> expr;
};

class Invoke : public Expression {
public:
  Invoke(std::shared_ptr<Expression> expr, std::string type_name,
         std::string name, std::list<std::shared_ptr<Expression>> arguments)
      : expr(expr), type_name(type_name), name(name), arguments(arguments) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> expr;
  std::string type_name;

  std::string name;
  std::list<std::shared_ptr<Expression>> arguments;
};

class If : public Expression {
public:
  If(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b,
     std::shared_ptr<Expression> c)
      : a(a), b(b), c(c) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b, c;
};

class While : public Expression {
public:
  While(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class Block : public Expression {
public:
  Block(std::list<std::shared_ptr<Expression>> expressions)
      : expressions(expressions) {}

  void print(std::ostream &o, int indent) override;

  std::list<std::shared_ptr<Expression>> expressions;
};

class Let : public Expression {
public:
  Let(std::string name, std::string type_name, std::shared_ptr<Expression> expr,
      std::shared_ptr<Expression> body)
      : name(name), type_name(type_name), expr(expr), body(body) {}

  void print(std::ostream &o, int indent) override;

  std::string name;
  std::string type_name;
  std::shared_ptr<Expression> expr;
  std::shared_ptr<Expression> body;
};

class CaseBranch : public Node {
public:
  CaseBranch(std::string name, std::string type_name,
             std::shared_ptr<Expression> expr)
      : name(name), type_name(type_name), expr(expr) {}

  void print(std::ostream &o, int indent) override;

  std::string name;
  std::string type_name;
  std::shared_ptr<Expression> expr;
};

class Case : public Expression {
public:
  Case(std::shared_ptr<Expression> expr,
       std::list<std::shared_ptr<CaseBranch>> branches)
      : expr(expr), branches(branches) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> expr;
  std::list<std::shared_ptr<CaseBranch>> branches;
};

class New : public Expression {
public:
  New(std::string type_name) : type_name(type_name) {}

  void print(std::ostream &o, int indent) override;

  std::string type_name;
};

class IsVoid : public Expression {
public:
  IsVoid(std::shared_ptr<Expression> expr) : expr(expr) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> expr;
};

class Add : public Expression {
public:
  Add(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class Sub : public Expression {
public:
  Sub(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class Mul : public Expression {
public:
  Mul(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class Div : public Expression {
public:
  Div(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class Neg : public Expression {
public:
  Neg(std::shared_ptr<Expression> expr) : expr(expr) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> expr;
};

class LessThan : public Expression {
public:
  LessThan(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class Equal : public Expression {
public:
  Equal(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class LessOrEqual : public Expression {
public:
  LessOrEqual(std::shared_ptr<Expression> a, std::shared_ptr<Expression> b)
      : a(a), b(b) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> a, b;
};

class Not : public Expression {
public:
  Not(std::shared_ptr<Expression> expr) : expr(expr) {}

  void print(std::ostream &o, int indent) override;

  std::shared_ptr<Expression> expr;
};

class Var : public Expression {
public:
  Var(std::string name) : name(name) {}

  void print(std::ostream &o, int indent) override;

  std::string name;
};

class IntConst : public Expression {
public:
  IntConst(int value) : value(value) {}

  void print(std::ostream &o, int indent) override;

  int value;
};

class StrConst : public Expression {
public:
  StrConst(std::string value) : value(value) {}

  void print(std::ostream &o, int indent) override;

  std::string escaped();

  std::string value;
};

class BoolConst : public Expression {
public:
  BoolConst(bool value) : value(value) {}

  void print(std::ostream &o, int indent) override;

  bool value;
};

class Feature : public Node {};

class Field : public Feature {
public:
  Field(std::string name, std::string type_name,
        std::shared_ptr<Expression> expr)
      : name(name), type_name(type_name), expr(expr) {}

  void print(std::ostream &o, int indent) override;

  std::string name;
  std::string type_name;
  std::shared_ptr<Expression> expr;
};

class Formal : public Node {
public:
  Formal(std::string name, std::string type_name)
      : name(name), type_name(type_name) {}

  void print(std::ostream &o, int indent) override;

  std::string name;
  std::string type_name;
};

class Method : public Feature {
public:
  Method(std::string name, std::list<std::shared_ptr<Formal>> formals,
         std::string ret_type_name, std::shared_ptr<Expression> expr)
      : name(name), formals(formals), ret_type_name(ret_type_name), expr(expr) {
  }

  void print(std::ostream &o, int indent) override;

  std::string name;
  std::list<std::shared_ptr<Formal>> formals;
  std::string ret_type_name;
  std::shared_ptr<Expression> expr;
};

class Class : public Node {
public:
  Class(std::string name, std::string parent_name,
        std::list<std::shared_ptr<Feature>> features)
      : name(name), parent_name(parent_name), features(features) {}

  void print(std::ostream &o, int indent) override;

  std::string name;
  std::string parent_name;
  std::list<std::shared_ptr<Feature>> features;
};

class Program {
public:
  Program(std::list<std::shared_ptr<Class>> classes) : classes(classes) {}

  void print(std::ostream &o);

  std::list<std::shared_ptr<Class>> classes;
};

} /* namespace ast */

#endif /* _AST_HH */

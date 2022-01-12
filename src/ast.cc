#include "ast.hh"

#include "cg.hh"
#include "sa.hh"
#include "util.hh"

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

void Assign::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # ASSIGN " + name + "\n";
  expr->generate(cg, o);
  o << "  movq %rax, " + cg->scope.find(name) + "\n";
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
  expr_sa = expr;
  if (std::dynamic_pointer_cast<Void>(expr_sa)) {
    expr_sa = std::make_shared<Var>("self");
  }

  auto expr_sa_type = expr_sa->type(sa);
  if (expr_sa_type == sa->errorClass.get()) {
    return expr_sa_type;
  }

  type_sa = expr_sa_type;
  if (!type_name.empty()) {
    if (sa->name2Class.find(type_name) == sa->name2Class.end()) {
      sa->error(get_loc(), "undefined class \"" + type_name + "\"");
      return sa->errorClass.get();
    }
    type_sa = sa->name2Class[type_name].get();
  }

  if (!sa->assignable(type_sa, expr_sa_type)) {
    sa->error(get_loc(), "type error");
    return sa->errorClass.get();
  }

  auto method = type_sa->get_method(name);
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

void Invoke::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # INVOKE " + name + "\n";

  o << "  pushq %rbx\n"; // save rbx
  cg->offset_rbp++;

  // push arguments
  for (auto it = arguments.rbegin(); it != arguments.rend(); ++it) {
    it->get()->generate(cg, o);
    o << "  pushq %rax\n";
    cg->offset_rbp++;
  }

  expr_sa->generate(cg, o);

  o << "  cmpq $0, %rax\n";
  o << "  je _invoke_on_void\n";

  o << "  movq %rax, %rbx\n"; // this

  // invoke

  if (type_name.empty()) {
    o << "  movq 32(%rbx), %rax\n"; // method table
  } else {
    o << "  movq $" + type_sa->name + "_method_table, %rax\n"; // method table
  }

  o << "  call *" + std::to_string(type_sa->methods_numbered[name] * 8) +
           "(%rax)\n";

  o << "  addq $" + std::to_string(arguments.size() * 8) +
           ", %rsp\n"; // pop arguments
  cg->offset_rbp -= arguments.size();

  o << "  popq %rbx\n"; // restore rbx
  cg->offset_rbp -= 1;
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

void If::generate(cool::CodeGenerator *cg, std::ostream &o) {
  auto label_1 = cg->next_label();
  auto label_2 = cg->next_label();
  o << "  # IF\n";
  a->generate(cg, o);
  o << "  cmpq $0, 40(%rax)\n";
  o << "  je " + label_1 + "\n";
  b->generate(cg, o);
  o << "  jmp " + label_2 + "\n";
  o << label_1 + ":\n";
  c->generate(cg, o);
  o << label_2 + ":\n";
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

  return sa->objectClass.get();
}

void While::generate(cool::CodeGenerator *cg, std::ostream &o) {
  auto label_1 = cg->next_label();
  auto label_2 = cg->next_label();

  o << "  # WHILE\n";
  o << "  xorq %rax, %rax\n";

  o << label_1 + ":\n";
  o << "  pushq %rax\n"; // save rax
  cg->offset_rbp++;

  a->generate(cg, o);

  o << "  popq %rcx\n"; // restore
  cg->offset_rbp--;

  o << "  cmpq $0, 40(%rax)\n";
  o << "  je " + label_2 + "\n";

  b->generate(cg, o);
  o << "  jmp " + label_1 + "\n";

  o << label_2 + ":\n";
  o << "  movq %rcx, %rax\n";
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

void Block::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # BLOCK\n";
  for (auto &expr : expressions) {
    expr->generate(cg, o);
  }
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

void Let::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # LET " + name + "\n";
  o << "  pushq $0\n"; // a local variable
  cg->offset_rbp++;

  cg->scope.enter();
  cg->scope.add(name, std::to_string(cg->offset_rbp * (-8)) + "(%rbp)");

  expr_cg = expr;
  if (std::dynamic_pointer_cast<Void>(expr_cg)) {
    auto cls = cg->sa->name2Class[type_name];
    if (cls == cg->sa->stringClass) {
      expr_cg = std::make_shared<StrConst>("");
    } else if (cls == cg->sa->intClass) {
      expr_cg = std::make_shared<IntConst>(0);
    } else if (cls == cg->sa->boolClass) {
      expr_cg = std::make_shared<BoolConst>(false);
    }
  }

  if (!std::dynamic_pointer_cast<Void>(expr_cg)) {
    expr_cg->generate(cg, o);
    o << "  movq %rax, " + cg->scope.find(name) + "\n";
  }

  body->generate(cg, o);

  cg->scope.exit();

  o << "  popq %rcx\n";
  cg->offset_rbp--;
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
    branch->type = sa->name2Class[branch->type_name].get();
    if (!sa->assignable(expr_type, branch->type) &&
        !sa->assignable(branch->type, expr_type)) {
      sa->error(branch->get_loc(), "type error");
      err = true;
      continue;
    }

    sa->scope.enter().add(branch->name, branch->type);
    branch->expr_type = branch->expr->type(sa);
    if (branch->expr_type == sa->errorClass.get()) {
      err = true;
    } else {
      branch_expr_types.push_back(branch->expr_type);
    }
    sa->scope.exit();
  }

  if (err) {
    return sa->errorClass.get();
  }

  return sa->common_ancestor(branch_expr_types);
}

void Case::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # CASE\n";
  expr->generate(cg, o);

  o << "  cmpq $0, %rax\n";
  o << "  je _case_on_void\n";

  auto label_done = cg->next_label();

  for (auto &branch : branches) {
    auto label_1 = cg->next_label();

    o << "  cmpq $" + std::to_string(branch->type->id) +
             ", 16(%rax)\n"; // compare class id
    o << "  jne " + label_1 + "\n";

    o << "  pushq %rax\n"; // a new local variable
    cg->offset_rbp++;
    cg->scope.enter();
    cg->scope.add(branch->name,
                  std::to_string(cg->offset_rbp * (-8)) + "(%rbp)");

    branch->expr->generate(cg, o);

    o << "  popq %rcx\n";
    cg->offset_rbp--;
    cg->scope.exit();

    o << "  jmp " + label_done + "\n";

    o << label_1 + ":\n";
  }

  o << "  jmp _case_no_match\n";

  o << label_done + ":\n";
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
  type_sa = sa->name2Class[type_name].get();
  return type_sa;
}

void new_generate(cool::CodeGenerator *cg, std::ostream &o, Class *cls) {
  o << "  pushq %rbx\n"; // save rbx

  // invoke `copy` method of the prototype object
  o << "  movq $" + cls->name + "_prototype, %rbx\n"; // this
  o << "  movq $" + cls->name + "_method_table, %rax\n";
  o << "  call *" + std::to_string(cls->methods_numbered["copy"] * 8) +
           "(%rax)\n";

  // invoke `__init__` method of the new object
  o << "  movq %rax, %rbx\n";
  o << "  movq $" + cls->name + "_method_table, %rax\n";
  o << "  call *" +
           std::to_string(cls->methods_numbered[cool::init_method_name] * 8) +
           "(%rax)\n";

  o << "  popq %rbx\n"; // restore rbx
}

void New::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # NEW " + type_name + "\n";
  new_generate(cg, o, type_sa);
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

void IsVoid::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # ISVOID\n";
  expr->generate(cg, o);

  o << "  cmpq $0, %rax\n";
  o << "  je 1f\n";
  o << "  movq $bool_constant_false, %rax\n";
  o << "  jmp 2f\n";
  o << "1:\n";
  o << "  movq $bool_constant_true, %rax\n";
  o << "2:\n";
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

void int_op_generate(cool::CodeGenerator *cg, std::ostream &o, Expression *a,
                     Expression *b, char op) {
  b->generate(cg, o);
  o << "  movq 40(%rax), %rax\n"; // 2nd operand

  o << "  pushq %rax\n";
  cg->offset_rbp++;

  a->generate(cg, o);
  o << "  movq 40(%rax), %rax\n"; // 1st operand

  o << "  popq %rcx\n"; // 2nd operand
  cg->offset_rbp--;

  switch (op) {
  case '+':
    o << "  addq %rcx, %rax\n";
    break;
  case '-':
    o << "  subq %rcx, %rax\n";
    break;
  case '*':
    o << "  imulq %rcx\n";
    break;
  case '/':
    o << "  cqto\n"; // Convert quadword in %rax to octoword in %rdx:%rax
    o << "  idivq %rcx\n";
    break;
  }

  o << "  pushq %rax\n";
  cg->offset_rbp++;

  new_generate(cg, o, cg->sa->intClass.get());

  o << "  popq %rcx\n";
  cg->offset_rbp--;

  o << "  movq %rcx, 40(%rax)\n";
}

void Add::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # ADD\n";
  int_op_generate(cg, o, a.get(), b.get(), '+');
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

void Sub::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # SUB\n";
  int_op_generate(cg, o, a.get(), b.get(), '-');
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

void Mul::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # MUL\n";
  int_op_generate(cg, o, a.get(), b.get(), '*');
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

void Div::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # DIV\n";
  int_op_generate(cg, o, a.get(), b.get(), '/');
}

void Neg::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "not"
    << " @" << loc << std::endl;
  expr->print(o, indent + 2);
}

Class *Neg::type(cool::SemanticAnalyser *sa) {
  return int_op_type(sa, std::list<std::shared_ptr<Expression>>{expr});
}

void Neg::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # NEG\n";
  expr->generate(cg, o);

  o << "  xorq %rcx, %rcx\n";
  o << "  subq 40(%rax), %rcx\n";
  o << "  pushq %rcx\n";
  cg->offset_rbp++;

  new_generate(cg, o, cg->sa->intClass.get());

  o << "  popq %rcx\n";
  cg->offset_rbp--;

  o << "  movq %rcx, 40(%rax)\n";
}

void int_rel_op_generate(cool::CodeGenerator *cg, std::ostream &o,
                         Expression *a, Expression *b, char op) {
  a->generate(cg, o);
  o << "  movq 40(%rax), %rax\n"; // 1st operand

  o << "  pushq %rax\n";
  cg->offset_rbp++;

  b->generate(cg, o);
  o << "  movq 40(%rax), %rax\n"; // 2nd operand

  o << "  popq %rcx\n"; // 1st operand
  cg->offset_rbp--;

  o << "  cmpq %rax, %rcx\n";

  switch (op) {
  case '<':
    o << "  jl 1f\n";
    break;
  case '=':
    o << "  je 1f\n";
    break;
  case '[':
    o << "  jle 1f\n";
    break;
  }
  o << "  movq $bool_constant_false, %rax\n";
  o << "  jmp 2f\n";
  o << "1:\n";
  o << "  movq $bool_constant_true, %rax\n";
  o << "2:\n";
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

void LessThan::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # LESSTHAN\n";
  int_rel_op_generate(cg, o, a.get(), b.get(), '<');
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

void Equal::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # EQUAL\n";
  int_rel_op_generate(cg, o, a.get(), b.get(), '=');
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

void LessOrEqual::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # LESSOREQUAL\n";
  int_rel_op_generate(cg, o, a.get(), b.get(), '[');
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

void Not::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # NOT\n";
  expr->generate(cg, o);
  o << "  cmpq $0, 40(%rax)\n";
  o << "  je 1f\n";
  o << "  movq $bool_constant_false, %rax\n";
  o << "  jmp 2f\n";
  o << "1:\n";
  o << "  movq $bool_constant_true, %rax\n";
  o << "2:\n";
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

void Var::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  # VAR " + name + "\n";
  if (name == "self") {
    o << "  movq %rbx, %rax\n";
  } else {
    o << "  movq " + cg->scope.find(name) + ", %rax\n";
  }
}

void IntConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << value << " @" << loc << std::endl;
}

Class *IntConst::type(cool::SemanticAnalyser *sa) { return sa->intClass.get(); }

void IntConst::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  movq $int_constant_" +
           std::to_string(cg->get_int_constant_no(value)) + ", %rax\n";
}

void StrConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << "\"" << escaped() << "\""
    << " @" << loc << std::endl;
}

Class *StrConst::type(cool::SemanticAnalyser *sa) {
  return sa->stringClass.get();
}

std::string StrConst::escaped() { return util::escape_string(value); }

void StrConst::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  movq $string_constant_" +
           std::to_string(cg->get_string_constant_no(value)) + ", %rax\n";
}

void BoolConst::print(std::ostream &o, int indent) {
  o << std::string(indent, ' ') << value << " @" << loc << std::endl;
}

Class *BoolConst::type(cool::SemanticAnalyser *sa) {
  return sa->boolClass.get();
}

void BoolConst::generate(cool::CodeGenerator *cg, std::ostream &o) {
  if (value) {
    o << "  movq $bool_constant_true, %rax\n";
  } else {
    o << "  movq $bool_constant_false, %rax\n";
  }
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

    if (!sa->assignable(type, expr_type)) {
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

void Method::generate(cool::CodeGenerator *cg, std::ostream &o) {
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  cg->offset_rbp = 0;

  cg->scope.enter();
  int i = 0;
  for (auto &formal : formals) {
    auto formal_ref = std::to_string((2 + i++) * 8) + "(%rbp)";
    cg->scope.add(formal->name, formal_ref);
  }

  expr->generate(cg, o);

  cg->scope.exit();

  o << "  popq %rbp\n";
  o << "  ret\n";
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

Field *Class::get_field(std::string name) {
  if (name2Field.find(name) != name2Field.end()) {
    return name2Field[name];
  }
  if (parent) {
    return parent->get_field(name);
  }
  return nullptr;
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

void Class::arrange() {
  auto init_block =
      std::make_shared<Block>(std::list<std::shared_ptr<Expression>>{});

  if (parent) {
    fields_ordered = parent->fields_ordered;
    fields_numbered = parent->fields_numbered;
  }

  for (auto it = name2Field.begin(); it != name2Field.end(); ++it) {
    fields_numbered[it->first] = fields_ordered.size();
    fields_ordered.push_back(it->first);

    auto field = it->second;
    if (!std::dynamic_pointer_cast<Void>(field->expr)) {
      init_block->expressions.push_back(
          std::make_shared<Assign>(field->name, field->expr));
    }
  }

  if (parent) {
    methods_ordered = parent->methods_ordered;
    methods_resolved = parent->methods_resolved;

    auto invoke = std::make_shared<Invoke>(
        std::make_shared<Var>("self"), parent->name, cool::init_method_name,
        std::list<std::shared_ptr<Expression>>{});
    invoke->expr_sa = invoke->expr;
    invoke->type_sa = parent;

    init_block->expressions.push_front(invoke);
  }

  init_block->expressions.push_back(std::make_shared<Var>("self"));

  // synthesize the `__init__` method
  init_method = std::make_shared<Method>(cool::init_method_name,
                                         std::list<std::shared_ptr<Formal>>{},
                                         "SELF_TYPE", init_block);
  name2Method[cool::init_method_name] = init_method.get();

  for (auto it = name2Method.begin(); it != name2Method.end(); ++it) {
    auto name = it->first;
    if (methods_resolved.find(name) == methods_resolved.end()) {
      methods_ordered.push_back(name);
    }
    methods_resolved[name] = this;
  }

  int i = 0;
  for (auto &name : methods_ordered) {
    methods_numbered[name] = i++;
  }

  for (auto &child : children) {
    child->arrange();
  }
}

void Program::print(std::ostream &o) {
  for (auto &cls : classes) {
    cls->print(o, 0);
  }
}

} /* namespace ast */

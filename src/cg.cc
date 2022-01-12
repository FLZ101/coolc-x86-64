#include "cg.hh"

#include "util.hh"

/*
 * System V Calling Convention
 *   Parameters
 *     rdi, rsi, rdx, rcx, r8, r9; stack (right to left)
 *   Return Value
 *     rax; rdx
 *   Preserved Registers
 *     rbx, rsp, rbp, r12, r13, r14, r15
 *   Scratch Registers
 *     rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11
 *   Stack Alignment
 *     16-byte at call
 *
 * COOL Calling Convention
 *   this
 *     rbx
 *   Parameters
 *     stack (right to left)
 *   Return Value
 *     rax
 *
 * Expression Code Generation Convention
 *   Result
 *     rax
 */

namespace cool {

std::string CodeGenerator::next_label() {
  return ".L" + std::to_string(label_no++);
}

int CodeGenerator::get_string_constant_no(std::string s) {
  if (string_constants_numbered.find(s) == string_constants_numbered.end()) {
    string_constants_numbered[s] = string_constants.size();
    string_constants.push_back(s);
  }
  return string_constants_numbered[s];
}

int CodeGenerator::get_int_constant_no(int i) {
  if (int_constants_numbered.find(i) == int_constants_numbered.end()) {
    int_constants_numbered[i] = int_constants.size();
    int_constants.push_back(i);
  }
  return int_constants_numbered[i];
}

void CodeGenerator::arrange_classes() {
  for (int i = 0; i < sa->classes.size(); ++i) {
    auto cls = sa->classes[i];
    cls->id = i + 1;

    get_string_constant_no(cls->name);
  }

  sa->objectClass->arrange();
}

void CodeGenerator::generate_prototypes(std::ostream &o) {
  o << "  .text\n\n";
  for (auto &cls : sa->classes) {
    o << "  .balign 8\n";
    o << cls->name + "_prototype:\n";
    o << "  .quad " + cls->name + "_prototype_END - " + cls->name +
             "_prototype\n";            // size
    o << "  .quad 0\n";                 // GC
    o << "  .quad " << cls->id << "\n"; // id
    o << "  .quad string_constant_" << get_string_constant_no(cls->name)
      << "\n";                                       // name
    o << "  .quad " + cls->name + "_method_table\n"; // method table

    // fields
    for (auto &name : cls->fields_ordered) {
      auto field = cls->get_field(name);
      if (field->type == sa->stringClass.get()) {
        o << "  .quad string_constant_" << get_string_constant_no("") << " # " + name + "\n";
      } else if (field->type == sa->intClass.get()) {
        o << "  .quad int_constant_" << get_int_constant_no(0) << " # " + name + "\n";
      } else if (field->type == sa->boolClass.get()) {
        o << "  .quad bool_constant_false" << " # " + name + "\n";
      } else {
        o << "  .quad 0" << "# " + name + "\n";
      }
    }

    // data
    if (cls == sa->stringClass) {
      o << "  .quad string_data_" << get_string_constant_no("") << "\n";
    } else if (cls == sa->intClass) {
      o << "  .quad 0\n";
    } else if (cls == sa->boolClass) {
      o << "  .quad 0\n";
    }

    o << cls->name + "_prototype_END:\n\n";
  }

  // prototype table
  o << "  .balign 8\n";
  o << "prototype_table:\n";
  o << "  .quad 0\n";
  for (auto &cls : sa->classes) {
    o << "  .quad " + cls->name + "_prototype\n";
  }
  o << "\n";

  // method tables
  for (auto &cls : sa->classes) {
    o << "  .balign 8\n";
    o << cls->name + "_method_table:\n";
    for (auto &method_name : cls->methods_ordered) {
      o << "  .quad " + cls->methods_resolved[method_name]->name + "." +
               method_name + "\n";
    }
    o << "\n";
  }
}

void CodeGenerator::generate_methods(std::ostream &o) {
  o << "  .text\n\n";
  for (auto &cls : program->classes) {
    selfClass = cls.get();

    scope.enter();
    for (auto &field_name : cls->fields_ordered) {
      auto field_ref =
          std::to_string((5 + cls->fields_numbered[field_name]) * 8) + "(%rbx)";
      scope.add(field_name, field_ref);
    }

    for (auto it = cls->name2Method.begin(); it != cls->name2Method.end();
         ++it) {
      o << cls->name + "." + it->first + ":\n";
      it->second->generate(this, o);
      o << "\n";
    }

    scope.exit();
  }
}

void CodeGenerator::generate_object_methods(std::ostream &o) {
  o << "Object.__init__:\n";
  o << "  movq %rbx, %rax\n";
  o << "  ret\n\n";

  o << "Object.copy:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  movq 0(%rbx), %rdi\n";
  o << "  call malloc\n";
  o << "  cmpq $0, %rax\n";
  o << "  je _error\n";

  o << "  pushq %rax\n";
  o << "  subq $8, %rsp\n"; // align stack
  o << "  movq %rax, %rdi\n";
  o << "  movq %rbx, %rsi\n";
  o << "  movq 0(%rbx), %rdx\n";
  o << "  call memcpy\n";
  o << "  addq $8, %rsp\n";
  o << "  popq %rax\n";

  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";

  o << "Object.abort:\n";
  o << "  jmp _abort\n\n";

  o << "Object.type_name:\n";
  o << "  movq 24(%rbx), %rax\n";
  o << "  ret\n\n";
}

void CodeGenerator::generate_string_methods(std::ostream &o) {
  o << "String.__init__:\n";
  o << "  movq %rbx, %rax\n";
  o << "  ret\n\n";

  o << "String.length:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  movq 40(%rbx), %rdi\n";
  o << "  call strlen\n";

  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";

  // str3 <- str1.concat(str2)
  o << "String.concat:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  movq 16(%rbp), %rdi\n"; // str2
  o << "  movq 40(%rdi), %rdi\n"; // str2 data
  o << "  call strlen\n";

  o << "  pushq %rax\n";          // str2 length
  o << "  pushq %rax\n";          // align stack
  o << "  movq 40(%rbx), %rdi\n"; // str1 data
  o << "  call strlen\n";
  o << "  movq %rax, %rdi\n"; // str1 length
  o << "  popq %rax\n";
  o << "  popq %rax\n";

  o << "  addq %rax, %rdi\n";
  o << "  incq %rdi\n"; // str1 length + str2 length + 1
  o << "  call malloc\n";
  o << "  cmpq $0, %rax\n";
  o << "  je _error\n";

  o << "  pushq %rax\n";
  o << "  pushq %rax\n";

  o << "  movq %rax, %rdi\n";     // str3 data
  o << "  movq 40(%rbx), %rsi\n"; // str1 data
  o << "  call strcpy\n";

  o << "  movq (%rsp), %rdi\n";   // str3 data
  o << "  movq 16(%rbp), %rsi\n"; // str2
  o << "  movq 40(%rsi), %rsi\n"; // str2 data
  o << "  call strcat\n";

  o << "  popq %rdi\n";
  o << "  popq %rdi\n"; // str3 data

  o << "  call String.__new__\n";

  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";

  // str2 <- str1.substr(i1, i2)
  o << "String.substr:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  movq 40(%rbx), %rdi\n"; // str1 data
  o << "  call strlen\n";

  o << "  movq 16(%rbp), %rdi\n";
  o << "  movq 40(%rdi), %rdi\n"; // i1
  o << "  movq 24(%rbp), %rsi\n";
  o << "  movq 40(%rsi), %rsi\n"; // i2

  o << "  cmpq %rax, %rdi\n";
  o << "  jae 3f\n"; // i1 >= str1 length
  o << "  cmpq %rax, %rsi\n";
  o << "  jbe 1f\n"; // i2 <= str1 length
  o << "  movq %rax, %rsi\n";
  o << "1:\n";
  o << "  cmpq %rdi, %rsi\n";
  o << "  jbe 3f\n"; // i2 <= i1

  o << "  pushq %rdi\n"; // i1
  o << "  subq %rdi, %rsi\n";
  o << "  incq %rsi\n";
  o << "  pushq %rsi\n"; // i2 - i1 + 1
  o << "  movq %rsi, %rdi\n";
  o << "  call malloc\n";
  o << "  cmpq $0, %rax\n";
  o << "  je _error\n";

  o << "  pushq %rax\n"; // str2 data
  o << "  pushq %rax\n"; // align stack

  o << "  movq %rax, %rdi\n";
  o << "  xorq %rsi, %rsi\n";
  o << "  movq 16(%rsp), %rdx\n"; // i2 - i1 + 1
  o << "  call memset\n";

  o << "  movq 8(%rsp), %rdi\n"; // str2 data
  o << "  movq 16(%rsp), %rdx\n";
  o << "  decq %rdx\n";           // i2 - i1
  o << "  movq 24(%rsp), %rsi\n"; // i1
  o << "  addq 40(%rbx), %rsi\n"; // str1 data + i1
  o << "  call memcpy\n";
  o << "  movq %rax, %rdi\n";
  o << "  addq $32, %rsp\n";
  o << "  call String.__new__\n";
  o << "  jmp 4f\n";

  o << "3:\n";
  o << "  movq $string_constant_" + std::to_string(get_string_constant_no("")) +
           ", %rax\n";
  o << "4:\n";
  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";

  // i1 <- str1.to_int()
  o << "String.to_int:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  movq 40(%rbx), %rdi\n"; // str1 data
  o << "  call atol\n";
  o << "  movq %rax, %rdi\n";
  o << "  call Int.__new__\n";

  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";

  o << "String.__new__:\n";
  o << "  pushq %rdi\n";

  o << "  pushq %rbx\n";
  o << "  movq $String_prototype, %rbx\n";
  o << "  call Object.copy\n";
  o << "  popq %rbx\n";

  o << "  popq %rdi\n";
  o << "  movq %rdi, 40(%rax)\n";
  o << "  ret\n\n";
}

void CodeGenerator::generate_int_methods(std::ostream &o) {
  o << "Int.__init__:\n";
  o << "  movq %rbx, %rax\n";
  o << "  ret\n\n";

  // str1 <- i1.to_string()
  o << "Int.to_string:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  movq $32, %rdi\n";
  o << "  call malloc\n";
  o << "  cmpq $0, %rax\n";
  o << "  je _error\n";

  o << "  pushq %rax\n"; // str1 data
  o << "  pushq %rax\n"; // align stack

  o << "  movq %rax, %rdi\n";
  o << "  movq $string_data_" + std::to_string(get_string_constant_no("%ld")) +
           ", %rsi\n";
  o << "  movq 40(%rbx), %rdx\n";
  o << "  call sprintf\n";

  o << "  popq %rdi\n";
  o << "  popq %rdi\n"; // str1 data
  o << "  call String.__new__\n";

  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";

  o << "Int.__new__:\n";
  o << "  pushq %rdi\n";

  o << "  pushq %rbx\n";
  o << "  movq $Int_prototype, %rbx\n";
  o << "  call Object.copy\n";
  o << "  popq %rbx\n";

  o << "  popq %rdi\n";
  o << "  movq %rdi, 40(%rax)\n";
  o << "  ret\n\n";
}

void CodeGenerator::generate_bool_methods(std::ostream &o) {
  o << "Bool.__init__:\n";
  o << "  movq %rbx, %rax\n";
  o << "  ret\n\n";
}

void CodeGenerator::generate_io_methods(std::ostream &o) {
  o << "IO.__init__:\n";
  o << "  movq %rbx, %rax\n";
  o << "  ret\n\n";

  // str1 <- io.in_string()
  o << "IO.in_string:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  pushq $0\n";           // char *line
  o << "  pushq $0\n";           // size_t n
  o << "  leaq 8(%rsp), %rdi\n"; // &line
  o << "  movq %rsp, %rsi\n";    // &n
  o << "  movq stdin, %rdx\n";
  o << "  call getline\n"; // getline(&line, &n, stdin)
  o << "  pop %rdi\n";
  o << "  pop %rdi\n"; // line
  o << "  call String.__new__\n";

  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";

  // io1 <- io1.out_string(str1)
  o << "IO.out_string:\n";
  o << "  pushq %rbp\n";
  o << "  movq %rsp, %rbp\n";

  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned

  o << "  movq $string_data_" + std::to_string(get_string_constant_no("%s")) +
           ", %rdi\n";
  o << "  movq 16(%rbp), %rsi\n"; // str1
  o << "  movq 40(%rsi), %rsi\n"; // str1 data
  o << "  call printf\n";

  o << "  movq %rbx, %rax\n";

  o << "  movq %rbp, %rsp\n";
  o << "  popq %rbp\n";
  o << "  ret\n\n";
}

void CodeGenerator::generate_system_methods(std::ostream &o) {
  o << "_invoke_on_void:\n";
  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned
  o << "  movq $string_data_" +
           std::to_string(get_string_constant_no("fatal error: invoke on void\n")) +
           ", %rdi\n";
  o << "  movq stderr, %rsi\n";
  o << "  call fputs\n";
  o << "  jmp _abort\n\n";

  o << "_case_on_void:\n";
  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned
  o << "  movq $string_data_" +
           std::to_string(get_string_constant_no("fatal error: case on void\n")) + ", %rdi\n";
  o << "  movq stderr, %rsi\n";
  o << "  call fputs\n";
  o << "  jmp _abort\n\n";

  o << "_case_no_match:\n";
  o << "  andq $-16, %rsp\n"; // make the stack 16-byte aligned
  o << "  movq $string_data_" +
           std::to_string(get_string_constant_no("fatal error: case no match\n")) + ", %rdi\n";
  o << "  movq stderr, %rsi\n";
  o << "  call fputs\n";
  o << "  jmp _abort\n\n";

  // ASSUME: stack is aligned
  o << "_error:\n";
  o << "  xorq %rdi, %rdi\n";
  o << "  call perror\n\n";

  // ASSUME: stack is aligned
  o << "_abort:\n";
  o << "  movq $-1, %rdi\n";
  o << "  call exit\n";
  o << "  jmp .\n\n";

  o << "  .globl main\n\n";
  o << "main:\n";
  o << "  push %rbp\n";
  o << "  movq %rsp, %rbp\n";

  ast::new_generate(this, o, sa->mainClass.get()); // new a Main object

  o << "  pushq %rbx\n";
  o << "  movq %rax, %rbx\n";
  o << "  call Main.main\n";
  o << "  popq %rbx\n";
  o << "  movq 40(%rax), %rax\n";

  o << "  popq %rbp\n";
  o << "  ret\n\n";
}

void CodeGenerator::generate_builtin_methods(std::ostream &o) {
  o << "  .text\n\n";
  generate_object_methods(o);
  generate_string_methods(o);
  generate_int_methods(o);
  generate_bool_methods(o);
  generate_io_methods(o);
  generate_system_methods(o);
}

void CodeGenerator::generate_constants(std::ostream &o) {
  o << "  .text\n\n";

  for (auto &s : string_constants) {
    o << "  .balign 8\n";
    auto idx = std::to_string(string_constants_numbered[s]);
    o << "string_constant_" + idx + ":\n";

    o << "  .quad string_constant_" + idx + "_END - string_constant_" + idx +
             "\n";                                  // size
    o << "  .quad 0\n";                             // GC
    o << "  .quad " << sa->stringClass->id << "\n"; // id
    o << "  .quad string_constant_"
      << get_string_constant_no(sa->stringClass->name) << "\n";  // name
    o << "  .quad " + sa->stringClass->name + "_method_table\n"; // method table

    o << "  .quad string_data_" + idx + "\n";
    o << "string_constant_" + idx + "_END:\n\n";

    o << "  .balign 8\n";
    o << "string_data_" + idx + ":\n";
    o << "  .string \"" + util::escape_string(s) + "\"\n";
    o << "\n";
  }

  for (auto &i : int_constants) {
    o << "  .balign 8\n";
    auto idx = std::to_string(int_constants_numbered[i]);
    o << "int_constant_" + idx + ":\n";

    o << "  .quad int_constant_" + idx + "_END - int_constant_" + idx +
             "\n";                               // size
    o << "  .quad 0\n";                          // GC
    o << "  .quad " << sa->intClass->id << "\n"; // id
    o << "  .quad string_constant_"
      << get_string_constant_no(sa->intClass->name) << "\n";  // name
    o << "  .quad " + sa->intClass->name + "_method_table\n"; // method table

    o << "  .quad " + std::to_string(i) + "\n";

    o << "int_constant_" + idx + "_END:\n\n";
  }

  auto bool_constants = std::vector<std::string>{"false", "true"};
  for (int i = 0; i < bool_constants.size(); ++i) {
    o << "  .balign 8\n";
    o << "bool_constant_" + bool_constants[i] + ":\n";

    o << "  .quad bool_constant_" + bool_constants[i] +
             "_END - bool_constant_" + bool_constants[i] + "\n"; // size
    o << "  .quad 0\n";                                          // GC
    o << "  .quad " << sa->boolClass->id << "\n";                // id
    o << "  .quad string_constant_"
      << get_string_constant_no(sa->boolClass->name) << "\n";  // name
    o << "  .quad " + sa->boolClass->name + "_method_table\n"; // method table
    o << "  .quad " + std::to_string(i) + "\n";

    o << "bool_constant_" + bool_constants[i] + "_END:\n\n";
  }
}

void CodeGenerator::generate(std::ostream &o) {
  get_string_constant_no("");
  get_int_constant_no(0);

  arrange_classes();

  generate_prototypes(o);

  generate_methods(o);

  generate_builtin_methods(o);

  generate_constants(o);
}

} // namespace cool

#ifndef _PARSER_H
#define _PARSER_H

#include <memory>
#include <string>
#include <vector>

#include <cstdio>

#include "ast.hh"

namespace yy {

class Parser;

}

typedef void *yyscan_t;

namespace cool {

class SyntaxError : public std::runtime_error {
public:
  SyntaxError(const std::string &what_arg) : std::runtime_error(what_arg) {}
};

class Parser {
public:
  explicit Parser(const std::vector<std::string> &filenames);

  Parser(const std::vector<std::string> &filenames, bool debug);

  void parse();

  std::shared_ptr<ast::Program> program;

  void error(const yy::location &loc, const std::string &msg);

  const std::string *next_filename();

  const std::string *curr_filename();

  yyscan_t yyscanner;

  std::string str_buf;

  int line_begin, col_begin;
  int line_end, col_end;
  int line_begin_2, col_begin_2;

  int nerrs;

private:
  bool debug;

  int idx;
  std::vector<std::string> filenames;

  std::shared_ptr<std::FILE> ptr_file;

  yy::Parser *ptr_yyparser;
};

} /* namespace cool */

#endif /* _PARSER_H */

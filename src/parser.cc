#include "parser.hh"

#include "cool.l.hh"
#include "cool.y.hh"

namespace cool {

Parser::Parser(const std::vector<std::string> &filenames, bool debug)
    : filenames(filenames), debug(debug) {
  if (!filenames.size()) {
    throw std::invalid_argument("no filename");
  }
}

Parser::Parser(const std::vector<std::string> &filenames) : Parser(filenames, false) {
}

void Parser::parse() {
  idx = -1;

  yylex_init(&yyscanner);
  std::shared_ptr<yyscan_t> ptr_yyscanner(
      &yyscanner, [](yyscan_t *p) { yylex_destroy(*p); });

  yy::Parser yyparser{yyscanner, this};
  ptr_yyparser = &yyparser;

  ptr_yyparser->set_debug_level(debug ? 1 : 0);

  next_filename();

  yyparser.parse();

  ptr_file.reset();

  if (nerrs > 0) {
    throw SyntaxError(std::to_string(nerrs) + " error(s)");
  }
}

void Parser::error(const yy::location &loc, const std::string &msg) {
  ptr_yyparser->error(loc, msg);
}

const std::string *Parser::next_filename() {
  if (idx >= int(filenames.size()) - 1) {
    return nullptr;
  }

  auto res = &filenames[++idx];
  auto file = std::fopen(res->c_str(), "r");
  if (!file) {
    throw std::runtime_error(std::string("fail to open ") + *res);
  }
  ptr_file.reset(file, std::fclose);

  yyset_in(file, yyscanner);

  line_begin = col_begin = 1;
  line_end = col_end = 1;
  line_begin_2 = col_begin_2 = 1;

  return res;
}

const std::string *Parser::curr_filename() { return &filenames[idx]; }

} /* namespace cool */

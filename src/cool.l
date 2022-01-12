%option noyywrap noyylineno nodefault batch never-interactive noline nounistd
%option reentrant
%option stack
%option header-file="cool.l.hh"
%option outfile="cool.l.cc"

%{
#include <string>

#include "cool.y.hh"
#include "ast.hh"

#define YY_USER_ACTION do { \
  BEGIN_HERE; \
  for (int i = 0; yytext[i]; i++) { \
    if (yytext[i] == '\n') { \
      parser->line_end++; \
      parser->col_end = 1; \
    } else { \
      parser->col_end++; \
    } \
  } \
} while (0);

#define BEGIN_HERE do { \
  parser->line_begin = parser->line_end; \
  parser->col_begin = parser->col_end; \
} while (0)

#define BEGIN_HERE_2 do { \
  parser->line_begin_2 = parser->line_begin; \
  parser->col_begin_2 = parser->col_begin; \
} while (0)

#define ERROR_HERE(msg) do { \
  yy::location loc(parser->curr_filename(), parser->line_end, parser->col_end); \
  parser->error(loc, (msg)); \
} while (0)

#define LOCATE_HERE do { \
  yylloc->begin.initialize(parser->curr_filename(), parser->line_begin, parser->col_begin); \
  yylloc->end.initialize(parser->curr_filename(), parser->line_end, parser->col_end); \
} while (0)

#define LOCATE_HERE_2 do { \
  yylloc->begin.initialize(parser->curr_filename(), parser->line_begin_2, parser->col_begin_2); \
  yylloc->end.initialize(parser->curr_filename(), parser->line_end, parser->col_end); \
} while (0)

#define HANDLE_EOF do { \
  BEGIN(INITIAL); \
  if (!parser->next_filename()) { \
    return yy::Parser::token::YYEOF; \
  } \
} while (0)

#define DELAY_EOF do { \
  BEGIN(INITIAL); \
  unput(' '); \
} while (0)

#define RETURN(x) do { \
  LOCATE_HERE; \
  return (x); \
} while (0)

#define RETURN_2(x) do { \
  LOCATE_HERE_2; \
  return (x); \
} while (0)

%}

%x str
%x str_err
%x block_comment
%x line_comment

%%

"\"" {
  BEGIN_HERE_2;
  parser->str_buf = "";
  BEGIN(str);
}

<str>{
  "\0" {
    ERROR_HERE("'\\0' in string literal");
    BEGIN(str_err);
  }

  "\\b" parser->str_buf.push_back('\b');
  "\\t" parser->str_buf.push_back('\t');
  "\\n" parser->str_buf.push_back('\n');
  "\\f" parser->str_buf.push_back('\f');
  "\\\n"

  "\\". parser->str_buf.push_back(yytext[1]);

  "\n" {
    ERROR_HERE("unterminated string literal");
    BEGIN(INITIAL);
    return yy::Parser::token::YYerror;
  }

  <<EOF>> {
    DELAY_EOF;
    ERROR_HERE("EOF in string literal");
    return yy::Parser::token::YYerror;
  }

  "\"" {
    BEGIN(INITIAL);
    yylval->emplace<std::string>(parser->str_buf);
    RETURN_2(yy::Parser::token::STR_CONST);
  }

  . parser->str_buf.push_back(yytext[0]);
}

<str_err>{
  "\\\""

  "\\\n"

  "\n"|"\"" {
    BEGIN(INITIAL);
    return yy::Parser::token::YYerror;
  }

  <<EOF>> {
    DELAY_EOF;
    return yy::Parser::token::YYerror;
  }

  .
}

"(*" yy_push_state(block_comment, yyscanner);

<block_comment>{

  "(*" yy_push_state(block_comment, yyscanner);

  "*)" yy_pop_state(yyscanner);

  .|"\n"

  <<EOF>> {
    DELAY_EOF;
    ERROR_HERE("EOF in comment");
    return yy::Parser::token::YYerror;
  }
}

"*)" {
  ERROR_HERE("unmatched *)");
  return yy::Parser::token::YYerror;
}

"--" {
  BEGIN(line_comment);
}

<line_comment>{
  "\n" {
    BEGIN(INITIAL);
  }
  <<EOF>> {
    HANDLE_EOF;
  }
  .
}

(?i:true) {
  yylval->emplace<bool>(true);
  RETURN(yy::Parser::token::BOOL_CONST);
}

(?i:false) {
  yylval->emplace<bool>(false);
  RETURN(yy::Parser::token::BOOL_CONST);
}

[0-9]+ {
  yylval->emplace<int>(std::stoi(yytext));
  RETURN(yy::Parser::token::INT_CONST);
}

"class" RETURN(yy::Parser::token::CLASS);
"inherits" RETURN(yy::Parser::token::INHERITS);
"else" RETURN(yy::Parser::token::ELSE);
"fi" RETURN(yy::Parser::token::FI);
"if" RETURN(yy::Parser::token::IF);
"in" RETURN(yy::Parser::token::IN);
"let" RETURN(yy::Parser::token::LET);
"loop" RETURN(yy::Parser::token::LOOP);
"pool" RETURN(yy::Parser::token::POOL);
"then" RETURN(yy::Parser::token::THEN);
"while" RETURN(yy::Parser::token::WHILE);
"case" RETURN(yy::Parser::token::CASE);
"esac" RETURN(yy::Parser::token::ESAC);
"of" RETURN(yy::Parser::token::OF);
"=>" RETURN(yy::Parser::token::DARROW);
"new" RETURN(yy::Parser::token::NEW);
"isvoid" RETURN(yy::Parser::token::ISVOID);
"<-" RETURN(yy::Parser::token::ASSIGN);
"not" RETURN(yy::Parser::token::NOT);
"<=" RETURN(yy::Parser::token::LE);

[A-Z]([[:alnum:]]|_)* {
  yylval->emplace<std::string>(yytext);
  RETURN(yy::Parser::token::TYPEID);
}

[a-z]([[:alnum:]]|_)* {
  yylval->emplace<std::string>(yytext);
  RETURN(yy::Parser::token::OBJECTID);
}

[-+*/=<{}();:@.,~] {
  RETURN(yytext[0]);
}

.|"\n"

<<EOF>> {
  HANDLE_EOF;
}

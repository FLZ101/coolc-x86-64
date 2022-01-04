%skeleton "lalr1.cc"
%require "3.7.4"
%language "c++"

%expect 18

%no-lines

%defines "cool.y.hh"
%output "cool.y.cc"

%define api.parser.class {Parser}
%define api.value.type variant
%define api.location.file "location.hh"

%define parse.assert
%define parse.error verbose
%define parse.lac full
%define parse.trace

%code requires {
#include "parser.hh"
#include "ast.hh"
}

%locations

%param {yyscan_t yyscanner} {cool::Parser *parser}

%code provides {

#define YY_DECL \
    int yylex(yy::Parser::semantic_type *yylval, yy::Parser::location_type *yylloc, yyscan_t yyscanner, cool::Parser *parser)
YY_DECL;

}

%code {
#include <string>
#include <list>
#include <memory>
}

%token CLASS
%token INHERITS

%token <std::string> TYPEID
%token <std::string> OBJECTID

%token IF
%token THEN
%token ELSE
%token FI

%token WHILE
%token LOOP
%token POOL

%token LET
%token IN

%token CASE
%token OF
%token ESAC

%token NEW
%token ISVOID
%token NOT

%token ASSIGN
%token DARROW
%token LE

%token <std::string> STR_CONST
%token <int> INT_CONST
%token <bool> BOOL_CONST

%type <std::list<std::shared_ptr<ast::Class>>> classes
%type <std::shared_ptr<ast::Class>> class

%type <std::list<std::shared_ptr<ast::Feature>>> features;
%type <std::shared_ptr<ast::Feature>> feature;
%type <std::list<std::shared_ptr<ast::Formal>>> formals formals_non_empty;
%type <std::shared_ptr<ast::Formal>> formal;
%type <std::list<std::shared_ptr<ast::CaseBranch>>> case_branches;
%type <std::shared_ptr<ast::CaseBranch>> case_branch;
%type <std::list<std::shared_ptr<ast::Expression>>> expressions expressions_comma expressions_comma_non_empty;
%type <std::shared_ptr<ast::Expression>> expression let_expr;

%left ASSIGN
%left NOT
%nonassoc LE '<' '='
%left '+' '-'
%left '*' '/'
%left ISVOID
%left '~'
%left '@'
%left '.'

%%

program: classes {
    parser->program = std::make_shared<ast::Program>($1);
    parser->nerrs = yynerrs_;
  }
  ;

classes: class {
    $$ = std::list<std::shared_ptr<ast::Class>>{$1};
  }
  | classes class {
    $1.push_back($2);
    $$ = $1;
  }
  ;

class: CLASS TYPEID '{' features '}' ';' {
    $$ = std::make_shared<ast::Class>($2, "Object", $4);
    $$->set_loc(@$);
  }
  | CLASS TYPEID INHERITS TYPEID '{' features '}' ';' {
    $$ = std::make_shared<ast::Class>($2, $4, $6);
    $$->set_loc(@$);
  }
  | error ';' {
  }
  ;

features: %empty {
    $$ = std::list<std::shared_ptr<ast::Feature>>{};
  }
  | features feature {
    $1.push_back($2);
    $$ = $1;
  }
  ;

feature: OBJECTID '(' formals ')' ':' TYPEID '{' expression '}' ';' {
    $$ = std::make_shared<ast::Method>($1, $3, $6, $8);
    $$->set_loc(@$);
  }
  | OBJECTID ':' TYPEID ';' {
    $$ = std::make_shared<ast::Field>($1, $3, std::make_shared<ast::Void>());
    $$->set_loc(@$);
  }
  | OBJECTID ':' TYPEID ASSIGN expression ';' {
    $$ = std::make_shared<ast::Field>($1, $3, $5);
    $$->set_loc(@$);
  }
  | error ';' {
  }
  ;

formals: %empty {
    $$ = std::list<std::shared_ptr<ast::Formal>>{};
  }
  | formals_non_empty {
    $$ = $1;
  }
  ;

formals_non_empty: formal {
    $$ = std::list<std::shared_ptr<ast::Formal>>{$1};
  }
  | formals_non_empty ',' formal {
    $1.push_back($3);
    $$ = $1;
  }
  ;

formal: OBJECTID ':' TYPEID {
    $$ = std::make_shared<ast::Formal>($1, $3);
    $$->set_loc(@$);
  }
  ;

expression: OBJECTID ASSIGN expression {
    $$ = std::make_shared<ast::Assign>($1, $3);
    $$->set_loc(@$);
  }
  | expression '.' OBJECTID '(' expressions_comma ')' {
    $$ = std::make_shared<ast::Invoke>($1, "", $3, $5);
    $$->set_loc(@$);
  }
  | expression '@' TYPEID '.' OBJECTID '(' expressions_comma ')' {
    $$ = std::make_shared<ast::Invoke>($1, $3, $5, $7);
    $$->set_loc(@$);
  }
  | OBJECTID '(' expressions_comma ')' {
    $$ = std::make_shared<ast::Invoke>(std::make_shared<ast::Void>(), "", $1, $3);
    $$->set_loc(@$);
  }
  | IF expression THEN expression ELSE expression FI {
    $$ = std::make_shared<ast::If>($2, $4, $6);
    $$->set_loc(@$);
  }
  | WHILE expression LOOP expression POOL {
    $$ = std::make_shared<ast::While>($2, $4);
    $$->set_loc(@$);
  }
  | '{' expressions '}' {
    $$ = std::make_shared<ast::Block>($2);
    $$->set_loc(@$);
  }
  | LET let_expr {
    $$ = $2;
    $$->set_loc(@$);
  }
  | CASE expression OF case_branches ESAC {
    $$ = std::make_shared<ast::Case>($2, $4);
    $$->set_loc(@$);
  }
  | NEW TYPEID {
    $$ = std::make_shared<ast::New>($2);
    $$->set_loc(@$);
  }
  | ISVOID expression {
    $$ = std::make_shared<ast::IsVoid>($2);
    $$->set_loc(@$);
  }
  | expression '+' expression {
    $$ = std::make_shared<ast::Add>($1, $3);
    $$->set_loc(@$);
  }
  | expression '-' expression {
    $$ = std::make_shared<ast::Sub>($1, $3);
    $$->set_loc(@$);
  }
  | expression '*' expression {
    $$ = std::make_shared<ast::Mul>($1, $3);
    $$->set_loc(@$);
  }
  | expression '/' expression {
    $$ = std::make_shared<ast::Div>($1, $3);
    $$->set_loc(@$);
  }
  | '~' expression {
    $$ = std::make_shared<ast::Neg>($2);
    $$->set_loc(@$);
  }
  | expression '<' expression {
    $$ = std::make_shared<ast::LessThan>($1, $3);
    $$->set_loc(@$);
  }
  | expression '=' expression {
    $$ = std::make_shared<ast::Equal>($1, $3);
    $$->set_loc(@$);
  }
  | expression LE expression {
    $$ = std::make_shared<ast::LessOrEqual>($1, $3);
    $$->set_loc(@$);
  }
  | NOT expression {
    $$ = std::make_shared<ast::Not>($2);
    $$->set_loc(@$);
  }
  | '(' expression ')' {
    $$ = $2;
    $$->set_loc(@$);
  }
  | OBJECTID {
    $$ = std::make_shared<ast::Var>($1);
    $$->set_loc(@$);
  }
  | INT_CONST {
    $$ = std::make_shared<ast::IntConst>($1);
    $$->set_loc(@$);
  }
  | STR_CONST {
    $$ = std::make_shared<ast::StrConst>($1);
    $$->set_loc(@$);
  }
  | BOOL_CONST {
    $$ = std::make_shared<ast::BoolConst>($1);
    $$->set_loc(@$);
  }
  ;

  let_expr: OBJECTID ':' TYPEID ASSIGN expression IN expression {
    $$ = std::make_shared<ast::Let>($1, $3, $5, $7);
    $$->set_loc(@$);
  }
  | OBJECTID ':' TYPEID IN expression {
    $$ = std::make_shared<ast::Let>($1, $3, std::make_shared<ast::Void>(), $5);
    $$->set_loc(@$);
  }
  | OBJECTID ':' TYPEID ASSIGN expression ',' let_expr {
    $$ = std::make_shared<ast::Let>($1, $3, $5, $7);
    $$->set_loc(@$);
  }
  | OBJECTID ':' TYPEID ',' let_expr {
    $$ = std::make_shared<ast::Let>($1, $3, std::make_shared<ast::Void>(), $5);
    $$->set_loc(@$);
  }
  | error ',' {
  }
  ;

  case_branches: case_branch {
    $$ = std::list<std::shared_ptr<ast::CaseBranch>>{$1};
  }
  | case_branches case_branch {
    $1.push_back($2);
    $$ = $1;
  }
  ;

  case_branch: OBJECTID ':' TYPEID DARROW expression ';' {
    $$ = std::make_shared<ast::CaseBranch>($1, $3, $5);
    $$->set_loc(@$);
  }

  expressions_comma: %empty {
    $$ = std::list<std::shared_ptr<ast::Expression>>{};
  }
  | expressions_comma_non_empty {
    $$ = $1;
  }
  ;

  expressions_comma_non_empty: expression {
    $$ = std::list<std::shared_ptr<ast::Expression>>{$1};
  }
  | expressions_comma_non_empty ',' expression {
    $1.push_back($3);
    $$ = $1;
  }
  ;

  expressions: expression ';' {
    $$ = std::list<std::shared_ptr<ast::Expression>>{$1};
  }
  | expressions expression ';' {
    $1.push_back($2);
    $$ = $1;
  }
  | error ';' {
  }
  ;

%%

void yy::Parser::error (const location_type& loc, const std::string& msg)
{
    std::cerr << loc << ": " << msg << std::endl;
}

%{
// $Id: parser.y,v 1.15 2018-04-06 15:14:40-07 - - $
// Dummy parser for scanner project.

#include <cassert>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%initial-action {
   parser::root = new astree (TOK_ROOT, {0, 0, 0}, "<<ROOT>>");
}


%token TOK_ROOT TOK_TYPEID TOK_FIELD TOK_DECLID TOK_PARAMS
%token TOK_POS TOK_NEG TOK_CALL TOK_VARDECL TOK_INDEX
%token TOK_NEWARRAY TOK_NEWSTR TOK_BLOCK TOK_FUNCTION
%token TOK_PARAM TOK_PROTO TOK_PROTOTYPE TOK_ARRAY 

%token TOK_KW_VOID TOK_KW_IF TOK_KW_ELSE 
%token TOK_KW_WHILE TOK_KW_RETURN TOK_KW_STRUCT
%token TOK_KW_NULL TOK_KW_INT TOK_KW_CHAR TOK_KW_STRING
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON
%token TOK_POINTER_GET TOK_DOUBLE_EQ TOK_NOT_EQ
%token TOK_BRACKETS TOK_FIELDS
%token INVALID_CHARACTER INVALID_STRING TOK_LESS_TH_EQ TOK_LARGE_TH_EQ
%token TOK_KW_NOT TOK_KW_NEW TOK_KW_BOOL TOK_KW_TRUE TOK_KW_FALSE

%token  '('  ')'  '['  ']'  '{'  '}'  ';'  ','  '.'
%token  '='  '+'  '-'  '*'  '/'  '%'  '!'

%right TOK_KW_IF TOK_KW_ELSE 
%right '='
%left TOK_DOUBLE_EQ TOK_NOT_EQ '<' '>' TOK_LESS_TH_EQ TOK_LARGE_TH_EQ
%left '+' '-'
%left '*' '/' '%'
%right TOK_POS TOK_NEG TOK_KW_NOT TOK_KW_NEW 
%left TOK_ARRAY TOK_FIELD TOK_FUNCTION
%left '[' TOK_POINTER_GET
%nonassoc '('

%start start 

%%
start: program                  { $$ = $1; }
        ;
program: program structdef      { $$ = $1->adopt ($2); }
        | program function      { $$ = $1->adopt ($2); }
        | program statement     { $$ = $1->adopt ($2); }
        | program error '}'     { $$ = $1; 
                                destroy($3); exit(EXIT_FAILURE); }
        | program error ';'     { $$ = $1;      
                                destroy($3); exit(EXIT_FAILURE); }
        |                       { $$ = parser::root; }
        ;
structdef: TOK_KW_STRUCT TOK_IDENT fielddecls ';' '}'   
                { 
                $2->change_symbol(TOK_TYPEID); 
                $$ = $1->adopt ($2, $3); 
                destroy($4, $5); }
         | TOK_KW_STRUCT TOK_IDENT '{' '}'   
                {  $3->change_symbol(TOK_FIELDS);
                $2->change_symbol(TOK_TYPEID); 
                $$ = $1->adopt ($2, $3); 
                destroy($4); }
         | TOK_KW_STRUCT TOK_IDENT ';'
                { destroy($3);
                  $2->change_symbol(TOK_TYPEID);
                  $1->adopt($2);
                }
        ;
fielddecls: '{' fielddecl
                { $$ = $1->adopt($2); $1->change_symbol(TOK_FIELDS);}
          | fielddecls ';' fielddecl
                { $$ = $1->adopt($3); destroy($2);}
        ;
fielddecl: basetype TOK_BRACKETS TOK_IDENT 
                { $3->change_symbol(TOK_FIELD); 
                $2->change_symbol(TOK_ARRAY); 
                $$ = $2->adopt ($1, $3); }
         | basetype TOK_IDENT 
                { $2->change_symbol(TOK_FIELD); 
                $$ = $1->adopt ($2); }
        ;
basetype:  TOK_KW_INT            { $$ = $1; }
           | TOK_KW_CHAR        { $$ = $1; }
           | TOK_KW_STRING      { $$ = $1; }
           | TOK_KW_VOID        { $$ = $1; }
           | TOK_KW_BOOL        { $$ = $1; }
           | TOK_IDENT          { $$ = $1; 
                                $1->change_symbol(TOK_TYPEID); }
        ;
function: identdecl parameters ')' block  
                { 
                  $$ = $3->adopt($1, $2, $4);  
                  $3->change_symbol(TOK_FUNCTION); 
                  $2->change_symbol(TOK_PARAMS); }
         | identdecl parameters ')' ';'
                {  destroy($4); 
                   $$ = $3->adopt($1, $2); 
                   $3->change_symbol(TOK_PROTO); 
                   $2->change_symbol(TOK_PARAMS); }
         | identdecl '(' ')' block
                {  $$ = $3->adopt($1, $2, $4); 
                $2->change_symbol(TOK_PARAMS); 
                $3->change_symbol(TOK_FUNCTION); }
         | identdecl '(' ')' ';'
                { destroy($4); 
                $$ = $3->adopt($1, $2); 
                $2->change_symbol(TOK_PARAMS); 
                $3->change_symbol(TOK_PROTO); }
        ;
parameters: '(' identdecl
                { $$ = $1->adopt($2); 
                $1->change_symbol(TOK_PARAMS); }
          | parameters ',' identdecl
                { $$ = $1->adopt($3); 
                $1->change_symbol(TOK_PARAMS); destroy($2); }
        ;
identdecl: basetype TOK_BRACKETS TOK_IDENT
                { $2->change_symbol(TOK_ARRAY); 
                $3->change_symbol(TOK_DECLID); 
                $$ = $2->adopt($1, $3); }
         | basetype TOK_IDENT   
                { $2->change_symbol(TOK_DECLID); 
                $$ = $1->adopt($2); }
        ;
block:  statements '}'   
                { destroy($2); $$ = $1; 
                $1->change_symbol(TOK_BLOCK); }
        | '{' '}'
                { destroy($2); $$ = $1; 
                $1->change_symbol(TOK_BLOCK); }
        ;
localdecl: identdecl '=' expr ';'      
                { destroy($4); $$ = $2->adopt($1, $3); 
                $2->change_symbol(TOK_VARDECL); }
        ;
statements: statements statement
                { $$ = $1->adopt($2); }
        | '{' statement
                { $$ = $1->adopt($2); }
        ;
statement: block       { $$ = $1; } 
          | localdecl   { $$ = $1; }
          | while       { $$ = $1; }
          | ifelse      { $$ = $1; }
          | return      { $$ = $1; }
          | expr ';'    { $$ = $1; destroy($2); } 
          | ';'         { $$ = $1; destroy($1); }
        ;
while: TOK_KW_WHILE '(' expr ')' statement 
                { $$ = $1->adopt($3, $5); destroy($2, $4); }
        ;
ifelse: TOK_KW_IF '(' expr ')' statement elsecase
                { $$ = $1->adopt($3, $5, $6); destroy($2, $4); }
        | TOK_KW_IF '(' expr ')' statement
                { $$ = $1->adopt($3, $5); destroy($2, $4); }
        ;
elsecase: TOK_KW_ELSE statement
                { $$ = $1->adopt($2); }
        ;
return: TOK_KW_RETURN expr ';'       
                { $$ = $1->adopt($2); destroy($3); }
      | TOK_KW_RETURN ';'            { $$ = $1; destroy($2); }
        ;
expr: binop                     { $$ = $1; }
     | unop expr                { $$ = $1->adopt($2); }
     | allocation               { $$ = $1; }
     | call                     { $$ = $1; }
     | '(' expr ')'             { $$ = $2; destroy($1, $3); }
     | variable                 { $$ = $1; } 
     | constant                 { $$ = $1; }
        ;
binop: expr '=' expr                 { $$ = $2->adopt($1, $3); }
      | expr '+' expr                { $$ = $2->adopt($1, $3); }
      | expr '-' expr                { $$ = $2->adopt($1, $3); }
      | expr '*' expr                { $$ = $2->adopt($1, $3); }
      | expr '/' expr                { $$ = $2->adopt($1, $3); }
      | expr '>' expr                { $$ = $2->adopt($1, $3); }
      | expr '<' expr                { $$ = $2->adopt($1, $3); }        
      | expr '^' expr                { $$ = $2->adopt($1, $3); }
      | expr TOK_DOUBLE_EQ expr      { $$ = $2->adopt($1, $3); }
      | expr TOK_NOT_EQ expr         { $$ = $2->adopt($1, $3); }
      | expr TOK_LARGE_TH_EQ expr    { $$ = $2->adopt($1, $3); }
      | expr TOK_LESS_TH_EQ expr     { $$ = $2->adopt($1, $3); }
        ;
unop: '+'      { $$ = $1; $1->change_symbol(TOK_POS); }
     | '-'      { $$ = $1; $1->change_symbol(TOK_NEG); }
     | '!'      { $$ = $1; }
     | TOK_KW_NOT       { $$ = $1; }
        ;
allocation: TOK_KW_NEW TOK_IDENT                        
                { $$ = $1->adopt($2); 
                $2->change_symbol(TOK_TYPEID); }
           | TOK_KW_NEW TOK_KW_STRING '(' expr ')'         
                { $$ = $1->adopt($2, $3); destroy($3, $5); }
           | TOK_KW_NEW basetype '[' expr ']'              
                { $$ = $1->adopt($2, $3, $4); 
                destroy($5); 
                $3->change_symbol(TOK_ARRAY);}
        ;
call:   TOK_IDENT exprs ')'    
                {  
                  $3->change_symbol(TOK_CALL); 
                  $$ = $3->adopt($1, $2); }
        | TOK_IDENT '(' ')'
                { $$ = $3->adopt($1, $2); 
                $3->change_symbol(TOK_CALL); }
        ;
exprs:   '(' expr
                { $$ = $1->adopt($2); }
        | exprs ',' expr 
                { $$ = $1->adopt($3); destroy($2); }
        ;
variable: TOK_IDENT                            { $$ = $1; }
         | expr '[' expr ']'                    
                        { $$ = $2->adopt($1, $3); 
                        destroy($4); 
                        $2->change_symbol(TOK_INDEX); }
         | expr TOK_POINTER_GET TOK_IDENT       
                        { $$ = $2->adopt($1, $3); }
        ;
constant: TOK_INTCON            { $$ = $1; }
         | TOK_CHARCON          { $$ = $1; }
         | TOK_STRINGCON        { $$ = $1; }
         | TOK_KW_TRUE          { $$ = $1; }
         | TOK_KW_FALSE         { $$ = $1; }
         | TOK_KW_NULL          { $$ = $1; }
        ;
%%


const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}



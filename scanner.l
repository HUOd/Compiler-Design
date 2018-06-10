/* $Id: lexer.l,v 1.7 2016-10-27 19:45:59-07 - - $ */

%{

#include "lyutils.h"
#include "astree.h"

#define YY_USER_ACTION  { lexer::advance(); }

int yylval_token (int symbol) {
   yylval = new astree (symbol, lexer::lloc, yytext);
   yylval->print_tok(stdout, yylval);
   return symbol;
}

%}

%option 8bit
%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
%option verbose

LETTER              [A-Za-z_]
DIGIT               [0-9]
TOK_DOUBLE_EQ       ==
TOK_NOT_EQ          !=
TOK_LESS_TH_EQ      <=
TOK_LARGE_TH_EQ     >=
TOK_POINTER_GET     ->
TOK_BRACKETS        [\[\]]+
TOK_KW_IF           \bif\b
TOK_KW_ELSE         \belse\b
TOK_KW_WHILE        \bwhile\b
TOK_KW_RETURN       \breturn\b
TOK_KW_INT          \bint\b  
TOK_KW_CHAR         \bchar\b
TOK_KW_STRING       \bstring\b
TOK_KW_BOOL         \bbool\b
TOK_KW_STRUCT       \bstruct\b
TOK_KW_VOID         \bvoid\b
TOK_KW_NEW          \bnew\b
TOK_KW_NULL         \bnull\b
TOK_KW_NOT          \bnot\b
TOK_KW_TRUE         \btrue\b
TOK_KW_FALSE        \bfalse\b
TOK_IDENT           ({LETTER}({LETTER}|{DIGIT})*)
TOK_INTCON ([1-9][0-9]*)|([0]+)|[0]+([0-7]*)|[0]+[xX]+([0-9a-fA-F]*)
TOK_CHARCON         ('([^\\'\n]|\\[\\'"0nt])')
TOK_STRINGCON       (\"([^\\"\n]|\\[\\'"0nt])*\")
INVALID_CHARACTER   ('([^\\'\n]|\\[\\'"0nt]))
INVALID_STRING      (\"([^\\"\n]|\\[\\'"0nt])*)

%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }

"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }
"="             { return yylval_token ('='); }
"=="            { return yylval_token (TOK_DOUBLE_EQ); }
"!="            { return yylval_token (TOK_NOT_EQ); }
"<"             { return yylval_token ('<'); }
"<="            { return yylval_token (TOK_LESS_TH_EQ); }
">"             { return yylval_token ('>'); }
">="            { return yylval_token (TOK_LARGE_TH_EQ); }
"->"            { return yylval_token (TOK_POINTER_GET) ;}
","             { return yylval_token (','); }
";"             { return yylval_token (';'); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
"[]"            { return yylval_token (TOK_BRACKETS); }
if              { return yylval_token (TOK_KW_IF); }
else            { return yylval_token (TOK_KW_ELSE); }
while           { return yylval_token (TOK_KW_WHILE); }
return          { return yylval_token (TOK_KW_RETURN); }
int             { return yylval_token (TOK_KW_INT); }
string          { return yylval_token (TOK_KW_STRING); }
bool            { return yylval_token (TOK_KW_BOOL); }
struct          { return yylval_token (TOK_KW_STRUCT); }
void            { return yylval_token (TOK_KW_VOID); }
new             { return yylval_token (TOK_KW_NEW); }
null            { return yylval_token (TOK_KW_NULL); }
not             { return yylval_token (TOK_KW_NOT); }
{TOK_INTCON}    { return yylval_token (TOK_INTCON); }
{TOK_CHARCON}   { return yylval_token (TOK_CHARCON); }
{TOK_STRINGCON} { return yylval_token (TOK_STRINGCON); }
{TOK_IDENT}     { return yylval_token (TOK_IDENT);}

{INVALID_CHARACTER} { lexer::badtoken (yytext);
                      return yylval_token (TOK_CHARCON); }
{INVALID_STRING} {  lexer::badtoken(yytext);
                    return yylval_token (TOK_STRINGCON); }

.               { lexer::badchar (*yytext); }

%%


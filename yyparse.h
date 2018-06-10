/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_YYPARSE_HPP_INCLUDED
# define YY_YY_YYPARSE_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_ROOT = 258,
    TOK_TYPEID = 259,
    TOK_FIELD = 260,
    TOK_DECLID = 261,
    TOK_PARAMS = 262,
    TOK_POS = 263,
    TOK_NEG = 264,
    TOK_CALL = 265,
    TOK_VARDECL = 266,
    TOK_INDEX = 267,
    TOK_NEWARRAY = 268,
    TOK_NEWSTR = 269,
    TOK_BLOCK = 270,
    TOK_FUNCTION = 271,
    TOK_PARAM = 272,
    TOK_PROTO = 273,
    TOK_PROTOTYPE = 274,
    TOK_ARRAY = 275,
    TOK_KW_VOID = 276,
    TOK_KW_IF = 277,
    TOK_KW_ELSE = 278,
    TOK_KW_WHILE = 279,
    TOK_KW_RETURN = 280,
    TOK_KW_STRUCT = 281,
    TOK_KW_NULL = 282,
    TOK_KW_INT = 283,
    TOK_KW_CHAR = 284,
    TOK_KW_STRING = 285,
    TOK_IDENT = 286,
    TOK_INTCON = 287,
    TOK_CHARCON = 288,
    TOK_STRINGCON = 289,
    TOK_POINTER_GET = 290,
    TOK_DOUBLE_EQ = 291,
    TOK_NOT_EQ = 292,
    TOK_BRACKETS = 293,
    TOK_FIELDS = 294,
    INVALID_CHARACTER = 295,
    INVALID_STRING = 296,
    TOK_LESS_TH_EQ = 297,
    TOK_LARGE_TH_EQ = 298,
    TOK_KW_NOT = 299,
    TOK_KW_NEW = 300,
    TOK_KW_BOOL = 301,
    TOK_KW_TRUE = 302,
    TOK_KW_FALSE = 303
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_YYPARSE_HPP_INCLUDED  */

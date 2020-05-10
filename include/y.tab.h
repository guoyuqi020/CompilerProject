
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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

/* Tokens.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
/* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
#include "IR.h"

Boost::Internal::Ref<const Boost::Internal::IRNode> yyparse_string(char *);

enum yytokentype
{
   Id = 258,
   INTEGER = 259,
   FLOAT = 260,
   DELIM = 261,
   WHITESPACE = 262,
   LEFTBRACKET = 263,
   RIGHTBRACKET = 264,
   LEFTANGLE = 265,
   RIGHTANGLE = 266,
   EXACTLYDIVIDE = 267
};
#endif
/* Tokens.  */
#define Id 258
#define INTEGER 259
#define FLOAT 260
#define DELIM 261
#define WHITESPACE 262
#define LEFTBRACKET 263
#define RIGHTBRACKET 264
#define LEFTANGLE 265
#define RIGHTANGLE 266
#define EXACTLYDIVIDE 267

#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define yystype YYSTYPE /* obsolescent; will be withdrawn */
#define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;
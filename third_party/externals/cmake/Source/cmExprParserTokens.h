/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison, Copyright (C) 1984,
   1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     exp_PLUS = 258,
     exp_MINUS = 259,
     exp_TIMES = 260,
     exp_DIVIDE = 261,
     exp_MOD = 262,
     exp_SHIFTLEFT = 263,
     exp_SHIFTRIGHT = 264,
     exp_OPENPARENT = 265,
     exp_CLOSEPARENT = 266,
     exp_OR = 267,
     exp_AND = 268,
     exp_XOR = 269,
     exp_NOT = 270,
     exp_NUMBER = 271
   };
#endif
#define exp_PLUS 258
#define exp_MINUS 259
#define exp_TIMES 260
#define exp_DIVIDE 261
#define exp_MOD 262
#define exp_SHIFTLEFT 263
#define exp_SHIFTRIGHT 264
#define exp_OPENPARENT 265
#define exp_CLOSEPARENT 266
#define exp_OR 267
#define exp_AND 268
#define exp_XOR 269
#define exp_NOT 270
#define exp_NUMBER 271




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif






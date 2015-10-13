/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C

      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.

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
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     EOSTMT = 258,
     ASSIGNMENT_OP = 259,
     GARBAGE = 260,
     CPP_INCLUDE = 261,
     F90PPR_INCLUDE = 262,
     COCO_INCLUDE = 263,
     F90PPR_DEFINE = 264,
     CPP_DEFINE = 265,
     F90PPR_UNDEF = 266,
     CPP_UNDEF = 267,
     CPP_IFDEF = 268,
     CPP_IFNDEF = 269,
     CPP_IF = 270,
     CPP_ELSE = 271,
     CPP_ELIF = 272,
     CPP_ENDIF = 273,
     F90PPR_IFDEF = 274,
     F90PPR_IFNDEF = 275,
     F90PPR_IF = 276,
     F90PPR_ELSE = 277,
     F90PPR_ELIF = 278,
     F90PPR_ENDIF = 279,
     COMMA = 280,
     DCOLON = 281,
     CPP_TOENDL = 282,
     UNTERMINATED_STRING = 283,
     STRING = 284,
     WORD = 285,
     CPP_INCLUDE_ANGLE = 286
   };
#endif
/* Tokens.  */
#define EOSTMT 258
#define ASSIGNMENT_OP 259
#define GARBAGE 260
#define CPP_INCLUDE 261
#define F90PPR_INCLUDE 262
#define COCO_INCLUDE 263
#define F90PPR_DEFINE 264
#define CPP_DEFINE 265
#define F90PPR_UNDEF 266
#define CPP_UNDEF 267
#define CPP_IFDEF 268
#define CPP_IFNDEF 269
#define CPP_IF 270
#define CPP_ELSE 271
#define CPP_ELIF 272
#define CPP_ENDIF 273
#define F90PPR_IFDEF 274
#define F90PPR_IFNDEF 275
#define F90PPR_IF 276
#define F90PPR_ELSE 277
#define F90PPR_ELIF 278
#define F90PPR_ENDIF 279
#define COMMA 280
#define DCOLON 281
#define CPP_TOENDL 282
#define UNTERMINATED_STRING 283
#define STRING 284
#define WORD 285
#define CPP_INCLUDE_ANGLE 286




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 89 "cmDependsFortranParser.y"

  char* string;



/* Line 2068 of yacc.c  */
#line 118 "cmDependsFortranParserTokens.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

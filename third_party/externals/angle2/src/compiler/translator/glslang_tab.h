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

#ifndef YY_YY_GLSLANG_TAB_H_INCLUDED
# define YY_YY_GLSLANG_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */


#define YYLTYPE TSourceLoc
#define YYLTYPE_IS_DECLARED 1



/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INVARIANT = 258,
    HIGH_PRECISION = 259,
    MEDIUM_PRECISION = 260,
    LOW_PRECISION = 261,
    PRECISION = 262,
    ATTRIBUTE = 263,
    CONST_QUAL = 264,
    BOOL_TYPE = 265,
    FLOAT_TYPE = 266,
    INT_TYPE = 267,
    UINT_TYPE = 268,
    BREAK = 269,
    CONTINUE = 270,
    DO = 271,
    ELSE = 272,
    FOR = 273,
    IF = 274,
    DISCARD = 275,
    RETURN = 276,
    SWITCH = 277,
    CASE = 278,
    DEFAULT = 279,
    BVEC2 = 280,
    BVEC3 = 281,
    BVEC4 = 282,
    IVEC2 = 283,
    IVEC3 = 284,
    IVEC4 = 285,
    VEC2 = 286,
    VEC3 = 287,
    VEC4 = 288,
    UVEC2 = 289,
    UVEC3 = 290,
    UVEC4 = 291,
    MATRIX2 = 292,
    MATRIX3 = 293,
    MATRIX4 = 294,
    IN_QUAL = 295,
    OUT_QUAL = 296,
    INOUT_QUAL = 297,
    UNIFORM = 298,
    VARYING = 299,
    MATRIX2x3 = 300,
    MATRIX3x2 = 301,
    MATRIX2x4 = 302,
    MATRIX4x2 = 303,
    MATRIX3x4 = 304,
    MATRIX4x3 = 305,
    CENTROID = 306,
    FLAT = 307,
    SMOOTH = 308,
    STRUCT = 309,
    VOID_TYPE = 310,
    WHILE = 311,
    SAMPLER2D = 312,
    SAMPLERCUBE = 313,
    SAMPLER_EXTERNAL_OES = 314,
    SAMPLER2DRECT = 315,
    SAMPLER2DARRAY = 316,
    ISAMPLER2D = 317,
    ISAMPLER3D = 318,
    ISAMPLERCUBE = 319,
    ISAMPLER2DARRAY = 320,
    USAMPLER2D = 321,
    USAMPLER3D = 322,
    USAMPLERCUBE = 323,
    USAMPLER2DARRAY = 324,
    SAMPLER3D = 325,
    SAMPLER3DRECT = 326,
    SAMPLER2DSHADOW = 327,
    SAMPLERCUBESHADOW = 328,
    SAMPLER2DARRAYSHADOW = 329,
    LAYOUT = 330,
    IDENTIFIER = 331,
    TYPE_NAME = 332,
    FLOATCONSTANT = 333,
    INTCONSTANT = 334,
    UINTCONSTANT = 335,
    BOOLCONSTANT = 336,
    FIELD_SELECTION = 337,
    LEFT_OP = 338,
    RIGHT_OP = 339,
    INC_OP = 340,
    DEC_OP = 341,
    LE_OP = 342,
    GE_OP = 343,
    EQ_OP = 344,
    NE_OP = 345,
    AND_OP = 346,
    OR_OP = 347,
    XOR_OP = 348,
    MUL_ASSIGN = 349,
    DIV_ASSIGN = 350,
    ADD_ASSIGN = 351,
    MOD_ASSIGN = 352,
    LEFT_ASSIGN = 353,
    RIGHT_ASSIGN = 354,
    AND_ASSIGN = 355,
    XOR_ASSIGN = 356,
    OR_ASSIGN = 357,
    SUB_ASSIGN = 358,
    LEFT_PAREN = 359,
    RIGHT_PAREN = 360,
    LEFT_BRACKET = 361,
    RIGHT_BRACKET = 362,
    LEFT_BRACE = 363,
    RIGHT_BRACE = 364,
    DOT = 365,
    COMMA = 366,
    COLON = 367,
    EQUAL = 368,
    SEMICOLON = 369,
    BANG = 370,
    DASH = 371,
    TILDE = 372,
    PLUS = 373,
    STAR = 374,
    SLASH = 375,
    PERCENT = 376,
    LEFT_ANGLE = 377,
    RIGHT_ANGLE = 378,
    VERTICAL_BAR = 379,
    CARET = 380,
    AMPERSAND = 381,
    QUESTION = 382
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


    struct {
        union {
            TString *string;
            float f;
            int i;
            unsigned int u;
            bool b;
        };
        TSymbol* symbol;
    } lex;
    struct {
        TOperator op;
        union {
            TIntermNode* intermNode;
            TIntermNodePair nodePair;
            TIntermTyped* intermTypedNode;
            TIntermAggregate* intermAggregate;
            TIntermSwitch* intermSwitch;
            TIntermCase* intermCase;
        };
        union {
            TPublicType type;
            TPrecision precision;
            TLayoutQualifier layoutQualifier;
            TQualifier qualifier;
            TFunction* function;
            TParameter param;
            TField* field;
            TFieldList* fieldList;
        };
    } interm;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (TParseContext* context, void *scanner);

#endif /* !YY_YY_GLSLANG_TAB_H_INCLUDED  */

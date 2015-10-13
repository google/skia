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
     jp_ABSTRACT = 258,
     jp_ASSERT = 259,
     jp_BOOLEAN_TYPE = 260,
     jp_BREAK = 261,
     jp_BYTE_TYPE = 262,
     jp_CASE = 263,
     jp_CATCH = 264,
     jp_CHAR_TYPE = 265,
     jp_CLASS = 266,
     jp_CONTINUE = 267,
     jp_DEFAULT = 268,
     jp_DO = 269,
     jp_DOUBLE_TYPE = 270,
     jp_ELSE = 271,
     jp_EXTENDS = 272,
     jp_FINAL = 273,
     jp_FINALLY = 274,
     jp_FLOAT_TYPE = 275,
     jp_FOR = 276,
     jp_IF = 277,
     jp_IMPLEMENTS = 278,
     jp_IMPORT = 279,
     jp_INSTANCEOF = 280,
     jp_INT_TYPE = 281,
     jp_INTERFACE = 282,
     jp_LONG_TYPE = 283,
     jp_NATIVE = 284,
     jp_NEW = 285,
     jp_PACKAGE = 286,
     jp_PRIVATE = 287,
     jp_PROTECTED = 288,
     jp_PUBLIC = 289,
     jp_RETURN = 290,
     jp_SHORT_TYPE = 291,
     jp_STATIC = 292,
     jp_STRICTFP = 293,
     jp_SUPER = 294,
     jp_SWITCH = 295,
     jp_SYNCHRONIZED = 296,
     jp_THIS = 297,
     jp_THROW = 298,
     jp_THROWS = 299,
     jp_TRANSIENT = 300,
     jp_TRY = 301,
     jp_VOID = 302,
     jp_VOLATILE = 303,
     jp_WHILE = 304,
     jp_BOOLEANLITERAL = 305,
     jp_CHARACTERLITERAL = 306,
     jp_DECIMALINTEGERLITERAL = 307,
     jp_FLOATINGPOINTLITERAL = 308,
     jp_HEXINTEGERLITERAL = 309,
     jp_NULLLITERAL = 310,
     jp_STRINGLITERAL = 311,
     jp_NAME = 312,
     jp_AND = 313,
     jp_ANDAND = 314,
     jp_ANDEQUALS = 315,
     jp_BRACKETEND = 316,
     jp_BRACKETSTART = 317,
     jp_CARROT = 318,
     jp_CARROTEQUALS = 319,
     jp_COLON = 320,
     jp_COMMA = 321,
     jp_CURLYEND = 322,
     jp_CURLYSTART = 323,
     jp_DIVIDE = 324,
     jp_DIVIDEEQUALS = 325,
     jp_DOLLAR = 326,
     jp_DOT = 327,
     jp_EQUALS = 328,
     jp_EQUALSEQUALS = 329,
     jp_EXCLAMATION = 330,
     jp_EXCLAMATIONEQUALS = 331,
     jp_GREATER = 332,
     jp_GTEQUALS = 333,
     jp_GTGT = 334,
     jp_GTGTEQUALS = 335,
     jp_GTGTGT = 336,
     jp_GTGTGTEQUALS = 337,
     jp_LESLESEQUALS = 338,
     jp_LESSTHAN = 339,
     jp_LTEQUALS = 340,
     jp_LTLT = 341,
     jp_MINUS = 342,
     jp_MINUSEQUALS = 343,
     jp_MINUSMINUS = 344,
     jp_PAREEND = 345,
     jp_PARESTART = 346,
     jp_PERCENT = 347,
     jp_PERCENTEQUALS = 348,
     jp_PIPE = 349,
     jp_PIPEEQUALS = 350,
     jp_PIPEPIPE = 351,
     jp_PLUS = 352,
     jp_PLUSEQUALS = 353,
     jp_PLUSPLUS = 354,
     jp_QUESTION = 355,
     jp_SEMICOL = 356,
     jp_TILDE = 357,
     jp_TIMES = 358,
     jp_TIMESEQUALS = 359,
     jp_ERROR = 360
   };
#endif
#define jp_ABSTRACT 258
#define jp_ASSERT 259
#define jp_BOOLEAN_TYPE 260
#define jp_BREAK 261
#define jp_BYTE_TYPE 262
#define jp_CASE 263
#define jp_CATCH 264
#define jp_CHAR_TYPE 265
#define jp_CLASS 266
#define jp_CONTINUE 267
#define jp_DEFAULT 268
#define jp_DO 269
#define jp_DOUBLE_TYPE 270
#define jp_ELSE 271
#define jp_EXTENDS 272
#define jp_FINAL 273
#define jp_FINALLY 274
#define jp_FLOAT_TYPE 275
#define jp_FOR 276
#define jp_IF 277
#define jp_IMPLEMENTS 278
#define jp_IMPORT 279
#define jp_INSTANCEOF 280
#define jp_INT_TYPE 281
#define jp_INTERFACE 282
#define jp_LONG_TYPE 283
#define jp_NATIVE 284
#define jp_NEW 285
#define jp_PACKAGE 286
#define jp_PRIVATE 287
#define jp_PROTECTED 288
#define jp_PUBLIC 289
#define jp_RETURN 290
#define jp_SHORT_TYPE 291
#define jp_STATIC 292
#define jp_STRICTFP 293
#define jp_SUPER 294
#define jp_SWITCH 295
#define jp_SYNCHRONIZED 296
#define jp_THIS 297
#define jp_THROW 298
#define jp_THROWS 299
#define jp_TRANSIENT 300
#define jp_TRY 301
#define jp_VOID 302
#define jp_VOLATILE 303
#define jp_WHILE 304
#define jp_BOOLEANLITERAL 305
#define jp_CHARACTERLITERAL 306
#define jp_DECIMALINTEGERLITERAL 307
#define jp_FLOATINGPOINTLITERAL 308
#define jp_HEXINTEGERLITERAL 309
#define jp_NULLLITERAL 310
#define jp_STRINGLITERAL 311
#define jp_NAME 312
#define jp_AND 313
#define jp_ANDAND 314
#define jp_ANDEQUALS 315
#define jp_BRACKETEND 316
#define jp_BRACKETSTART 317
#define jp_CARROT 318
#define jp_CARROTEQUALS 319
#define jp_COLON 320
#define jp_COMMA 321
#define jp_CURLYEND 322
#define jp_CURLYSTART 323
#define jp_DIVIDE 324
#define jp_DIVIDEEQUALS 325
#define jp_DOLLAR 326
#define jp_DOT 327
#define jp_EQUALS 328
#define jp_EQUALSEQUALS 329
#define jp_EXCLAMATION 330
#define jp_EXCLAMATIONEQUALS 331
#define jp_GREATER 332
#define jp_GTEQUALS 333
#define jp_GTGT 334
#define jp_GTGTEQUALS 335
#define jp_GTGTGT 336
#define jp_GTGTGTEQUALS 337
#define jp_LESLESEQUALS 338
#define jp_LESSTHAN 339
#define jp_LTEQUALS 340
#define jp_LTLT 341
#define jp_MINUS 342
#define jp_MINUSEQUALS 343
#define jp_MINUSMINUS 344
#define jp_PAREEND 345
#define jp_PARESTART 346
#define jp_PERCENT 347
#define jp_PERCENTEQUALS 348
#define jp_PIPE 349
#define jp_PIPEEQUALS 350
#define jp_PIPEPIPE 351
#define jp_PLUS 352
#define jp_PLUSEQUALS 353
#define jp_PLUSPLUS 354
#define jp_QUESTION 355
#define jp_SEMICOL 356
#define jp_TILDE 357
#define jp_TIMES 358
#define jp_TIMESEQUALS 359
#define jp_ERROR 360




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif






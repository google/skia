#ifndef RE2C_PARSER_H
#define RE2C_PARSER_H

/* Tokens */
enum yytokentype {
    CLOSESIZE = 258,
    CLOSE = 259,
    ID = 260,
    CODE = 261,
    RANGE = 262,
    STRING = 263,
    NONE = 264
};

#define CLOSESIZE 258
#define CLOSE 259
#define ID 260
#define CODE 261
#define RANGE 262
#define STRING 263
#define NONE 264

typedef union {
    Symbol	*symbol;
    RegExp	*regexp;
    Token	*token;
    char	op;
    ExtOp	extop;
} yystype;

extern yystype yylval;

#endif

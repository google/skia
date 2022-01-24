// *****************
// *** IMPORTANT ***
// *****************
//
// 1. This file is only used when gn arg sksl_lex is set to true. It is used to regenerate the
//    SkSLLexer.h and SkSLLexer.cpp files.
// 2. Since token IDs are used to identify operators and baked into the .dehydrated.sksl files,
//    after modifying this file it is likely everything will break until you update the dehydrated
//    binaries. If things break after updating the lexer, set REHYDRATE in SkSLCompiler.cpp to 0,
//    rebuild, and then set it back to 1.

FLOAT_LITERAL  = [0-9]*\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|[0-9]+([eE][+-]?[0-9]+)
INT_LITERAL    = ([1-9][0-9]*|0[0-7]*|0[xX][0-9a-fA-F]+)[uU]?
BAD_OCTAL      = (0[0-9]+)[uU]?
TRUE_LITERAL   = "true"
FALSE_LITERAL  = "false"
IF             = "if"
STATIC_IF      = "@if"
ELSE           = "else"
FOR            = "for"
WHILE          = "while"
DO             = "do"
SWITCH         = "switch"
STATIC_SWITCH  = "@switch"
CASE           = "case"
DEFAULT        = "default"
BREAK          = "break"
CONTINUE       = "continue"
DISCARD        = "discard"
RETURN         = "return"
IN             = "in"
OUT            = "out"
INOUT          = "inout"
UNIFORM        = "uniform"
CONST          = "const"
FLAT           = "flat"
NOPERSPECTIVE  = "noperspective"
INLINE         = "inline"
NOINLINE       = "noinline"
HASSIDEEFFECTS = "sk_has_side_effects"
STRUCT         = "struct"
LAYOUT         = "layout"
HIGHP          = "highp"
MEDIUMP        = "mediump"
LOWP           = "lowp"
ES3            = "$es3"
RESERVED       = attribute|varying|precision|invariant|asm|class|union|enum|typedef|template|this|packed|goto|volatile|public|static|extern|external|interface|long|double|fixed|unsigned|superp|input|output|hvec[234]|dvec[234]|fvec[234]|sampler[12]DShadow|sampler3DRect|sampler2DRectShadow|samplerCube|sizeof|cast|namespace|using|gl_[0-9a-zA-Z_]*
IDENTIFIER     = [a-zA-Z_$][0-9a-zA-Z_]*
DIRECTIVE      = #[a-zA-Z_][0-9a-zA-Z_]*
LPAREN         = "("
RPAREN         = ")"
LBRACE         = "{"
RBRACE         = "}"
LBRACKET       = "["
RBRACKET       = "]"
DOT            = "."
COMMA          = ","
PLUSPLUS       = "++"
MINUSMINUS     = "--"
PLUS           = "+"
MINUS          = "-"
STAR           = "*"
SLASH          = "/"
PERCENT        = "%"
SHL            = "<<"
SHR            = ">>"
BITWISEOR      = "|"
BITWISEXOR     = "^"
BITWISEAND     = "&"
BITWISENOT     = "~"
LOGICALOR      = "||"
LOGICALXOR     = "^^"
LOGICALAND     = "&&"
LOGICALNOT     = "!"
QUESTION       = "?"
COLON          = ":"
EQ             = "="
EQEQ           = "=="
NEQ            = "!="
GT             = ">"
LT             = "<"
GTEQ           = ">="
LTEQ           = "<="
PLUSEQ         = "+="
MINUSEQ        = "-="
STAREQ         = "*="
SLASHEQ        = "/="
PERCENTEQ      = "%="
SHLEQ          = "<<="
SHREQ          = ">>="
BITWISEOREQ    = "|="
BITWISEXOREQ   = "^="
BITWISEANDEQ   = "&="
SEMICOLON      = ";"
WHITESPACE     = \s+
LINE_COMMENT   = //.*
BLOCK_COMMENT  = /\*([^*]|\*[^/])*\*/
INVALID        = .

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*

    This file is IGNORED during the build process!

    As this file is updated so infrequently and flex is not universally present on build machines,
    the lex.sksl.c file must be manually regenerated if you make any changes to this file. Just run:

        flex sksl.flex

    You will have to manually add a copyright notice to the top of lex.sksl.c.

*/

%option prefix="sksl"
%option reentrant
%option yylineno
%option never-interactive
%option nounistd

DIGIT  [0-9]
LETTER [a-zA-Z_$]

%%

{DIGIT}*"."{DIGIT}+([eE][+-]?{DIGIT}+)? { return SkSL::Token::FLOAT_LITERAL; }

{DIGIT}+"."{DIGIT}*([eE][+-]?{DIGIT}+)? { return SkSL::Token::FLOAT_LITERAL; }

{DIGIT}+([eE][+-]?{DIGIT}+) { return SkSL::Token::FLOAT_LITERAL; }

{DIGIT}+ { return SkSL::Token::INT_LITERAL; }

"0x"[0-9a-fA-F]+ { return SkSL::Token::INT_LITERAL; }

true { return SkSL::Token::TRUE_LITERAL; }

false { return SkSL::Token::FALSE_LITERAL; }

if { return SkSL::Token::IF; }

@if { return SkSL::Token::STATIC_IF; }

else { return SkSL::Token::ELSE; }

for { return SkSL::Token::FOR; }

while { return SkSL::Token::WHILE; }

do { return SkSL::Token::DO; }

switch { return SkSL::Token::SWITCH; }

@switch { return SkSL::Token::STATIC_SWITCH; }

case { return SkSL::Token::CASE; }

default { return SkSL::Token::DEFAULT; }

break { return SkSL::Token::BREAK; }

continue { return SkSL::Token::CONTINUE; }

discard { return SkSL::Token::DISCARD; }

return { return SkSL::Token::RETURN; }

in { return SkSL::Token::IN; }

out { return SkSL::Token::OUT; }

inout { return SkSL::Token::INOUT; }

uniform { return SkSL::Token::UNIFORM; }

const { return SkSL::Token::CONST; }

lowp { return SkSL::Token::LOWP; }

mediump { return SkSL::Token::MEDIUMP; }

highp { return SkSL::Token::HIGHP; }

flat { return SkSL::Token::FLAT; }

noperspective { return SkSL::Token::NOPERSPECTIVE; }

readonly { return SkSL::Token::READONLY; }

writeonly { return SkSL::Token::WRITEONLY; }

coherent { return SkSL::Token::COHERENT; }

volatile { return SkSL::Token::VOLATILE; }

restrict { return SkSL::Token::RESTRICT; }

buffer { return SkSL::Token::BUFFER; }

sk_has_side_effects { return SkSL::Token::HASSIDEEFFECTS; }

struct { return SkSL::Token::STRUCT; }

layout { return SkSL::Token::LAYOUT; }

precision { return SkSL::Token::PRECISION; }

{LETTER}({DIGIT}|{LETTER})* { return SkSL::Token::IDENTIFIER; }

"#"{LETTER}({DIGIT}|{LETTER})* { return SkSL::Token::DIRECTIVE; }

"@"{LETTER}({DIGIT}|{LETTER})* { return SkSL::Token::SECTION; }

"(" { return SkSL::Token::LPAREN; }

")" { return SkSL::Token::RPAREN; }

"{" { return SkSL::Token::LBRACE; }

"}" { return SkSL::Token::RBRACE; }

"[" { return SkSL::Token::LBRACKET; }

"]" { return SkSL::Token::RBRACKET; }

"." { return SkSL::Token::DOT; }

"," { return SkSL::Token::COMMA; }

"++" { return SkSL::Token::PLUSPLUS; }

"--" { return SkSL::Token::MINUSMINUS; }

"+" { return SkSL::Token::PLUS; }

"-" { return SkSL::Token::MINUS; }

"*" { return SkSL::Token::STAR; }

"/" { return SkSL::Token::SLASH; }

"%" { return SkSL::Token::PERCENT; }

"<<" { return SkSL::Token::SHL; }

">>" { return SkSL::Token::SHR; }

"|" { return SkSL::Token::BITWISEOR; }

"^" { return SkSL::Token::BITWISEXOR; }

"&" { return SkSL::Token::BITWISEAND; }

"~" { return SkSL::Token::BITWISENOT; }

"||" { return SkSL::Token::LOGICALOR; }

"^^" { return SkSL::Token::LOGICALXOR; }

"&&" { return SkSL::Token::LOGICALAND; }

"!" { return SkSL::Token::LOGICALNOT; }

"?" { return SkSL::Token::QUESTION; }

":" { return SkSL::Token::COLON; }

"=" { return SkSL::Token::EQ; }

"==" { return SkSL::Token::EQEQ; }

"!=" { return SkSL::Token::NEQ; }

">" { return SkSL::Token::GT; }

"<" { return SkSL::Token::LT; }

">=" { return SkSL::Token::GTEQ; }

"<=" { return SkSL::Token::LTEQ; }

"+=" { return SkSL::Token::PLUSEQ; }

"-=" { return SkSL::Token::MINUSEQ; }

"*=" { return SkSL::Token::STAREQ; }

"/=" { return SkSL::Token::SLASHEQ; }

"%=" { return SkSL::Token::PERCENTEQ; }

"<<=" { return SkSL::Token::SHLEQ; }

">>=" { return SkSL::Token::SHREQ; }

"|=" { return SkSL::Token::BITWISEOREQ; }

"^=" { return SkSL::Token::BITWISEXOREQ; }

"&=" { return SkSL::Token::BITWISEANDEQ; }

"||=" { return SkSL::Token::LOGICALOREQ; }

"^^=" { return SkSL::Token::LOGICALXOREQ; }

"&&=" { return SkSL::Token::LOGICALANDEQ; }

";" { return SkSL::Token::SEMICOLON; }

"->" { return SkSL::Token::ARROW; }

"::" { return SkSL::Token::COLONCOLON; }

[ \t\r\n]+ { return SkSL::Token::WHITESPACE; }

"//".* /* line comment */

"/*"([^*]|"*"[^/])*"*/" /* block comment */

.    { return SkSL::Token::INVALID_TOKEN; }

%%

int skslwrap(yyscan_t scanner) {
    return 1; // terminate
}

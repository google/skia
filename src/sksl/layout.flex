/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*

    This file is IGNORED during the build process!

    As this file is updated so infrequently and flex is not universally present on build machines,
    the lex.layout.c file must be manually regenerated if you make any changes to this file. Just
    run:

        flex layout.flex

    You will have to manually add a copyright notice to the top of lex.layout.c.

*/

%option prefix="layout"
%option reentrant
%option yylineno
%option never-interactive
%option nounistd

%{
#include "SkSLToken.h"
%}

%%

"location"                    { return SkSL::Token::LOCATION; }
"offset"                      { return SkSL::Token::OFFSET; }
"binding"                     { return SkSL::Token::BINDING; }
"index"                       { return SkSL::Token::INDEX; }
"set"                         { return SkSL::Token::SET; }
"builtin"                     { return SkSL::Token::BUILTIN; }
"input_attachment_index"      { return SkSL::Token::INPUT_ATTACHMENT_INDEX; }
"origin_upper_left"           { return SkSL::Token::ORIGIN_UPPER_LEFT; }
"override_coverage"           { return SkSL::Token::OVERRIDE_COVERAGE; }
"blend_support_all_equations" { return SkSL::Token::BLEND_SUPPORT_ALL_EQUATIONS; }
"push_constant"               { return SkSL::Token::PUSH_CONSTANT; }
"points"                      { return SkSL::Token::POINTS; }
"lines"                       { return SkSL::Token::LINES; }
"line_strip"                  { return SkSL::Token::LINE_STRIP; }
"lines_adjacency"             { return SkSL::Token::LINES_ADJACENCY; }
"triangles"                   { return SkSL::Token::TRIANGLES; }
"triangle_strip"              { return SkSL::Token::TRIANGLE_STRIP; }
"triangles_adjacency"         { return SkSL::Token::TRIANGLES_ADJACENCY; }
"max_vertices"                { return SkSL::Token::MAX_VERTICES; }
"invocations"                 { return SkSL::Token::INVOCATIONS; }

. { return SkSL::Token::INVALID_TOKEN; }

%%

int layoutwrap(yyscan_t scanner) {
    return 1; // terminate
}

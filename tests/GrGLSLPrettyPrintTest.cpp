/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU
#include "Test.h"
#include "gl/GrGLSLPrettyPrint.h"

#define ASSERT(x) REPORTER_ASSERT(r, x)

const SkString input1("#this is not a realshader\nvec4 some stuff;outside of a function;"
                     "int i(int b, int c) { { some stuff;} fake block; //comments\n return i;}"
                     "void main()"
                     "{nowin a function;{indenting;{abit more;dreadedfor((;;)(;)((;;);)){doingstuff"
                     ";for(;;;){and more stufff;mixed garbage\n\n\t\t\t\t\n/*using this"
                     " comment\n is"
                     " dangerous\ndo so at your own\n risk*/;\n\n\t\t\t\n"
                     "//a comment\n}}a; little ;  love; for   ; leading;  spaces;} "
                     "an struct = { int a; int b; };"
                     "int[5] arr = int[5](1,2,3,4,5);} some code at the bottom; for(;;) {} }");

const SkString output1(
        "   1\t#this is not a realshader\n"
        "   2\tvec4 some stuff;\n"
        "   3\toutside of a function;\n"
        "   4\tint i(int b, int c) \n"
        "   5\t{\n"
        "   6\t\t{\n"
        "   7\t\t\tsome stuff;\n"
        "   8\t\t}\n"
        "   9\t\tfake block;\n"
        "  10\t\t//comments\n"
        "  11\t\treturn i;\n"
        "  12\t}\n"
        "  13\tvoid main()\n"
        "  14\t{\n"
        "  15\t\tnowin a function;\n"
        "  16\t\t{\n"
        "  17\t\t\tindenting;\n"
        "  18\t\t\t{\n"
        "  19\t\t\t\tabit more;\n"
        "  20\t\t\t\tdreadedfor((;;)(;)((;;);))\n"
        "  21\t\t\t\t{\n"
        "  22\t\t\t\t\tdoingstuff;\n"
        "  23\t\t\t\t\tfor(;;;)\n"
        "  24\t\t\t\t\t{\n"
        "  25\t\t\t\t\t\tand more stufff;\n"
        "  26\t\t\t\t\t\tmixed garbage/*using this comment\n"
        "  27\t\t\t\t\t\t is dangerous\n"
        "  28\t\t\t\t\t\tdo so at your own\n"
        "  29\t\t\t\t\t\t risk*/;\n"
        "  30\t\t\t\t\t\t//a comment\n"
        "  31\t\t\t\t\t}\n"
        "  32\t\t\t\t}\n"
        "  33\t\t\t\ta;\n"
        "  34\t\t\t\tlittle ;\n"
        "  35\t\t\t\tlove;\n"
        "  36\t\t\t\tfor   ;\n"
        "  37\t\t\t\tleading;\n"
        "  38\t\t\t\tspaces;\n"
        "  39\t\t\t}\n"
        "  40\t\t\tan struct = \n"
        "  41\t\t\t{\n"
        "  42\t\t\t\tint a;\n"
        "  43\t\t\t\tint b;\n"
        "  44\t\t\t}\n"
        "  45\t\t\t;\n"
        "  46\t\t\tint[5] arr = int[5](1,2,3,4,5);\n"
        "  47\t\t}\n"
        "  48\t\tsome code at the bottom;\n"
        "  49\t\tfor(;;) \n"
        "  50\t\t{\n"
        "  51\t\t}\n"
        "  52\t}\n"
        "  53\t");

const SkString input2("{;;{{{{;;;{{{{{{{{{{{###\n##\n#####(((((((((((((unbalanced verything;;;"
        "}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}"
        ";;;;;;/////");

DEF_TEST(GrGLSLPrettyPrint, r) {
    SkString test = GrGLSLPrettyPrint::PrettyPrintGLSL(input1, true);
    ASSERT(output1 == test);

    // Just test we don't crash with garbage input
    ASSERT(GrGLSLPrettyPrint::PrettyPrintGLSL(input2, true).c_str() != NULL);
}

#endif

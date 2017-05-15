/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrSKSLPrettyPrint.h"

#define ASSERT(x) REPORTER_ASSERT(r, x)

const SkString input1("#this is not a realshader\nvec4 some stuff;outside of a function;"
                     "int i(int b, int c) { { some stuff;} fake block; //comments\n return i;}"
                     "void main()");
const SkString input2("{nowin a function;{indenting;{abit more;dreadedfor((;;)(;)((;;);)){"
                     "doingstuff"
                     ";for(;;;){and more stufff;mixed garbage\n\n\t\t\t\t\n/*using this"
                     " comment\n is");
const SkString input3(" dangerous\ndo so at your own\n risk*/;\n\n\t\t\t\n"
                     "//a comment");
const SkString input4("breaking in comment");
const SkString input5("continuing the comment");
const SkString input6("\n}}a; little ;  love; for   ; leading;  spaces;} "
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
        "  30\t\t\t\t\t\t//a commentbreaking in commentcontinuing the comment\n"
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

const SkString neg1("{;;{{{{;;;{{{{{{{{{{{");
const SkString neg2("###\n##\n#####(((((((((((((unbalanced verything;;;");
const SkString neg3("}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}"
        ";;;;;;/////");

DEF_TEST(GrSKSLPrettyPrint, r) {
    SkTArray<const char*> testStr;
    SkTArray<int> lengths;
    testStr.push_back(input1.c_str());
    lengths.push_back((int)input1.size());
    testStr.push_back(input2.c_str());
    lengths.push_back((int)input2.size());
    testStr.push_back(input3.c_str());
    lengths.push_back((int)input3.size());
    testStr.push_back(input4.c_str());
    lengths.push_back((int)input4.size());
    testStr.push_back(input5.c_str());
    lengths.push_back((int)input5.size());
    testStr.push_back(input6.c_str());
    lengths.push_back((int)input6.size());

    SkString test = GrSKSLPrettyPrint::PrettyPrint(testStr.begin(), lengths.begin(),
                                                   testStr.count(), true);
    ASSERT(output1 == test);

    testStr.reset();
    lengths.reset();
    testStr.push_back(neg1.c_str());
    lengths.push_back((int)neg1.size());
    testStr.push_back(neg2.c_str());
    lengths.push_back((int)neg2.size());
    testStr.push_back(neg3.c_str());
    lengths.push_back((int)neg3.size());

    // Just test we don't crash with garbage input
    ASSERT(GrSKSLPrettyPrint::PrettyPrint(testStr.begin(), lengths.begin(), 1,
                                          true).c_str() != nullptr);
}

#endif

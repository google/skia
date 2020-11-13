/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "tests/Test.h"

// Note that the optimizer will aggressively kill dead code and substitute constants in place of
// variables, so we have to jump through a few hoops to ensure that the code in these tests has the
// necessary side-effects to remain live. In some cases we rely on the optimizer not (yet) being
// smart enough to optimize around certain constructs; as the optimizer gets smarter it will
// undoubtedly end up breaking some of these tests. That is a good thing, as long as the new code is
// equivalent!

static void test(skiatest::Reporter* r,
                 const GrShaderCaps& caps,
                 const char* src,
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    settings.fInlineThreshold = 99999999;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkSL::String(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
        REPORTER_ASSERT(r, output != "");
        //SkDebugf("GLSL output:\n\n%s", output.c_str());
    }
}

DEF_TEST(SkSLGLSLTestbed, r) {
    // Add in your SkSL here.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(

void fn1()  { sk_FragColor.x = 0; }
void fn2()  { fn1(); fn1(); fn1(); }
void fn3()  { fn2(); fn2(); fn2(); }
void fn4()  { fn3(); fn3(); fn3(); }
void fn5()  { fn4(); fn4(); fn4(); }
void fn6()  { fn5(); fn5(); fn5(); }
void fn7()  { fn6(); fn6(); fn6(); }
void fn8()  { fn7(); fn7(); fn7(); }
void fn9()  { fn8(); fn8(); fn8(); }
void fnA()  { fn9(); fn9(); fn9(); }
void fnB()  { fnA(); fnA(); fnA(); }
void fnC()  { fnB(); fnB(); fnB(); }
void fnD()  { fnC(); fnC(); fnC(); }
void fnE()  { fnD(); fnD(); fnD(); }
void fnF()  { fnE(); fnE(); fnE(); }
void fnG()  { fnF(); fnF(); fnF(); }
void fnH()  { fnG(); fnG(); fnG(); }
void fnI()  { fnH(); fnH(); fnH(); }
void fnJ()  { fnI(); fnI(); fnI(); }
void fnK()  { fnJ(); fnJ(); fnJ(); }
void fnL()  { fnK(); fnK(); fnK(); }
void fnM()  { fnL(); fnL(); fnL(); }
void fnN()  { fnM(); fnM(); fnM(); }
void main() { fnN(); }

         )__SkSL__");
}

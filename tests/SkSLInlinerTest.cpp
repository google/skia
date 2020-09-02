/*
 * Copyright 2016 Google Inc.
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

static void test(skiatest::Reporter* r, const SkSL::Program::Settings& settings,
                 const char* src, const char* expectedGLSL, SkSL::Program::Inputs* inputs,
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Compiler compiler;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkSL::String(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, program);
    if (program) {
        *inputs = program->fInputs;
        REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
        if (program) {
            SkSL::String skExpected(expectedGLSL);
            if (output != skExpected) {
                SkDebugf("GLSL MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'",
                         src, expectedGLSL, output.c_str());
            }
            REPORTER_ASSERT(r, output == skExpected);
        }
    }
}

static void test(skiatest::Reporter* r, const GrShaderCaps& caps,
                 const char* src, const char* expectedGLSL,
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Program::Inputs inputs;
    test(r, settings, src, expectedGLSL, &inputs, kind);
}

DEF_TEST(SkSLFunctionInlineThreshold, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         "void tooBig(inout int x) {"
         "    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;"
         "    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;"
         "}"
         "void main() { int x = 0; tooBig(x); }",
         "#version 400\n"
         "void tooBig(inout int x) {\n"
         "    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n"
         "    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n"
         "    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n"
         "    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n    ++x;\n"
         "    ++x;\n    ++x;\n"
         "}\n"
         "void main() {\n"
         "    int x = 0;\n"
         "    tooBig(x);\n"
         "}\n"
         );
}

DEF_TEST(SkSLFunctionInlineKeywordOverridesThreshold, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         "inline void tooBig(inout int x) {"
         "    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;"
         "    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;"
         "}"
         "void main() { int y = 0; tooBig(y); }",
         "#version 400\n"
         "void main() {\n"
         "    int y = 0;\n"
         "    {\n"
         "        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n"
         "        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n"
         "        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n"
         "        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n"
         "        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n        ++y;\n"
         "        ++y;\n        ++y;\n        ++y;\n        ++y;\n"
         "    }\n"
         "\n"
         "}\n"
         );
}

DEF_TEST(SkSLFunctionUnableToInlineReturnsInsideLoop, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         "inline void cantActuallyInline(inout int x) {"
         "    for (;;) {"
         "        ++x;"
         "        if (x > 10) return;"
         "    }"
         "}"
         "void main() { int x = 0; cantActuallyInline(x); }",
         "#version 400\n"
         "void cantActuallyInline(inout int x) {\n"
         "    for (; ; ) {\n"
         "        ++x;\n"
         "        if (x > 10) return;\n"
         "    }\n"
         "}\n"
         "void main() {\n"
         "    int x = 0;\n"
         "    cantActuallyInline(x);\n"
         "}\n");
}

DEF_TEST(SkSLFunctionInlineWithUnmodifiedArgument, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         "half basic(half x) {"
         "    return x * 2;"
         "}"
         "void main() {"
         "    sk_FragColor.x = basic(1);"
         "    half y = 2;"
         "    sk_FragColor.y = basic(y);"
         "}",
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 2.0;\n"
         "\n"
         "    sk_FragColor.y = 4.0;\n"
         "\n"
         "}\n");
}

DEF_TEST(SkSLFunctionInlineWithModifiedArgument, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         "half parameterWrite(half x) {"
         "    x *= 2;"
         "    return x;"
         "}"
         "void main() {"
         "    sk_FragColor.x = parameterWrite(1);"
         "}",
R"__GLSL__(#version 400
out vec4 sk_FragColor;
void main() {
    float _0_parameterWrite;
    float _1_x = 1.0;
    {
        _1_x *= 2.0;
        _0_parameterWrite = _1_x;
    }

    sk_FragColor.x = _0_parameterWrite;

}
)__GLSL__");
}

DEF_TEST(SkSLFunctionInlineWithInoutArgument, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         "void outParameter(inout half x) {"
         "    x *= 2;"
         "}"
         "void main() {"
         "    half x = 1;"
         "    outParameter(x);"
         "    sk_FragColor.x = x;"
         "}",
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 1.0;\n"
         "    {\n"
         "        x *= 2.0;\n"
         "    }\n"
         "\n"
         "    sk_FragColor.x = x;\n"
         "}\n");
}

DEF_TEST(SkSLFunctionInlineWithNestedCall, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         "void foo(out half x) {"
         "    ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;"
         "    --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x;"
         "    x = 42;"
         "}"
         "half bar(half y) {"
         "    foo(y);"
         "    return y;"
         "}"
         "void main() {"
         "    half _1_y = 123;"  // make sure the inliner doesn't try to reuse this name
         "    half z = 0;"
         "    bar(z);"
         "    sk_FragColor.x = z;"
         "}",
R"__GLSL__(#version 400
out vec4 sk_FragColor;
void foo(out float x) {
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    x = 42.0;
}
void main() {
    float _2_y = 0.0;
    {
        foo(_2_y);
    }


    sk_FragColor.x = 0.0;
}
)__GLSL__");
}

DEF_TEST(SkSLFPTernaryExpressionsShouldNotInlineResults, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half count = 0;
             bool test(half4 v) {
                 return v.x <= 0.5;
             }
             half4 trueSide(half4 v) {
                 count += 1;
                 return half4(sin(v.x), sin(v.y), sin(v.z), sin(v.w));
             }
             half4 falseSide(half4 v) {
                 count += 1;
                 return half4(cos(v.y), cos(v.z), cos(v.w), cos(v.z));
             }
             void main() {
                 sk_FragColor = test(color) ? trueSide(color) : falseSide(color);
                 sk_FragColor *= count;
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
float count = 0.0;
vec4 trueSide(vec4 v) {
    count += 1.0;
    return vec4(sin(v.x), sin(v.y), sin(v.z), sin(v.w));
}
vec4 falseSide(vec4 v) {
    count += 1.0;
    return vec4(cos(v.y), cos(v.z), cos(v.w), cos(v.z));
}
void main() {
    bool _0_test;
    {
        _0_test = color.x <= 0.5;
    }

    sk_FragColor = _0_test ? trueSide(color) : falseSide(color);

    sk_FragColor *= count;
}
)__GLSL__");
}

DEF_TEST(SkSLFPShortCircuitEvaluationsCannotInlineRightHandSide, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             bool testA(half4 v) {
                 return v.x <= 0.5;
             }
             bool testB(half4 v) {
                 return v.x > 0.5;
             }
             void main() {
                 sk_FragColor = half4(0);
                 if (testA(color) && testB(color)) {
                    sk_FragColor = half4(0.5);
                 }
                 if (testB(color) || testA(color)) {
                    sk_FragColor = half4(1.0);
                 }
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
bool testA(vec4 v) {
    return v.x <= 0.5;
}
bool testB(vec4 v) {
    return v.x > 0.5;
}
void main() {
    sk_FragColor = vec4(0.0);
    bool _0_testA;
    {
        _0_testA = color.x <= 0.5;
    }

    if (_0_testA && testB(color)) {
        sk_FragColor = vec4(0.5);
    }

    bool _1_testB;
    {
        _1_testB = color.x > 0.5;
    }

    if (_1_testB || testA(color)) {
        sk_FragColor = vec4(1.0);
    }

}
)__GLSL__");
}

DEF_TEST(SkSLFPWhileTestCannotBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             bool shouldLoop(half4 v) {
                 return v.x < 0.5;
             }
             void main() {
                 sk_FragColor = half4(0);
                 while (shouldLoop(sk_FragColor)) {
                     sk_FragColor += half4(0.125);
                 }
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
bool shouldLoop(vec4 v) {
    return v.x < 0.5;
}
void main() {
    sk_FragColor = vec4(0.0);
    while (shouldLoop(sk_FragColor)) {
        sk_FragColor += vec4(0.125);
    }
}
)__GLSL__");
}

DEF_TEST(SkSLFPInlinedWhileBodyMustBeInAScope, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             half4 adjust(half4 v) {
                 return v + half4(0.125);
             }
             void main() {
                 sk_FragColor = half4(0);
                 while (sk_FragColor.x < 0.5)
                     sk_FragColor = adjust(sk_FragColor);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(0.0);
    while (sk_FragColor.x < 0.5) {
        vec4 _0_adjust;
        {
            _0_adjust = sk_FragColor + vec4(0.125);
        }

        sk_FragColor = _0_adjust;
    }
}
)__GLSL__");
}

DEF_TEST(SkSLFPDoWhileTestCannotBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             bool shouldLoop(half4 v) {
                 return v.x < 0.5;
             }
             void main() {
                 sk_FragColor = half4(0);
                 do {
                     sk_FragColor += half4(0.125);
                 } while (shouldLoop(sk_FragColor));
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
bool shouldLoop(vec4 v) {
    return v.x < 0.5;
}
void main() {
    sk_FragColor = vec4(0.0);
    do {
        sk_FragColor += vec4(0.125);
    } while (shouldLoop(sk_FragColor));
}
)__GLSL__");
}

DEF_TEST(SkSLFPInlinedDoWhileBodyMustBeInAScope, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             half4 adjust(half4 v) {
                 return v + half4(0.125);
             }
             void main() {
                 sk_FragColor = half4(0);
                 do
                     sk_FragColor = adjust(sk_FragColor);
                 while (sk_FragColor.x < 0.5);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(0.0);
    do {
        vec4 _0_adjust;
        {
            _0_adjust = sk_FragColor + vec4(0.125);
        }

        sk_FragColor = _0_adjust;
    } while (sk_FragColor.x < 0.5);
}
)__GLSL__");
}

DEF_TEST(SkSLFPOnlyForInitializerExpressionsCanBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             half4 initLoopVar() {
                 return half4(0.0625);
             }
             bool shouldLoop(half4 v) {
                 return v.x < 0.5;
             }
             half4 grow(half4 v) {
                 return v + half4(0.125);
             }
             void main() {
                 for (sk_FragColor = initLoopVar();
                      shouldLoop(sk_FragColor);
                      sk_FragColor = grow(sk_FragColor)) {
                 }
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
bool shouldLoop(vec4 v) {
    return v.x < 0.5;
}
vec4 grow(vec4 v) {
    return v + vec4(0.125);
}
void main() {
    for (sk_FragColor = vec4(0.0625);
    shouldLoop(sk_FragColor); sk_FragColor = grow(sk_FragColor)) {
    }
}
)__GLSL__");
}

DEF_TEST(SkSLFPInlinedForBodyMustBeInAScope, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             half4 adjust(half4 v) {
                 return v + half4(0.125);
             }
             void main() {
                 sk_FragColor = half4(0);
                 for (int x=0; x<4; ++x)
                     sk_FragColor = adjust(sk_FragColor);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(0.0);
    for (int x = 0;x < 4; ++x) {
        vec4 _0_adjust;
        {
            _0_adjust = sk_FragColor + vec4(0.125);
        }

        sk_FragColor = _0_adjust;
    }
}
)__GLSL__");
}

DEF_TEST(SkSLFPIfTestsCanBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             bool ifTest(half4 v) {
                 return color.x >= 0.5;
             }
             void main() {
                 if (ifTest(color))
                     sk_FragColor = half4(1.0);
                 else
                     sk_FragColor = half4(0.5);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    bool _0_ifTest;
    {
        _0_ifTest = color.x >= 0.5;
    }

    if (_0_ifTest) sk_FragColor = vec4(1.0); else sk_FragColor = vec4(0.5);

}
)__GLSL__");
}

DEF_TEST(SkSLFPInlinedIfBodyMustBeInAScope, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 ifBody() {
                 return color + half4(0.125);
             }
             void main() {
                 half4 c = color;
                 if (c.x >= 0.5)
                     c = ifBody();
                 sk_FragColor = c;
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 c = color;
    if (c.x >= 0.5) {
        vec4 _0_ifBody;
        {
            _0_ifBody = color + vec4(0.125);
        }

        c = _0_ifBody;
    }
    sk_FragColor = c;
}
)__GLSL__");
}

DEF_TEST(SkSLFPInlinedElseBodyMustBeInAScope, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 elseBody() {
                 return color + half4(0.125);
             }
             void main() {
                 half4 c = color;
                 if (c.x >= 0.5)
                     ;
                 else
                     c = elseBody();
                 sk_FragColor = c;
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 c = color;
    if (c.x >= 0.5) {
    } else {
        vec4 _0_elseBody;
        {
            _0_elseBody = color + vec4(0.125);
        }

        c = _0_elseBody;
    }
    sk_FragColor = c;
}
)__GLSL__");
}

DEF_TEST(SkSLFPSwitchWithReturnInsideCannotBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 switchy(half4 c) {
                 switch (int(c.x)) {
                     case 0: return c.yyyy;
                 }
                 return c.zzzz;
             }
             void main() {
                 sk_FragColor = switchy(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
vec4 switchy(vec4 c) {
    switch (int(c.x)) {
        case 0:
            return c.yyyy;
    }
    return c.zzzz;
}
void main() {
    sk_FragColor = switchy(color);
}
)__GLSL__");
}

DEF_TEST(SkSLFPSwitchWithoutReturnInsideCanBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 switchy(half4 c) {
                 half4 result;
                 switch (int(c.x)) {
                     case 0: result = c.yyyy;
                 }
                 result = c.zzzz;
                 return result;
             }
             void main() {
                 sk_FragColor = switchy(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_switchy;
    {
        vec4 result;
        switch (int(color.x)) {
            case 0:
                result = color.yyyy;
        }
        result = color.zzzz;
        _0_switchy = result;
    }

    sk_FragColor = _0_switchy;

}
)__GLSL__");
}

DEF_TEST(SkSLFPForLoopWithReturnInsideCannotBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 loopy(half4 c) {
                 for (int x=0; x<5; ++x) {
                     if (x == int(c.w)) return c.yyyy;
                 }
                 return c.zzzz;
             }
             void main() {
                 sk_FragColor = loopy(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
vec4 loopy(vec4 c) {
    for (int x = 0;x < 5; ++x) {
        if (x == int(c.w)) return c.yyyy;
    }
    return c.zzzz;
}
void main() {
    sk_FragColor = loopy(color);
}
)__GLSL__");
}

DEF_TEST(SkSLFPSwitchWithCastCanBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 switchy(half4 c) {
                 half4 result;
                 switch (int(c.x)) {
                     case 1: result = c.yyyy; break;
                     default: result = c.zzzz; break;
                 }
                 return result;
             }
             void main() {
                 sk_FragColor = switchy(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_switchy;
    {
        vec4 result;
        switch (int(color.x)) {
            case 1:
                result = color.yyyy;
                break;
            default:
                result = color.zzzz;
                break;
        }
        _0_switchy = result;
    }

    sk_FragColor = _0_switchy;

}
)__GLSL__");
}

DEF_TEST(SkSLFPForLoopWithoutReturnInsideCanBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 loopy(half4 c) {
                 half4 pix;
                 for (int x=0; x<5; ++x) {
                     if (x == int(c.w)) pix = c.yyyy;
                 }
                 pix = c.zzzz;
                 return pix;
             }
             void main() {
                 sk_FragColor = loopy(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_loopy;
    {
        vec4 pix;
        for (int x = 0;x < 5; ++x) {
            if (x == int(color.w)) pix = color.yyyy;
        }
        pix = color.zzzz;
        _0_loopy = pix;
    }

    sk_FragColor = _0_loopy;

}
)__GLSL__");
}

DEF_TEST(SkSLFPInlinerManglesOverlappingNames, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half add(half a, half b) {
                 half c = a + b;
                 return c;
             }
             half mul(half a, half b) {
                 return a * b;
             }
             half fma(half a, half b, half c) {
                 return add(mul(a, b), c);
             }
             half4 main() {
                 half a = fma(color.x, color.y, color.z);
                 half b = fma(color.y, color.z, color.w);
                 half c = fma(color.z, color.w, color.x);
                 return half4(a, b, mul(c, c), mul(a, mul(b, c)));
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
uniform vec4 color;
vec4 main() {
    float _2_fma;
    float _3_a = color.x;
    float _4_b = color.y;
    float _5_c = color.z;
    {
        float _0_mul;
        {
            _0_mul = _3_a * _4_b;
        }

        float _1_add;
        {
            float c = _0_mul + _5_c;
            _1_add = c;
        }

        _2_fma = _1_add;

    }

    float a = _2_fma;

    float _6_fma;
    float _7_a = color.y;
    float _8_b = color.z;
    float _9_c = color.w;
    {
        float _0_mul;
        {
            _0_mul = _7_a * _8_b;
        }

        float _1_add;
        {
            float c = _0_mul + _9_c;
            _1_add = c;
        }

        _6_fma = _1_add;

    }

    float b = _6_fma;

    float _10_fma;
    float _11_a = color.z;
    float _12_b = color.w;
    float _13_c = color.x;
    {
        float _0_mul;
        {
            _0_mul = _11_a * _12_b;
        }

        float _1_add;
        {
            float c = _0_mul + _13_c;
            _1_add = c;
        }

        _10_fma = _1_add;

    }

    float c = _10_fma;

    float _14_mul;
    {
        _14_mul = c * c;
    }

    float _15_mul;
    {
        _15_mul = b * c;
    }

    float _16_mul;
    {
        _16_mul = a * _15_mul;
    }

    return vec4(a, b, _14_mul, _16_mul);

}
)__GLSL__");
}

DEF_TEST(SkSLFPIfStatementWithReturnInsideCanBeInlined, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 branchy(half4 c) {
                 if (c.z == c.w) return c.yyyy; else return c.zzzz;
             }
             void main() {
                 sk_FragColor = branchy(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_branchy;
    {
        if (color.z == color.w) _0_branchy = color.yyyy; else _0_branchy = color.zzzz;
    }

    sk_FragColor = _0_branchy;

}
)__GLSL__");
}

DEF_TEST(SkSLFPUnnecessaryBlocksDoNotAffectEarlyReturnDetection, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             half4 blocky(half4 c) {
                 {
                     return c;
                 }
             }
             void main() {
                 sk_FragColor = blocky(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_blocky;
    {
        {
            _0_blocky = color;
        }
    }

    sk_FragColor = _0_blocky;

}
)__GLSL__");
}

DEF_TEST(SkSLFPInlinedEarlyReturnsAreWrappedInDoWhileBlock, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             inline half4 returny(half4 c) {
                 if (c.x > c.y) return c.xxxx;
                 if (c.y > c.z) return c.yyyy;
                 return c.zzzz;
             }
             void main() {
                 sk_FragColor = returny(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_returny;
    do {
        if (color.x > color.y) {
            _0_returny = color.xxxx;
            break;
        }
        if (color.y > color.z) {
            _0_returny = color.yyyy;
            break;
        }
        {
            _0_returny = color.zzzz;
            break;
        }
    } while (false);

    sk_FragColor = _0_returny;

}
)__GLSL__");
}

DEF_TEST(SkSLFPEarlyReturnDetectionSupportsIfElse, r) {
    // An if-else statement at the end of a function, with a return as the last statement on all
    // paths, are not actually "early" returns. The inliner is able to recognize this pattern.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         /*src=*/R"__SkSL__(
             uniform half4 color;
             inline half4 branchy(half4 c) {
                 c *= 0.5;
                 if (c.x > 0)
                     return c.xxxx;
                 else if (c.y > 0)
                     return c.yyyy;
                 else if (c.z > 0)
                     return c.zzzz;
                 else
                     return c.wwww;
             }
             inline half4 branchyAndBlocky(half4 c) {{{
                 if (c.x > 0) {
                     half4 d = c * 0.5;
                     return d.xxxx;
                 } else {{{
                     if (c.x < 0) {
                         return c.wwww;
                     } else {
                         return c.yyyy;
                     }
                 }}}
             }}}
             void main() {
                 sk_FragColor = branchy(color) * branchyAndBlocky(color);
             }
         )__SkSL__",
         /*expectedGLSL=*/R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_branchy;
    vec4 _1_c = color;
    {
        _1_c *= 0.5;
        if (_1_c.x > 0.0) _0_branchy = _1_c.xxxx; else if (_1_c.y > 0.0) _0_branchy = _1_c.yyyy; else if (_1_c.z > 0.0) _0_branchy = _1_c.zzzz; else _0_branchy = _1_c.wwww;
    }

    vec4 _2_branchyAndBlocky;
    {
        {
            {
                if (color.x > 0.0) {
                    vec4 d = color * 0.5;
                    _2_branchyAndBlocky = d.xxxx;
                } else {
                    {
                        {
                            if (color.x < 0.0) {
                                _2_branchyAndBlocky = color.wwww;
                            } else {
                                _2_branchyAndBlocky = color.yyyy;
                            }
                        }
                    }
                }
            }
        }
    }

    sk_FragColor = _0_branchy * _2_branchyAndBlocky;

}
)__GLSL__");
}

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
        vec4 _1_result;
        switch (int(color.x)) {
            case 0:
                _1_result = color.yyyy;
        }
        _1_result = color.zzzz;
        _0_switchy = _1_result;
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
        vec4 _1_result;
        switch (int(color.x)) {
            case 1:
                _1_result = color.yyyy;
                break;
            default:
                _1_result = color.zzzz;
                break;
        }
        _0_switchy = _1_result;
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
        vec4 _1_pix;
        for (int _2_x = 0;_2_x < 5; ++_2_x) {
            if (_2_x == int(color.w)) _1_pix = color.yyyy;
        }
        _1_pix = color.zzzz;
        _0_loopy = _1_pix;
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
    float _3_fma;
    float _4_a = color.x;
    float _5_b = color.y;
    float _6_c = color.z;
    {
        float _7_0_mul;
        {
            _7_0_mul = _4_a * _5_b;
        }

        float _8_1_add;
        {
            float _9_2_c = _7_0_mul + _6_c;
            _8_1_add = _9_2_c;
        }

        _3_fma = _8_1_add;

    }

    float a = _3_fma;

    float _10_fma;
    float _11_a = color.y;
    float _12_b = color.z;
    float _13_c = color.w;
    {
        float _14_0_mul;
        {
            _14_0_mul = _11_a * _12_b;
        }

        float _15_1_add;
        {
            float _16_2_c = _14_0_mul + _13_c;
            _15_1_add = _16_2_c;
        }

        _10_fma = _15_1_add;

    }

    float b = _10_fma;

    float _17_fma;
    float _18_a = color.z;
    float _19_b = color.w;
    float _20_c = color.x;
    {
        float _21_0_mul;
        {
            _21_0_mul = _18_a * _19_b;
        }

        float _22_1_add;
        {
            float _23_2_c = _21_0_mul + _20_c;
            _22_1_add = _23_2_c;
        }

        _17_fma = _22_1_add;

    }

    float c = _17_fma;

    float _24_mul;
    {
        _24_mul = c * c;
    }

    float _25_mul;
    {
        _25_mul = b * c;
    }

    float _26_mul;
    {
        _26_mul = a * _25_mul;
    }

    return vec4(a, b, _24_mul, _26_mul);

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
                    vec4 _3_d = color * 0.5;
                    _2_branchyAndBlocky = _3_d.xxxx;
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

DEF_TEST(SkSLFunctionMultipleInlinesOnOneLine, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
            uniform half val;
            inline half BigX(half x) {
                ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;
                --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x;
                x = 123;
                return x;
            }
            inline half BigY(half x) {
                ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x; ++x;
                --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x; --x;
                x = 456;
                return x;
            }
            void main() {
                sk_FragColor = BigX(BigY(val)).xxxx;
            }
         )__SkSL__",
         R"__GLSL__(#version 400
out vec4 sk_FragColor;
uniform float val;
void main() {
    float _1_x = val;
    {
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        ++_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        --_1_x;
        _1_x = 456.0;
    }

    float _3_x = 456.0;
    {
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        ++_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        --_3_x;
        _3_x = 123.0;
    }

    sk_FragColor = vec4(123.0);

}
)__GLSL__");
}

/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramKind.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/tracing/SkRPDebugTrace.h"
#include "tests/Test.h"

#include <memory>
#include <optional>
#include <string>

//#define DUMP_PROGRAMS 1
#if defined(DUMP_PROGRAMS)
#include "src/core/SkStreamPriv.h"
#endif

static void test(skiatest::Reporter* r,
                 const char* src,
                 SkSpan<const float> uniforms,
                 SkColor4f startingColor,
                 std::optional<SkColor4f> expectedResult) {
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Default());
    SkSL::ProgramSettings settings;
    settings.fMaxVersionAllowed = SkSL::Version::k300;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::ProgramKind::kRuntimeColorFilter, std::string(src), settings);
    if (!program) {
        ERRORF(r, "Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        return;
    }
    const SkSL::FunctionDeclaration* main = program->getFunction("main");
    if (!main) {
        ERRORF(r, "Program must have a 'main' function");
        return;
    }
    SkArenaAlloc alloc(/*firstHeapAllocation=*/1000);
    SkRasterPipeline pipeline(&alloc);
    pipeline.append_constant_color(&alloc, startingColor);
    SkSL::SkRPDebugTrace debugTrace;
    std::unique_ptr<SkSL::RP::Program> rasterProg =
            SkSL::MakeRasterPipelineProgram(*program, *main->definition(), &debugTrace);
    if (!rasterProg && !expectedResult.has_value()) {
        // We didn't get a program, as expected. Test passes.
        return;
    }
    if (!rasterProg && expectedResult.has_value()) {
        ERRORF(r, "MakeRasterPipelineProgram failed");
        return;
    }
    if (rasterProg && !expectedResult.has_value()) {
        ERRORF(r, "MakeRasterPipelineProgram should have failed, but didn't");
        return;
    }

#if defined(DUMP_PROGRAMS)
    // Dump the program instructions via SkDebugf.
    SkDebugf("-----\n\n");
    SkDebugfStream stream;
    rasterProg->dump(&stream);
    SkDebugf("\n-----\n\n");
#endif

    // Append the SkSL program to the raster pipeline.
    rasterProg->appendStages(&pipeline, &alloc, uniforms);

    // Move the float values from RGBA into an 8888 memory buffer.
    uint32_t out[SkRasterPipeline_kMaxStride_highp] = {};
    SkRasterPipeline_MemoryCtx outCtx{/*pixels=*/out, /*stride=*/SkRasterPipeline_kMaxStride_highp};
    pipeline.append(SkRasterPipeline::store_8888, &outCtx);
    pipeline.run(0, 0, 1, 1);

    // Make sure the first pixel (exclusively) of `out` matches RGBA.
    uint32_t expected = expectedResult->toBytes_RGBA();
    REPORTER_ASSERT(r, out[0] == expected,
                    "Got:%02X%02X%02X%02X Expected:%02X%02X%02X%02X",
                    (out[0] >> 24) & 0xFF,
                    (out[0] >> 16) & 0xFF,
                    (out[0] >> 8) & 0xFF,
                    out[0] & 0xFF,
                    (expected >> 24) & 0xFF,
                    (expected >> 16) & 0xFF,
                    (expected >> 8) & 0xFF,
                    expected & 0xFF);

    // Make sure the rest of the pixels are untouched.
    for (size_t i = 1; i < std::size(out); ++i) {
        REPORTER_ASSERT(r, out[i] == 0);
    }
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorPassthroughTest, r) {
    test(r,
         R"__SkSL__(
             half4 main(half4 startingColor) {
                 return startingColor;
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{1.0f, 1.0f, 0.0f, 1.0f},
         /*expectedResult=*/SkColor4f{1.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorDarkGreenTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(half4) {
                 return half4(half2(0, 0.499), half2(0, 1));
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 0.499f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorTransparentGrayTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(half4) {
                 return half4(0.499);
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.499f, 0.499f, 0.499f, 0.499f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorVariableGreenTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(half4) {
                 half2 zeroOne = half2(0, 1);
                 half one = zeroOne.y, zero = zeroOne.x;
                 return half4(zero, zeroOne.yx, one);
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorAdditionTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(half4 y) {
                 half4 x = half4(-1, 0, 1, 0);
                 half4 z = x.wzyx;
                 z += y.000w;
                 return y + z;
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 1.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorIfElseTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             const half4 colorWhite = half4(1);
             half4 colorBlue  = colorWhite.00b1,
                   colorGreen = colorWhite.0g01,
                   colorRed   = colorWhite.r001;
             half4 main(half4) {
                 half4 result = half4(0);
                 if (colorWhite != colorBlue) {    // TRUE
                     if (colorGreen == colorRed) { // FALSE
                         result = colorRed;
                     } else {
                         result = colorGreen;
                     }
                 } else {
                     if (colorRed != colorGreen) { // TRUE, but in a false branch
                         result = colorBlue;
                     } else {                      // FALSE, and in a false branch
                         result = colorWhite;
                     }
                 }
                 if (colorRed == colorBlue) { // FALSE
                     return colorWhite;
                 }
                 if (colorRed != colorGreen) { // TRUE
                     return result;
                 }
                 if (colorRed == colorWhite) { // FALSE
                     return colorBlue;
                 }
                 return colorRed;
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorTernaryTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(half4 colorWhite) {
                 half4 colorBlue  = colorWhite.00ba,
                       colorGreen = colorWhite.0g0a,
                       colorRed   = colorWhite.r00a;
                 // This ternary matches the initial if-else block inside IfElseTest.
                 half4 result;
                 result = (colorWhite != colorBlue)                              // TRUE
                            ? (colorGreen == colorRed ? colorRed : colorGreen)   // FALSE
                            : (colorRed != colorGreen ? colorBlue : colorWhite); // in false branch

                 // This ternary matches the second portion of IfElseTest.
                 return colorRed == colorBlue  ? colorWhite :
                        colorRed != colorGreen ? result :     // TRUE
                        colorRed == colorWhite ? colorBlue :
                                                 colorRed;
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{1.0, 1.0, 1.0, 1.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorTernarySideEffectTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             const half4 colorGreen = half4(0,1,0,1),
                         colorRed   = half4(1,0,0,1);
             half4 main(half4) {
                 half x = 1, y = 1;
                 (x == y) ? (x += 1) : (y += 1);  // TRUE,   x=2 y=1
                 (x == y) ? (x += 3) : (y += 3);  // FALSE,  x=2 y=4
                 (x <  y) ? (x += 5) : (y += 5);  // TRUE,   x=7 y=4
                 (y >= x) ? (x += 9) : (y += 9);  // FALSE,  x=7 y=13

                 bool b = true;
                 bool c = (b = false) ? false : b;

                 return c ? colorRed : (x == 7 && y == 13) ? colorGreen : colorRed;
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorNestedTernaryTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(half4) {
                 half three = 3, one = 1, two = 2;
                 half result = (three > (one > two ? 2.0 : 5.0)) ? 1.0 : 0.499;
                 return half4(result);
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.499f, 0.499f, 0.499f, 0.499f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorDoWhileTest, r) {
    // This is based on shared/DoWhileControlFlow.sksl (but avoids swizzles).
    test(r,
         R"__SkSL__(
            half r = 1.0, g = 1.0, b = 1.0;
            half4 main(half4) {
                half a = 1.0;
                // Verify that break is allowed in a do-while loop.
                do {
                    r -= 0.25;
                    if (r <= 0) break;
                } while (a == 1.0);

                // Verify that continue is allowed in a do-while loop.
                do {
                    b -= 0.25;
                    if (a == 1) continue; // should always happen
                    g = 0;
                } while (b > 0.0);

                return half4(r, g, b, a);
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorArithmeticTest, r) {
    test(r,
         R"__SkSL__(
            half4 main(half4) {
                const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);

                half a = 3.0, b = 4.0, c = a + b - 2.0;
                if (a*a + b*b == c*c*c/5.0) {
                    int A = 3, B = 4, C = A + B - 2;
                    if (A*A + B*B == C*C*C/5) {
                        return colorGreen;
                    }
                }

                return colorRed;
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorShortCircuitLogicalOr, r) {
    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 1) || ((y += 1) == 2)) { // LHS true, RHS not executed but would be true
                    return (x == 1 && y == 1) ? colorGreen : colorRed;
                } else {
                    return colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});

    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 1) || ((y += 1) == 3)) { // LHS true, RHS not executed but would be false
                    return (x == 1 && y == 1) ? colorGreen : colorRed;
                } else {
                    return colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});

    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 2) || ((y += 1) == 2)) { // LHS false, RHS is executed and is true
                    return (x == 1 && y == 2) ? colorGreen : colorRed;
                } else {
                    return colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});

    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 2) || ((y += 1) == 3)) { // LHS false, RHS is executed and is false
                    return colorRed;
                } else {
                    return (x == 1 && y == 2) ? colorGreen : colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorShortCircuitLogicalAnd, r) {
    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 1) && ((y += 1) == 2)) { // LHS true, RHS is executed and is true
                    return (x == 1 && y == 2) ? colorGreen : colorRed;
                } else {
                    return colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});

    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 1) && ((y += 1) == 3)) { // LHS true, RHS is executed and is false
                    return colorRed;
                } else {
                    return (x == 1 && y == 2) ? colorGreen : colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});

    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 2) && ((y += 1) == 2)) { // LHS false, RHS not executed but would be true
                    return colorRed;
                } else {
                    return (x == 1 && y == 1) ? colorGreen : colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});

    test(r,
         R"__SkSL__(
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);
            half4 main(half4) {
                int x = 1, y = 1;
                if ((x == 2) && ((y += 1) == 3)) { // LHS false, RHS not executed but would be false
                    return colorRed;
                } else {
                    return (x == 1 && y == 1) ? colorGreen : colorRed;
                }
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorSwizzleLValueTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(half4 color) {                        // 0,    0.5, 0,    0
                 color.a     = 2.0;                           // 0,    0.5, 0,    2
                 color.g    /= 0.25;                          // 0,    2,   0,    2
                 color.gba  *= half3(0.5);                    // 0,    1,   0,    1
                 color.bgar += half4(0.249, 0.0, 0.0, 0.749); // 0.75, 1,   0.25, 1
                 return color;
             }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.0, 0.5, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.749f, 1.0f, 0.249f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorUniformDeclarationTest, r) {
    static constexpr float kUniforms[] = {0.0, 1.0, 0.0, 1.0,
                                          1.0, 0.0, 0.0, 1.0};
    test(r,
         R"__SkSL__(
             uniform half4 colorGreen, colorRed;
             half4 main(half4 color) {
                 return color;
             }
         )__SkSL__",
         kUniforms,
         /*startingColor=*/SkColor4f{0.0, 1.0, 0.0, 1.0},
         /*expectedResult=*/SkColor4f{0.0, 1.0, 0.0, 1.0});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorUniformUsageTest, r) {
    static constexpr float kUniforms[] = {0.0, 1.0, 0.0, 1.0,
                                          1.0, 0.0, 0.0, 1.0,
                                          1.0, 2.0, 3.0, 4.0};
    test(r,
         R"__SkSL__(
             uniform half4 colorGreen, colorRed;
             uniform half2x2 testMatrix2x2;
             half4 main(half4 color) {
                 return testMatrix2x2 == half2x2(1,2,3,4) ? colorGreen : colorRed;
             }
         )__SkSL__",
         kUniforms,
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0, 1.0, 0.0, 1.0});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorCoercedTypeTest, r) {
    static constexpr float kUniforms[] = {0.0, 1.0, 0.0, 1.0,
                                          1.0, 0.0, 0.0, 1.0};
    test(r,
         R"__SkSL__(
             uniform half4 colorGreen;
             uniform float4 colorRed;
             half4 main(half4 color) {
                 return ((colorGreen + colorRed) == float4(1.0, 1.0, 0.0, 2.0)) ? colorGreen
                                                                                : colorGreen.gr01;
             }
         )__SkSL__",
         kUniforms,
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0, 1.0, 0.0, 1.0});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorVectorScalarFoldingTest, r) {
    // This test matches the floating-point test in VectorScalarFolding.rts.
    // (The function call has been manually inlined.)
    static constexpr float kUniforms[] = {0.0, 1.0, 0.0, 1.0,
                                          1.0, 0.0, 0.0, 1.0,
                                          1.0};
    test(r,
         R"__SkSL__(
            uniform half4 colorGreen, colorRed;
            uniform half  unknownInput;

            half4 main(vec4) {
                bool ok = true;

                // Vector op scalar
                half4 x = half4(half2(1), half2(2, 3)) + 5;
                ok = ok && (x == half4(6, 6, 7, 8));
                x = half4(8, half3(10)) - 1;
                ok = ok && (x == half4(7, 9, 9, 9));
                x = half4(half2(8), half2(9)) + 1;
                ok = ok && (x == half4(9, 9, 10, 10));
                x.xyz = half3(2) * 3;
                ok = ok && (x == half4(6, 6, 6, 10));
                x.xy = half2(12) / 4;
                ok = ok && (x == half4(3, 3, 6, 10));

                // (Vector op scalar).swizzle
                x = (half4(12) / 2).yxwz;
                ok = ok && (x == half4(6));

                // Scalar op vector
                x = 5 + half4(half2(1), half2(2, 3));
                ok = ok && (x == half4(6, 6, 7, 8));
                x = 1 - half4(8, half3(10));
                ok = ok && (x == half4(-7, -9, -9, -9));
                x = 1 + half4(half2(8), half2(9));
                ok = ok && (x == half4(9, 9, 10, 10));
                x.xyz = 3 * half3(2);
                ok = ok && (x == half4(6, 6, 6, 10));
                x.xy = 4 / half2(0.5);
                ok = ok && (x == half4(8, 8, 6, 10));
                x = 20 / half4(10, 20, 40, 80);
                ok = ok && (x == half4(2, 1, 0.5, 0.25));

                // (Scalar op vector).swizzle
                x = (12 / half4(2)).yxwz;
                ok = ok && (x == half4(6));

                // Vector op unknown scalar
                half  unknown = unknownInput;
                x = half4(0) + unknown;
                ok = ok && (x == half4(unknown));
                x = half4(0) * unknown;
                ok = ok && (x == half4(0));
                x = half4(0) / unknown;
                ok = ok && (x == half4(0));
                x = half4(1) * unknown;
                ok = ok && (x == half4(unknown));

                // Unknown scalar op vector
                x = unknown * half4(1);
                ok = ok && (x == half4(unknown));
                x = unknown + half4(0);
                ok = ok && (x == half4(unknown));
                x = unknown - half4(0);
                ok = ok && (x == half4(unknown));
                x = unknown / half4(1);
                ok = ok && (x == half4(unknown));

                // Scalar op unknown vector
                x = 0 + half4(unknown);
                ok = ok && (x == half4(unknown));
                x = 0 * half4(unknown);
                ok = ok && (x == half4(0));
                x = 0 / half4(unknown);  // this should NOT optimize away
                ok = ok && (x == half4(0));
                x = 1 * half4(unknown);
                ok = ok && (x == half4(unknown));

                // X = Unknown op scalar
                x = half4(unknown) + 0;
                ok = ok && (x == half4(unknown));
                x = half4(unknown) * 0;
                ok = ok && (x == half4(0));
                x = half4(unknown) * 1;
                ok = ok && (x == half4(unknown));
                x = half4(unknown) - 0;
                ok = ok && (x == half4(unknown));

                // X op= scalar.
                x = half4(unknown);
                x += 1;
                x += 0;
                x -= 1;
                x -= 0;
                x *= 1;
                x /= 1;
                ok = ok && (x == half4(unknown));

                // X = X op scalar.
                x = half4(unknown);
                x = x + 1;
                x = x + 0;
                x = x - 1;
                x = x - 0;
                x = x * 1;
                x = x / 1;
                ok = ok && (x == half4(unknown));

                return ok ? colorGreen : colorRed;
            }
         )__SkSL__",
         kUniforms,
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0, 1.0, 0.0, 1.0});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorLumaTernaryTest, r) {
    test(r,
         R"__SkSL__(
            half4 main(vec4 color) {
                half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

                half scale = luma < 0.33333 ? 0.5
                           : luma < 0.66666 ? (0.166666 + 2.0 * (luma - 0.33333)) / luma
                           :   /* else */     (0.833333 + 0.5 * (luma - 0.66666)) / luma;
                return half4(color.rgb * scale, color.a);
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.25, 0.00, 0.75, 1.0},
         /*expectedResult=*/SkColor4f{0.125, 0.0, 0.375, 1.0});

}

DEF_TEST(SkSLRasterPipelineCodeGeneratorLumaIfNoEarlyReturnTest, r) {
    test(r,
         R"__SkSL__(
            half4 main(vec4 color) {
                half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

                half scale = 0;
                if (luma < 0.33333) {
                    scale = 0.5;
                } else if (luma < 0.66666) {
                    scale = (0.166666 + 2.0 * (luma - 0.33333)) / luma;
                } else {
                    scale = (0.833333 + 0.5 * (luma - 0.66666)) / luma;
                }
                return half4(color.rgb * scale, color.a);
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.25, 0.00, 0.75, 1.0},
         /*expectedResult=*/SkColor4f{0.125, 0.0, 0.375, 1.0});

}

DEF_TEST(SkSLRasterPipelineCodeGeneratorLumaWithEarlyReturnTest, r) {
    test(r,
         R"__SkSL__(
            half4 main(half4 color) {
                half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

                half scale = 0;
                if (luma < 0.33333) {
                    return half4(color.rgb * 0.5, color.a);
                } else if (luma < 0.66666) {
                    scale = 0.166666 + 2.0 * (luma - 0.33333);
                } else {
                    scale = 0.833333 + 0.5 * (luma - 0.66666);
                }
                return half4(color.rgb * (scale/luma), color.a);
            }
         )__SkSL__",
         /*uniforms=*/{},
         /*startingColor=*/SkColor4f{0.25, 0.00, 0.75, 1.0},
         /*expectedResult=*/SkColor4f{0.125, 0.0, 0.375, 1.0});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorDotTest, r) {
    // This matches the test at intrinsics/Dot.sksl (but as a color filter).
    static constexpr float kUniforms[] = {1.0, 2.0, 3.0, 4.0,
                                          5.0, 6.0, 7.0, 8.0,
                                          0.0, 1.0, 0.0, 1.0,
                                          1.0, 0.0, 0.0, 1.0};
    test(r,
         R"__SkSL__(
            uniform half4 inputA, inputB;
            uniform half4 colorGreen, colorRed;

            half4 main(vec4) {
                const half4 constValA = half4(1, 2, 3, 4);
                const half4 constValB = half4(5, 6, 7, 8);
                half4 expected = half4(5, 17, 38, 70);

                return (dot(inputA.x,       inputB.x)       == expected.x &&
                        dot(inputA.xy,      inputB.xy)      == expected.y &&
                        dot(inputA.xyz,     inputB.xyz)     == expected.z &&
                        dot(inputA.xyzw,    inputB.xyzw)    == expected.w &&
                        dot(constValA.x,    constValB.x)    == expected.x &&
                        dot(constValA.xy,   constValB.xy)   == expected.y &&
                        dot(constValA.xyz,  constValB.xyz)  == expected.z &&
                        dot(constValA.xyzw, constValB.xyzw) == expected.w) ? colorGreen : colorRed;
            }
         )__SkSL__",
         kUniforms,
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0, 1.0, 0.0, 1.0});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorLogicalNotTest, r) {
    // This largely matches the test at intrinsics/Not.sksl (sans typecasting, as a color filter).
    static constexpr float kUniforms[] = {0.2f, 0.0f, 0.4f, 0.0f,
                                          0.0f, 0.6f, 0.0f, 0.8f,
                                          0.0f, 1.0f, 0.0f, 1.0f,
                                          1.0f, 0.0f, 0.0f, 1.0f};
    test(r,
         R"__SkSL__(
            uniform half4 inputH4, expectedH4;
            uniform half4 colorGreen, colorRed;

            half4 main(vec4) {
                bool4 inputVal = bool4(inputH4.x != 0.0,
                                       inputH4.y != 0.0,
                                       inputH4.z != 0.0,
                                       inputH4.w != 0.0);
                bool4 expected = bool4(!(expectedH4.x == 0.0),
                                       !(expectedH4.y == 0.0),
                                       !(expectedH4.z == 0.0),
                                       !(expectedH4.w == 0.0));
                const bool4 constVal = bool4(true, false, true, false);
                return (not(inputVal.xy)   == expected.xy    &&
                        not(inputVal.xyz)  == expected.xyz   &&
                        not(inputVal.xyzw) == expected.xyzw  &&
                        not(constVal.xy)   == expected.xy    &&
                        not(constVal.xyz)  == expected.xyz   &&
                        not(constVal.xyzw) == expected.xyzw) ? colorGreen : colorRed;
            }
         )__SkSL__",
         kUniforms,
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0, 1.0, 0.0, 1.0});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorBitwiseNotTest, r) {
    static constexpr int32_t kUniforms[] = { 0,  12,  3456,  4567890,
                                            ~0, ~12, ~3456, ~4567890};
    test(r,
         R"__SkSL__(
            uniform int4 value, expected;
            const half4 colorGreen = half4(0,1,0,1), colorRed = half4(1,0,0,1);

            half4 main(vec4) {
                return (~value.x    == expected.x     &&
                        ~value.xy   == expected.xy    &&
                        ~value.xyz  == expected.xyz   &&
                        ~value.xyzw == expected.xyzw) ? colorGreen : colorRed;
            }
         )__SkSL__",
         SkSpan((const float*)kUniforms, std::size(kUniforms)),
         /*startingColor=*/SkColor4f{0.0, 0.0, 0.0, 0.0},
         /*expectedResult=*/SkColor4f{0.0, 1.0, 0.0, 1.0});
}

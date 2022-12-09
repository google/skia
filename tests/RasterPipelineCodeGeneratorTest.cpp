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
                 std::optional<SkColor4f> color) {
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Default());
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program =
            compiler.convertProgram(SkSL::ProgramKind::kFragment, std::string(src), settings);
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
    SkSL::SkRPDebugTrace debugTrace;
    std::unique_ptr<SkSL::RP::Program> rasterProg =
            SkSL::MakeRasterPipelineProgram(*program, *main->definition(), &debugTrace);
    if (!rasterProg && !color.has_value()) {
        // We didn't get a program, as expected. Test passes.
        return;
    }
    if (!rasterProg && color.has_value()) {
        ERRORF(r, "MakeRasterPipelineProgram failed");
        return;
    }
    if (rasterProg && !color.has_value()) {
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
    rasterProg->appendStages(&pipeline, &alloc);

    // Move the float values from RGBA into an 8888 memory buffer.
    uint32_t out[SkRasterPipeline_kMaxStride_highp] = {};
    SkRasterPipeline_MemoryCtx outCtx{/*pixels=*/out, /*stride=*/SkRasterPipeline_kMaxStride_highp};
    pipeline.append(SkRasterPipeline::store_8888, &outCtx);
    pipeline.run(0, 0, 1, 1);

    // Make sure the first pixel (exclusively) of `out` matches RGBA.
    uint32_t expected = color->toBytes_RGBA();
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

DEF_TEST(SkSLRasterPipelineCodeGeneratorMagentaTest, r) {
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 return half4(1, 1, 0, 1);
             }
         )__SkSL__",
         SkColor4f{1.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorDarkGreenTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 return half4(half2(0, 0.499), half2(0, 1));
             }
         )__SkSL__",
         SkColor4f{0.0f, 0.499f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorTransparentGrayTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 return half4(0.499);
             }
         )__SkSL__",
         SkColor4f{0.499f, 0.499f, 0.499f, 0.499f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorVariableGreenTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 half _1 = 1, _0 = 0;
                 half2 _0_1;
                 _0_1 = half2(_0, _1);
                 return half4(_0, _1, _0_1);
             }
         )__SkSL__",
         SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorAdditionTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 half4 x = half4(0, 1, 0, -1);
                 half4 y = half4(0, 0, 0,  1);
                 half4 z = x;
                 z += y;
                 return y + z;
             }
         )__SkSL__",
         SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorIfElseTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 half4 colorBlue  = half4(0,0,1,1),
                       colorGreen = half4(0,1,0,1),
                       colorRed   = half4(1,0,0,1),
                       colorWhite = half4(1);
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
         SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorTernaryTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 half4 colorBlue  = half4(0,0,1,1),
                       colorGreen = half4(0,1,0,1),
                       colorRed   = half4(1,0,0,1),
                       colorWhite = half4(1);
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
         SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorNestedTernaryTest, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2 coords) {
                 half three = 3, one = 1, two = 2;
                 half result = (three > (one > two ? 2.0 : 5.0)) ? 1.0 : 0.499;
                 return half4(result);
             }
         )__SkSL__",
         SkColor4f{0.499f, 0.499f, 0.499f, 0.499f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorDoWhileTest, r) {
    // This is based on shared/DoWhileControlFlow.sksl (but avoids swizzles).
    test(r,
         R"__SkSL__(
            half4 main(float2 coords) {
                half r = 1.0, g = 1.0, b = 1.0, a = 1.0;

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
         SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

DEF_TEST(SkSLRasterPipelineCodeGeneratorArithmeticTest, r) {
    test(r,
         R"__SkSL__(
            half4 main(float2 coords) {
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
         SkColor4f{0.0f, 1.0f, 0.0f, 1.0f});
}

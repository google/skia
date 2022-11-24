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
#include "tests/Test.h"

#include <memory>
#include <optional>
#include <string>

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
    std::unique_ptr<SkSL::RP::Program> rasterProg =
            SkSL::MakeRasterPipelineProgram(*program, *main->definition());
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

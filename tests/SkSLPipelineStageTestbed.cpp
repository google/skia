/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "tests/Test.h"

#include <memory>
#include <string>

static void test(skiatest::Reporter* r,
                 const char* src,
                 SkSL::ProgramKind kind = SkSL::ProgramKind::kPrivateRuntimeShader) {
    // These callbacks match the behavior of skslc when given a .stage file.
    class Callbacks : public SkSL::PipelineStage::Callbacks {
    public:
        std::string getMangledName(const char* name) override {
            return std::string(name) + "_0";
        }

        std::string declareUniform(const SkSL::VarDeclaration* decl) override {
            fOutput += decl->description();
            return std::string(decl->var()->name());
        }

        void defineFunction(const char* decl, const char* body, bool /*isMain*/) override {
            fOutput += std::string(decl) + '{' + body + '}';
        }

        void declareFunction(const char* decl) override { fOutput += decl; }

        void defineStruct(const char* definition) override { fOutput += definition; }

        void declareGlobal(const char* declaration) override { fOutput += declaration; }

        std::string sampleShader(int index, std::string coords) override {
            return "child_" + std::to_string(index) + ".eval(" + coords + ')';
        }

        std::string sampleColorFilter(int index, std::string color) override {
            return "child_" + std::to_string(index) + ".eval(" + color + ')';
        }

        std::string sampleBlender(int index, std::string src, std::string dst) override {
            return "child_" + std::to_string(index) + ".eval(" + src + ", " + dst + ')';
        }

        std::string toLinearSrgb(std::string color) override {
            return "toLinearSrgb(" + color + ')';
        }
        std::string fromLinearSrgb(std::string color) override {
            return "fromLinearSrgb(" + color + ')';
        }

        std::string fOutput;
    };

    SkSL::Compiler compiler;
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, std::string(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        Callbacks callbacks;
        SkSL::PipelineStage::ConvertProgram(*program, "_coords", "_inColor",
                                            "_canvasColor", &callbacks);
        REPORTER_ASSERT(r, !callbacks.fOutput.empty());
        // SkDebugf("Pipeline stage output:\n\n%s", callbacks.fOutput.c_str());
    }
}

DEF_TEST(SkSLPipelineStageTestbed, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             half4 main(float2) {
                 return half4(0);
             }
         )__SkSL__");
}

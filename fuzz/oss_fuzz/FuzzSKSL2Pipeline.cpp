/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include "fuzz/Fuzz.h"

bool FuzzSKSL2Pipeline(sk_sp<SkData> bytes) {
    std::unique_ptr<SkSL::ShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    SkSL::Compiler compiler(caps.get());
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                    SkSL::ProgramKind::kRuntimeShader,
                                                    std::string((const char*) bytes->data(),
                                                                bytes->size()),
                                                    settings);
    if (!program) {
        return false;
    }

    class Callbacks : public SkSL::PipelineStage::Callbacks {
        std::string declareUniform(const SkSL::VarDeclaration* decl) override {
            return std::string(decl->var().name());
        }

        void defineFunction(const char* /*decl*/, const char* /*body*/, bool /*isMain*/) override {}
        void declareFunction(const char* /*decl*/) override {}
        void defineStruct(const char* /*definition*/) override {}
        void declareGlobal(const char* /*declaration*/) override {}

        std::string sampleShader(int index, std::string coords) override {
            return "child_" + skstd::to_string(index) + ".eval(" + coords + ")";
        }

        std::string sampleColorFilter(int index, std::string color) override {
            return "child_" + skstd::to_string(index) + ".eval(" + color + ")";
        }

        std::string sampleBlender(int index, std::string src, std::string dst) override {
            return "child_" + skstd::to_string(index) + ".eval(" + src + ", " + dst + ")";
        }

        std::string toLinearSrgb(std::string color) override { return color; }
        std::string fromLinearSrgb(std::string color) override { return color; }
    };

    Callbacks callbacks;
    SkSL::PipelineStage::ConvertProgram(*program, "coords", "inColor", "half4(1)", &callbacks);
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKSL2Pipeline(bytes);
    return 0;
}
#endif

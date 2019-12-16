/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRuntimeEffect.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

static inline int new_sksl_index() {
    static std::atomic<int> nextIndex{ 0 };
    return nextIndex++;
}

sk_sp<SkRuntimeEffect> SkRuntimeEffect::Make(SkString sksl) {
    return sk_sp<SkRuntimeEffect>(new SkRuntimeEffect(std::move(sksl)));
}

SkRuntimeEffect::SkRuntimeEffect(SkString sksl)
        : fIndex(new_sksl_index())
        , fSkSL(std::move(sksl)) {
    fBaseProgram = fCompiler.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(fSkSL.c_str(), fSkSL.size()),
                                            SkSL::Program::Settings());
    if (!fBaseProgram) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
        return;
    }
    SkASSERT(!fCompiler.errorCount());

    for (const auto& e : *fBaseProgram) {
        if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
            SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
            for (const auto& varStatement : v.fVars) {
                const SkSL::Variable& var = *((SkSL::VarDeclaration&) * varStatement).fVar;
                if ((var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) ||
                    (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag)) {
                    fInAndUniformVars.push_back(&var);
                }
                // "in uniform" doesn't make sense outside of .fp files
                SkASSERT((var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) == 0 ||
                    (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) == 0);
                // "layout(key)" doesn't make sense outside of .fp files; all 'in' variables are
                // part of the key
                SkASSERT(!var.fModifiers.fLayout.fKey);
            }
        }
    }
}

#if SK_SUPPORT_GPU
static std::tuple<const SkSL::Type*, int> strip_array(const SkSL::Type* type) {
    int arrayCount = 0;
    if (type->kind() == SkSL::Type::kArray_Kind) {
        arrayCount = type->columns();
        type = &type->componentType();
    }
    return std::make_tuple(type, arrayCount);
}

bool SkRuntimeEffect::toPipelineStage(const void* inputs, size_t inputSize,
                                      const GrShaderCaps* shaderCaps,
                                      SkSL::String* outCode,
                                      std::vector<SkSL::Compiler::FormatArg>* outFormatArgs,
                                      std::vector<SkSL::Compiler::GLSLFunction>* outFunctions) {
    // This function is used by the GPU backend, and can't reuse our previously built fBaseProgram.
    // If the supplied shaderCaps have any non-default values, we have baked in the wrong settings.
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;

    auto baseProgram = fCompiler.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                                SkSL::String(fSkSL.c_str(), fSkSL.size()),
                                                settings);
    SkASSERT(baseProgram);

    std::unordered_map<SkSL::String, SkSL::Program::Settings::Value> inputMap;
    size_t offset = 0;
    for (const auto& v : fInAndUniformVars) {
        auto [type, arrayCount] = strip_array(&v->fType);
        arrayCount = SkTMax(1, arrayCount);
        SkSL::String name(v->fName);
        if (type == fCompiler.context().fInt_Type.get() ||
            type == fCompiler.context().fShort_Type.get()) {
            offset = SkAlign4(offset);
            if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                int32_t v = *(int32_t*)(((uint8_t*)inputs) + offset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            }
            offset += sizeof(int32_t) * arrayCount;
        } else if (type == fCompiler.context().fFloat_Type.get() ||
                   type == fCompiler.context().fHalf_Type.get()) {
            offset = SkAlign4(offset);
            if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                float v = *(float*)(((uint8_t*)inputs) + offset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            }
            offset += sizeof(float) * arrayCount;
        } else if (type == fCompiler.context().fBool_Type.get()) {
            if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                bool v = *(((bool*)inputs) + offset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            }
            offset += sizeof(bool) * arrayCount;
        } else if (type == fCompiler.context().fFloat2_Type.get() ||
                   type == fCompiler.context().fHalf2_Type.get()) {
            offset = SkAlign4(offset) + sizeof(float) * 2 * arrayCount;
        } else if (type == fCompiler.context().fFloat3_Type.get() ||
                   type == fCompiler.context().fHalf3_Type.get()) {
            offset = SkAlign4(offset) + sizeof(float) * 3 * arrayCount;
        } else if (type == fCompiler.context().fFloat4_Type.get() ||
                   type == fCompiler.context().fHalf4_Type.get()) {
            offset = SkAlign4(offset) + sizeof(float) * 4 * arrayCount;
        } else if (type == fCompiler.context().fFragmentProcessor_Type.get()) {
            // do nothing
        } else {
            printf("can't handle input var: %s\n", SkSL::String(v->fType.fName).c_str());
            SkASSERT(false);
            return false;
        }
    }

    auto specialized = fCompiler.specialize(*baseProgram, inputMap);
    bool optimized = fCompiler.optimize(*specialized);
    if (!optimized) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
        SkASSERT(false);
        return false;
    }

    if (!fCompiler.toPipelineStage(*specialized, outCode, outFormatArgs, outFunctions)) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
        SkASSERT(false);
        return false;
    }

    return true;
}
#endif

std::tuple<std::unique_ptr<SkSL::ByteCode>, int, SkString> SkRuntimeEffect::toByteCode() {
    auto byteCode = fCompiler.toByteCode(*fBaseProgram);
    return std::make_tuple(std::move(byteCode), fCompiler.errorCount(),
                           SkString(fCompiler.errorText().c_str()));
}

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

static size_t uniform_type_size(SkRuntimeEffect::CPUType type) {
    switch (type) {
        case SkRuntimeEffect::CPUType::kBool:   return 1;
        case SkRuntimeEffect::CPUType::kInt:    return sizeof(int32_t);
        case SkRuntimeEffect::CPUType::kFloat:  return sizeof(float);
        case SkRuntimeEffect::CPUType::kFloat2: return sizeof(float) * 2;
        case SkRuntimeEffect::CPUType::kFloat3: return sizeof(float) * 3;
        case SkRuntimeEffect::CPUType::kFloat4: return sizeof(float) * 4;

        case SkRuntimeEffect::CPUType::kFloat2x2: return sizeof(float) * 4;
        case SkRuntimeEffect::CPUType::kFloat3x3: return sizeof(float) * 9;
        case SkRuntimeEffect::CPUType::kFloat4x4: return sizeof(float) * 16;
    }
    SkUNREACHABLE;
}

size_t SkRuntimeEffect::Variable::sizeInBytes() const {
    return uniform_type_size(fCPUType) * SkTMax(1, fArrayCount);
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

    size_t offset = 0;
    for (const auto& e : *fBaseProgram) {
        if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
            SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
            for (const auto& varStatement : v.fVars) {
                const SkSL::Variable& var = *((SkSL::VarDeclaration&) * varStatement).fVar;

                // Sanity check some rules that should be enforced by the IR generator.
                // These are all layout options that only make sense in .fp files.
                SkASSERT(!var.fModifiers.fLayout.fKey);
                SkASSERT((var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) == 0 ||
                         (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) == 0);
                SkASSERT(var.fModifiers.fLayout.fCType == SkSL::Layout::CType::kDefault);
                SkASSERT(var.fModifiers.fLayout.fWhen.fLength == 0);
                SkASSERT((var.fModifiers.fLayout.fFlags & SkSL::Layout::kTracked_Flag) == 0);

                if ((var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) ||
                    (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag)) {
                    // TODO: Scrape these into a separate list for child shaders
                    if (&var.fType == fCompiler.context().fFragmentProcessor_Type.get()) {
                        continue;
                    }

                    Variable v;
                    v.fName = var.fName;
                    v.fOffset = offset;
                    v.fUniform = var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag;

                    const SkSL::Type* type = &var.fType;
                    if (type->kind() == SkSL::Type::kArray_Kind) {
                        v.fArrayCount = type->columns();
                        type = &type->componentType();
                    } else {
                        v.fArrayCount = 0;
                    }

#if SK_SUPPORT_GPU
#define SET_TYPES(cpuType, gpuType) do { v.fCPUType = cpuType; v.fGPUType = gpuType;} while (false)
#else
#define SET_TYPES(cpuType, gpuType) do { v.fCpuType = cpuType; } while (false)
#endif

                    const SkSL::Context& ctx(fCompiler.context());
                    if (type == ctx.fBool_Type.get()) {
                        SET_TYPES(CPUType::kBool, kVoid_GrSLType);
                    } else if (type == ctx.fInt_Type.get()) {
                        SET_TYPES(CPUType::kInt, kVoid_GrSLType);
                    } else if (type == ctx.fFloat_Type.get()) {
                        SET_TYPES(CPUType::kFloat, kFloat_GrSLType);
                    } else if (type == ctx.fHalf_Type.get()) {
                        SET_TYPES(CPUType::kFloat, kHalf_GrSLType);
                    } else if (type == ctx.fFloat2_Type.get()) {
                        SET_TYPES(CPUType::kFloat2, kFloat2_GrSLType);
                    } else if (type == ctx.fHalf2_Type.get()) {
                        SET_TYPES(CPUType::kFloat2, kHalf2_GrSLType);
                    } else if (type == ctx.fFloat3_Type.get()) {
                        SET_TYPES(CPUType::kFloat3, kFloat3_GrSLType);
                    } else if (type == ctx.fHalf3_Type.get()) {
                        SET_TYPES(CPUType::kFloat3, kHalf3_GrSLType);
                    } else if (type == ctx.fFloat4_Type.get()) {
                        SET_TYPES(CPUType::kFloat4, kFloat4_GrSLType);
                    } else if (type == ctx.fHalf4_Type.get()) {
                        SET_TYPES(CPUType::kFloat4, kHalf4_GrSLType);
                    } else if (type == ctx.fFloat2x2_Type.get()) {
                        SET_TYPES(CPUType::kFloat2x2, kFloat2x2_GrSLType);
                    } else if (type == ctx.fHalf2x2_Type.get()) {
                        SET_TYPES(CPUType::kFloat2x2, kHalf2x2_GrSLType);
                    } else if (type == ctx.fFloat3x3_Type.get()) {
                        SET_TYPES(CPUType::kFloat3x3, kFloat3x3_GrSLType);
                    } else if (type == ctx.fHalf3x3_Type.get()) {
                        SET_TYPES(CPUType::kFloat3x3, kHalf3x3_GrSLType);
                    } else if (type == ctx.fFloat4x4_Type.get()) {
                        SET_TYPES(CPUType::kFloat4x4, kFloat4x4_GrSLType);
                    } else if (type == ctx.fHalf4x4_Type.get()) {
                        SET_TYPES(CPUType::kFloat4x4, kHalf4x4_GrSLType);
                    } else {
                        SkDEBUGFAILF("Unsupported input/uniform type: %s\n",
                                     type->description().c_str());

                    }

#undef SET_TYPES

                    offset += v.sizeInBytes();
                    fInAndUniformVars.push_back(v);
                }
            }
        }
    }
}

size_t SkRuntimeEffect::inputSize() const {
    return fInAndUniformVars.empty()
        ? 0
        : fInAndUniformVars.back().fOffset + fInAndUniformVars.back().sizeInBytes();
}

#if SK_SUPPORT_GPU
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
    for (const auto& v : fInAndUniformVars) {
        if (v.fUniform) {
            continue;
        }
        // 'in' arrays are not supported
        SkASSERT(v.fArrayCount == 0);
        SkSL::String name(v.fName.c_str(), v.fName.size());
        switch (v.fCPUType) {
            case CPUType::kBool: {
                bool b = *SkTAddOffset<const bool>(inputs, v.fOffset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(b)));
                break;
            }
            case CPUType::kInt: {
                int32_t i = *SkTAddOffset<const int32_t>(inputs, v.fOffset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(i)));
                break;
            }
            case CPUType::kFloat: {
                float f = *SkTAddOffset<const float>(inputs, v.fOffset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(f)));
                break;
            }
            default:
                SkDEBUGFAIL("Unsupported input variable type");
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

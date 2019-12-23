/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "src/core/SkRuntimeEffect.h"
#include "src/shaders/SkRTShader.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

static inline int new_sksl_index() {
    static std::atomic<int> nextIndex{ 0 };
    return nextIndex++;
}

SkRuntimeEffect::EffectResult SkRuntimeEffect::Make(SkString sksl) {
    auto compiler = std::make_unique<SkSL::Compiler>();
    auto program = compiler->convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(sksl.c_str(), sksl.size()),
                                            SkSL::Program::Settings());
    if (!program) {
        return std::make_pair(nullptr, SkString(compiler->errorText().c_str()));
    }
    SkASSERT(!compiler->errorCount());

    sk_sp<SkRuntimeEffect> effect(new SkRuntimeEffect(std::move(sksl), std::move(compiler),
                                                      std::move(program)));
    return std::make_pair(std::move(effect), SkString());
}

size_t SkRuntimeEffect::Variable::sizeInBytes() const {
    auto element_size = [](Type type) -> size_t {
        switch (type) {
            case Type::kBool:   return 1;
            case Type::kInt:    return sizeof(int32_t);
            case Type::kFloat:  return sizeof(float);
            case Type::kFloat2: return sizeof(float) * 2;
            case Type::kFloat3: return sizeof(float) * 3;
            case Type::kFloat4: return sizeof(float) * 4;

            case Type::kFloat2x2: return sizeof(float) * 4;
            case Type::kFloat3x3: return sizeof(float) * 9;
            case Type::kFloat4x4: return sizeof(float) * 16;
            default: SkUNREACHABLE;
        }
    };
    return element_size(fType) * fCount;
}

SkRuntimeEffect::SkRuntimeEffect(SkString sksl, std::unique_ptr<SkSL::Compiler> compiler,
                                 std::unique_ptr<SkSL::Program> baseProgram)
        : fIndex(new_sksl_index())
        , fSkSL(std::move(sksl))
        , fCompiler(std::move(compiler))
        , fBaseProgram(std::move(baseProgram)) {
    SkASSERT(fCompiler && fBaseProgram);

    size_t offset = 0;
    auto gather_variables = [this, &offset](SkSL::Modifiers::Flag flag) {
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

                    if (var.fModifiers.fFlags & flag) {
                        if (&var.fType == fCompiler->context().fFragmentProcessor_Type.get()) {
                            fChildren.push_back(var.fName);
                            continue;
                        }

                        Variable v;
                        v.fName = var.fName;
                        v.fQualifier = (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag)
                            ? Variable::Qualifier::kUniform
                            : Variable::Qualifier::kIn;
                        v.fFlags = 0;
                        v.fCount = 1;

                        const SkSL::Type* type = &var.fType;
                        if (type->kind() == SkSL::Type::kArray_Kind) {
                            v.fFlags |= Variable::kArray_Flag;
                            v.fCount = type->columns();
                            type = &type->componentType();
                        }

#if SK_SUPPORT_GPU
#define SET_TYPES(cpuType, gpuType) do { v.fType = cpuType; v.fGPUType = gpuType;} while (false)
#else
#define SET_TYPES(cpuType, gpuType) do { v.fType = cpuType; } while (false)
#endif

                        const SkSL::Context& ctx(fCompiler->context());
                        if (type == ctx.fBool_Type.get()) {
                            SET_TYPES(Variable::Type::kBool, kVoid_GrSLType);
                        } else if (type == ctx.fInt_Type.get()) {
                            SET_TYPES(Variable::Type::kInt, kVoid_GrSLType);
                        } else if (type == ctx.fFloat_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat, kFloat_GrSLType);
                        } else if (type == ctx.fHalf_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat, kHalf_GrSLType);
                        } else if (type == ctx.fFloat2_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat2, kFloat2_GrSLType);
                        } else if (type == ctx.fHalf2_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat2, kHalf2_GrSLType);
                        } else if (type == ctx.fFloat3_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat3, kFloat3_GrSLType);
                        } else if (type == ctx.fHalf3_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat3, kHalf3_GrSLType);
                        } else if (type == ctx.fFloat4_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat4, kFloat4_GrSLType);
                        } else if (type == ctx.fHalf4_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat4, kHalf4_GrSLType);
                        } else if (type == ctx.fFloat2x2_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat2x2, kFloat2x2_GrSLType);
                        } else if (type == ctx.fHalf2x2_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat2x2, kHalf2x2_GrSLType);
                        } else if (type == ctx.fFloat3x3_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat3x3, kFloat3x3_GrSLType);
                        } else if (type == ctx.fHalf3x3_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat3x3, kHalf3x3_GrSLType);
                        } else if (type == ctx.fFloat4x4_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat4x4, kFloat4x4_GrSLType);
                        } else if (type == ctx.fHalf4x4_Type.get()) {
                            SET_TYPES(Variable::Type::kFloat4x4, kHalf4x4_GrSLType);
                        } else {
                            SkDEBUGFAILF("Unsupported input/uniform type: %s\n",
                                         type->description().c_str());

                        }

#undef SET_TYPES

                        if (v.fType != Variable::Type::kBool) {
                            offset = SkAlign4(offset);
                        }
                        v.fOffset = offset;
                        offset += v.sizeInBytes();
                        fInAndUniformVars.push_back(v);
                    }
                }
            }
        }
    };

    // Gather the inputs in two passes, to de-interleave them in our input layout
    gather_variables(SkSL::Modifiers::kIn_Flag);
    gather_variables(SkSL::Modifiers::kUniform_Flag);
}

size_t SkRuntimeEffect::inputSize() const {
    return fInAndUniformVars.empty()
        ? 0
        : fInAndUniformVars.back().fOffset + fInAndUniformVars.back().sizeInBytes();
}

#if SK_SUPPORT_GPU
bool SkRuntimeEffect::toPipelineStage(const void* inputs, const GrShaderCaps* shaderCaps,
                                      SkSL::String* outCode,
                                      std::vector<SkSL::Compiler::FormatArg>* outFormatArgs,
                                      std::vector<SkSL::Compiler::GLSLFunction>* outFunctions) {
    // This function is used by the GPU backend, and can't reuse our previously built fBaseProgram.
    // If the supplied shaderCaps have any non-default values, we have baked in the wrong settings.
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;

    auto baseProgram = fCompiler->convertProgram(SkSL::Program::kPipelineStage_Kind,
                                                 SkSL::String(fSkSL.c_str(), fSkSL.size()),
                                                 settings);
    SkASSERT(baseProgram);

    std::unordered_map<SkSL::String, SkSL::Program::Settings::Value> inputMap;
    for (const auto& v : fInAndUniformVars) {
        if (v.fQualifier != Variable::Qualifier::kIn) {
            continue;
        }
        // 'in' arrays are not supported
        SkASSERT(!v.isArray());
        SkSL::String name(v.fName.c_str(), v.fName.size());
        switch (v.fType) {
            case Variable::Type::kBool: {
                bool b = *SkTAddOffset<const bool>(inputs, v.fOffset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(b)));
                break;
            }
            case Variable::Type::kInt: {
                int32_t i = *SkTAddOffset<const int32_t>(inputs, v.fOffset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(i)));
                break;
            }
            case Variable::Type::kFloat: {
                float f = *SkTAddOffset<const float>(inputs, v.fOffset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(f)));
                break;
            }
            default:
                SkDEBUGFAIL("Unsupported input variable type");
                return false;
        }
    }

    auto specialized = fCompiler->specialize(*baseProgram, inputMap);
    bool optimized = fCompiler->optimize(*specialized);
    if (!optimized) {
        SkDebugf("%s\n", fCompiler->errorText().c_str());
        SkASSERT(false);
        return false;
    }

    if (!fCompiler->toPipelineStage(*specialized, outCode, outFormatArgs, outFunctions)) {
        SkDebugf("%s\n", fCompiler->errorText().c_str());
        SkASSERT(false);
        return false;
    }

    return true;
}
#endif

SkRuntimeEffect::ByteCodeResult SkRuntimeEffect::toByteCode() {
    auto byteCode = fCompiler->toByteCode(*fBaseProgram);
    return std::make_tuple(std::move(byteCode), SkString(fCompiler->errorText().c_str()));
}

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<SkData> inputs,
                                            sk_sp<SkShader> children[], size_t childCount,
                                            const SkMatrix* localMatrix, bool isOpaque) {
    return inputs->size() >= this->inputSize() && childCount >= this->childCount()
        ? sk_sp<SkShader>(new SkRTShader(sk_ref_sp(this), std::move(inputs), localMatrix,
                                         children, childCount, isOpaque))
        : nullptr;
}

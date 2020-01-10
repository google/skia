/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/effects/SkRuntimeEffect.h"
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
    // TODO: Many errors aren't caught until we process the generated Program here. Catching those
    // in the IR generator would provide better errors messages (with locations).
    #define RETURN_FAILURE(...) return std::make_pair(nullptr, SkStringPrintf(__VA_ARGS__))

    if (!program) {
        RETURN_FAILURE("%s", compiler->errorText().c_str());
    }
    SkASSERT(!compiler->errorCount());

    size_t offset = 0, uniformSize = 0;
    std::vector<Variable> inAndUniformVars;
    std::vector<SkString> children;
    const SkSL::Context& ctx(compiler->context());

    // Gather the inputs in two passes, to de-interleave them in our input layout.
    // We put the uniforms *first*, so that the CPU backend can alias the combined input block as
    // the uniform block when calling the interpreter.
    for (auto flag : { SkSL::Modifiers::kUniform_Flag, SkSL::Modifiers::kIn_Flag }) {
        if (flag == SkSL::Modifiers::kIn_Flag) {
            uniformSize = offset;
        }
        for (const auto& e : *program) {
            if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
                SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
                for (const auto& varStatement : v.fVars) {
                    const SkSL::Variable& var = *((SkSL::VarDeclaration&) *varStatement).fVar;

                    // Sanity check some rules that should be enforced by the IR generator.
                    // These are all layout options that only make sense in .fp files.
                    SkASSERT(!var.fModifiers.fLayout.fKey);
                    SkASSERT((var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) == 0 ||
                        (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) == 0);
                    SkASSERT(var.fModifiers.fLayout.fCType == SkSL::Layout::CType::kDefault);
                    SkASSERT(var.fModifiers.fLayout.fWhen.fLength == 0);
                    SkASSERT((var.fModifiers.fLayout.fFlags & SkSL::Layout::kTracked_Flag) == 0);

                    if (var.fModifiers.fFlags & flag) {
                        if (&var.fType == ctx.fFragmentProcessor_Type.get()) {
                            children.push_back(var.fName);
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
                            RETURN_FAILURE("Invalid input/uniform type: '%s'",
                                           type->displayName().c_str());
                        }

#undef SET_TYPES

                        switch (v.fType) {
                            case Variable::Type::kBool:
                            case Variable::Type::kInt:
                                if (v.fQualifier == Variable::Qualifier::kUniform) {
                                    RETURN_FAILURE("'uniform' variables may not have '%s' type",
                                                   type->displayName().c_str());
                                }
                                break;

                            case Variable::Type::kFloat:
                                // Floats can be 'in' or 'uniform'
                                break;

                            case Variable::Type::kFloat2:
                            case Variable::Type::kFloat3:
                            case Variable::Type::kFloat4:
                            case Variable::Type::kFloat2x2:
                            case Variable::Type::kFloat3x3:
                            case Variable::Type::kFloat4x4:
                                if (v.fQualifier == Variable::Qualifier::kIn) {
                                    RETURN_FAILURE("'in' variables may not have '%s' type",
                                                   type->displayName().c_str());
                                }
                                break;
                        }

                        if (v.fType != Variable::Type::kBool) {
                            offset = SkAlign4(offset);
                        }
                        v.fOffset = offset;
                        offset += v.sizeInBytes();
                        inAndUniformVars.push_back(v);
                    }
                }
            }
        }
    }

#undef RETURN_FAILURE

    sk_sp<SkRuntimeEffect> effect(new SkRuntimeEffect(std::move(sksl),
                                                      std::move(compiler),
                                                      std::move(program),
                                                      std::move(inAndUniformVars),
                                                      std::move(children),
                                                      uniformSize));
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
                                 std::unique_ptr<SkSL::Program> baseProgram,
                                 std::vector<Variable>&& inAndUniformVars,
                                 std::vector<SkString>&& children,
                                 size_t uniformSize)
        : fIndex(new_sksl_index())
        , fSkSL(std::move(sksl))
        , fCompiler(std::move(compiler))
        , fBaseProgram(std::move(baseProgram))
        , fInAndUniformVars(std::move(inAndUniformVars))
        , fChildren(std::move(children))
        , fUniformSize(uniformSize) {
    SkASSERT(fCompiler && fBaseProgram);
    SkASSERT(SkIsAlign4(fUniformSize));
    SkASSERT(fUniformSize <= this->inputSize());
}

size_t SkRuntimeEffect::inputSize() const {
    return fInAndUniformVars.empty()
        ? 0
        : fInAndUniformVars.back().fOffset + fInAndUniformVars.back().sizeInBytes();
}

SkRuntimeEffect::SpecializeResult SkRuntimeEffect::specialize(SkSL::Program& baseProgram,
                                                              const void* inputs) {
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
                return SpecializeResult{nullptr, SkString("Unsupported input variable type")};
        }
    }

    auto specialized = fCompiler->specialize(baseProgram, inputMap);
    bool optimized = fCompiler->optimize(*specialized);
    if (!optimized) {
        return SpecializeResult{nullptr, SkString(fCompiler->errorText().c_str())};
    }
    return SpecializeResult{std::move(specialized), SkString()};
}

#if SK_SUPPORT_GPU
bool SkRuntimeEffect::toPipelineStage(const void* inputs, const GrShaderCaps* shaderCaps,
                                      SkSL::PipelineStageArgs* outArgs) {
    // This function is used by the GPU backend, and can't reuse our previously built fBaseProgram.
    // If the supplied shaderCaps have any non-default values, we have baked in the wrong settings.
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;

    auto baseProgram = fCompiler->convertProgram(SkSL::Program::kPipelineStage_Kind,
                                                 SkSL::String(fSkSL.c_str(), fSkSL.size()),
                                                 settings);
    if (!baseProgram) {
        SkDebugf("%s\n", fCompiler->errorText().c_str());
        SkASSERT(false);
        return false;
    }

    auto specialized = std::get<0>(this->specialize(*baseProgram, inputs));
    if (!specialized) {
        return false;
    }

    if (!fCompiler->toPipelineStage(*specialized, outArgs)) {
        SkDebugf("%s\n", fCompiler->errorText().c_str());
        SkASSERT(false);
        return false;
    }

    return true;
}
#endif

SkRuntimeEffect::ByteCodeResult SkRuntimeEffect::toByteCode(const void* inputs) {
    auto [specialized, errorText] = this->specialize(*fBaseProgram, inputs);
    if (!specialized) {
        return ByteCodeResult{nullptr, errorText};
    }
    auto byteCode = fCompiler->toByteCode(*specialized);
    return ByteCodeResult(std::move(byteCode), SkString(fCompiler->errorText().c_str()));
}

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<SkData> inputs,
                                            sk_sp<SkShader> children[], size_t childCount,
                                            const SkMatrix* localMatrix, bool isOpaque) {
    return inputs && inputs->size() >= this->inputSize() && childCount >= fChildren.size()
        ? sk_sp<SkShader>(new SkRTShader(sk_ref_sp(this), std::move(inputs), localMatrix,
                                         children, childCount, isOpaque))
        : nullptr;
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> inputs) {
    extern sk_sp<SkColorFilter> SkMakeRuntimeColorFilter(sk_sp<SkRuntimeEffect>, sk_sp<SkData>);

    return inputs && inputs->size() >= this->inputSize()
        ? SkMakeRuntimeColorFilter(sk_ref_sp(this), std::move(inputs))
        : nullptr;
}

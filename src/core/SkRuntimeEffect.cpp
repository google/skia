/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkMutex.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkUtils.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrFPArgs.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/generated/GrMatrixEffect.h"
#endif

#include <algorithm>

namespace SkSL {
class SharedCompiler {
public:
    SharedCompiler() : fLock(compiler_mutex()) {
        if (!gCompiler) {
            gCompiler = new SkSL::Compiler{};
        }
    }

    SkSL::Compiler* operator->() const { return gCompiler; }

private:
    SkAutoMutexExclusive fLock;

    static SkMutex& compiler_mutex() {
        static SkMutex& mutex = *(new SkMutex);
        return mutex;
    }

    static SkSL::Compiler* gCompiler;
};
SkSL::Compiler* SharedCompiler::gCompiler = nullptr;
}

// Accepts a valid marker, or "normals(<marker>)"
static bool parse_marker(const SkSL::StringFragment& marker, uint32_t* id, uint32_t* flags) {
    SkString s = marker;
    if (s.startsWith("normals(") && s.endsWith(')')) {
        *flags |= SkRuntimeEffect::Variable::kMarkerNormals_Flag;
        s.set(marker.fChars + 8, marker.fLength - 9);
    }
    if (!SkCanvasPriv::ValidateMarker(s.c_str())) {
        return false;
    }
    *id = SkOpts::hash_fn(s.c_str(), s.size(), 0);
    return true;
}

SkRuntimeEffect::EffectResult SkRuntimeEffect::Make(SkString sksl) {
    SkSL::SharedCompiler compiler;
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
    std::vector<Varying> varyings;
    const SkSL::Context& ctx(compiler->context());

    // Scrape the varyings
    for (const auto& e : *program) {
        if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
            SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
            for (const auto& varStatement : v.fVars) {
                const SkSL::Variable& var = *((SkSL::VarDeclaration&) *varStatement).fVar;

                if (var.fModifiers.fFlags & SkSL::Modifiers::kVarying_Flag) {
                    varyings.push_back({var.fName, var.fType.kind() == SkSL::Type::kVector_Kind
                                                           ? var.fType.columns()
                                                           : 1});
                }
            }
        }
    }

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

                        const SkSL::StringFragment& marker(var.fModifiers.fLayout.fMarker);
                        if (marker.fLength) {
                            // Rules that should be enforced by the IR generator:
                            SkASSERT(v.fQualifier == Variable::Qualifier::kUniform);
                            SkASSERT(v.fType == Variable::Type::kFloat4x4);
                            v.fFlags |= Variable::kMarker_Flag;
                            if (!parse_marker(marker, &v.fMarker, &v.fFlags)) {
                                RETURN_FAILURE("Invalid 'marker' string: '%.*s'",
                                               (int)marker.fLength, marker.fChars);
                            }
                        }

                        if (var.fModifiers.fLayout.fFlags &
                            SkSL::Layout::Flag::kSRGBUnpremul_Flag) {
                            v.fFlags |= Variable::kSRGBUnpremul_Flag;
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
                                                      std::move(program),
                                                      std::move(inAndUniformVars),
                                                      std::move(children),
                                                      std::move(varyings),
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

SkRuntimeEffect::SkRuntimeEffect(SkString sksl,
                                 std::unique_ptr<SkSL::Program> baseProgram,
                                 std::vector<Variable>&& inAndUniformVars,
                                 std::vector<SkString>&& children,
                                 std::vector<Varying>&& varyings,
                                 size_t uniformSize)
        : fHash(SkGoodHash()(sksl))
        , fSkSL(std::move(sksl))
        , fBaseProgram(std::move(baseProgram))
        , fInAndUniformVars(std::move(inAndUniformVars))
        , fChildren(std::move(children))
        , fVaryings(std::move(varyings))
        , fUniformSize(uniformSize) {
    SkASSERT(fBaseProgram);
    SkASSERT(SkIsAlign4(fUniformSize));
    SkASSERT(fUniformSize <= this->inputSize());
}

SkRuntimeEffect::~SkRuntimeEffect() = default;

size_t SkRuntimeEffect::inputSize() const {
    return fInAndUniformVars.empty() ? 0
                                     : SkAlign4(fInAndUniformVars.back().fOffset +
                                                fInAndUniformVars.back().sizeInBytes());
}

const SkRuntimeEffect::Variable* SkRuntimeEffect::findInput(const char* name) const {
    auto iter = std::find_if(fInAndUniformVars.begin(), fInAndUniformVars.end(),
                             [name](const Variable& v) { return v.fName.equals(name); });
    return iter == fInAndUniformVars.end() ? nullptr : &(*iter);
}

int SkRuntimeEffect::findChild(const char* name) const {
    auto iter = std::find_if(fChildren.begin(), fChildren.end(),
                             [name](const SkString& s) { return s.equals(name); });
    return iter == fChildren.end() ? -1 : static_cast<int>(iter - fChildren.begin());
}

SkRuntimeEffect::SpecializeResult
SkRuntimeEffect::specialize(SkSL::Program& baseProgram,
                            const void* inputs,
                            const SkSL::SharedCompiler& compiler) const {
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

    auto specialized = compiler->specialize(baseProgram, inputMap);
    bool optimized = compiler->optimize(*specialized);
    if (!optimized) {
        return SpecializeResult{nullptr, SkString(compiler->errorText().c_str())};
    }
    return SpecializeResult{std::move(specialized), SkString()};
}

#if SK_SUPPORT_GPU
bool SkRuntimeEffect::toPipelineStage(const void* inputs, const GrShaderCaps* shaderCaps,
                                      GrContextOptions::ShaderErrorHandler* errorHandler,
                                      SkSL::PipelineStageArgs* outArgs) {
    SkSL::SharedCompiler compiler;

    // This function is used by the GPU backend, and can't reuse our previously built fBaseProgram.
    // If the supplied shaderCaps have any non-default values, we have baked in the wrong settings.
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;

    auto baseProgram = compiler->convertProgram(SkSL::Program::kPipelineStage_Kind,
                                                SkSL::String(fSkSL.c_str(), fSkSL.size()),
                                                settings);
    if (!baseProgram) {
        errorHandler->compileError(fSkSL.c_str(), compiler->errorText().c_str());
        return false;
    }

    auto [specialized, errorText] = this->specialize(*baseProgram, inputs, compiler);
    if (!specialized) {
        errorHandler->compileError(fSkSL.c_str(), errorText.c_str());
        return false;
    }

    if (!compiler->toPipelineStage(*specialized, outArgs)) {
        errorHandler->compileError(fSkSL.c_str(), compiler->errorText().c_str());
        return false;
    }

    return true;
}
#endif

SkRuntimeEffect::ByteCodeResult SkRuntimeEffect::toByteCode(const void* inputs) const {
    SkSL::SharedCompiler compiler;

    auto [specialized, errorText] = this->specialize(*fBaseProgram, inputs, compiler);
    if (!specialized) {
        return ByteCodeResult{nullptr, errorText};
    }
    auto byteCode = compiler->toByteCode(*specialized);
    return ByteCodeResult(std::move(byteCode), SkString(compiler->errorText().c_str()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<skvm::F32> program_fn(skvm::Builder* p,
                                         const SkSL::ByteCodeFunction& fn,
                                         const std::vector<skvm::F32>& uniform,
                                         std::vector<skvm::F32> stack) {
    auto push = [&](skvm::F32 x) { stack.push_back(x); };
    auto pop  = [&]{ skvm::F32 x = stack.back(); stack.pop_back(); return x; };

    for (int i = 0; i < fn.getLocalCount(); i++) {
        push(p->splat(0.0f));
    }

    for (const uint8_t *ip = fn.code(), *end = ip + fn.size(); ip != end; ) {
        using Inst = SkSL::ByteCodeInstruction;

        auto inst = sk_unaligned_load<Inst>(ip);
        ip += sizeof(Inst);

        auto u8  = [&]{ auto x = sk_unaligned_load<uint8_t >(ip); ip += sizeof(x); return x; };
      //auto u16 = [&]{ auto x = sk_unaligned_load<uint16_t>(ip); ip += sizeof(x); return x; };
        auto u32 = [&]{ auto x = sk_unaligned_load<uint32_t>(ip); ip += sizeof(x); return x; };

        auto unary = [&](Inst base, auto&& fn) {
            const int N = (int)base - (int)inst + 1;
            SkASSERT(0 < N && N <= 4);
            skvm::F32 args[4];
            for (int i = 0; i < N; ++i) {
                args[i] = pop();
            }
            for (int i = N; i --> 0;) {
                push(fn(args[i]));
            }
        };

        auto binary = [&](Inst base, auto&& fn) {
            const int N = (int)base - (int)inst + 1;
            SkASSERT(0 < N && N <= 4);
            skvm::F32 right[4];
            for (int i = 0; i < N; ++i) {
                right[i] = pop();
            }
            skvm::F32 left[4];
            for (int i = 0; i < N; ++i) {
                left[i] = pop();
            }
            for (int i = N; i --> 0;) {
                push(fn(left[i], right[i]));
            }
        };

        switch (inst) {
            default:
                #if 0
                    fn.disassemble();
                    SkDebugf("inst %04x unimplemented\n", inst);
                    __builtin_debugtrap();
                #endif
                return {};

            case Inst::kLoad: {
                int ix = u8();
                push(stack[ix + 0]);
            } break;

            case Inst::kLoad2: {
                int ix = u8();
                push(stack[ix + 0]);
                push(stack[ix + 1]);
            } break;

            case Inst::kLoad3: {
                int ix = u8();
                push(stack[ix + 0]);
                push(stack[ix + 1]);
                push(stack[ix + 2]);
            } break;

            case Inst::kLoad4: {
                int ix = u8();
                push(stack[ix + 0]);
                push(stack[ix + 1]);
                push(stack[ix + 2]);
                push(stack[ix + 3]);
            } break;

            case Inst::kLoadUniform: {
                int ix = u8();
                push(uniform[ix]);
            } break;

            case Inst::kLoadUniform2: {
                int ix = u8();
                push(uniform[ix + 0]);
                push(uniform[ix + 1]);
            } break;

            case Inst::kLoadUniform3: {
                int ix = u8();
                push(uniform[ix + 0]);
                push(uniform[ix + 1]);
                push(uniform[ix + 2]);
            } break;

            case Inst::kLoadUniform4: {
                int ix = u8();
                push(uniform[ix + 0]);
                push(uniform[ix + 1]);
                push(uniform[ix + 2]);
                push(uniform[ix + 3]);
            } break;

            case Inst::kStore: {
                int ix = u8();
                stack[ix + 0] = pop();
            } break;

            case Inst::kStore2: {
                int ix = u8();
                stack[ix + 1] = pop();
                stack[ix + 0] = pop();
            } break;

            case Inst::kStore3: {
                int ix = u8();
                stack[ix + 2] = pop();
                stack[ix + 1] = pop();
                stack[ix + 0] = pop();
            } break;

            case Inst::kStore4: {
                int ix = u8();
                stack[ix + 3] = pop();
                stack[ix + 2] = pop();
                stack[ix + 1] = pop();
                stack[ix + 0] = pop();
            } break;


            case Inst::kPushImmediate: {
                push(bit_cast(p->splat(u32())));
            } break;

            case Inst::kDup: {
                push(stack[stack.size() - 1]);
            } break;

            case Inst::kDup2: {
                push(stack[stack.size() - 2]);
                push(stack[stack.size() - 2]);
            } break;

            case Inst::kDup3: {
                push(stack[stack.size() - 3]);
                push(stack[stack.size() - 3]);
                push(stack[stack.size() - 3]);
            } break;

            case Inst::kDup4: {
                push(stack[stack.size() - 4]);
                push(stack[stack.size() - 4]);
                push(stack[stack.size() - 4]);
                push(stack[stack.size() - 4]);
            } break;

            case Inst::kAddF:
            case Inst::kAddF2:
            case Inst::kAddF3:
            case Inst::kAddF4: binary(Inst::kAddF, std::plus<>{}); break;

            case Inst::kSubtractF:
            case Inst::kSubtractF2:
            case Inst::kSubtractF3:
            case Inst::kSubtractF4: binary(Inst::kSubtractF, std::minus<>{}); break;

            case Inst::kMultiplyF:
            case Inst::kMultiplyF2:
            case Inst::kMultiplyF3:
            case Inst::kMultiplyF4: binary(Inst::kMultiplyF, std::multiplies<>{}); break;

            case Inst::kDivideF:
            case Inst::kDivideF2:
            case Inst::kDivideF3:
            case Inst::kDivideF4: binary(Inst::kDivideF, std::divides<>{}); break;

            case Inst::kATan:
            case Inst::kATan2:
            case Inst::kATan3:
            case Inst::kATan4: unary(Inst::kATan, skvm::approx_atan); break;

            case Inst::kFract:
            case Inst::kFract2:
            case Inst::kFract3:
            case Inst::kFract4: unary(Inst::kFract, skvm::fract); break;

            case Inst::kSqrt:
            case Inst::kSqrt2:
            case Inst::kSqrt3:
            case Inst::kSqrt4: unary(Inst::kSqrt, skvm::sqrt); break;

            case Inst::kSin:
            case Inst::kSin2:
            case Inst::kSin3:
            case Inst::kSin4: unary(Inst::kSin, skvm::approx_sin); break;

            // Baby steps... just leaving test conditions on the stack for now.
            case Inst::kMaskPush:   break;
            case Inst::kMaskNegate: break;

            case Inst::kCompareFLT: {
                skvm::F32 x = pop(),
                          a = pop();
                push(bit_cast(a<x));
            } break;

            case Inst::kMaskBlend: {
                std::vector<skvm::F32> if_true,
                                       if_false;
                int count = u8();
                for (int i = 0; i < count; i++) { if_false.push_back(pop()); }
                for (int i = 0; i < count; i++) { if_true .push_back(pop()); }

                skvm::I32 cond = bit_cast(pop());
                for (int i = count; i --> 0; ) {
                    push(select(cond, if_true[i], if_false[i]));
                }
            } break;

            case Inst::kReturn: {
                SkAssertResult(u8() == 0);
                SkASSERT(ip == end);
            } break;
        }
    }
    for (int i = 0; i < fn.getLocalCount(); i++) {
        pop();
    }
    return stack;
}


class SkRuntimeColorFilter : public SkColorFilter {
public:
    SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs,
                         sk_sp<SkColorFilter> children[], size_t childCount)
            : fEffect(std::move(effect))
            , fInputs(std::move(inputs))
            , fChildren(children, children + childCount) {}

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext* context, const GrColorInfo& colorInfo) const override {
        auto fp = GrSkSLFP::Make(context, fEffect, "Runtime_Color_Filter", fInputs);
        for (const auto& child : fChildren) {
            auto childFP = child ? child->asFragmentProcessor(context, colorInfo) : nullptr;
            if (!childFP) {
                // TODO: This is the case that should eventually mean "the original input color"
                return nullptr;
            }
            fp->addChild(std::move(childFP));
        }
        return std::move(fp);
    }
#endif

    const SkSL::ByteCode* byteCode() const {
        SkAutoMutexExclusive ama(fByteCodeMutex);
        if (!fByteCode) {
            auto [byteCode, errorText] = fEffect->toByteCode(fInputs->data());
            if (!byteCode) {
                SkDebugf("%s\n", errorText.c_str());
                return nullptr;
            }
            fByteCode = std::move(byteCode);
        }
        return fByteCode.get();
    }

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        auto ctx = rec.fAlloc->make<SkRasterPipeline_InterpreterCtx>();
        // don't need to set ctx->paintColor
        ctx->inputs = fInputs;
        ctx->ninputs = fEffect->uniformSize() / 4;
        ctx->shaderConvention = false;

        ctx->byteCode = this->byteCode();
        if (!ctx->byteCode) {
            return false;
        }

        ctx->fn = ctx->byteCode->getFunction("main");
        rec.fPipeline->append(SkRasterPipeline::interpreter, ctx);
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          SkColorSpace* /*dstCS*/,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override {
        const SkSL::ByteCode* bc = this->byteCode();
        if (!bc) {
            return {};
        }

        const SkSL::ByteCodeFunction* fn = bc->getFunction("main");
        if (!fn) {
            return {};
        }

        std::vector<skvm::F32> uniform;
        for (int i = 0; i < (int)fEffect->uniformSize() / 4; i++) {
            float f;
            memcpy(&f, (const char*)fInputs->data() + 4*i, 4);
            uniform.push_back(p->uniformF(uniforms->pushF(f)));
        }

        std::vector<skvm::F32> stack =
            program_fn(p, *fn, uniform, {c.r, c.g, c.b, c.a});

        if (stack.size() == 4) {
            return {stack[0], stack[1], stack[2], stack[3]};
        }
        return {};
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        if (fInputs) {
            buffer.writeDataAsByteArray(fInputs.get());
        } else {
            buffer.writeByteArray(nullptr, 0);
        }
        buffer.write32(fChildren.size());
        for (const auto& child : fChildren) {
            buffer.writeFlattenable(child.get());
        }
    }

    SK_FLATTENABLE_HOOKS(SkRuntimeColorFilter)

private:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData> fInputs;
    std::vector<sk_sp<SkColorFilter>> fChildren;

    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;
};

sk_sp<SkFlattenable> SkRuntimeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();

    auto effect = std::get<0>(SkRuntimeEffect::Make(std::move(sksl)));
    if (!effect) {
        buffer.validate(false);
        return nullptr;
    }

    size_t childCount = buffer.read32();
    if (childCount != effect->children().count()) {
        buffer.validate(false);
        return nullptr;
    }

    std::vector<sk_sp<SkColorFilter>> children;
    children.resize(childCount);
    for (size_t i = 0; i < children.size(); ++i) {
        children[i] = buffer.readColorFilter();
    }

    return effect->makeColorFilter(std::move(inputs), children.data(), children.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs, const SkMatrix* localMatrix,
               sk_sp<SkShader>* children, size_t childCount, bool isOpaque)
            : SkShaderBase(localMatrix)
            , fEffect(std::move(effect))
            , fIsOpaque(isOpaque)
            , fInputs(std::move(inputs))
            , fChildren(children, children + childCount) {}

    bool isOpaque() const override { return fIsOpaque; }

    sk_sp<SkData> getUniforms(const SkMatrixProvider& matrixProvider,
                              const SkColorSpace* dstCS) const {
        using Flags = SkRuntimeEffect::Variable::Flags;
        using Type = SkRuntimeEffect::Variable::Type;
        SkColorSpaceXformSteps steps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                     dstCS,               kUnpremul_SkAlphaType);

        sk_sp<SkData> inputs = nullptr;
        auto writableData = [&]() {
            if (!inputs) {
                inputs =  SkData::MakeWithCopy(fInputs->data(), fInputs->size());
            }
            return inputs->writable_data();
        };

        for (const auto& v : fEffect->inputs()) {
            if (v.fFlags & Flags::kMarker_Flag) {
                SkASSERT(v.fType == Type::kFloat4x4);
                SkM44* localToMarker = SkTAddOffset<SkM44>(writableData(), v.fOffset);
                if (!matrixProvider.getLocalToMarker(v.fMarker, localToMarker)) {
                    // We couldn't provide a matrix that was requested by the SkSL
                    return nullptr;
                }
                if (v.fFlags & Flags::kMarkerNormals_Flag) {
                    // Normals need to be transformed by the inverse-transpose of the upper-left
                    // 3x3 portion (scale + rotate) of the matrix.
                    localToMarker->setRow(3, {0, 0, 0, 1});
                    localToMarker->setCol(3, {0, 0, 0, 1});
                    if (!localToMarker->invert(localToMarker)) {
                        return nullptr;
                    }
                    *localToMarker = localToMarker->transpose();
                }
            } else if (v.fFlags & Flags::kSRGBUnpremul_Flag) {
                SkASSERT(v.fType == Type::kFloat3 || v.fType == Type::kFloat4);
                if (steps.flags.mask()) {
                    float* color = SkTAddOffset<float>(writableData(), v.fOffset);
                    if (v.fType == Type::kFloat4) {
                        // RGBA, easy case
                        for (int i = 0; i < v.fCount; ++i) {
                            steps.apply(color);
                            color += 4;
                        }
                    } else {
                        // RGB, need to pad out to include alpha. Technically, this isn't necessary,
                        // because steps shouldn't include unpremul or premul, and thus shouldn't
                        // read or write the fourth element. But let's be safe.
                        float rgba[4];
                        for (int i = 0; i < v.fCount; ++i) {
                            memcpy(rgba, color, 3 * sizeof(float));
                            rgba[3] = 1.0f;
                            steps.apply(rgba);
                            memcpy(color, rgba, 3 * sizeof(float));
                            color += 3;
                        }
                    }
                }
            }
        }
        return inputs ? inputs : fInputs;
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args) const override {
        SkMatrix matrix;
        if (!this->totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }

        sk_sp<SkData> inputs =
                this->getUniforms(args.fMatrixProvider, args.fDstColorInfo->colorSpace());
        if (!inputs) {
            return nullptr;
        }

        auto fp = GrSkSLFP::Make(args.fContext, fEffect, "runtime_shader", std::move(inputs));
        for (const auto& child : fChildren) {
            auto childFP = child ? as_SB(child)->asFragmentProcessor(args) : nullptr;
            if (!childFP) {
                // TODO: This is the case that should eventually mean "the original input color"
                return nullptr;
            }
            fp->addChild(std::move(childFP));
        }
        std::unique_ptr<GrFragmentProcessor> result = std::move(fp);
        if (!matrix.isIdentity()) {
            result = GrMatrixEffect::Make(matrix, std::move(result));
        }
        if (GrColorTypeClampType(args.fDstColorInfo->colorType()) != GrClampType::kNone) {
            return GrFragmentProcessor::ClampPremulOutput(std::move(result));
        } else {
            return result;
        }
    }
#endif

    const SkSL::ByteCode* byteCode() const {
        SkAutoMutexExclusive ama(fByteCodeMutex);
        if (!fByteCode) {
            auto [byteCode, errorText] = fEffect->toByteCode(fInputs->data());
            if (!byteCode) {
                SkDebugf("%s\n", errorText.c_str());
                return nullptr;
            }
            fByteCode = std::move(byteCode);
        }
        return fByteCode.get();
    }

    bool onAppendStages(const SkStageRec& rec) const override {
        SkMatrix inverse;
        if (!this->computeTotalInverse(rec.fMatrixProvider.localToDevice(), rec.fLocalM,
                                       &inverse)) {
            return false;
        }

        auto ctx = rec.fAlloc->make<SkRasterPipeline_InterpreterCtx>();
        ctx->paintColor = rec.fPaint.getColor4f();
        ctx->inputs = this->getUniforms(rec.fMatrixProvider, rec.fDstCS);
        if (!ctx->inputs) {
            return false;
        }
        ctx->ninputs = fEffect->uniformSize() / 4;
        ctx->shaderConvention = true;

        ctx->byteCode = this->byteCode();
        if (!ctx->byteCode) {
            return false;
        }
        ctx->fn = ctx->byteCode->getFunction("main");
        rec.fPipeline->append(SkRasterPipeline::seed_shader);
        rec.fPipeline->append_matrix(rec.fAlloc, inverse);
        rec.fPipeline->append(SkRasterPipeline::interpreter, ctx);
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::F32 x, skvm::F32 y, skvm::Color paint,
                          const SkMatrix& ctm, const SkMatrix* localM,
                          SkFilterQuality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override {
        const SkSL::ByteCode* bc = this->byteCode();
        if (!bc) {
            return {};
        }

        const SkSL::ByteCodeFunction* fn = bc->getFunction("main");
        if (!fn) {
            return {};
        }

        // TODO: Eventually, plumb SkMatrixProvider here (instead of just ctm). For now, we will
        // simply fail if our effect requires any marked matrices (SkSimpleMatrixProvider always
        // returns false in getLocalToMarker).
        SkSimpleMatrixProvider matrixProvider(SkMatrix::I());
        sk_sp<SkData> inputs = this->getUniforms(matrixProvider, dst.colorSpace());
        if (!inputs) {
            return {};
        }

        std::vector<skvm::F32> uniform;
        for (int i = 0; i < (int)fEffect->uniformSize() / 4; i++) {
            float f;
            memcpy(&f, (const char*)inputs->data() + 4*i, 4);
            uniform.push_back(p->uniformF(uniforms->pushF(f)));
        }

        SkMatrix inv;
        if (!this->computeTotalInverse(ctm, localM, &inv)) {
            return {};
        }
        SkShaderBase::ApplyMatrix(p,inv, &x,&y,uniforms);

        std::vector<skvm::F32> stack =
            program_fn(p, *fn, uniform, {x,y, paint.r, paint.g, paint.b, paint.a});

        if (stack.size() == 6) {
            return {stack[2], stack[3], stack[4], stack[5]};
        }
        return {};
    }

    void flatten(SkWriteBuffer& buffer) const override {
        uint32_t flags = 0;
        if (fIsOpaque) {
            flags |= kIsOpaque_Flag;
        }
        if (!this->getLocalMatrix().isIdentity()) {
            flags |= kHasLocalMatrix_Flag;
        }

        buffer.writeString(fEffect->source().c_str());
        if (fInputs) {
            buffer.writeDataAsByteArray(fInputs.get());
        } else {
            buffer.writeByteArray(nullptr, 0);
        }
        buffer.write32(flags);
        if (flags & kHasLocalMatrix_Flag) {
            buffer.writeMatrix(this->getLocalMatrix());
        }
        buffer.write32(fChildren.size());
        for (const auto& child : fChildren) {
            buffer.writeFlattenable(child.get());
        }
    }

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    SK_FLATTENABLE_HOOKS(SkRTShader)

private:
    enum Flags {
        kIsOpaque_Flag          = 1 << 0,
        kHasLocalMatrix_Flag    = 1 << 1,
    };

    sk_sp<SkRuntimeEffect> fEffect;
    bool fIsOpaque;

    sk_sp<SkData> fInputs;
    std::vector<sk_sp<SkShader>> fChildren;

    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;
};

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();
    uint32_t flags = buffer.read32();

    bool isOpaque = SkToBool(flags & kIsOpaque_Flag);
    SkMatrix localM, *localMPtr = nullptr;
    if (flags & kHasLocalMatrix_Flag) {
        buffer.readMatrix(&localM);
        localMPtr = &localM;
    }

    auto effect = std::get<0>(SkRuntimeEffect::Make(std::move(sksl)));
    if (!effect) {
        buffer.validate(false);
        return nullptr;
    }

    size_t childCount = buffer.read32();
    if (childCount != effect->children().count()) {
        buffer.validate(false);
        return nullptr;
    }

    std::vector<sk_sp<SkShader>> children;
    children.resize(childCount);
    for (size_t i = 0; i < children.size(); ++i) {
        children[i] = buffer.readShader();
    }

    return effect->makeShader(std::move(inputs), children.data(), children.size(), localMPtr,
                              isOpaque);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<SkData> inputs,
                                            sk_sp<SkShader> children[], size_t childCount,
                                            const SkMatrix* localMatrix, bool isOpaque) {
    if (!inputs) {
        inputs = SkData::MakeEmpty();
    }
    return inputs->size() == this->inputSize() && childCount == fChildren.size()
        ? sk_sp<SkShader>(new SkRTShader(sk_ref_sp(this), std::move(inputs), localMatrix,
                                         children, childCount, isOpaque))
        : nullptr;
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> inputs,
                                                      sk_sp<SkColorFilter> children[],
                                                      size_t childCount) {
    if (!inputs) {
        inputs = SkData::MakeEmpty();
    }
    return inputs && inputs->size() == this->inputSize() && childCount == fChildren.size()
        ? sk_sp<SkColorFilter>(new SkRuntimeColorFilter(sk_ref_sp(this), std::move(inputs),
                                                        children, childCount))
        : nullptr;
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> inputs) {
    return this->makeColorFilter(std::move(inputs), nullptr, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkRuntimeEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkRuntimeColorFilter);
    SK_REGISTER_FLATTENABLE(SkRTShader);
}

SkRuntimeShaderBuilder::SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect> effect)
    : fEffect(std::move(effect))
    , fInputs(SkData::MakeUninitialized(fEffect->inputSize()))
    , fChildren(fEffect->children().count()) {}

SkRuntimeShaderBuilder::~SkRuntimeShaderBuilder() = default;

sk_sp<SkShader> SkRuntimeShaderBuilder::makeShader(const SkMatrix* localMatrix, bool isOpaque) {
    return fEffect->makeShader(fInputs, fChildren.data(), fChildren.size(), localMatrix, isOpaque);
}

SkRuntimeShaderBuilder::BuilderChild&
SkRuntimeShaderBuilder::BuilderChild::operator=(const sk_sp<SkShader>& val) {
    if (fIndex < 0) {
        SkDEBUGFAIL("Assigning to missing child");
    } else {
        fOwner->fChildren[fIndex] = val;
    }
    return *this;
}

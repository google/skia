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
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkUtils.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrFPArgs.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
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
}  // namespace SkSL

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

static bool init_variable_type(const SkSL::Context& ctx,
                               const SkSL::Type* type,
                               SkRuntimeEffect::Variable* v) {
#define SET_TYPES(cpuType, gpuType)                          \
    do {                                                     \
        v->fType = SkRuntimeEffect::Variable::Type::cpuType; \
        v->fGPUType = gpuType;                               \
        return true;                                         \
    } while (false)

    if (type == ctx.fBool_Type.get())     { SET_TYPES(kBool,     kVoid_GrSLType);     }
    if (type == ctx.fInt_Type.get())      { SET_TYPES(kInt,      kVoid_GrSLType);     }
    if (type == ctx.fFloat_Type.get())    { SET_TYPES(kFloat,    kFloat_GrSLType);    }
    if (type == ctx.fHalf_Type.get())     { SET_TYPES(kFloat,    kHalf_GrSLType);     }
    if (type == ctx.fFloat2_Type.get())   { SET_TYPES(kFloat2,   kFloat2_GrSLType);   }
    if (type == ctx.fHalf2_Type.get())    { SET_TYPES(kFloat2,   kHalf2_GrSLType);    }
    if (type == ctx.fFloat3_Type.get())   { SET_TYPES(kFloat3,   kFloat3_GrSLType);   }
    if (type == ctx.fHalf3_Type.get())    { SET_TYPES(kFloat3,   kHalf3_GrSLType);    }
    if (type == ctx.fFloat4_Type.get())   { SET_TYPES(kFloat4,   kFloat4_GrSLType);   }
    if (type == ctx.fHalf4_Type.get())    { SET_TYPES(kFloat4,   kHalf4_GrSLType);    }
    if (type == ctx.fFloat2x2_Type.get()) { SET_TYPES(kFloat2x2, kFloat2x2_GrSLType); }
    if (type == ctx.fHalf2x2_Type.get())  { SET_TYPES(kFloat2x2, kHalf2x2_GrSLType);  }
    if (type == ctx.fFloat3x3_Type.get()) { SET_TYPES(kFloat3x3, kFloat3x3_GrSLType); }
    if (type == ctx.fHalf3x3_Type.get())  { SET_TYPES(kFloat3x3, kHalf3x3_GrSLType);  }
    if (type == ctx.fFloat4x4_Type.get()) { SET_TYPES(kFloat4x4, kFloat4x4_GrSLType); }
    if (type == ctx.fHalf4x4_Type.get())  { SET_TYPES(kFloat4x4, kHalf4x4_GrSLType);  }

#undef SET_TYPES

    return false;
}

SkRuntimeEffect::EffectResult SkRuntimeEffect::Make(SkString sksl) {
    SkSL::SharedCompiler compiler;
    auto program = compiler->convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(sksl.c_str(), sksl.size()),
                                            SkSL::Program::Settings());
    // TODO: Many errors aren't caught until we process the generated Program here. Catching those
    // in the IR generator would provide better errors messages (with locations).
    #define RETURN_FAILURE(...) return std::make_tuple(nullptr, SkStringPrintf(__VA_ARGS__))

    if (!program) {
        RETURN_FAILURE("%s", compiler->errorText().c_str());
    }
    SkASSERT(!compiler->errorCount());

    bool hasMain = false;
    const bool usesSampleCoords = SkSL::Analysis::ReferencesSampleCoords(*program);
    const bool usesFragCoords   = SkSL::Analysis::ReferencesFragCoords(*program);

    // Color filters are not allowed to depend on position (local or device) in any way, but they
    // can sample children with matrices or explicit coords. Because the children are color filters,
    // we know (by induction) that they don't use those coords, so we keep the overall invariant.
    //
    // Further down, we also ensure that color filters can't use layout(marker), which would allow
    // them to change behavior based on the CTM.
    bool allowColorFilter = !usesSampleCoords && !usesFragCoords;

    std::vector<const SkSL::Variable*> inVars;
    std::vector<const SkSL::Variable*> uniformVars;
    std::vector<SkString> children;
    std::vector<SkSL::SampleUsage> sampleUsages;
    std::vector<Varying> varyings;
    const SkSL::Context& ctx(compiler->context());

    // Go through program elements, pulling out information that we need
    for (const auto& elem : *program) {
        // Variables (in, uniform, varying, etc.)
        if (elem.fKind == SkSL::ProgramElement::kVar_Kind) {
            const auto& varDecls = static_cast<const SkSL::VarDeclarations&>(elem);
            for (const auto& varDecl : varDecls.fVars) {
                const SkSL::Variable& var =
                        *(static_cast<const SkSL::VarDeclaration&>(*varDecl).fVar);

                // Varyings (only used in conjunction with drawVertices)
                if (var.fModifiers.fFlags & SkSL::Modifiers::kVarying_Flag) {
                    varyings.push_back({var.fName, var.fType.kind() == SkSL::Type::kVector_Kind
                                                           ? var.fType.columns()
                                                           : 1});
                }
                // Fragment Processors (aka 'shader'): These are child effects
                else if (&var.fType == ctx.fFragmentProcessor_Type.get()) {
                    children.push_back(var.fName);
                    sampleUsages.push_back(SkSL::Analysis::GetSampleUsage(*program, var));
                }
                // 'in' variables (other than fragment processors)
                else if (var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                    inVars.push_back(&var);
                }
                // 'uniform' variables
                else if (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                    uniformVars.push_back(&var);
                }
            }
        }
        // Functions
        else if (elem.fKind == SkSL::ProgramElement::kFunction_Kind) {
            const auto& func = static_cast<const SkSL::FunctionDefinition&>(elem);
            const SkSL::FunctionDeclaration& decl = func.fDeclaration;
            if (decl.fName == "main") {
                hasMain = true;
            }
        }
    }

    if (!hasMain) {
        RETURN_FAILURE("missing 'main' function");
    }

    size_t offset = 0, uniformSize = 0;
    std::vector<Variable> inAndUniformVars;
    inAndUniformVars.reserve(inVars.size() + uniformVars.size());

    // We've gathered the 'in' and 'uniform' variables in separate lists. We build a single list of
    // both, in our own structure. We put the uniforms *first* in our input layout, so that the CPU
    // backend can alias the combined input block as the uniform block when calling the interpreter.
    for (bool uniform : {true, false}) {
        if (!uniform) {
            uniformSize = offset;
        }
        for (const SkSL::Variable* var : (uniform ? uniformVars : inVars)) {
            Variable v;
            v.fName = var->fName;
            v.fFlags = 0;
            v.fQualifier = (var->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag)
                                   ? Variable::Qualifier::kUniform
                                   : Variable::Qualifier::kIn;
            v.fCount = 1;

            const SkSL::Type* type = &var->fType;
            if (type->kind() == SkSL::Type::kArray_Kind) {
                v.fFlags |= Variable::kArray_Flag;
                v.fCount = type->columns();
                type = &type->componentType();
            }

            if (!init_variable_type(ctx, type, &v)) {
                RETURN_FAILURE("Invalid input/uniform type: '%s'", type->displayName().c_str());
            }

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

            const SkSL::StringFragment& marker(var->fModifiers.fLayout.fMarker);
            if (marker.fLength) {
                v.fFlags |= Variable::kMarker_Flag;
                allowColorFilter = false;
                if (!parse_marker(marker, &v.fMarker, &v.fFlags)) {
                    RETURN_FAILURE("Invalid 'marker' string: '%.*s'", (int)marker.fLength,
                                   marker.fChars);
                }
            }

            if (var->fModifiers.fLayout.fFlags & SkSL::Layout::Flag::kSRGBUnpremul_Flag) {
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

#undef RETURN_FAILURE

    sk_sp<SkRuntimeEffect> effect(new SkRuntimeEffect(std::move(sksl),
                                                      std::move(program),
                                                      std::move(inAndUniformVars),
                                                      std::move(children),
                                                      std::move(sampleUsages),
                                                      std::move(varyings),
                                                      uniformSize,
                                                      usesSampleCoords,
                                                      allowColorFilter));
    return std::make_tuple(std::move(effect), SkString());
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
                                 std::vector<SkSL::SampleUsage>&& sampleUsages,
                                 std::vector<Varying>&& varyings,
                                 size_t uniformSize,
                                 bool usesSampleCoords,
                                 bool allowColorFilter)
        : fHash(SkGoodHash()(sksl))
        , fSkSL(std::move(sksl))
        , fBaseProgram(std::move(baseProgram))
        , fInAndUniformVars(std::move(inAndUniformVars))
        , fChildren(std::move(children))
        , fSampleUsages(std::move(sampleUsages))
        , fVaryings(std::move(varyings))
        , fUniformSize(uniformSize)
        , fUsesSampleCoords(usesSampleCoords)
        , fAllowColorFilter(allowColorFilter) {
    SkASSERT(fBaseProgram);
    SkASSERT(SkIsAlign4(fUniformSize));
    SkASSERT(fUniformSize <= this->inputSize());
    SkASSERT(fChildren.size() == fSampleUsages.size());
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

using SampleChildFn = std::function<skvm::Color(int, skvm::Coord)>;

static std::vector<skvm::F32> program_fn(skvm::Builder* p,
                                         const SkSL::ByteCodeFunction& fn,
                                         const std::vector<skvm::F32>& uniform,
                                         std::vector<skvm::F32> stack,
                                         SampleChildFn sampleChild,
                                         skvm::Coord device, skvm::Coord local) {
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

        auto unary = [&](auto&& fn) {
            int N = u8();
            std::vector<skvm::F32> a(N);
            for (int i = N; i --> 0; ) { a[i] = pop(); }

            for (int i = 0; i < N; i++) {
                push(fn(a[i]));
            }
        };

        auto binary = [&](auto&& fn) {
            int N = u8();
            std::vector<skvm::F32> a(N), b(N);
            for (int i = N; i --> 0; ) { b[i] = pop(); }
            for (int i = N; i --> 0; ) { a[i] = pop(); }

            for (int i = 0; i < N; i++) {
                push(fn(a[i], b[i]));
            }
        };

        auto ternary = [&](auto&& fn) {
            int N = u8();
            std::vector<skvm::F32> a(N), b(N), c(N);
            for (int i = N; i --> 0; ) { c[i] = pop(); }
            for (int i = N; i --> 0; ) { b[i] = pop(); }
            for (int i = N; i --> 0; ) { a[i] = pop(); }

            for (int i = 0; i < N; i++) {
                push(fn(a[i], b[i], c[i]));
            }
        };

        auto sample = [&](int ix, skvm::Coord coord) {
            if (skvm::Color c = sampleChild(ix, coord)) {
                push(c.r);
                push(c.g);
                push(c.b);
                push(c.a);
                return true;
            }
            return false;
        };

        switch (inst) {
            default:
                #if 0
                    fn.disassemble();
                    SkDebugf("inst %04x unimplemented\n", inst);
                    __builtin_debugtrap();
                #endif
                return {};

            case Inst::kSample: {
                // Child shader to run.
                int ix = u8();
                if (!sample(ix, local)) {
                    return {};
                }
            } break;

            case Inst::kSampleMatrix: {
                // Child shader to run.
                int ix = u8();

                // Stack contains matrix to apply to sample coordinates.
                skvm::F32 m[9];
                for (int i = 9; i --> 0; ) { m[i] = pop(); }

                // TODO: Optimize this for simpler matrices
                skvm::F32 x = m[0]*local.x + m[3]*local.y + m[6],
                          y = m[1]*local.x + m[4]*local.y + m[7],
                          w = m[2]*local.x + m[5]*local.y + m[8];
                x = x * (1.0f / w);
                y = y * (1.0f / w);

                if (!sample(ix, {x,y})) {
                    return {};
                }
            } break;

            case Inst::kSampleExplicit: {
                // Child shader to run.
                int ix = u8();

                // Stack contains x,y to sample at.
                skvm::F32 y = pop(),
                          x = pop();

                if (!sample(ix, {x,y})) {
                    return {};
                }
            } break;

            case Inst::kLoad: {
                int N  = u8(),
                    ix = u8();
                for (int i = 0; i < N; ++i) {
                    push(stack[ix + i]);
                }
            } break;

            case Inst::kLoadUniform: {
                int N  = u8(),
                    ix = u8();
                for (int i = 0; i < N; ++i) {
                    push(uniform[ix + i]);
                }
            } break;

            case Inst::kLoadFragCoord: {
                // TODO: Actually supply Z and 1/W from the rasterizer?
                push(device.x);
                push(device.y);
                push(p->splat(0.0f));  // Z
                push(p->splat(1.0f));  // 1/W
            } break;

            case Inst::kStore: {
                int N  = u8(),
                    ix = u8();
                for (int i = N; i --> 0; ) {
                    stack[ix + i] = pop();
                }
            } break;

            case Inst::kPushImmediate: {
                push(bit_cast(p->splat(u32())));
            } break;

            case Inst::kDup: {
                int N = u8();
                for (int i = 0; i < N; ++i) {
                    push(stack[stack.size() - N]);
                }
            } break;

            case Inst::kSwizzle: {
                skvm::F32 tmp[4];
                for (int i = u8(); i --> 0;) {
                    tmp[i] = pop();
                }
                for (int i = u8(); i --> 0;) {
                    push(tmp[u8()]);
                }
            } break;

            case Inst::kAddF:      binary(std::plus<>{});       break;
            case Inst::kSubtractF: binary(std::minus<>{});      break;
            case Inst::kMultiplyF: binary(std::multiplies<>{}); break;
            case Inst::kDivideF:   binary(std::divides<>{});    break;
            case Inst::kNegateF:    unary(std::negate<>{});     break;

            case Inst::kMinF:
                binary([](skvm::F32 x, skvm::F32 y) { return skvm::min(x,y); });
                break;

            case Inst::kMaxF:
                binary([](skvm::F32 x, skvm::F32 y) { return skvm::max(x,y); });
                break;

            case Inst::kPow:
                binary([](skvm::F32 x, skvm::F32 y) { return skvm::approx_powf(x,y); });
                break;

            case Inst::kLerp:
                ternary([](skvm::F32 x, skvm::F32 y, skvm::F32 t) { return skvm::lerp(x, y, t); });
                break;

            case Inst::kATan:  unary(skvm::approx_atan); break;
            case Inst::kCeil:  unary(skvm::ceil);        break;
            case Inst::kFloor: unary(skvm::floor);       break;
            case Inst::kFract: unary(skvm::fract);       break;
            case Inst::kSqrt:  unary(skvm::sqrt);        break;
            case Inst::kSin:   unary(skvm::approx_sin);  break;

            case Inst::kMatrixMultiply: {
                // Computes M = A*B (all stored column major)
                int aCols = u8(),
                    aRows = u8(),
                    bCols = u8(),
                    bRows = aCols;
                std::vector<skvm::F32> A(aCols*aRows),
                                       B(bCols*bRows);
                for (auto i = B.size(); i --> 0;) { B[i] = pop(); }
                for (auto i = A.size(); i --> 0;) { A[i] = pop(); }

                for (int c = 0; c < bCols; ++c)
                for (int r = 0; r < aRows; ++r) {
                    skvm::F32 sum = p->splat(0.0f);
                    for (int j = 0; j < aCols; ++j) {
                        sum += A[j*aRows + r] * B[c*bRows + j];
                    }
                    push(sum);
                }
            } break;

            // Baby steps... just leaving test conditions on the stack for now.
            case Inst::kMaskPush:   break;
            case Inst::kMaskNegate: break;

            case Inst::kCompareFLT:
                binary([](skvm::F32 x, skvm::F32 y) { return bit_cast(x<y); });
                break;

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

static sk_sp<SkData> get_xformed_inputs(const SkRuntimeEffect* effect,
                                        sk_sp<SkData> baseInputs,
                                        const SkMatrixProvider* matrixProvider,
                                        const SkColorSpace* dstCS) {
    using Flags = SkRuntimeEffect::Variable::Flags;
    using Type = SkRuntimeEffect::Variable::Type;
    SkColorSpaceXformSteps steps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                 dstCS,               kUnpremul_SkAlphaType);

    sk_sp<SkData> inputs = nullptr;
    auto writableData = [&]() {
        if (!inputs) {
            inputs =  SkData::MakeWithCopy(baseInputs->data(), baseInputs->size());
        }
        return inputs->writable_data();
    };

    for (const auto& v : effect->inputs()) {
        if (v.fFlags & Flags::kMarker_Flag) {
            SkASSERT(v.fType == Type::kFloat4x4);
            // Color filters don't provide a matrix provider, but shouldn't be allowed to get here
            SkASSERT(matrixProvider);
            SkM44* localToMarker = SkTAddOffset<SkM44>(writableData(), v.fOffset);
            if (!matrixProvider->getLocalToMarker(v.fMarker, localToMarker)) {
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
    return inputs ? inputs : baseInputs;
}

class SkRuntimeColorFilter : public SkColorFilterBase {
public:
    SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> inputs,
                         sk_sp<SkColorFilter> children[],
                         size_t childCount)
            : fEffect(std::move(effect))
            , fInputs(std::move(inputs))
            , fChildren(children, children + childCount) {}

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& colorInfo) const override {
        sk_sp<SkData> inputs =
                get_xformed_inputs(fEffect.get(), fInputs, nullptr, colorInfo.colorSpace());
        if (!inputs) {
            return GrFPFailure(nullptr);
        }

        auto fp = GrSkSLFP::Make(context, fEffect, "Runtime_Color_Filter", std::move(inputs));
        for (const auto& child : fChildren) {
            std::unique_ptr<GrFragmentProcessor> childFP;
            if (child) {
                bool success;
                std::tie(success, childFP) = as_CFB(child)->asFragmentProcessor(
                        /*inputFP=*/nullptr, context, colorInfo);
                if (!success) {
                    return GrFPFailure(std::move(inputFP));
                }
            }
            fp->addChild(std::move(childFP));
        }

        // Runtime effect scripts are written to take an input color, not a fragment processor.
        // We need to pass the input to the runtime filter using Compose. This ensures that it will
        // be invoked exactly once, and the result will be returned when null children are sampled,
        // or as the (default) input color for non-null children.
        return GrFPSuccess(GrFragmentProcessor::Compose(std::move(inputFP), std::move(fp)));
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
        return false;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        const SkSL::ByteCode* bc = this->byteCode();
        if (!bc) {
            return {};
        }

        const SkSL::ByteCodeFunction* fn = bc->getFunction("main");
        if (!fn) {
            return {};
        }

        sk_sp<SkData> inputs = get_xformed_inputs(fEffect.get(), fInputs, nullptr, dstCS);
        if (!inputs) {
            return {};
        }

        std::vector<skvm::F32> uniform;
        for (int i = 0; i < (int)fEffect->uniformSize() / 4; i++) {
            float f;
            memcpy(&f, (const char*)inputs->data() + 4*i, 4);
            uniform.push_back(p->uniformF(uniforms->pushF(f)));
        }

        auto sampleChild = [&](int ix, skvm::Coord /*coord*/) {
            if (fChildren[ix]) {
                return as_CFB(fChildren[ix])->program(p, c, dstCS, uniforms, alloc);
            } else {
                return c;
            }
        };

        // The color filter code might use sample-with-matrix (even though the matrix/coords are
        // ignored by the child). There should be no way for the color filter to use device coords.
        // Regardless, just to be extra-safe, we pass something valid (0, 0) as both coords, so
        // the builder isn't trying to do math on invalid values.
        skvm::Coord zeroCoord = { p->splat(0.0f), p->splat(0.0f) };
        std::vector<skvm::F32> stack =
                program_fn(p, *fn, uniform, {c.r, c.g, c.b, c.a}, sampleChild,
                           /*device=*/ zeroCoord, /*local=*/ zeroCoord);

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
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    size_t childCount = buffer.read32();
    if (!buffer.validate(childCount == effect->children().count())) {
        return nullptr;
    }

    std::vector<sk_sp<SkColorFilter>> children(childCount);
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

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args) const override {
        SkMatrix matrix;
        if (!this->totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }

        sk_sp<SkData> inputs = get_xformed_inputs(fEffect.get(), fInputs, &args.fMatrixProvider,
                                                  args.fDstColorInfo->colorSpace());
        if (!inputs) {
            return nullptr;
        }

        auto fp = GrSkSLFP::Make(args.fContext, fEffect, "runtime_shader", std::move(inputs));
        for (const auto& child : fChildren) {
            auto childFP = child ? as_SB(child)->asFragmentProcessor(args) : nullptr;
            fp->addChild(std::move(childFP));
        }
        std::unique_ptr<GrFragmentProcessor> result = std::move(fp);
        result = GrMatrixEffect::Make(matrix, std::move(result));
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
        return false;
    }

    skvm::Color onProgram(skvm::Builder* p,
                          skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider& matrices, const SkMatrix* localM,
                          SkFilterQuality quality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        const SkSL::ByteCode* bc = this->byteCode();
        if (!bc) {
            return {};
        }

        const SkSL::ByteCodeFunction* fn = bc->getFunction("main");
        if (!fn) {
            return {};
        }

        sk_sp<SkData> inputs =
                get_xformed_inputs(fEffect.get(), fInputs, &matrices, dst.colorSpace());
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
        if (!this->computeTotalInverse(matrices.localToDevice(), localM, &inv)) {
            return {};
        }
        local = SkShaderBase::ApplyMatrix(p,inv,local,uniforms);

        auto sampleChild = [&](int ix, skvm::Coord coord) {
            if (fChildren[ix]) {
                SkOverrideDeviceMatrixProvider mats{matrices, SkMatrix::I()};
                return as_SB(fChildren[ix])->program(p, device, coord, paint,
                                                     mats, nullptr,
                                                     quality, dst,
                                                     uniforms, alloc);
            } else {
                return paint;
            }
        };

        std::vector<skvm::F32> stack =
            program_fn(p, *fn, uniform,
                       {local.x,local.y, paint.r, paint.g, paint.b, paint.a},
                       sampleChild, device, local);

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
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    size_t childCount = buffer.read32();
    if (!buffer.validate(childCount == effect->children().count())) {
        return nullptr;
    }

    std::vector<sk_sp<SkShader>> children(childCount);
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
    if (!fAllowColorFilter) {
        return nullptr;
    }
    if (!inputs) {
        inputs = SkData::MakeEmpty();
    }
    return inputs->size() == this->inputSize() && childCount == fChildren.size()
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

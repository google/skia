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
            gInlineThreshold = SkSL::Program::Settings().fInlineThreshold;
        }
    }

    SkSL::Compiler* operator->() const { return gCompiler; }

    int  getInlineThreshold() const { return gInlineThreshold; }
    void setInlineThreshold(int threshold) { gInlineThreshold = threshold; }

private:
    SkAutoMutexExclusive fLock;

    static SkMutex& compiler_mutex() {
        static SkMutex& mutex = *(new SkMutex);
        return mutex;
    }

    static SkSL::Compiler* gCompiler;
    static int             gInlineThreshold;
};
SkSL::Compiler* SharedCompiler::gCompiler = nullptr;
int             SharedCompiler::gInlineThreshold = 0;
}  // namespace SkSL

void SkRuntimeEffect_SetInlineThreshold(int threshold) {
    SkSL::SharedCompiler compiler;
    compiler.setInlineThreshold(threshold);
}

// Accepts a valid marker, or "normals(<marker>)"
static bool parse_marker(const SkSL::StringFragment& marker, uint32_t* id, uint32_t* flags) {
    SkString s = marker;
    if (s.startsWith("normals(") && s.endsWith(')')) {
        *flags |= SkRuntimeEffect::Uniform::kMarkerNormals_Flag;
        s.set(marker.fChars + 8, marker.fLength - 9);
    }
    if (!SkCanvasPriv::ValidateMarker(s.c_str())) {
        return false;
    }
    *id = SkOpts::hash_fn(s.c_str(), s.size(), 0);
    return true;
}

static bool init_uniform_type(const SkSL::Context& ctx,
                              const SkSL::Type* type,
                              SkRuntimeEffect::Uniform* v) {
#define SET_TYPES(cpuType, gpuType)                         \
    do {                                                    \
        v->fType = SkRuntimeEffect::Uniform::Type::cpuType; \
        v->fGPUType = gpuType;                              \
        return true;                                        \
    } while (false)

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
    SkSL::Program::Settings settings;
    settings.fInlineThreshold = compiler.getInlineThreshold();
    auto program = compiler->convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(sksl.c_str(), sksl.size()),
                                            settings);
    // TODO: Many errors aren't caught until we process the generated Program here. Catching those
    // in the IR generator would provide better errors messages (with locations).
    #define RETURN_FAILURE(...) return std::make_tuple(nullptr, SkStringPrintf(__VA_ARGS__))

    if (!program) {
        RETURN_FAILURE("%s", compiler->errorText().c_str());
    }
    if (!compiler->optimize(*program)) {
        RETURN_FAILURE("%s", compiler->errorText().c_str());
    }

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

    size_t offset = 0;
    std::vector<Uniform> uniforms;
    std::vector<SkString> children;
    std::vector<SkSL::SampleUsage> sampleUsages;
    std::vector<Varying> varyings;
    const SkSL::Context& ctx(compiler->context());

    // Go through program elements, pulling out information that we need
    for (const auto& elem : *program) {
        // Variables (uniform, varying, etc.)
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
                // 'uniform' variables
                else if (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                    Uniform uni;
                    uni.fName = var.fName;
                    uni.fFlags = 0;
                    uni.fCount = 1;

                    const SkSL::Type* type = &var.fType;
                    if (type->kind() == SkSL::Type::kArray_Kind) {
                        uni.fFlags |= Uniform::kArray_Flag;
                        uni.fCount = type->columns();
                        type = &type->componentType();
                    }

                    if (!init_uniform_type(ctx, type, &uni)) {
                        RETURN_FAILURE("Invalid uniform type: '%s'", type->displayName().c_str());
                    }

                    const SkSL::StringFragment& marker(var.fModifiers.fLayout.fMarker);
                    if (marker.fLength) {
                        uni.fFlags |= Uniform::kMarker_Flag;
                        allowColorFilter = false;
                        if (!parse_marker(marker, &uni.fMarker, &uni.fFlags)) {
                            RETURN_FAILURE("Invalid 'marker' string: '%.*s'", (int)marker.fLength,
                                            marker.fChars);
                        }
                    }

                    if (var.fModifiers.fLayout.fFlags & SkSL::Layout::Flag::kSRGBUnpremul_Flag) {
                        uni.fFlags |= Uniform::kSRGBUnpremul_Flag;
                    }

                    uni.fOffset = offset;
                    offset += uni.sizeInBytes();
                    SkASSERT(SkIsAlign4(offset));

                    uniforms.push_back(uni);
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

#undef RETURN_FAILURE

    sk_sp<SkRuntimeEffect> effect(new SkRuntimeEffect(std::move(sksl),
                                                      std::move(program),
                                                      std::move(uniforms),
                                                      std::move(children),
                                                      std::move(sampleUsages),
                                                      std::move(varyings),
                                                      usesSampleCoords,
                                                      allowColorFilter));
    return std::make_tuple(std::move(effect), SkString());
}

size_t SkRuntimeEffect::Uniform::sizeInBytes() const {
    auto element_size = [](Type type) -> size_t {
        switch (type) {
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
                                 std::vector<Uniform>&& uniforms,
                                 std::vector<SkString>&& children,
                                 std::vector<SkSL::SampleUsage>&& sampleUsages,
                                 std::vector<Varying>&& varyings,
                                 bool usesSampleCoords,
                                 bool allowColorFilter)
        : fHash(SkGoodHash()(sksl))
        , fSkSL(std::move(sksl))
        , fBaseProgram(std::move(baseProgram))
        , fUniforms(std::move(uniforms))
        , fChildren(std::move(children))
        , fSampleUsages(std::move(sampleUsages))
        , fVaryings(std::move(varyings))
        , fUsesSampleCoords(usesSampleCoords)
        , fAllowColorFilter(allowColorFilter) {
    SkASSERT(fBaseProgram);
    SkASSERT(fChildren.size() == fSampleUsages.size());
}

SkRuntimeEffect::~SkRuntimeEffect() = default;

size_t SkRuntimeEffect::uniformSize() const {
    return fUniforms.empty() ? 0
                             : SkAlign4(fUniforms.back().fOffset + fUniforms.back().sizeInBytes());
}

const SkRuntimeEffect::Uniform* SkRuntimeEffect::findUniform(const char* name) const {
    auto iter = std::find_if(fUniforms.begin(), fUniforms.end(),
                             [name](const Uniform& u) { return u.fName.equals(name); });
    return iter == fUniforms.end() ? nullptr : &(*iter);
}

int SkRuntimeEffect::findChild(const char* name) const {
    auto iter = std::find_if(fChildren.begin(), fChildren.end(),
                             [name](const SkString& s) { return s.equals(name); });
    return iter == fChildren.end() ? -1 : static_cast<int>(iter - fChildren.begin());
}

#if SK_SUPPORT_GPU
bool SkRuntimeEffect::toPipelineStage(const GrShaderCaps* shaderCaps,
                                      GrContextOptions::ShaderErrorHandler* errorHandler,
                                      SkSL::PipelineStageArgs* outArgs) {
    SkSL::SharedCompiler compiler;

    // This function is used by the GPU backend, and can't reuse our previously built fBaseProgram.
    // If the supplied shaderCaps have any non-default values, we have baked in the wrong settings.
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    settings.fInlineThreshold = compiler.getInlineThreshold();

    auto program = compiler->convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(fSkSL.c_str(), fSkSL.size()),
                                            settings);
    if (!program) {
        errorHandler->compileError(fSkSL.c_str(), compiler->errorText().c_str());
        return false;
    }

    if (!compiler->toPipelineStage(*program, outArgs)) {
        errorHandler->compileError(fSkSL.c_str(), compiler->errorText().c_str());
        return false;
    }

    return true;
}
#endif

SkRuntimeEffect::ByteCodeResult SkRuntimeEffect::toByteCode() const {
    SkSL::SharedCompiler compiler;

    auto byteCode = compiler->toByteCode(*fBaseProgram);
    return ByteCodeResult(std::move(byteCode), SkString(compiler->errorText().c_str()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

using SampleChildFn = std::function<skvm::Color(int, skvm::Coord)>;

static skvm::Color program_fn(skvm::Builder* p,
                              const SkSL::ByteCodeFunction& fn,
                              const std::vector<skvm::F32>& uniform,
                              skvm::Color inColor,
                              SampleChildFn sampleChild,
                              skvm::Coord device, skvm::Coord local) {
    std::vector<skvm::F32> stack;

    auto push = [&](skvm::F32 x) { stack.push_back(x); };
    auto pop  = [&]{ skvm::F32 x = stack.back(); stack.pop_back(); return x; };

    // main(inout half4 color) or main(float2 local, inout half4 color)
    SkASSERT(fn.getParameterCount() == 4 || fn.getParameterCount() == 6);
    if (fn.getParameterCount() == 6) {
        push(local.x);
        push(local.y);
    }
    push(inColor.r);
    push(inColor.g);
    push(inColor.b);
    push(inColor.a);

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
                    SkDebugf("inst %02x unimplemented\n", inst);
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
    SkASSERT(stack.size() == (size_t)fn.getParameterCount());
    skvm::F32 a = pop(),
              b = pop(),
              g = pop(),
              r = pop();
    return { r, g, b, a };
}

static sk_sp<SkData> get_xformed_uniforms(const SkRuntimeEffect* effect,
                                          sk_sp<SkData> baseUniforms,
                                          const SkMatrixProvider* matrixProvider,
                                          const SkColorSpace* dstCS) {
    using Flags = SkRuntimeEffect::Uniform::Flags;
    using Type = SkRuntimeEffect::Uniform::Type;
    SkColorSpaceXformSteps steps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                 dstCS,               kUnpremul_SkAlphaType);

    sk_sp<SkData> uniforms = nullptr;
    auto writableData = [&]() {
        if (!uniforms) {
            uniforms =  SkData::MakeWithCopy(baseUniforms->data(), baseUniforms->size());
        }
        return uniforms->writable_data();
    };

    for (const auto& v : effect->uniforms()) {
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
    return uniforms ? uniforms : baseUniforms;
}

class SkRuntimeColorFilter : public SkColorFilterBase {
public:
    SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> uniforms,
                         sk_sp<SkColorFilter> children[],
                         size_t childCount)
            : fEffect(std::move(effect))
            , fUniforms(std::move(uniforms))
            , fChildren(children, children + childCount) {}

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& colorInfo) const override {
        sk_sp<SkData> uniforms =
                get_xformed_uniforms(fEffect.get(), fUniforms, nullptr, colorInfo.colorSpace());
        if (!uniforms) {
            return GrFPFailure(nullptr);
        }

        auto fp = GrSkSLFP::Make(context, fEffect, "Runtime_Color_Filter", std::move(uniforms));
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
            auto [byteCode, errorText] = fEffect->toByteCode();
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

        sk_sp<SkData> inputs = get_xformed_uniforms(fEffect.get(), fUniforms, nullptr, dstCS);
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
        return program_fn(p, *fn, uniform, c, sampleChild,
                          /*device=*/zeroCoord, /*local=*/zeroCoord);
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        if (fUniforms) {
            buffer.writeDataAsByteArray(fUniforms.get());
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
    sk_sp<SkData> fUniforms;
    std::vector<sk_sp<SkColorFilter>> fChildren;

    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;
};

sk_sp<SkFlattenable> SkRuntimeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

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

    return effect->makeColorFilter(std::move(uniforms), children.data(), children.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> uniforms, const SkMatrix* localMatrix,
               sk_sp<SkShader>* children, size_t childCount, bool isOpaque)
            : SkShaderBase(localMatrix)
            , fEffect(std::move(effect))
            , fIsOpaque(isOpaque)
            , fUniforms(std::move(uniforms))
            , fChildren(children, children + childCount) {}

    bool isOpaque() const override { return fIsOpaque; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args) const override {
        SkMatrix matrix;
        if (!this->totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }

        sk_sp<SkData> uniforms = get_xformed_uniforms(
                fEffect.get(), fUniforms, &args.fMatrixProvider, args.fDstColorInfo->colorSpace());
        if (!uniforms) {
            return nullptr;
        }

        auto fp = GrSkSLFP::Make(args.fContext, fEffect, "runtime_shader", std::move(uniforms));
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
            auto [byteCode, errorText] = fEffect->toByteCode();
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
                get_xformed_uniforms(fEffect.get(), fUniforms, &matrices, dst.colorSpace());
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

        return program_fn(p, *fn, uniform, paint, sampleChild, device, local);
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
        if (fUniforms) {
            buffer.writeDataAsByteArray(fUniforms.get());
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

    sk_sp<SkData> fUniforms;
    std::vector<sk_sp<SkShader>> fChildren;

    mutable SkMutex fByteCodeMutex;
    mutable std::unique_ptr<SkSL::ByteCode> fByteCode;
};

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();
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

    return effect->makeShader(std::move(uniforms), children.data(), children.size(), localMPtr,
                              isOpaque);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<SkData> uniforms,
                                            sk_sp<SkShader> children[], size_t childCount,
                                            const SkMatrix* localMatrix, bool isOpaque) {
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    return uniforms->size() == this->uniformSize() && childCount == fChildren.size()
        ? sk_sp<SkShader>(new SkRTShader(sk_ref_sp(this), std::move(uniforms), localMatrix,
                                         children, childCount, isOpaque))
        : nullptr;
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> uniforms,
                                                      sk_sp<SkColorFilter> children[],
                                                      size_t childCount) {
    if (!fAllowColorFilter) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    return uniforms->size() == this->uniformSize() && childCount == fChildren.size()
        ? sk_sp<SkColorFilter>(new SkRuntimeColorFilter(sk_ref_sp(this), std::move(uniforms),
                                                        children, childCount))
        : nullptr;
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> uniforms) {
    return this->makeColorFilter(std::move(uniforms), nullptr, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkRuntimeEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkRuntimeColorFilter);
    SK_REGISTER_FLATTENABLE(SkRTShader);
}

SkRuntimeShaderBuilder::SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect> effect)
    : fEffect(std::move(effect))
    , fUniforms(SkData::MakeUninitialized(fEffect->uniformSize()))
    , fChildren(fEffect->children().count()) {}

SkRuntimeShaderBuilder::~SkRuntimeShaderBuilder() = default;

sk_sp<SkShader> SkRuntimeShaderBuilder::makeShader(const SkMatrix* localMatrix, bool isOpaque) {
    return fEffect->makeShader(fUniforms, fChildren.data(), fChildren.size(), localMatrix, isOpaque);
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

/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkRuntimeEffect.h"

#include "include/core/SkCapabilities.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkOnce.h"
#include "src/base/SkUtils.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkFilterColorProgram.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLVMCodeGenerator.h"
#include "src/sksl/dsl/DSLCore.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#if defined(SK_GANESH)
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

// Set `skia_enable_sksl_in_raster_pipeline = true` in your GN args to use Raster Pipeline SkSL.
#ifdef SK_ENABLE_SKSL_IN_RASTER_PIPELINE
#include "src/core/SkStreamPriv.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
constexpr bool kRPEnableLiveTrace = false;
#endif

#include <algorithm>

using namespace skia_private;

#if defined(SK_BUILD_FOR_DEBUGGER)
    #define SK_LENIENT_SKSL_DESERIALIZATION 1
#else
    #define SK_LENIENT_SKSL_DESERIALIZATION 0
#endif

#ifdef SK_ENABLE_SKSL

using ChildType = SkRuntimeEffect::ChildType;

static bool init_uniform_type(const SkSL::Context& ctx,
                              const SkSL::Type* type,
                              SkRuntimeEffect::Uniform* v) {
    using Type = SkRuntimeEffect::Uniform::Type;
    if (type->matches(*ctx.fTypes.fFloat))    { v->type = Type::kFloat;    return true; }
    if (type->matches(*ctx.fTypes.fHalf))     { v->type = Type::kFloat;    return true; }
    if (type->matches(*ctx.fTypes.fFloat2))   { v->type = Type::kFloat2;   return true; }
    if (type->matches(*ctx.fTypes.fHalf2))    { v->type = Type::kFloat2;   return true; }
    if (type->matches(*ctx.fTypes.fFloat3))   { v->type = Type::kFloat3;   return true; }
    if (type->matches(*ctx.fTypes.fHalf3))    { v->type = Type::kFloat3;   return true; }
    if (type->matches(*ctx.fTypes.fFloat4))   { v->type = Type::kFloat4;   return true; }
    if (type->matches(*ctx.fTypes.fHalf4))    { v->type = Type::kFloat4;   return true; }
    if (type->matches(*ctx.fTypes.fFloat2x2)) { v->type = Type::kFloat2x2; return true; }
    if (type->matches(*ctx.fTypes.fHalf2x2))  { v->type = Type::kFloat2x2; return true; }
    if (type->matches(*ctx.fTypes.fFloat3x3)) { v->type = Type::kFloat3x3; return true; }
    if (type->matches(*ctx.fTypes.fHalf3x3))  { v->type = Type::kFloat3x3; return true; }
    if (type->matches(*ctx.fTypes.fFloat4x4)) { v->type = Type::kFloat4x4; return true; }
    if (type->matches(*ctx.fTypes.fHalf4x4))  { v->type = Type::kFloat4x4; return true; }

    if (type->matches(*ctx.fTypes.fInt))  { v->type = Type::kInt;  return true; }
    if (type->matches(*ctx.fTypes.fInt2)) { v->type = Type::kInt2; return true; }
    if (type->matches(*ctx.fTypes.fInt3)) { v->type = Type::kInt3; return true; }
    if (type->matches(*ctx.fTypes.fInt4)) { v->type = Type::kInt4; return true; }

    return false;
}

SkRuntimeEffect::Uniform SkRuntimeEffectPriv::VarAsUniform(const SkSL::Variable& var,
                                                           const SkSL::Context& context,
                                                           size_t* offset) {
    using Uniform = SkRuntimeEffect::Uniform;
    SkASSERT(var.modifiers().fFlags & SkSL::Modifiers::kUniform_Flag);
    Uniform uni;
    uni.name = var.name();
    uni.flags = 0;
    uni.count = 1;

    const SkSL::Type* type = &var.type();
    if (type->isArray()) {
        uni.flags |= Uniform::kArray_Flag;
        uni.count = type->columns();
        type = &type->componentType();
    }

    if (type->hasPrecision() && !type->highPrecision()) {
        uni.flags |= Uniform::kHalfPrecision_Flag;
    }

    SkAssertResult(init_uniform_type(context, type, &uni));
    if (var.modifiers().fLayout.fFlags & SkSL::Layout::Flag::kColor_Flag) {
        uni.flags |= Uniform::kColor_Flag;
    }

    uni.offset = *offset;
    *offset += uni.sizeInBytes();
    SkASSERT(SkIsAlign4(*offset));
    return uni;
}

sk_sp<const SkData> SkRuntimeEffectPriv::TransformUniforms(
        SkSpan<const SkRuntimeEffect::Uniform> uniforms,
        sk_sp<const SkData> originalData,
        const SkColorSpace* dstCS) {
    if (!dstCS) {
        // There's no destination color-space; we can early-out immediately.
        return originalData;
    }
    SkColorSpaceXformSteps steps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                 dstCS,               kUnpremul_SkAlphaType);
    return TransformUniforms(uniforms, std::move(originalData), steps);
}

sk_sp<const SkData> SkRuntimeEffectPriv::TransformUniforms(
        SkSpan<const SkRuntimeEffect::Uniform> uniforms,
        sk_sp<const SkData> originalData,
        const SkColorSpaceXformSteps& steps) {
    using Flags = SkRuntimeEffect::Uniform::Flags;
    using Type  = SkRuntimeEffect::Uniform::Type;

    sk_sp<SkData> data = nullptr;
    auto writableData = [&]() {
        if (!data) {
            data = SkData::MakeWithCopy(originalData->data(), originalData->size());
        }
        return data->writable_data();
    };

    for (const auto& u : uniforms) {
        if (u.flags & Flags::kColor_Flag) {
            SkASSERT(u.type == Type::kFloat3 || u.type == Type::kFloat4);
            if (steps.flags.mask()) {
                float* color = SkTAddOffset<float>(writableData(), u.offset);
                if (u.type == Type::kFloat4) {
                    // RGBA, easy case
                    for (int i = 0; i < u.count; ++i) {
                        steps.apply(color);
                        color += 4;
                    }
                } else {
                    // RGB, need to pad out to include alpha. Technically, this isn't necessary,
                    // because steps shouldn't include unpremul or premul, and thus shouldn't
                    // read or write the fourth element. But let's be safe.
                    float rgba[4];
                    for (int i = 0; i < u.count; ++i) {
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
    return data ? data : originalData;
}

const SkSL::RP::Program* SkRuntimeEffect::getRPProgram(SkSL::DebugTracePriv* debugTrace) const {
    // Lazily compile the program the first time `getRPProgram` is called.
    // By using an SkOnce, we avoid thread hazards and behave in a conceptually const way, but we
    // can avoid the cost of invoking the RP code generator until it's actually needed.
    fCompileRPProgramOnce([&] {
#ifdef SK_ENABLE_SKSL_IN_RASTER_PIPELINE
        SkSL::DebugTracePriv tempDebugTrace;
        if (debugTrace) {
            const_cast<SkRuntimeEffect*>(this)->fRPProgram = MakeRasterPipelineProgram(
                    *fBaseProgram, fMain, debugTrace, /*writeTraceOps=*/true);
        } else if (kRPEnableLiveTrace) {
            debugTrace = &tempDebugTrace;
            const_cast<SkRuntimeEffect*>(this)->fRPProgram = MakeRasterPipelineProgram(
                    *fBaseProgram, fMain, debugTrace, /*writeTraceOps=*/false);
        } else {
            const_cast<SkRuntimeEffect*>(this)->fRPProgram = MakeRasterPipelineProgram(
                    *fBaseProgram, fMain, /*debugTrace=*/nullptr, /*writeTraceOps=*/false);
        }

        if (kRPEnableLiveTrace) {
            if (fRPProgram) {
                SkDebugf("-----\n\n");
                SkDebugfStream stream;
                fRPProgram->dump(&stream);
                SkDebugf("\n-----\n\n");
            } else {
                SkDebugf("----- RP unsupported -----\n\n");
            }
        }
#endif
    });

    return fRPProgram.get();
}

[[maybe_unused]] static SkSpan<const float> uniforms_as_span(
        SkSpan<const SkRuntimeEffect::Uniform> uniforms,
        sk_sp<const SkData> originalData,
        bool alwaysCopyIntoAlloc,
        const SkColorSpace* destColorSpace,
        SkArenaAlloc* alloc) {
    // Transform the uniforms into the destination colorspace.
    sk_sp<const SkData> transformedData = SkRuntimeEffectPriv::TransformUniforms(uniforms,
                                                                                 originalData,
                                                                                 destColorSpace);
    if (alwaysCopyIntoAlloc || originalData != transformedData) {
        // The transformed uniform data's lifetime is not long enough to reuse; instead, we copy the
        // uniform data directly into the alloc.
        int numBytes = transformedData->size();
        int numFloats = numBytes / sizeof(float);
        float* uniformsInAlloc = alloc->makeArrayDefault<float>(numFloats);
        memcpy(uniformsInAlloc, transformedData->data(), numBytes);
        return SkSpan{uniformsInAlloc, numFloats};
    }
    // It's safe to return a pointer into existing data.
    return SkSpan{static_cast<const float*>(originalData->data()),
                  originalData->size() / sizeof(float)};
}

class RuntimeEffectRPCallbacks : public SkSL::RP::Callbacks {
public:
    RuntimeEffectRPCallbacks(const SkStageRec& s,
                             const SkShaderBase::MatrixRec& m,
                             SkSpan<const SkRuntimeEffect::ChildPtr> c,
                             SkSpan<const SkSL::SampleUsage> u)
            : fStage(s), fMatrix(m), fChildren(c), fSampleUsages(u) {}

    bool appendShader(int index) override {
        if (SkShader* shader = fChildren[index].shader()) {
            if (fSampleUsages[index].isPassThrough()) {
                // Given a passthrough sample, the total-matrix is still as valid as before.
                return as_SB(shader)->appendStages(fStage, fMatrix);
            }
            // For a non-passthrough sample, we need to explicitly mark the total-matrix as invalid.
            SkShaderBase::MatrixRec nonPassthroughMatrix = fMatrix;
            nonPassthroughMatrix.markTotalMatrixInvalid();
            return as_SB(shader)->appendStages(fStage, nonPassthroughMatrix);
        }
        // Return the paint color when a null child shader is evaluated.
        fStage.fPipeline->append_constant_color(fStage.fAlloc, fStage.fPaintColor);
        return true;
    }
    bool appendColorFilter(int index) override {
        if (SkColorFilter* colorFilter = fChildren[index].colorFilter()) {
            return as_CFB(colorFilter)->appendStages(fStage, /*shaderIsOpaque=*/false);
        }
        // Return the original color as-is when a null child color filter is evaluated.
        return true;
    }
    bool appendBlender(int index) override {
        if (SkBlender* blender = fChildren[index].blender()) {
            return as_BB(blender)->appendStages(fStage);
        }
        // Return a source-over blend when a null blender is evaluated.
        fStage.fPipeline->append(SkRasterPipelineOp::srcover);
        return true;
    }

    // TODO: If an effect calls these intrinsics more than once, we could cache and re-use the steps
    // object(s), rather than re-creating them in the arena repeatedly.
    void toLinearSrgb() override {
        if (!fStage.fDstCS) {
            // These intrinsics do nothing when color management is disabled
            return;
        }
        fStage.fAlloc
                ->make<SkColorSpaceXformSteps>(fStage.fDstCS,              kUnpremul_SkAlphaType,
                                               sk_srgb_linear_singleton(), kUnpremul_SkAlphaType)
                ->apply(fStage.fPipeline);
    }
    void fromLinearSrgb() override {
        if (!fStage.fDstCS) {
            // These intrinsics do nothing when color management is disabled
            return;
        }
        fStage.fAlloc
                ->make<SkColorSpaceXformSteps>(sk_srgb_linear_singleton(), kUnpremul_SkAlphaType,
                                               fStage.fDstCS,              kUnpremul_SkAlphaType)
                ->apply(fStage.fPipeline);
    }

    const SkStageRec& fStage;
    const SkShaderBase::MatrixRec& fMatrix;
    SkSpan<const SkRuntimeEffect::ChildPtr> fChildren;
    SkSpan<const SkSL::SampleUsage> fSampleUsages;
};

bool SkRuntimeEffectPriv::CanDraw(const SkCapabilities* caps, const SkSL::Program* program) {
    SkASSERT(caps && program);
    SkASSERT(program->fConfig->enforcesSkSLVersion());
    return program->fConfig->fRequiredSkSLVersion <= caps->skslVersion();
}

bool SkRuntimeEffectPriv::CanDraw(const SkCapabilities* caps, const SkRuntimeEffect* effect) {
    SkASSERT(effect);
    return CanDraw(caps, effect->fBaseProgram.get());
}

//////////////////////////////////////////////////////////////////////////////

static bool flattenable_is_valid_as_child(const SkFlattenable* f) {
    if (!f) { return true; }
    switch (f->getFlattenableType()) {
        case SkFlattenable::kSkShader_Type:
        case SkFlattenable::kSkColorFilter_Type:
        case SkFlattenable::kSkBlender_Type:
            return true;
        default:
            return false;
    }
}

SkRuntimeEffect::ChildPtr::ChildPtr(sk_sp<SkFlattenable> f) : fChild(std::move(f)) {
    SkASSERT(flattenable_is_valid_as_child(fChild.get()));
}

static sk_sp<SkSL::DebugTracePriv> make_debug_trace(SkRuntimeEffect* effect,
                                                    const SkIPoint& coord) {
    auto debugTrace = sk_make_sp<SkSL::DebugTracePriv>();
    debugTrace->setSource(effect->source());
    debugTrace->setTraceCoord(coord);
    return debugTrace;
}

static ChildType child_type(const SkSL::Type& type) {
    switch (type.typeKind()) {
        case SkSL::Type::TypeKind::kBlender:     return ChildType::kBlender;
        case SkSL::Type::TypeKind::kColorFilter: return ChildType::kColorFilter;
        case SkSL::Type::TypeKind::kShader:      return ChildType::kShader;
        default: SkUNREACHABLE;
    }
}

static bool verify_child_effects(const std::vector<SkRuntimeEffect::Child>& reflected,
                                 SkSpan<SkRuntimeEffect::ChildPtr> effectPtrs) {
    // Verify that the number of passed-in child-effect pointers matches the SkSL code.
    if (reflected.size() != effectPtrs.size()) {
        return false;
    }

    // Verify that each child object's type matches its declared type in the SkSL.
    for (size_t i = 0; i < effectPtrs.size(); ++i) {
        std::optional<ChildType> effectType = effectPtrs[i].type();
        if (effectType && effectType != reflected[i].type) {
            return false;
        }
    }
    return true;
}

/**
 * If `effect` is specified, then the number and type of child objects are validated against the
 * children() of `effect`. If it's nullptr, this is skipped, allowing deserialization of children,
 * even when the effect could not be constructed (ie, due to malformed SkSL).
 */
static bool read_child_effects(SkReadBuffer& buffer,
                               const SkRuntimeEffect* effect,
                               TArray<SkRuntimeEffect::ChildPtr>* children) {
    size_t childCount = buffer.read32();
    if (effect && !buffer.validate(childCount == effect->children().size())) {
        return false;
    }

    children->clear();
    children->reserve_back(childCount);

    for (size_t i = 0; i < childCount; i++) {
        sk_sp<SkFlattenable> obj(buffer.readRawFlattenable());
        if (!flattenable_is_valid_as_child(obj.get())) {
            buffer.validate(false);
            return false;
        }
        children->push_back(std::move(obj));
    }

    // If we are validating against an effect, make sure any (non-null) children are the right type
    if (effect) {
        auto childInfo = effect->children();
        SkASSERT(childInfo.size() == SkToSizeT(children->size()));
        for (size_t i = 0; i < childCount; i++) {
            std::optional<ChildType> ct = (*children)[i].type();
            if (ct.has_value() && (*ct) != childInfo[i].type) {
                buffer.validate(false);
            }
        }
    }

    return buffer.isValid();
}

static void write_child_effects(SkWriteBuffer& buffer,
                                const std::vector<SkRuntimeEffect::ChildPtr>& children) {
    buffer.write32(children.size());
    for (const auto& child : children) {
        buffer.writeFlattenable(child.flattenable());
    }
}

#ifdef SK_ENABLE_SKVM
static std::vector<skvm::Val> make_skvm_uniforms(skvm::Builder* p,
                                                 skvm::Uniforms* uniforms,
                                                 size_t inputSize,
                                                 const SkData& inputs) {
    SkASSERTF(!(inputSize & 3), "inputSize was %zu, expected a multiple of 4", inputSize);

    const int32_t* data = reinterpret_cast<const int32_t*>(inputs.data());
    const size_t uniformCount = inputSize / sizeof(int32_t);
    std::vector<skvm::Val> uniform;
    uniform.reserve(uniformCount);
    for (size_t index = 0; index < uniformCount; ++index) {
        int32_t bits;
        memcpy(&bits, data + index, sizeof(int32_t));
        uniform.push_back(p->uniform32(uniforms->push(bits)).id);
    }

    return uniform;
}
#endif

SkSL::ProgramSettings SkRuntimeEffect::MakeSettings(const Options& options) {
    SkSL::ProgramSettings settings;
    settings.fInlineThreshold = 0;
    settings.fForceNoInline = options.forceUnoptimized;
    settings.fOptimize = !options.forceUnoptimized;
    settings.fMaxVersionAllowed = options.maxVersionAllowed;

    // SkSL created by the GPU backend is typically parsed, converted to a backend format,
    // and the IR is immediately discarded. In that situation, it makes sense to use node
    // pools to accelerate the IR allocations. Here, SkRuntimeEffect instances are often
    // long-lived (especially those created internally for runtime FPs). In this situation,
    // we're willing to pay for a slightly longer compile so that we don't waste huge
    // amounts of memory.
    settings.fUseMemoryPool = false;
    return settings;
}

// TODO: Many errors aren't caught until we process the generated Program here. Catching those
// in the IR generator would provide better errors messages (with locations).
#define RETURN_FAILURE(...) return Result{nullptr, SkStringPrintf(__VA_ARGS__)}

SkRuntimeEffect::Result SkRuntimeEffect::MakeFromSource(SkString sksl,
                                                        const Options& options,
                                                        SkSL::ProgramKind kind) {
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());
    SkSL::ProgramSettings settings = MakeSettings(options);
    std::unique_ptr<SkSL::Program> program =
            compiler.convertProgram(kind, std::string(sksl.c_str(), sksl.size()), settings);

    if (!program) {
        RETURN_FAILURE("%s", compiler.errorText().c_str());
    }

    return MakeInternal(std::move(program), options, kind);
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeInternal(std::unique_ptr<SkSL::Program> program,
                                                      const Options& options,
                                                      SkSL::ProgramKind kind) {
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());

    uint32_t flags = 0;
    switch (kind) {
        case SkSL::ProgramKind::kPrivateRuntimeColorFilter:
        case SkSL::ProgramKind::kRuntimeColorFilter:
            // TODO(skia:11209): Figure out a way to run ES3+ color filters on the CPU. This doesn't
            // need to be fast - it could just be direct IR evaluation. But without it, there's no
            // way for us to fully implement the SkColorFilter API (eg, `filterColor`)
            if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(),
                                              program.get())) {
                RETURN_FAILURE("SkSL color filters must target #version 100");
            }
            flags |= kAllowColorFilter_Flag;
            break;
        case SkSL::ProgramKind::kPrivateRuntimeShader:
        case SkSL::ProgramKind::kRuntimeShader:
            flags |= kAllowShader_Flag;
            break;
        case SkSL::ProgramKind::kPrivateRuntimeBlender:
        case SkSL::ProgramKind::kRuntimeBlender:
            flags |= kAllowBlender_Flag;
            break;
        default:
            SkUNREACHABLE;
    }

    // Find 'main', then locate the sample coords parameter. (It might not be present.)
    const SkSL::FunctionDeclaration* main = program->getFunction("main");
    if (!main) {
        RETURN_FAILURE("missing 'main' function");
    }
    const auto& mainParams = main->parameters();
    auto iter = std::find_if(mainParams.begin(), mainParams.end(), [](const SkSL::Variable* p) {
        return p->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN;
    });
    const SkSL::ProgramUsage::VariableCounts sampleCoordsUsage =
            iter != mainParams.end() ? program->usage()->get(**iter)
                                     : SkSL::ProgramUsage::VariableCounts{};

    if (sampleCoordsUsage.fRead || sampleCoordsUsage.fWrite) {
        flags |= kUsesSampleCoords_Flag;
    }

    // Color filters and blends are not allowed to depend on position (local or device) in any way.
    // The signature of main, and the declarations in sksl_rt_colorfilter/sksl_rt_blend should
    // guarantee this.
    if (flags & (kAllowColorFilter_Flag | kAllowBlender_Flag)) {
        SkASSERT(!(flags & kUsesSampleCoords_Flag));
        SkASSERT(!SkSL::Analysis::ReferencesFragCoords(*program));
    }

    if (SkSL::Analysis::CallsSampleOutsideMain(*program)) {
        flags |= kSamplesOutsideMain_Flag;
    }

    // Look for color filters that preserve the input alpha. This analysis is very conservative, and
    // only returns true when the input alpha is returned as-is from main() with no intervening
    // copies or arithmetic.
    if (flags & kAllowColorFilter_Flag) {
        if (SkSL::Analysis::ReturnsInputAlpha(*main->definition(), *program->usage())) {
            flags |= kAlphaUnchanged_Flag;
        }
    }

    // Determine if this effect uses of the color transform intrinsics. Effects need to know this
    // so they can allocate color transform objects, etc.
    if (SkSL::Analysis::CallsColorTransformIntrinsics(*program)) {
        flags |= kUsesColorTransform_Flag;
    }

    // Shaders are the only thing that cares about this, but it's inexpensive (and safe) to call.
    if (SkSL::Analysis::ReturnsOpaqueColor(*main->definition())) {
        flags |= kAlwaysOpaque_Flag;
    }

    // Go through program elements, pulling out information that we need
    size_t offset = 0;
    std::vector<Uniform> uniforms;
    std::vector<Child> children;
    std::vector<SkSL::SampleUsage> sampleUsages;
    int elidedSampleCoords = 0;
    const SkSL::Context& ctx(compiler.context());

    for (const SkSL::ProgramElement* elem : program->elements()) {
        // Variables (uniform, etc.)
        if (elem->is<SkSL::GlobalVarDeclaration>()) {
            const SkSL::GlobalVarDeclaration& global = elem->as<SkSL::GlobalVarDeclaration>();
            const SkSL::VarDeclaration& varDecl = global.declaration()->as<SkSL::VarDeclaration>();

            const SkSL::Variable& var = *varDecl.var();
            const SkSL::Type& varType = var.type();

            // Child effects that can be sampled ('shader', 'colorFilter', 'blender')
            if (varType.isEffectChild()) {
                Child c;
                c.name  = var.name();
                c.type  = child_type(varType);
                c.index = children.size();
                children.push_back(c);
                auto usage = SkSL::Analysis::GetSampleUsage(
                        *program, var, sampleCoordsUsage.fWrite != 0, &elidedSampleCoords);
                // If the child is never sampled, we pretend that it's actually in PassThrough mode.
                // Otherwise, the GP code for collecting transforms and emitting transform code gets
                // very confused, leading to asserts and bad (backend) shaders. There's an implicit
                // assumption that every FP is used by its parent. (skbug.com/12429)
                sampleUsages.push_back(usage.isSampled() ? usage
                                                         : SkSL::SampleUsage::PassThrough());
            }
            // 'uniform' variables
            else if (var.modifiers().fFlags & SkSL::Modifiers::kUniform_Flag) {
                uniforms.push_back(SkRuntimeEffectPriv::VarAsUniform(var, ctx, &offset));
            }
        }
    }

    // If the sample coords are never written to, then we will have converted sample calls that use
    // them unmodified into "passthrough" sampling. If all references to the sample coords were of
    // that form, then we don't actually "use" sample coords. We unset the flag to prevent creating
    // an extra (unused) varying holding the coords.
    if (elidedSampleCoords == sampleCoordsUsage.fRead && sampleCoordsUsage.fWrite == 0) {
        flags &= ~kUsesSampleCoords_Flag;
    }

#undef RETURN_FAILURE

    sk_sp<SkRuntimeEffect> effect(new SkRuntimeEffect(std::move(program),
                                                      options,
                                                      *main->definition(),
                                                      std::move(uniforms),
                                                      std::move(children),
                                                      std::move(sampleUsages),
                                                      flags));
    return Result{std::move(effect), SkString()};
}

sk_sp<SkRuntimeEffect> SkRuntimeEffect::makeUnoptimizedClone() {
    // Compile with maximally-permissive options; any restrictions we need to enforce were already
    // handled when the original SkRuntimeEffect was made. We don't keep around the Options struct
    // from when it was initially made so we don't know what was originally requested.
    Options options;
    options.forceUnoptimized = true;
    options.maxVersionAllowed = SkSL::Version::k300;
    options.allowPrivateAccess = true;

    // We do know the original ProgramKind, so we don't need to re-derive it.
    SkSL::ProgramKind kind = fBaseProgram->fConfig->fKind;

    // Attempt to recompile the program's source with optimizations off. This ensures that the
    // Debugger shows results on every line, even for things that could be optimized away (static
    // branches, unused variables, etc). If recompilation fails, we fall back to the original code.
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());
    SkSL::ProgramSettings settings = MakeSettings(options);
    std::unique_ptr<SkSL::Program> program =
            compiler.convertProgram(kind, *fBaseProgram->fSource, settings);

    if (!program) {
        // Turning off compiler optimizations can theoretically expose a program error that
        // had been optimized away (e.g. "all control paths return a value" might be found on a path
        // that is completely eliminated in the optimized program).
        // If this happens, the debugger will just have to show the optimized code.
        return sk_ref_sp(this);
    }

    SkRuntimeEffect::Result result = MakeInternal(std::move(program), options, kind);
    if (!result.effect) {
        // Nothing in MakeInternal should change as a result of optimizations being toggled.
        SkDEBUGFAILF("makeUnoptimizedClone: MakeInternal failed\n%s",
                     result.errorText.c_str());
        return sk_ref_sp(this);
    }

    return result.effect;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForColorFilter(SkString sksl, const Options& options) {
    auto programKind = options.allowPrivateAccess ? SkSL::ProgramKind::kPrivateRuntimeColorFilter
                                                  : SkSL::ProgramKind::kRuntimeColorFilter;
    auto result = MakeFromSource(std::move(sksl), options, programKind);
    SkASSERT(!result.effect || result.effect->allowColorFilter());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForShader(SkString sksl, const Options& options) {
    auto programKind = options.allowPrivateAccess ? SkSL::ProgramKind::kPrivateRuntimeShader
                                                  : SkSL::ProgramKind::kRuntimeShader;
    auto result = MakeFromSource(std::move(sksl), options, programKind);
    SkASSERT(!result.effect || result.effect->allowShader());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForBlender(SkString sksl, const Options& options) {
    auto programKind = options.allowPrivateAccess ? SkSL::ProgramKind::kPrivateRuntimeBlender
                                                  : SkSL::ProgramKind::kRuntimeBlender;
    auto result = MakeFromSource(std::move(sksl), options, programKind);
    SkASSERT(!result.effect || result.effect->allowBlender());
    return result;
}

sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::Result (*make)(SkString sksl, const SkRuntimeEffect::Options&),
        SkString sksl) {
    SK_BEGIN_REQUIRE_DENSE
    struct Key {
        uint32_t skslHashA;
        uint32_t skslHashB;

        bool operator==(const Key& that) const {
            return this->skslHashA == that.skslHashA
                && this->skslHashB == that.skslHashB;
        }

        explicit Key(const SkString& sksl)
            : skslHashA(SkOpts::hash(sksl.c_str(), sksl.size(), 0))
            , skslHashB(SkOpts::hash(sksl.c_str(), sksl.size(), 1)) {}
    };
    SK_END_REQUIRE_DENSE

    static auto* mutex = new SkMutex;
    static auto* cache = new SkLRUCache<Key, sk_sp<SkRuntimeEffect>>(11/*totally arbitrary*/);

    Key key(sksl);
    {
        SkAutoMutexExclusive _(*mutex);
        if (sk_sp<SkRuntimeEffect>* found = cache->find(key)) {
            return *found;
        }
    }

    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::AllowPrivateAccess(&options);

    auto [effect, err] = make(std::move(sksl), options);
    if (!effect) {
        SkDEBUGFAILF("%s", err.c_str());
        return nullptr;
    }
    SkASSERT(err.isEmpty());

    {
        SkAutoMutexExclusive _(*mutex);
        cache->insert_or_update(key, effect);
    }
    return effect;
}

static size_t uniform_element_size(SkRuntimeEffect::Uniform::Type type) {
    switch (type) {
        case SkRuntimeEffect::Uniform::Type::kFloat:  return sizeof(float);
        case SkRuntimeEffect::Uniform::Type::kFloat2: return sizeof(float) * 2;
        case SkRuntimeEffect::Uniform::Type::kFloat3: return sizeof(float) * 3;
        case SkRuntimeEffect::Uniform::Type::kFloat4: return sizeof(float) * 4;

        case SkRuntimeEffect::Uniform::Type::kFloat2x2: return sizeof(float) * 4;
        case SkRuntimeEffect::Uniform::Type::kFloat3x3: return sizeof(float) * 9;
        case SkRuntimeEffect::Uniform::Type::kFloat4x4: return sizeof(float) * 16;

        case SkRuntimeEffect::Uniform::Type::kInt:  return sizeof(int);
        case SkRuntimeEffect::Uniform::Type::kInt2: return sizeof(int) * 2;
        case SkRuntimeEffect::Uniform::Type::kInt3: return sizeof(int) * 3;
        case SkRuntimeEffect::Uniform::Type::kInt4: return sizeof(int) * 4;
        default: SkUNREACHABLE;
    }
}

size_t SkRuntimeEffect::Uniform::sizeInBytes() const {
    static_assert(sizeof(int) == sizeof(float));
    return uniform_element_size(this->type) * this->count;
}

SkRuntimeEffect::SkRuntimeEffect(std::unique_ptr<SkSL::Program> baseProgram,
                                 const Options& options,
                                 const SkSL::FunctionDefinition& main,
                                 std::vector<Uniform>&& uniforms,
                                 std::vector<Child>&& children,
                                 std::vector<SkSL::SampleUsage>&& sampleUsages,
                                 uint32_t flags)
        : fHash(SkOpts::hash_fn(baseProgram->fSource->c_str(), baseProgram->fSource->size(), 0))
        , fBaseProgram(std::move(baseProgram))
        , fMain(main)
        , fUniforms(std::move(uniforms))
        , fChildren(std::move(children))
        , fSampleUsages(std::move(sampleUsages))
        , fFlags(flags) {
    SkASSERT(fBaseProgram);
    SkASSERT(fChildren.size() == fSampleUsages.size());

    // Everything from SkRuntimeEffect::Options which could influence the compiled result needs to
    // be accounted for in `fHash`. If you've added a new field to Options and caused the static-
    // assert below to trigger, please incorporate your field into `fHash` and update KnownOptions
    // to match the layout of Options.
    struct KnownOptions {
        bool forceUnoptimized, allowPrivateAccess;
        SkSL::Version maxVersionAllowed;
    };
    static_assert(sizeof(Options) == sizeof(KnownOptions));
    fHash = SkOpts::hash_fn(&options.forceUnoptimized,
                      sizeof(options.forceUnoptimized), fHash);
    fHash = SkOpts::hash_fn(&options.allowPrivateAccess,
                      sizeof(options.allowPrivateAccess), fHash);
    fHash = SkOpts::hash_fn(&options.maxVersionAllowed,
                      sizeof(options.maxVersionAllowed), fHash);

    fFilterColorProgram = SkFilterColorProgram::Make(this);
}

SkRuntimeEffect::~SkRuntimeEffect() = default;

const std::string& SkRuntimeEffect::source() const {
    return *fBaseProgram->fSource;
}

size_t SkRuntimeEffect::uniformSize() const {
    return fUniforms.empty() ? 0
                             : SkAlign4(fUniforms.back().offset + fUniforms.back().sizeInBytes());
}

const SkRuntimeEffect::Uniform* SkRuntimeEffect::findUniform(std::string_view name) const {
    auto iter = std::find_if(fUniforms.begin(), fUniforms.end(), [name](const Uniform& u) {
        return u.name == name;
    });
    return iter == fUniforms.end() ? nullptr : &(*iter);
}

const SkRuntimeEffect::Child* SkRuntimeEffect::findChild(std::string_view name) const {
    auto iter = std::find_if(fChildren.begin(), fChildren.end(), [name](const Child& c) {
        return c.name == name;
    });
    return iter == fChildren.end() ? nullptr : &(*iter);
}

const SkFilterColorProgram* SkRuntimeEffect::getFilterColorProgram() const {
    return fFilterColorProgram.get();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)
static GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
                                 const char* name,
                                 sk_sp<const SkData> uniforms,
                                 std::unique_ptr<GrFragmentProcessor> inputFP,
                                 std::unique_ptr<GrFragmentProcessor> destColorFP,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> children,
                                 const GrFPArgs& childArgs) {
    STArray<8, std::unique_ptr<GrFragmentProcessor>> childFPs;
    for (const auto& child : children) {
        std::optional<ChildType> type = child.type();
        if (type == ChildType::kShader) {
            // Convert a SkShader into a child FP.
            SkShaderBase::MatrixRec mRec(SkMatrix::I());
            mRec.markTotalMatrixInvalid();
            auto childFP = as_SB(child.shader())->asFragmentProcessor(childArgs, mRec);
            if (!childFP) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else if (type == ChildType::kColorFilter) {
            // Convert a SkColorFilter into a child FP.
            auto [success, childFP] = as_CFB(child.colorFilter())
                                              ->asFragmentProcessor(/*inputFP=*/nullptr,
                                                                    childArgs.fContext,
                                                                    *childArgs.fDstColorInfo,
                                                                    childArgs.fSurfaceProps);
            if (!success) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else if (type == ChildType::kBlender) {
            // Convert a SkBlender into a child FP.
            auto childFP = as_BB(child.blender())->asFragmentProcessor(
                    /*srcFP=*/nullptr,
                    GrFragmentProcessor::DestColor(),
                    childArgs);
            if (!childFP) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else {
            // We have a null child effect.
            childFPs.push_back(nullptr);
        }
    }
    auto fp = GrSkSLFP::MakeWithData(std::move(effect),
                                     name,
                                     childArgs.fDstColorInfo->refColorSpace(),
                                     std::move(inputFP),
                                     std::move(destColorFP),
                                     std::move(uniforms),
                                     SkSpan(childFPs));
    SkASSERT(fp);
    return GrFPSuccess(std::move(fp));
}
#endif

#if defined(SK_GRAPHITE)
static void add_children_to_key(SkSpan<const SkRuntimeEffect::ChildPtr> children,
                                SkSpan<const SkRuntimeEffect::Child> childInfo,
                                const skgpu::graphite::KeyContext& keyContext,
                                skgpu::graphite::PaintParamsKeyBuilder* builder,
                                skgpu::graphite::PipelineDataGatherer* gatherer) {
    using namespace skgpu::graphite;

    SkASSERT(children.size() == childInfo.size());

    for (size_t index = 0; index < children.size(); ++index) {
        const SkRuntimeEffect::ChildPtr& child = children[index];
        std::optional<ChildType> type = child.type();
        if (type == ChildType::kShader) {
            as_SB(child.shader())->addToKey(keyContext, builder, gatherer);
        } else if (type == ChildType::kColorFilter) {
            as_CFB(child.colorFilter())->addToKey(keyContext, builder, gatherer);
        } else if (type == ChildType::kBlender) {
            as_BB(child.blender())->addToKey(keyContext, builder, gatherer);
        } else {
            // We don't have a child effect. Substitute in a no-op effect.
            switch (childInfo[index].type) {
                case ChildType::kShader:
                case ChildType::kColorFilter:
                    // A "passthrough" shader returns the input color as-is.
                    PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
                    builder->endBlock();
                    break;

                case ChildType::kBlender:
                    // A "passthrough" blender performs `blend_src_over(src, dest)`.
                    BlendModeBlenderBlock::BeginBlock(
                            keyContext, builder, gatherer, SkBlendMode::kSrcOver);
                    builder->endBlock();
                    break;
            }
        }
    }
}
#endif

#if defined(SK_ENABLE_SKVM)
class RuntimeEffectVMCallbacks : public SkSL::SkVMCallbacks {
public:
    RuntimeEffectVMCallbacks(skvm::Builder* builder,
                             skvm::Uniforms* uniforms,
                             SkArenaAlloc* alloc,
                             const std::vector<SkRuntimeEffect::ChildPtr>& children,
                             const SkShaderBase::MatrixRec& mRec,
                             skvm::Color inColor,
                             const SkColorInfo& colorInfo)
            : fBuilder(builder)
            , fUniforms(uniforms)
            , fAlloc(alloc)
            , fChildren(children)
            , fMRec(mRec)
            , fInColor(inColor)
            , fColorInfo(colorInfo) {}

    skvm::Color sampleShader(int ix, skvm::Coord coord) override {
        // We haven't tracked device coords and the runtime effect could have arbitrarily
        // manipulated the passed coords. We should be in a state where any pending matrix was
        // already applied before the runtime effect's code could have manipulated the coords
        // and the total matrix from child shader to device space is flagged as unknown.
        SkASSERT(!fMRec.hasPendingMatrix());
        SkASSERT(!fMRec.totalMatrixIsValid());
        if (SkShader* shader = fChildren[ix].shader()) {
            return as_SB(shader)->program(fBuilder,
                                          coord,
                                          coord,
                                          fInColor,
                                          fMRec,
                                          fColorInfo,
                                          fUniforms,
                                          fAlloc);
        }
        return fInColor;
    }

    skvm::Color sampleColorFilter(int ix, skvm::Color color) override {
        if (SkColorFilter* colorFilter = fChildren[ix].colorFilter()) {
            return as_CFB(colorFilter)->program(fBuilder, color, fColorInfo, fUniforms, fAlloc);
        }
        return color;
    }

    skvm::Color sampleBlender(int ix, skvm::Color src, skvm::Color dst) override {
        if (SkBlender* blender = fChildren[ix].blender()) {
            return as_BB(blender)->program(fBuilder, src, dst, fColorInfo, fUniforms, fAlloc);
        }
        return blend(SkBlendMode::kSrcOver, src, dst);
    }

    skvm::Color toLinearSrgb(skvm::Color color) override {
        if (!fColorInfo.colorSpace()) {
            // These intrinsics do nothing when color management is disabled
            return color;
        }
        return SkColorSpaceXformSteps{fColorInfo.colorSpace(),    kUnpremul_SkAlphaType,
                                      sk_srgb_linear_singleton(), kUnpremul_SkAlphaType}
                .program(fBuilder, fUniforms, color);
    }

    skvm::Color fromLinearSrgb(skvm::Color color) override {
        if (!fColorInfo.colorSpace()) {
            // These intrinsics do nothing when color management is disabled
            return color;
        }
        return SkColorSpaceXformSteps{sk_srgb_linear_singleton(), kUnpremul_SkAlphaType,
                                      fColorInfo.colorSpace(),    kUnpremul_SkAlphaType}
                .program(fBuilder, fUniforms, color);
    }

    skvm::Builder* fBuilder;
    skvm::Uniforms* fUniforms;
    SkArenaAlloc* fAlloc;
    const std::vector<SkRuntimeEffect::ChildPtr>& fChildren;
    const SkShaderBase::MatrixRec& fMRec;
    const skvm::Color fInColor;
    const SkColorInfo& fColorInfo;
};
#endif  // defined(SK_ENABLE_SKVM)

class SkRuntimeColorFilter : public SkColorFilterBase {
public:
    SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<const SkData> uniforms,
                         SkSpan<SkRuntimeEffect::ChildPtr> children)
            : fEffect(std::move(effect))
            , fUniforms(std::move(uniforms))
            , fChildren(children.begin(), children.end()) {}

#if defined(SK_GANESH)
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& colorInfo,
                                   const SkSurfaceProps& props) const override {
        sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
                fEffect->uniforms(),
                fUniforms,
                colorInfo.colorSpace());
        SkASSERT(uniforms);

        GrFPArgs childArgs(context, &colorInfo, props);
        return make_effect_fp(fEffect,
                              "runtime_color_filter",
                              std::move(uniforms),
                              std::move(inputFP),
                              /*destColorFP=*/nullptr,
                              SkSpan(fChildren),
                              childArgs);
    }
#endif

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override {
        using namespace skgpu::graphite;

        sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
                fEffect->uniforms(),
                fUniforms,
                keyContext.dstColorInfo().colorSpace());
        SkASSERT(uniforms);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                       { fEffect, std::move(uniforms) });

        add_children_to_key(fChildren, fEffect->children(), keyContext, builder, gatherer);

        builder->endBlock();
    }
#endif

    bool appendStages(const SkStageRec& rec, bool) const override {
#ifdef SK_ENABLE_SKSL_IN_RASTER_PIPELINE
        if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
            // SkRP has support for many parts of #version 300 already, but for now, we restrict its
            // usage in runtime effects to just #version 100.
            return false;
        }
        if (const SkSL::RP::Program* program = fEffect->getRPProgram(/*debugTrace=*/nullptr)) {
            SkSpan<const float> uniforms = uniforms_as_span(fEffect->uniforms(),
                                                            fUniforms,
                                                            /*alwaysCopyIntoAlloc=*/false,
                                                            rec.fDstCS,
                                                            rec.fAlloc);
            SkShaderBase::MatrixRec matrix(SkMatrix::I());
            matrix.markCTMApplied();
            RuntimeEffectRPCallbacks callbacks(rec, matrix, fChildren, fEffect->fSampleUsages);
            bool success = program->appendStages(rec.fPipeline, rec.fAlloc, &callbacks, uniforms);
            return success;
        }
#endif
        return false;
    }

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          const SkColorInfo& colorInfo,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        SkASSERT(SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(),
                                              fEffect.get()));

        sk_sp<const SkData> inputs = SkRuntimeEffectPriv::TransformUniforms(
                fEffect->uniforms(),
                fUniforms,
                colorInfo.colorSpace());
        SkASSERT(inputs);

        SkShaderBase::MatrixRec mRec(SkMatrix::I());
        mRec.markTotalMatrixInvalid();
        RuntimeEffectVMCallbacks callbacks(p, uniforms, alloc, fChildren, mRec, c, colorInfo);
        std::vector<skvm::Val> uniform = make_skvm_uniforms(p, uniforms, fEffect->uniformSize(),
                                                            *inputs);

        // There should be no way for the color filter to use device coords, but we need to supply
        // something. (Uninitialized values can trigger asserts in skvm::Builder).
        skvm::Coord zeroCoord = { p->splat(0.0f), p->splat(0.0f) };
        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p,/*debugTrace=*/nullptr,
                                   SkSpan(uniform), /*device=*/zeroCoord, /*local=*/zeroCoord,
                                   c, c, &callbacks);
    }
#endif

    SkPMColor4f onFilterColor4f(const SkPMColor4f& color, SkColorSpace* dstCS) const override {
#if defined(SK_ENABLE_SKVM)
        // Get the generic program for filtering a single color
        if (const SkFilterColorProgram* program = fEffect->getFilterColorProgram()) {
            // Get our specific uniform values
            sk_sp<const SkData> inputs = SkRuntimeEffectPriv::TransformUniforms(
                    fEffect->uniforms(),
                    fUniforms,
                    dstCS);
            SkASSERT(inputs);

            auto evalChild = [&](int index, SkPMColor4f inColor) {
                const auto& child = fChildren[index];

                // SkFilterColorProgram::Make has guaranteed that any children will be color filters.
                SkASSERT(!child.shader());
                SkASSERT(!child.blender());
                if (SkColorFilter* colorFilter = child.colorFilter()) {
                    return as_CFB(colorFilter)->onFilterColor4f(inColor, dstCS);
                }
                return inColor;
            };

            return program->eval(color, inputs->data(), evalChild);
        }
#endif
        // We were unable to build a cached (per-effect) program. Use the base-class fallback,
        // which builds a program for the specific filter instance.
        return SkColorFilterBase::onFilterColor4f(color, dstCS);
    }

    bool onIsAlphaUnchanged() const override {
#ifdef SK_ENABLE_SKSL_IN_RASTER_PIPELINE
        return fEffect->isAlphaUnchanged();
#else
        return fEffect->getFilterColorProgram() &&
               fEffect->isAlphaUnchanged();
#endif
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        buffer.writeDataAsByteArray(fUniforms.get());
        write_child_effects(buffer, fChildren);
    }

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    SK_FLATTENABLE_HOOKS(SkRuntimeColorFilter)

private:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<const SkData> fUniforms;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

sk_sp<SkFlattenable> SkRuntimeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForColorFilter, std::move(sksl));
#if !SK_LENIENT_SKSL_DESERIALIZATION
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }
#endif

    STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!read_child_effects(buffer, effect.get(), &children)) {
        return nullptr;
    }

#if SK_LENIENT_SKSL_DESERIALIZATION
    if (!effect) {
        SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL color filter.\n");
        return nullptr;
    }
#endif

    return effect->makeColorFilter(std::move(uniforms), SkSpan(children));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

using UniformsCallback = SkRuntimeEffectPriv::UniformsCallback;

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(sk_sp<SkRuntimeEffect> effect,
               sk_sp<SkSL::DebugTracePriv> debugTrace,
               sk_sp<const SkData> uniforms,
               SkSpan<SkRuntimeEffect::ChildPtr> children)
            : fEffect(std::move(effect))
            , fDebugTrace(std::move(debugTrace))
            , fUniformData(std::move(uniforms))
            , fChildren(children.begin(), children.end()) {}

    SkRTShader(sk_sp<SkRuntimeEffect> effect,
               sk_sp<SkSL::DebugTracePriv> debugTrace,
               UniformsCallback uniformsCallback,
               SkSpan<SkRuntimeEffect::ChildPtr> children)
            : fEffect(std::move(effect))
            , fDebugTrace(std::move(debugTrace))
            , fUniformsCallback(std::move(uniformsCallback))
            , fChildren(children.begin(), children.end()) {}

    SkRuntimeEffect::TracedShader makeTracedClone(const SkIPoint& coord) {
        sk_sp<SkRuntimeEffect> unoptimized = fEffect->makeUnoptimizedClone();
        sk_sp<SkSL::DebugTracePriv> debugTrace = make_debug_trace(unoptimized.get(), coord);
        auto debugShader = sk_make_sp<SkRTShader>(
                unoptimized, debugTrace, this->uniformData(nullptr), SkSpan(fChildren));

        return SkRuntimeEffect::TracedShader{std::move(debugShader), std::move(debugTrace)};
    }

    bool isOpaque() const override { return fEffect->alwaysOpaque(); }

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args,
                                                             const MatrixRec& mRec) const override {
        if (!SkRuntimeEffectPriv::CanDraw(args.fContext->priv().caps(), fEffect.get())) {
            return nullptr;
        }

        sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
                fEffect->uniforms(),
                this->uniformData(args.fDstColorInfo->colorSpace()),
                args.fDstColorInfo->colorSpace());
        SkASSERT(uniforms);

        bool success;
        std::unique_ptr<GrFragmentProcessor> fp;
        std::tie(success, fp) = make_effect_fp(fEffect,
                                               "runtime_shader",
                                               std::move(uniforms),
                                               /*inputFP=*/nullptr,
                                               /*destColorFP=*/nullptr,
                                               SkSpan(fChildren),
                                               args);
        if (!success) {
            return nullptr;
        }

        std::tie(success, fp) = mRec.apply(std::move(fp));
        if (!success) {
            return nullptr;
        }
        return fp;
    }
#endif

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override {
        using namespace skgpu::graphite;

        sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
                fEffect->uniforms(),
                this->uniformData(keyContext.dstColorInfo().colorSpace()),
                keyContext.dstColorInfo().colorSpace());
        SkASSERT(uniforms);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                       { fEffect, std::move(uniforms) });

        add_children_to_key(fChildren, fEffect->children(), keyContext, builder, gatherer);

        builder->endBlock();
    }
#endif

    bool appendStages(const SkStageRec& rec, const MatrixRec& mRec) const override {
#ifdef SK_ENABLE_SKSL_IN_RASTER_PIPELINE
        if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
            // SkRP has support for many parts of #version 300 already, but for now, we restrict its
            // usage in runtime effects to just #version 100.
            return false;
        }
        if (const SkSL::RP::Program* program = fEffect->getRPProgram(fDebugTrace.get())) {
            std::optional<MatrixRec> newMRec = mRec.apply(rec);
            if (!newMRec.has_value()) {
                return false;
            }
            SkSpan<const float> uniforms =
                    uniforms_as_span(fEffect->uniforms(),
                                     this->uniformData(rec.fDstCS),
                                     /*alwaysCopyIntoAlloc=*/fUniformData == nullptr,
                                     rec.fDstCS,
                                     rec.fAlloc);
            RuntimeEffectRPCallbacks callbacks(rec, *newMRec, fChildren, fEffect->fSampleUsages);
            bool success = program->appendStages(rec.fPipeline, rec.fAlloc, &callbacks, uniforms);
            return success;
        }
#endif
        return false;
    }

#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder* p,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec& mRec,
                        const SkColorInfo& colorInfo,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const override {
        if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
            return {};
        }

        sk_sp<const SkData> inputs =
                SkRuntimeEffectPriv::TransformUniforms(fEffect->uniforms(),
                                                       this->uniformData(colorInfo.colorSpace()),
                                                       colorInfo.colorSpace());
        SkASSERT(inputs);

        // Ensure any pending transform is applied before running the runtime shader's code, which
        // gets to use and manipulate the coordinates.
        std::optional<MatrixRec> newMRec = mRec.apply(p, &local, uniforms);
        if (!newMRec.has_value()) {
            return {};
        }
        // We could omit this for children that are only sampled with passthrough coords.
        newMRec->markTotalMatrixInvalid();

        RuntimeEffectVMCallbacks callbacks(p,
                                           uniforms,
                                           alloc,
                                           fChildren,
                                           *newMRec,
                                           paint,
                                           colorInfo);
        std::vector<skvm::Val> uniform = make_skvm_uniforms(p, uniforms, fEffect->uniformSize(),
                                                            *inputs);

        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p, fDebugTrace.get(),
                                   SkSpan(uniform), device, local, paint, paint, &callbacks);
    }
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        buffer.writeDataAsByteArray(this->uniformData(nullptr).get());
        write_child_effects(buffer, fChildren);
    }

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    SK_FLATTENABLE_HOOKS(SkRTShader)

private:
    enum Flags {
        kHasLegacyLocalMatrix_Flag = 1 << 1,
    };

    sk_sp<const SkData> uniformData(const SkColorSpace* dstCS) const {
        if (fUniformData) {
            return fUniformData;
        }

        // We want to invoke the uniforms-callback each time a paint occurs.
        SkASSERT(fUniformsCallback);
        sk_sp<const SkData> uniforms = fUniformsCallback({dstCS});
        SkASSERT(uniforms && uniforms->size() == fEffect->uniformSize());
        return uniforms;
    }

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkSL::DebugTracePriv> fDebugTrace;
    sk_sp<const SkData> fUniformData;
    UniformsCallback fUniformsCallback;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    SkTLazy<SkMatrix> localM;
    if (buffer.isVersionLT(SkPicturePriv::kNoShaderLocalMatrix)) {
        uint32_t flags = buffer.read32();
        if (flags & kHasLegacyLocalMatrix_Flag) {
            buffer.readMatrix(localM.init());
        }
    }

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
#if !SK_LENIENT_SKSL_DESERIALIZATION
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }
#endif

    STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!read_child_effects(buffer, effect.get(), &children)) {
        return nullptr;
    }

#if SK_LENIENT_SKSL_DESERIALIZATION
    if (!effect) {
        // If any children were SkShaders, return the first one. This is a reasonable fallback.
        for (int i = 0; i < children.size(); i++) {
            if (children[i].shader()) {
                SkDebugf("Serialized SkSL failed to compile. Replacing shader with child %d.\n", i);
                return sk_ref_sp(children[i].shader());
            }
        }

        // We don't know what to do, so just return nullptr (but *don't* poison the buffer).
        SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL shader.\n");
        return nullptr;
    }
#endif

    return effect->makeShader(std::move(uniforms), SkSpan(children), localM.getMaybeNull());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkRuntimeBlender : public SkBlenderBase {
public:
    SkRuntimeBlender(sk_sp<SkRuntimeEffect> effect,
                     sk_sp<const SkData> uniforms,
                     SkSpan<SkRuntimeEffect::ChildPtr> children)
            : fEffect(std::move(effect))
            , fUniforms(std::move(uniforms))
            , fChildren(children.begin(), children.end()) {}

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    bool onAppendStages(const SkStageRec& rec) const override {
#ifdef SK_ENABLE_SKSL_IN_RASTER_PIPELINE
        if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
            // SkRP has support for many parts of #version 300 already, but for now, we restrict its
            // usage in runtime effects to just #version 100.
            return false;
        }
        if (const SkSL::RP::Program* program = fEffect->getRPProgram(/*debugTrace=*/nullptr)) {
            SkSpan<const float> uniforms = uniforms_as_span(fEffect->uniforms(),
                                                            fUniforms,
                                                            /*alwaysCopyIntoAlloc=*/false,
                                                            rec.fDstCS,
                                                            rec.fAlloc);
            SkShaderBase::MatrixRec matrix(SkMatrix::I());
            matrix.markCTMApplied();
            RuntimeEffectRPCallbacks callbacks(rec, matrix, fChildren, fEffect->fSampleUsages);
            bool success = program->appendStages(rec.fPipeline, rec.fAlloc, &callbacks, uniforms);
            return success;
        }
#endif
        return false;
    }

#ifdef SK_ENABLE_SKVM
    skvm::Color onProgram(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                          const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                          SkArenaAlloc* alloc) const override {
        if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
            return {};
        }

        sk_sp<const SkData> inputs = SkRuntimeEffectPriv::TransformUniforms(fEffect->uniforms(),
                                                                            fUniforms,
                                                                            colorInfo.colorSpace());
        SkASSERT(inputs);

        SkShaderBase::MatrixRec mRec(SkMatrix::I());
        mRec.markTotalMatrixInvalid();
        RuntimeEffectVMCallbacks callbacks(p, uniforms, alloc, fChildren, mRec, src, colorInfo);
        std::vector<skvm::Val> uniform = make_skvm_uniforms(p, uniforms, fEffect->uniformSize(),
                                                            *inputs);

        // Emit the blend function as an SkVM program.
        skvm::Coord zeroCoord = {p->splat(0.0f), p->splat(0.0f)};
        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p,/*debugTrace=*/nullptr,
                                   SkSpan(uniform), /*device=*/zeroCoord, /*local=*/zeroCoord,
                                   src, dst, &callbacks);
    }
#endif

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            std::unique_ptr<GrFragmentProcessor> srcFP,
            std::unique_ptr<GrFragmentProcessor> dstFP,
            const GrFPArgs& args) const override {
        if (!SkRuntimeEffectPriv::CanDraw(args.fContext->priv().caps(), fEffect.get())) {
            return nullptr;
        }

        sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
                fEffect->uniforms(),
                fUniforms,
                args.fDstColorInfo->colorSpace());
        SkASSERT(uniforms);
        auto [success, fp] = make_effect_fp(fEffect,
                                            "runtime_blender",
                                            std::move(uniforms),
                                            std::move(srcFP),
                                            std::move(dstFP),
                                            SkSpan(fChildren),
                                            args);

        return success ? std::move(fp) : nullptr;
    }
#endif

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override {
        using namespace skgpu::graphite;

        sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
                fEffect->uniforms(),
                fUniforms,
                keyContext.dstColorInfo().colorSpace());
        SkASSERT(uniforms);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                       { fEffect, std::move(uniforms) });

        add_children_to_key(fChildren, fEffect->children(), keyContext, builder, gatherer);

        builder->endBlock();
    }
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        buffer.writeDataAsByteArray(fUniforms.get());
        write_child_effects(buffer, fChildren);
    }

    SK_FLATTENABLE_HOOKS(SkRuntimeBlender)

private:
    using INHERITED = SkBlenderBase;

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<const SkData> fUniforms;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

sk_sp<SkFlattenable> SkRuntimeBlender::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForBlender, std::move(sksl));
#if !SK_LENIENT_SKSL_DESERIALIZATION
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }
#endif

    STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!read_child_effects(buffer, effect.get(), &children)) {
        return nullptr;
    }

#if SK_LENIENT_SKSL_DESERIALIZATION
    if (!effect) {
        SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL blender.\n");
        return nullptr;
    }
#endif

    return effect->makeBlender(std::move(uniforms), SkSpan(children));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRuntimeEffectPriv::MakeDeferredShader(const SkRuntimeEffect* effect,
                                                        UniformsCallback uniformsCallback,
                                                        SkSpan<SkRuntimeEffect::ChildPtr> children,
                                                        const SkMatrix* localMatrix) {
    if (!effect->allowShader()) {
        return nullptr;
    }
    if (!verify_child_effects(effect->fChildren, children)) {
        return nullptr;
    }
    if (!uniformsCallback) {
        return nullptr;
    }
    return SkLocalMatrixShader::MakeWrapped<SkRTShader>(localMatrix,
                                                        sk_ref_sp(effect),
                                                        /*debugTrace=*/nullptr,
                                                        std::move(uniformsCallback),
                                                        children);
}

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<const SkData> uniforms,
                                            sk_sp<SkShader> childShaders[],
                                            size_t childCount,
                                            const SkMatrix* localMatrix) const {
    STArray<4, ChildPtr> children(childCount);
    for (size_t i = 0; i < childCount; ++i) {
        children.emplace_back(childShaders[i]);
    }
    return this->makeShader(std::move(uniforms), SkSpan(children), localMatrix);
}

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<const SkData> uniforms,
                                            SkSpan<ChildPtr> children,
                                            const SkMatrix* localMatrix) const {
    if (!this->allowShader()) {
        return nullptr;
    }
    if (!verify_child_effects(fChildren, children)) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    if (uniforms->size() != this->uniformSize()) {
        return nullptr;
    }
    return SkLocalMatrixShader::MakeWrapped<SkRTShader>(localMatrix,
                                                        sk_ref_sp(this),
                                                        /*debugTrace=*/nullptr,
                                                        std::move(uniforms),
                                                        children);
}

sk_sp<SkImage> SkRuntimeEffect::makeImage(GrRecordingContext* rContext,
                                          sk_sp<const SkData> uniforms,
                                          SkSpan<ChildPtr> children,
                                          const SkMatrix* localMatrix,
                                          SkImageInfo resultInfo,
                                          bool mipmapped) const {
    if (resultInfo.alphaType() == kUnpremul_SkAlphaType ||
        resultInfo.alphaType() == kUnknown_SkAlphaType) {
        return nullptr;
    }
    sk_sp<SkSurface> surface;
    if (rContext) {
#if defined(SK_GANESH)
        if (!rContext->priv().caps()->mipmapSupport()) {
            mipmapped = false;
        }
        surface = SkSurface::MakeRenderTarget(rContext,
                                              skgpu::Budgeted::kYes,
                                              resultInfo,
                                              1,
                                              kTopLeft_GrSurfaceOrigin,
                                              nullptr,
                                              mipmapped);
#endif
    } else {
        surface = SkSurface::MakeRaster(resultInfo);
    }
    if (!surface) {
        return nullptr;
    }
    SkCanvas* canvas = surface->getCanvas();
    auto shader = this->makeShader(std::move(uniforms), children, localMatrix);
    if (!shader) {
        return nullptr;
    }
    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);
    return surface->makeImageSnapshot();
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<const SkData> uniforms,
                                                      sk_sp<SkColorFilter> childColorFilters[],
                                                      size_t childCount) const {
    STArray<4, ChildPtr> children(childCount);
    for (size_t i = 0; i < childCount; ++i) {
        children.emplace_back(childColorFilters[i]);
    }
    return this->makeColorFilter(std::move(uniforms), SkSpan(children));
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<const SkData> uniforms,
                                                      SkSpan<ChildPtr> children) const {
    if (!this->allowColorFilter()) {
        return nullptr;
    }
    if (!verify_child_effects(fChildren, children)) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    if (uniforms->size() != this->uniformSize()) {
        return nullptr;
    }
    return sk_make_sp<SkRuntimeColorFilter>(sk_ref_sp(this), std::move(uniforms), children);
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<const SkData> uniforms) const {
    return this->makeColorFilter(std::move(uniforms), /*children=*/{});
}

sk_sp<SkBlender> SkRuntimeEffect::makeBlender(sk_sp<const SkData> uniforms,
                                              SkSpan<ChildPtr> children) const {
    if (!this->allowBlender()) {
        return nullptr;
    }
    if (!verify_child_effects(fChildren, children)) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    if (uniforms->size() != this->uniformSize()) {
        return nullptr;
    }
    return sk_make_sp<SkRuntimeBlender>(sk_ref_sp(this), std::move(uniforms), children);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRuntimeEffect::TracedShader SkRuntimeEffect::MakeTraced(sk_sp<SkShader> shader,
                                                          const SkIPoint& traceCoord) {
    SkRuntimeEffect* effect = as_SB(shader)->asRuntimeEffect();
    if (!effect) {
        return TracedShader{nullptr, nullptr};
    }
    // An SkShader with an attached SkRuntimeEffect must be an SkRTShader.
    SkRTShader* rtShader = static_cast<SkRTShader*>(shader.get());
    return rtShader->makeTracedClone(traceCoord);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::optional<ChildType> SkRuntimeEffect::ChildPtr::type() const {
    if (fChild) {
        switch (fChild->getFlattenableType()) {
            case SkFlattenable::kSkShader_Type:
                return ChildType::kShader;
            case SkFlattenable::kSkColorFilter_Type:
                return ChildType::kColorFilter;
            case SkFlattenable::kSkBlender_Type:
                return ChildType::kBlender;
            default:
                break;
        }
    }
    return std::nullopt;
}

SkShader* SkRuntimeEffect::ChildPtr::shader() const {
    return (fChild && fChild->getFlattenableType() == SkFlattenable::kSkShader_Type)
                   ? static_cast<SkShader*>(fChild.get())
                   : nullptr;
}

SkColorFilter* SkRuntimeEffect::ChildPtr::colorFilter() const {
    return (fChild && fChild->getFlattenableType() == SkFlattenable::kSkColorFilter_Type)
                   ? static_cast<SkColorFilter*>(fChild.get())
                   : nullptr;
}

SkBlender* SkRuntimeEffect::ChildPtr::blender() const {
    return (fChild && fChild->getFlattenableType() == SkFlattenable::kSkBlender_Type)
                   ? static_cast<SkBlender*>(fChild.get())
                   : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkRuntimeEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkRuntimeColorFilter);
    SK_REGISTER_FLATTENABLE(SkRTShader);
    SK_REGISTER_FLATTENABLE(SkRuntimeBlender);
}

SkRuntimeShaderBuilder::SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect> effect)
        : INHERITED(std::move(effect)) {}

SkRuntimeShaderBuilder::~SkRuntimeShaderBuilder() = default;

sk_sp<SkImage> SkRuntimeShaderBuilder::makeImage(GrRecordingContext* recordingContext,
                                                 const SkMatrix* localMatrix,
                                                 SkImageInfo resultInfo,
                                                 bool mipmapped) {
    return this->effect()->makeImage(recordingContext,
                                     this->uniforms(),
                                     this->children(),
                                     localMatrix,
                                     resultInfo,
                                     mipmapped);
}

sk_sp<SkShader> SkRuntimeShaderBuilder::makeShader(const SkMatrix* localMatrix) {
    return this->effect()->makeShader(this->uniforms(), this->children(), localMatrix);
}

SkRuntimeBlendBuilder::SkRuntimeBlendBuilder(sk_sp<SkRuntimeEffect> effect)
        : INHERITED(std::move(effect)) {}

SkRuntimeBlendBuilder::~SkRuntimeBlendBuilder() = default;

sk_sp<SkBlender> SkRuntimeBlendBuilder::makeBlender() {
    return this->effect()->makeBlender(this->uniforms(), this->children());
}

SkRuntimeColorFilterBuilder::SkRuntimeColorFilterBuilder(sk_sp<SkRuntimeEffect> effect)
        : INHERITED(std::move(effect)) {}

SkRuntimeColorFilterBuilder::~SkRuntimeColorFilterBuilder() = default;

sk_sp<SkColorFilter> SkRuntimeColorFilterBuilder::makeColorFilter() {
    return this->effect()->makeColorFilter(this->uniforms(), this->children());
}

#endif  // SK_ENABLE_SKSL

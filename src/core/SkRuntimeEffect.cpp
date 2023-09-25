/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkRuntimeEffect.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCapabilities.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeBlender.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkStreamPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/effects/colorfilters/SkRuntimeColorFilter.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkRuntimeShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include <algorithm>

using namespace skia_private;

class SkColorSpace;
struct SkIPoint;

constexpr bool kRPEnableLiveTrace = false;

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
    SkASSERT(var.modifierFlags().isUniform());
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
    if (var.layout().fFlags & SkSL::LayoutFlag::kColor) {
        uni.flags |= Uniform::kColor_Flag;
    }

    uni.offset = *offset;
    *offset += uni.sizeInBytes();
    SkASSERT(SkIsAlign4(*offset));
    return uni;
}

static ChildType child_type(const SkSL::Type& type) {
    switch (type.typeKind()) {
        case SkSL::Type::TypeKind::kBlender:     return ChildType::kBlender;
        case SkSL::Type::TypeKind::kColorFilter: return ChildType::kColorFilter;
        case SkSL::Type::TypeKind::kShader:      return ChildType::kShader;
        default: SkUNREACHABLE;
    }
}

const char* SkRuntimeEffectPriv::ChildTypeToStr(ChildType type) {
    switch (type) {
        case ChildType::kBlender:     return "blender";
        case ChildType::kColorFilter: return "color filter";
        case ChildType::kShader:      return "shader";
        default: SkUNREACHABLE;
    }
}

SkRuntimeEffect::Child SkRuntimeEffectPriv::VarAsChild(const SkSL::Variable& var, int index) {
    SkRuntimeEffect::Child c;
    c.name  = var.name();
    c.type  = child_type(var.type());
    c.index = index;
    return c;
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
        // We generally do not run the inliner when an SkRuntimeEffect program is initially created,
        // because the final compile to native shader code will do this. However, in SkRP, there's
        // no additional compilation occurring, so we need to manually inline here if we want the
        // performance boost of inlining.
        if (!(fFlags & kDisableOptimization_Flag)) {
            SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());
            fBaseProgram->fConfig->fSettings.fInlineThreshold = SkSL::kDefaultInlineThreshold;
            compiler.runInliner(*fBaseProgram);
        }

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
                fRPProgram->dump(&stream, /*writeInstructionCount=*/true);
                SkDebugf("\n-----\n\n");
            } else {
                SkDebugf("----- RP unsupported -----\n\n");
            }
        }
    });

    return fRPProgram.get();
}

SkSpan<const float> SkRuntimeEffectPriv::UniformsAsSpan(
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

bool RuntimeEffectRPCallbacks::appendShader(int index) {
    if (SkShader* shader = fChildren[index].shader()) {
        if (fSampleUsages[index].isPassThrough()) {
            // Given a passthrough sample, the total-matrix is still as valid as before.
            return as_SB(shader)->appendStages(fStage, fMatrix);
        }
        // For a non-passthrough sample, we need to explicitly mark the total-matrix as invalid.
        SkShaders::MatrixRec nonPassthroughMatrix = fMatrix;
        nonPassthroughMatrix.markTotalMatrixInvalid();
        return as_SB(shader)->appendStages(fStage, nonPassthroughMatrix);
    }
    // Return transparent black when a null shader is evaluated.
    fStage.fPipeline->append_constant_color(fStage.fAlloc, SkColors::kTransparent);
    return true;
}
bool RuntimeEffectRPCallbacks::appendColorFilter(int index) {
    if (SkColorFilter* colorFilter = fChildren[index].colorFilter()) {
        return as_CFB(colorFilter)->appendStages(fStage, /*shaderIsOpaque=*/false);
    }
    // Return the original color as-is when a null child color filter is evaluated.
    return true;
}
bool RuntimeEffectRPCallbacks::appendBlender(int index) {
    if (SkBlender* blender = fChildren[index].blender()) {
        return as_BB(blender)->appendStages(fStage);
    }
    // Return a source-over blend when a null blender is evaluated.
    fStage.fPipeline->append(SkRasterPipelineOp::srcover);
    return true;
}

// TODO: If an effect calls these intrinsics more than once, we could cache and re-use the steps
// object(s), rather than re-creating them in the arena repeatedly.
void RuntimeEffectRPCallbacks::toLinearSrgb(const void* color) {
    if (fStage.fDstCS) {
        SkColorSpaceXformSteps xform{fStage.fDstCS,              kUnpremul_SkAlphaType,
                                     sk_srgb_linear_singleton(), kUnpremul_SkAlphaType};
        if (xform.flags.mask()) {
            // We have a non-identity colorspace transform; apply it.
            this->applyColorSpaceXform(xform, color);
        }
    }
}

void RuntimeEffectRPCallbacks::fromLinearSrgb(const void* color) {
    if (fStage.fDstCS) {
        SkColorSpaceXformSteps xform{sk_srgb_linear_singleton(), kUnpremul_SkAlphaType,
                                     fStage.fDstCS,              kUnpremul_SkAlphaType};
        if (xform.flags.mask()) {
            // We have a non-identity colorspace transform; apply it.
            this->applyColorSpaceXform(xform, color);
        }
    }
}

void RuntimeEffectRPCallbacks::applyColorSpaceXform(const SkColorSpaceXformSteps& tempXform,
                                                    const void* color) {
    // Copy the transform steps into our alloc.
    SkColorSpaceXformSteps* xform = fStage.fAlloc->make<SkColorSpaceXformSteps>(tempXform);

    // Put the color into src.rgba (and temporarily stash the execution mask there instead).
    fStage.fPipeline->append(SkRasterPipelineOp::exchange_src, color);
    // Add the color space transform to our raster pipeline.
    xform->apply(fStage.fPipeline);
    // Restore the execution mask, and move the color back into program data.
    fStage.fPipeline->append(SkRasterPipelineOp::exchange_src, color);
}

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

static bool verify_child_effects(const std::vector<SkRuntimeEffect::Child>& reflected,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> effectPtrs) {
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
bool SkRuntimeEffectPriv::ReadChildEffects(SkReadBuffer& buffer,
                                           const SkRuntimeEffect* effect,
                                           TArray<SkRuntimeEffect::ChildPtr>* children) {
    size_t childCount = buffer.read32();
    if (effect && !buffer.validate(childCount == effect->children().size())) {
        return false;
    }

    children->clear();
    children->reserve_exact(childCount);

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

void SkRuntimeEffectPriv::WriteChildEffects(
        SkWriteBuffer& buffer, SkSpan<const SkRuntimeEffect::ChildPtr> children) {
    buffer.write32(children.size());
    for (const auto& child : children) {
        buffer.writeFlattenable(child.flattenable());
    }
}

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

    if (options.forceUnoptimized) {
        flags |= kDisableOptimization_Flag;
    }

    // Find 'main', then locate the sample coords parameter. (It might not be present.)
    const SkSL::FunctionDeclaration* main = program->getFunction("main");
    if (!main) {
        RETURN_FAILURE("missing 'main' function");
    }
    const SkSL::Variable* coordsParam = main->getMainCoordsParameter();

    const SkSL::ProgramUsage::VariableCounts sampleCoordsUsage =
            coordsParam ? program->usage()->get(*coordsParam)
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

            // Child effects that can be sampled ('shader', 'colorFilter', 'blender')
            if (var.type().isEffectChild()) {
                children.push_back(SkRuntimeEffectPriv::VarAsChild(var, children.size()));
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
            else if (var.modifierFlags().isUniform()) {
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
    static SkNoDestructor<SkMutex> mutex;
    static SkNoDestructor<SkLRUCache<uint64_t, sk_sp<SkRuntimeEffect>>> cache(11 /*arbitrary*/);

    uint64_t key = SkChecksum::Hash64(sksl.c_str(), sksl.size());
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
        : fHash(SkChecksum::Hash32(baseProgram->fSource->c_str(), baseProgram->fSource->size()))
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
    fHash = SkChecksum::Hash32(&options.forceUnoptimized,
                               sizeof(options.forceUnoptimized), fHash);
    fHash = SkChecksum::Hash32(&options.allowPrivateAccess,
                               sizeof(options.allowPrivateAccess), fHash);
    fHash = SkChecksum::Hash32(&options.maxVersionAllowed,
                               sizeof(options.maxVersionAllowed), fHash);
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

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRuntimeEffectPriv::MakeDeferredShader(
        const SkRuntimeEffect* effect,
        UniformsCallback uniformsCallback,
        SkSpan<const SkRuntimeEffect::ChildPtr> children,
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
    return SkLocalMatrixShader::MakeWrapped<SkRuntimeShader>(localMatrix,
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
                                            SkSpan<const ChildPtr> children,
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
    return SkLocalMatrixShader::MakeWrapped<SkRuntimeShader>(localMatrix,
                                                             sk_ref_sp(this),
                                                             /*debugTrace=*/nullptr,
                                                             std::move(uniforms),
                                                             children);
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
                                                      SkSpan<const ChildPtr> children) const {
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
                                              SkSpan<const ChildPtr> children) const {
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
    // An SkShader with an attached SkRuntimeEffect must be an SkRuntimeShader.
    SkRuntimeShader* rtShader = static_cast<SkRuntimeShader*>(shader.get());
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
    SK_REGISTER_FLATTENABLE(SkRuntimeBlender);
    SK_REGISTER_FLATTENABLE(SkRuntimeColorFilter);
    SK_REGISTER_FLATTENABLE(SkRuntimeShader);

    // Previous name
    SkFlattenable::Register("SkRTShader", SkRuntimeShader::CreateProc);
}

SkRuntimeShaderBuilder::SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect> effect)
        : SkRuntimeEffectBuilder(std::move(effect)) {}

SkRuntimeShaderBuilder::~SkRuntimeShaderBuilder() = default;

sk_sp<SkShader> SkRuntimeShaderBuilder::makeShader(const SkMatrix* localMatrix) const {
    return this->effect()->makeShader(this->uniforms(), this->children(), localMatrix);
}

SkRuntimeBlendBuilder::SkRuntimeBlendBuilder(sk_sp<SkRuntimeEffect> effect)
        : SkRuntimeEffectBuilder(std::move(effect)) {}

SkRuntimeBlendBuilder::~SkRuntimeBlendBuilder() = default;

sk_sp<SkBlender> SkRuntimeBlendBuilder::makeBlender() const {
    return this->effect()->makeBlender(this->uniforms(), this->children());
}

SkRuntimeColorFilterBuilder::SkRuntimeColorFilterBuilder(sk_sp<SkRuntimeEffect> effect)
        : SkRuntimeEffectBuilder(std::move(effect)) {}

SkRuntimeColorFilterBuilder::~SkRuntimeColorFilterBuilder() = default;

sk_sp<SkColorFilter> SkRuntimeColorFilterBuilder::makeColorFilter() const {
    return this->effect()->makeColorFilter(this->uniforms(), this->children());
}

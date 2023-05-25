/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrFragmentProcessors.h"

#include "include/core/SkColorSpace.h"  // IWYU pragma: keep
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkRuntimeBlender.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/effects/SkShaderMaskFilterImpl.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/shaders/SkShaderBase.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace GrFragmentProcessors {
static std::unique_ptr<GrFragmentProcessor>
        make_fp_from_shader_mask_filter(const SkMaskFilterBase* maskfilter,
                                        const GrFPArgs& args,
                                        const SkMatrix& ctm) {
    SkASSERT(maskfilter);
    auto shaderMF = static_cast<const SkShaderMaskFilterImpl*>(maskfilter);
    auto fp = as_SB(shaderMF->shader())->asFragmentProcessor(args, SkShaderBase::MatrixRec(ctm));
    return GrFragmentProcessor::MulInputByChildAlpha(std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter* maskfilter,
                                          const GrFPArgs& args,
                                          const SkMatrix& ctm) {
    if (!maskfilter) {
        return nullptr;
    }
    auto mfb = as_MFB(maskfilter);
    switch (mfb->type()) {
        case SkMaskFilterBase::Type::kShader:
            return make_fp_from_shader_mask_filter(mfb, args, ctm);
        case SkMaskFilterBase::Type::kBlur:
        case SkMaskFilterBase::Type::kEmboss:
        case SkMaskFilterBase::Type::kSDF:
        case SkMaskFilterBase::Type::kTable:
            return nullptr;
    }
    SkUNREACHABLE;
}

using ChildType = SkRuntimeEffect::ChildType;

GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
                          const char* name,
                          sk_sp<const SkData> uniforms,
                          std::unique_ptr<GrFragmentProcessor> inputFP,
                          std::unique_ptr<GrFragmentProcessor> destColorFP,
                          SkSpan<const SkRuntimeEffect::ChildPtr> children,
                          const GrFPArgs& childArgs) {
    skia_private::STArray<8, std::unique_ptr<GrFragmentProcessor>> childFPs;
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
            auto childFP = GrFragmentProcessors::Make(as_BB(child.blender()),
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

static std::unique_ptr<GrFragmentProcessor> make_blender_fp(
        const SkRuntimeBlender* rtb,
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) {
    SkASSERT(rtb);
    if (!SkRuntimeEffectPriv::CanDraw(fpArgs.fContext->priv().caps(), rtb->effect().get())) {
        return nullptr;
    }

    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            rtb->effect()->uniforms(),
            rtb->uniforms(),
            fpArgs.fDstColorInfo->colorSpace());
    SkASSERT(uniforms);
    auto children = rtb->children();
    auto [success, fp] = make_effect_fp(rtb->effect(),
                                        "runtime_blender",
                                        std::move(uniforms),
                                        std::move(srcFP),
                                        std::move(dstFP),
                                        SkSpan(children),
                                        fpArgs);

    return success ? std::move(fp) : nullptr;
}

static std::unique_ptr<GrFragmentProcessor> make_blender_fp(
        const SkBlendModeBlender* blender,
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) {
    SkASSERT(blender);
    return GrBlendFragmentProcessor::Make(std::move(srcFP), std::move(dstFP), blender->mode());
}

std::unique_ptr<GrFragmentProcessor> Make(const SkBlenderBase* blender,
                                          std::unique_ptr<GrFragmentProcessor> srcFP,
                                          std::unique_ptr<GrFragmentProcessor> dstFP,
                                          const GrFPArgs& fpArgs) {
    if (!blender) {
        return nullptr;
    }
    switch (blender->type()) {
#define M(type)                                                                \
    case SkBlenderBase::BlenderType::k##type:                                  \
        return make_blender_fp(static_cast<const Sk##type##Blender*>(blender), \
                               std::move(srcFP),                               \
                               std::move(dstFP),                               \
                               fpArgs);
        SK_ALL_BLENDERS(M)
#undef M
    }
    SkUNREACHABLE;
}

bool IsSupported(const SkMaskFilter* maskfilter) {
    if (!maskfilter) {
        return false;
    }
    auto mfb = as_MFB(maskfilter);
    switch (mfb->type()) {
        case SkMaskFilterBase::Type::kShader:
            return true;
        case SkMaskFilterBase::Type::kBlur:
        case SkMaskFilterBase::Type::kEmboss:
        case SkMaskFilterBase::Type::kSDF:
        case SkMaskFilterBase::Type::kTable:
            return false;
    }
    SkUNREACHABLE;
}
}  // namespace GrFragmentProcessors

/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SkGr.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkPixelRef.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/SkImageInfoPriv.h"
#include "include/private/SkTPin.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/generated/GrClampFragmentProcessor.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/gpu/effects/generated/GrDitherEffect.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkShaderBase.h"

void GrMakeKeyFromImageID(GrUniqueKey* key, uint32_t imageID, const SkIRect& imageBounds) {
    SkASSERT(key);
    SkASSERT(imageID);
    SkASSERT(!imageBounds.isEmpty());
    static const GrUniqueKey::Domain kImageIDDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kImageIDDomain, 5, "Image");
    builder[0] = imageID;
    builder[1] = imageBounds.fLeft;
    builder[2] = imageBounds.fTop;
    builder[3] = imageBounds.fRight;
    builder[4] = imageBounds.fBottom;
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkIDChangeListener> GrMakeUniqueKeyInvalidationListener(GrUniqueKey* key,
                                                              uint32_t contextID) {
    class Listener : public SkIDChangeListener {
    public:
        Listener(const GrUniqueKey& key, uint32_t contextUniqueID) : fMsg(key, contextUniqueID) {}

        void changed() override {
            SkMessageBus<GrUniqueKeyInvalidatedMessage, uint32_t>::Post(fMsg);
        }

    private:
        GrUniqueKeyInvalidatedMessage fMsg;
    };

    auto listener = sk_make_sp<Listener>(*key, contextID);

    // We stick a SkData on the key that calls invalidateListener in its destructor.
    auto invalidateListener = [](const void* ptr, void* /*context*/) {
        auto listener = reinterpret_cast<const sk_sp<Listener>*>(ptr);
        (*listener)->markShouldDeregister();
        delete listener;
    };
    auto data = SkData::MakeWithProc(new sk_sp<Listener>(listener),
                                     sizeof(sk_sp<Listener>),
                                     invalidateListener,
                                     nullptr);
    SkASSERT(!key->getCustomData());
    key->setCustomData(std::move(data));
    return std::move(listener);
}

sk_sp<GrSurfaceProxy> GrCopyBaseMipMapToTextureProxy(GrRecordingContext* ctx,
                                                     sk_sp<GrSurfaceProxy> baseProxy,
                                                     GrSurfaceOrigin origin,
                                                     SkBudgeted budgeted) {
    SkASSERT(baseProxy);

    // We don't allow this for promise proxies i.e. if they need mips they need to give them
    // to us upfront.
    if (baseProxy->isPromiseProxy()) {
        return nullptr;
    }
    if (!ctx->priv().caps()->isFormatCopyable(baseProxy->backendFormat())) {
        return nullptr;
    }
    auto copy = GrSurfaceProxy::Copy(ctx, std::move(baseProxy), origin, GrMipmapped::kYes,
                                     SkBackingFit::kExact, budgeted);
    if (!copy) {
        return nullptr;
    }
    SkASSERT(copy->asTextureProxy());
    return copy;
}

GrSurfaceProxyView GrCopyBaseMipMapToView(GrRecordingContext* context,
                                          GrSurfaceProxyView src,
                                          SkBudgeted budgeted) {
    auto origin = src.origin();
    auto swizzle = src.swizzle();
    auto proxy = src.refProxy();
    return {GrCopyBaseMipMapToTextureProxy(context, proxy, origin, budgeted), origin, swizzle};
}

GrSurfaceProxyView GrRefCachedBitmapView(GrRecordingContext* ctx, const SkBitmap& bitmap,
                                         GrMipmapped mipMapped) {
    GrBitmapTextureMaker maker(ctx, bitmap, GrImageTexGenPolicy::kDraw);
    return maker.view(mipMapped);
}

GrSurfaceProxyView GrMakeCachedBitmapProxyView(GrRecordingContext* context,
                                               const SkBitmap& bitmap) {
    if (!bitmap.peekPixels(nullptr)) {
        return {};
    }

    GrBitmapTextureMaker maker(context, bitmap, GrImageTexGenPolicy::kDraw);
    return maker.view(GrMipmapped::kNo);
}

///////////////////////////////////////////////////////////////////////////////

SkPMColor4f SkColorToPMColor4f(SkColor c, const GrColorInfo& colorInfo) {
    SkColor4f color = SkColor4f::FromColor(c);
    if (auto* xform = colorInfo.colorSpaceXformFromSRGB()) {
        color = xform->apply(color);
    }
    return color.premul();
}

SkColor4f SkColor4fPrepForDst(SkColor4f color, const GrColorInfo& colorInfo) {
    if (auto* xform = colorInfo.colorSpaceXformFromSRGB()) {
        color = xform->apply(color);
    }
    return color;
}

///////////////////////////////////////////////////////////////////////////////

static inline bool blend_requires_shader(const SkBlendMode mode) {
    return SkBlendMode::kDst != mode;
}

#ifndef SK_IGNORE_GPU_DITHER
static inline float dither_range_for_config(GrColorType dstColorType) {
    // We use 1 / (2^bitdepth-1) as the range since each channel can hold 2^bitdepth values
    switch (dstColorType) {
        // 4 bit
        case GrColorType::kABGR_4444:
        case GrColorType::kARGB_4444:
        case GrColorType::kBGRA_4444:
            return 1 / 15.f;
        // 6 bit
        case GrColorType::kBGR_565:
            return 1 / 63.f;
        // 8 bit
        case GrColorType::kUnknown:
        case GrColorType::kAlpha_8:
        case GrColorType::kAlpha_8xxx:
        case GrColorType::kGray_8:
        case GrColorType::kGrayAlpha_88:
        case GrColorType::kGray_8xxx:
        case GrColorType::kR_8:
        case GrColorType::kRG_88:
        case GrColorType::kRGB_888:
        case GrColorType::kRGB_888x:
        case GrColorType::kRGBA_8888:
        case GrColorType::kRGBA_8888_SRGB:
        case GrColorType::kBGRA_8888:
            return 1 / 255.f;
        // 10 bit
        case GrColorType::kRGBA_1010102:
        case GrColorType::kBGRA_1010102:
            return 1 / 1023.f;
        // 16 bit
        case GrColorType::kAlpha_16:
        case GrColorType::kR_16:
        case GrColorType::kRG_1616:
        case GrColorType::kRGBA_16161616:
            return 1 / 32767.f;
        // Half
        case GrColorType::kAlpha_F16:
        case GrColorType::kGray_F16:
        case GrColorType::kR_F16:
        case GrColorType::kRG_F16:
        case GrColorType::kRGBA_F16:
        case GrColorType::kRGBA_F16_Clamped:
        // Float
        case GrColorType::kAlpha_F32xxx:
        case GrColorType::kRGBA_F32:
            return 0.f; // no dithering
    }
    SkUNREACHABLE;
}
#endif

static inline bool skpaint_to_grpaint_impl(GrRecordingContext* context,
                                           const GrColorInfo& dstColorInfo,
                                           const SkPaint& skPaint,
                                           const SkMatrixProvider& matrixProvider,
                                           std::unique_ptr<GrFragmentProcessor>* shaderProcessor,
                                           SkBlendMode* primColorMode,
                                           GrPaint* grPaint) {
    // TODO: take sampling directly
    SkSamplingOptions sampling(SkPaintPriv::GetFQ(skPaint),
                               SkSamplingOptions::kMedium_asMipmapLinear);

    // Convert SkPaint color to 4f format in the destination color space
    SkColor4f origColor = SkColor4fPrepForDst(skPaint.getColor4f(), dstColorInfo);

    GrFPArgs fpArgs(context, matrixProvider, sampling, &dstColorInfo);

    // Setup the initial color considering the shader, the SkPaint color, and the presence or not
    // of per-vertex colors.
    std::unique_ptr<GrFragmentProcessor> paintFP;
    if (!primColorMode || blend_requires_shader(*primColorMode)) {
        fpArgs.fInputColorIsOpaque = origColor.isOpaque();
        if (shaderProcessor) {
            paintFP = std::move(*shaderProcessor);
        } else {
            if (const SkShaderBase* shader = as_SB(skPaint.getShader())) {
                paintFP = shader->asFragmentProcessor(fpArgs);
                if (paintFP == nullptr) {
                    return false;
                }
            }
        }
    }

    // Set this in below cases if the output of the shader/paint-color/paint-alpha/primXfermode is
    // a known constant value. In that case we can simply apply a color filter during this
    // conversion without converting the color filter to a GrFragmentProcessor.
    bool applyColorFilterToPaintColor = false;
    if (paintFP) {
        if (primColorMode) {
            // There is a blend between the primitive color and the shader color. The shader sees
            // the opaque paint color. The shader's output is blended using the provided mode by
            // the primitive color. The blended color is then modulated by the paint's alpha.

            // The geometry processor will insert the primitive color to start the color chain, so
            // the GrPaint color will be ignored.

            SkPMColor4f shaderInput = origColor.makeOpaque().premul();
            paintFP = GrFragmentProcessor::OverrideInput(std::move(paintFP), shaderInput);
            paintFP = GrBlendFragmentProcessor::Make(std::move(paintFP), /*dst=*/nullptr,
                                                     *primColorMode);

            // We can ignore origColor here - alpha is unchanged by gamma
            float paintAlpha = skPaint.getColor4f().fA;
            if (1.0f != paintAlpha) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                paintFP = GrFragmentProcessor::ModulateRGBA(
                        std::move(paintFP), {paintAlpha, paintAlpha, paintAlpha, paintAlpha});
            }
        } else {
            grPaint->setColor4f(origColor.premul());
        }
    } else {
        if (primColorMode) {
            // There is a blend between the primitive color and the paint color. The blend considers
            // the opaque paint color. The paint's alpha is applied to the post-blended color.
            SkPMColor4f opaqueColor = origColor.makeOpaque().premul();
            paintFP = GrConstColorProcessor::Make(opaqueColor);
            paintFP = GrBlendFragmentProcessor::Make(std::move(paintFP), /*dst=*/nullptr,
                                                     *primColorMode);
            grPaint->setColor4f(opaqueColor);

            // We can ignore origColor here - alpha is unchanged by gamma
            float paintAlpha = skPaint.getColor4f().fA;
            if (1.0f != paintAlpha) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                paintFP = GrFragmentProcessor::ModulateRGBA(
                        std::move(paintFP), {paintAlpha, paintAlpha, paintAlpha, paintAlpha});
            }
        } else {
            // No shader, no primitive color.
            grPaint->setColor4f(origColor.premul());
            applyColorFilterToPaintColor = true;
        }
    }

    SkColorFilter* colorFilter = skPaint.getColorFilter();
    if (colorFilter) {
        if (applyColorFilterToPaintColor) {
            SkColorSpace* dstCS = dstColorInfo.colorSpace();
            grPaint->setColor4f(colorFilter->filterColor4f(origColor, dstCS, dstCS).premul());
        } else {
            auto [success, fp] = as_CFB(colorFilter)->asFragmentProcessor(std::move(paintFP),
                                                                          context, dstColorInfo);
            if (!success) {
                return false;
            }
            paintFP = std::move(fp);
        }
    }

    SkMaskFilterBase* maskFilter = as_MFB(skPaint.getMaskFilter());
    if (maskFilter) {
        // We may have set this before passing to the SkShader.
        fpArgs.fInputColorIsOpaque = false;
        if (auto mfFP = maskFilter->asFragmentProcessor(fpArgs)) {
            grPaint->setCoverageFragmentProcessor(std::move(mfFP));
        }
    }

    // When the xfermode is null on the SkPaint (meaning kSrcOver) we need the XPFactory field on
    // the GrPaint to also be null (also kSrcOver).
    SkASSERT(!grPaint->getXPFactory());
    if (!skPaint.isSrcOver()) {
        grPaint->setXPFactory(SkBlendMode_AsXPFactory(skPaint.getBlendMode()));
    }

#ifndef SK_IGNORE_GPU_DITHER
    GrColorType ct = dstColorInfo.colorType();
    if (SkPaintPriv::ShouldDither(skPaint, GrColorTypeToSkColorType(ct)) && paintFP != nullptr) {
        float ditherRange = dither_range_for_config(ct);
        paintFP = GrDitherEffect::Make(std::move(paintFP), ditherRange);
    }
#endif

    if (GrColorTypeClampType(dstColorInfo.colorType()) == GrClampType::kManual) {
        if (paintFP != nullptr) {
            paintFP = GrClampFragmentProcessor::Make(std::move(paintFP), /*clampToPremul=*/false);
        } else {
            auto color = grPaint->getColor4f();
            grPaint->setColor4f({SkTPin(color.fR, 0.f, 1.f),
                                 SkTPin(color.fG, 0.f, 1.f),
                                 SkTPin(color.fB, 0.f, 1.f),
                                 SkTPin(color.fA, 0.f, 1.f)});
        }
    }

    if (paintFP) {
        grPaint->setColorFragmentProcessor(std::move(paintFP));
    }

    return true;
}

bool SkPaintToGrPaint(GrRecordingContext* context,
                      const GrColorInfo& dstColorInfo,
                      const SkPaint& skPaint,
                      const SkMatrixProvider& matrixProvider,
                      GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, matrixProvider,
                                   /*shaderProcessor=*/nullptr, /*primColorMode=*/nullptr, grPaint);
}

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. */
bool SkPaintToGrPaintReplaceShader(GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo,
                                   const SkPaint& skPaint,
                                   const SkMatrixProvider& matrixProvider,
                                   std::unique_ptr<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint) {
    if (!shaderFP) {
        return false;
    }
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, matrixProvider, &shaderFP,
                                   /*primColorMode=*/nullptr, grPaint);
}

/** Ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrRecordingContext* context,
                              const GrColorInfo& dstColorInfo,
                              const SkPaint& skPaint,
                              const SkMatrixProvider& matrixProvider,
                              GrPaint* grPaint) {
    // Use a ptr to a nullptr to to indicate that the SkShader is ignored and not replaced.
    std::unique_ptr<GrFragmentProcessor> nullShaderFP(nullptr);
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, matrixProvider, &nullShaderFP,
                                   /*primColorMode=*/nullptr, grPaint);
}

/** Blends the SkPaint's shader (or color if no shader) with a per-primitive color which must
    be setup as a vertex attribute using the specified SkBlendMode. */
bool SkPaintToGrPaintWithBlend(GrRecordingContext* context,
                               const GrColorInfo& dstColorInfo,
                               const SkPaint& skPaint,
                               const SkMatrixProvider& matrixProvider,
                               SkBlendMode primColorMode,
                               GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, matrixProvider,
                                   /*shaderProcessor=*/nullptr, &primColorMode, grPaint);
}

/** Blends the passed-in shader with a per-primitive color which must be setup as a vertex attribute
    using the specified SkBlendMode. */
bool SkPaintToGrPaintWithBlendReplaceShader(GrRecordingContext* context,
                                            const GrColorInfo& dstColorInfo,
                                            const SkPaint& skPaint,
                                            const SkMatrixProvider& matrixProvider,
                                            std::unique_ptr<GrFragmentProcessor> shaderFP,
                                            SkBlendMode primColorMode,
                                            GrPaint* grPaint) {
    if (!shaderFP) {
        return false;
    }
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, matrixProvider, &shaderFP,
                                   &primColorMode, grPaint);
}

bool SkPaintToGrPaintWithTexture(GrRecordingContext* context,
                                 const GrColorInfo& dstColorInfo,
                                 const SkPaint& paint,
                                 const SkMatrixProvider& matrixProvider,
                                 std::unique_ptr<GrFragmentProcessor> fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint) {
    // TODO: take sampling directly
    SkSamplingOptions sampling(SkPaintPriv::GetFQ(paint),
                               SkSamplingOptions::kMedium_asMipmapLinear);

    std::unique_ptr<GrFragmentProcessor> shaderFP;
    if (textureIsAlphaOnly) {
        if (const auto* shader = as_SB(paint.getShader())) {
            shaderFP = shader->asFragmentProcessor(
                    GrFPArgs(context, matrixProvider, sampling, &dstColorInfo));
            if (!shaderFP) {
                return false;
            }
            shaderFP = GrFragmentProcessor::Compose(std::move(fp), std::move(shaderFP));
        } else {
            shaderFP = GrFragmentProcessor::MakeInputPremulAndMulByOutput(std::move(fp));
        }
    } else {
        if (paint.getColor4f().isOpaque()) {
            shaderFP = GrFragmentProcessor::OverrideInput(std::move(fp), SK_PMColor4fWHITE, false);
        } else {
            shaderFP = GrFragmentProcessor::MulChildByInputAlpha(std::move(fp));
        }
    }

    return SkPaintToGrPaintReplaceShader(context, dstColorInfo, paint, matrixProvider,
                                         std::move(shaderFP), grPaint);
}

////////////////////////////////////////////////////////////////////////////////////////////////

std::tuple<GrSamplerState::Filter,
           GrSamplerState::MipmapMode,
           SkCubicResampler>
GrInterpretSamplingOptions(SkISize imageDims,
                           const SkSamplingOptions& sampling,
                           const SkMatrix& viewM,
                           const SkMatrix& localM,
                           bool sharpenMipmappedTextures,
                           bool allowFilterQualityReduction) {
    using Filter = GrSamplerState::Filter;
    using MipmapMode = GrSamplerState::MipmapMode;

    if (sampling.useCubic) {
        SkASSERT(GrValidCubicResampler(sampling.cubic));
        return {Filter::kNearest, MipmapMode::kNone, sampling.cubic};
    }

    Filter     f = sampling.filter;
    MipmapMode m = sampling.mipmap;
    if (allowFilterQualityReduction && (m != MipmapMode::kNone)) {
        SkMatrix matrix;
        matrix.setConcat(viewM, localM);
        // With sharp mips, we bias lookups by -0.5. That means our final LOD is >= 0 until
        // the computed LOD is >= 0.5. At what scale factor does a texture get an LOD of
        // 0.5?
        //
        // Want:  0       = log2(1/s) - 0.5
        //        0.5     = log2(1/s)
        //        2^0.5   = 1/s
        //        1/2^0.5 = s
        //        2^0.5/2 = s
        SkScalar mipScale = sharpenMipmappedTextures ? SK_ScalarRoot2Over2 : SK_Scalar1;
        if (matrix.getMinScale() >= mipScale) {
            m = MipmapMode::kNone;
        }
    }
    return {f, m, kInvalidCubicResampler};
}

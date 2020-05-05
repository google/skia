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
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/SkImageInfoPriv.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"
#include "src/gpu/effects/generated/GrClampFragmentProcessor.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkShaderBase.h"

GR_FP_SRC_STRING SKSL_DITHER_SRC = R"(
// This controls the range of values added to color channels
in int rangeType;

void main(float2 p, inout half4 color) {
    half value;
    half range;
    @switch (rangeType) {
        case 0:
            range = 1.0 / 255.0;
            break;
        case 1:
            range = 1.0 / 63.0;
            break;
        default:
            // Experimentally this looks better than the expected value of 1/15.
            range = 1.0 / 15.0;
            break;
    }
    @if (sk_Caps.integerSupport) {
        // This ordered-dither code is lifted from the cpu backend.
        uint x = uint(p.x);
        uint y = uint(p.y);
        uint m = (y & 1) << 5 | (x & 1) << 4 |
                 (y & 2) << 2 | (x & 2) << 1 |
                 (y & 4) >> 1 | (x & 4) >> 2;
        value = half(m) * 1.0 / 64.0 - 63.0 / 128.0;
    } else {
        // Simulate the integer effect used above using step/mod. For speed, simulates a 4x4
        // dither pattern rather than an 8x8 one.
        half4 modValues = mod(half4(half(p.x), half(p.y), half(p.x), half(p.y)), half4(2.0, 2.0, 4.0, 4.0));
        half4 stepValues = step(modValues, half4(1.0, 1.0, 2.0, 2.0));
        value = dot(stepValues, half4(8.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0)) - 15.0 / 32.0;
    }
    // For each color channel, add the random offset to the channel value and then clamp
    // between 0 and alpha to keep the color premultiplied.
    color = half4(clamp(color.rgb + value * range, 0.0, color.a), color.a);
}
)";

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

        void changed() override { SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg); }

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

GrSurfaceProxyView GrCopyBaseMipMapToTextureProxy(GrRecordingContext* ctx,
                                                  GrSurfaceProxy* baseProxy,
                                                  GrSurfaceOrigin origin,
                                                  GrColorType srcColorType,
                                                  SkBudgeted budgeted) {
    SkASSERT(baseProxy);

    if (!ctx->priv().caps()->isFormatCopyable(baseProxy->backendFormat())) {
        return {};
    }
    GrSurfaceProxyView view = GrSurfaceProxy::Copy(ctx,
                                                   baseProxy,
                                                   origin,
                                                   srcColorType,
                                                   GrMipMapped::kYes,
                                                   SkBackingFit::kExact,
                                                   budgeted);
    SkASSERT(!view.proxy() || view.asTextureProxy());
    return view;
}

GrSurfaceProxyView GrRefCachedBitmapView(GrRecordingContext* ctx, const SkBitmap& bitmap,
                                         GrMipMapped mipMapped) {
    GrBitmapTextureMaker maker(ctx, bitmap, GrImageTexGenPolicy::kDraw);
    return maker.view(mipMapped);
}

GrSurfaceProxyView GrMakeCachedBitmapProxyView(GrRecordingContext* context,
                                               const SkBitmap& bitmap) {
    if (!bitmap.peekPixels(nullptr)) {
        return {};
    }

    GrBitmapTextureMaker maker(context, bitmap, GrImageTexGenPolicy::kDraw);
    return maker.view(GrMipMapped::kNo);
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
static inline int32_t dither_range_type_for_config(GrColorType dstColorType) {
    switch (dstColorType) {
        case GrColorType::kUnknown:
        case GrColorType::kGray_8:
        case GrColorType::kRGBA_8888:
        case GrColorType::kRGB_888x:
        case GrColorType::kRG_88:
        case GrColorType::kBGRA_8888:
        case GrColorType::kRG_1616:
        case GrColorType::kRGBA_16161616:
        case GrColorType::kRG_F16:
        case GrColorType::kRGBA_8888_SRGB:
        case GrColorType::kRGBA_1010102:
        case GrColorType::kBGRA_1010102:
        case GrColorType::kAlpha_F16:
        case GrColorType::kRGBA_F32:
        case GrColorType::kRGBA_F16:
        case GrColorType::kRGBA_F16_Clamped:
        case GrColorType::kAlpha_8:
        case GrColorType::kAlpha_8xxx:
        case GrColorType::kAlpha_16:
        case GrColorType::kAlpha_F32xxx:
        case GrColorType::kGray_8xxx:
        case GrColorType::kRGB_888:
        case GrColorType::kR_8:
        case GrColorType::kR_16:
        case GrColorType::kR_F16:
        case GrColorType::kGray_F16:
            return 0;
        case GrColorType::kBGR_565:
            return 1;
        case GrColorType::kABGR_4444:
            return 2;
    }
    SkUNREACHABLE;
}
#endif

static inline bool skpaint_to_grpaint_impl(GrRecordingContext* context,
                                           const GrColorInfo& dstColorInfo,
                                           const SkPaint& skPaint,
                                           const SkMatrix& viewM,
                                           std::unique_ptr<GrFragmentProcessor>* shaderProcessor,
                                           SkBlendMode* primColorMode,
                                           GrPaint* grPaint) {
    // Convert SkPaint color to 4f format in the destination color space
    SkColor4f origColor = SkColor4fPrepForDst(skPaint.getColor4f(), dstColorInfo);

    GrFPArgs fpArgs(context, &viewM, skPaint.getFilterQuality(), &dstColorInfo);

    // Setup the initial color considering the shader, the SkPaint color, and the presence or not
    // of per-vertex colors.
    std::unique_ptr<GrFragmentProcessor> shaderFP;
    if (!primColorMode || blend_requires_shader(*primColorMode)) {
        fpArgs.fInputColorIsOpaque = origColor.isOpaque();
        if (shaderProcessor) {
            shaderFP = std::move(*shaderProcessor);
        } else if (const auto* shader = as_SB(skPaint.getShader())) {
            shaderFP = shader->asFragmentProcessor(fpArgs);
            if (!shaderFP) {
                return false;
            }
        }
    }

    // Set this in below cases if the output of the shader/paint-color/paint-alpha/primXfermode is
    // a known constant value. In that case we can simply apply a color filter during this
    // conversion without converting the color filter to a GrFragmentProcessor.
    bool applyColorFilterToPaintColor = false;
    if (shaderFP) {
        if (primColorMode) {
            // There is a blend between the primitive color and the shader color. The shader sees
            // the opaque paint color. The shader's output is blended using the provided mode by
            // the primitive color. The blended color is then modulated by the paint's alpha.

            // The geometry processor will insert the primitive color to start the color chain, so
            // the GrPaint color will be ignored.

            SkPMColor4f shaderInput = origColor.makeOpaque().premul();
            shaderFP = GrFragmentProcessor::OverrideInput(std::move(shaderFP), shaderInput);
            shaderFP = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(shaderFP),
                                                                         *primColorMode);

            // The above may return null if compose results in a pass through of the prim color.
            if (shaderFP) {
                grPaint->addColorFragmentProcessor(std::move(shaderFP));
            }

            // We can ignore origColor here - alpha is unchanged by gamma
            float paintAlpha = skPaint.getColor4f().fA;
            if (1.0f != paintAlpha) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                grPaint->addColorFragmentProcessor(GrConstColorProcessor::Make(
                    { paintAlpha, paintAlpha, paintAlpha, paintAlpha },
                    GrConstColorProcessor::InputMode::kModulateRGBA));
            }
        } else {
            // The shader's FP sees the paint *unpremul* color
            SkPMColor4f origColorAsPM = { origColor.fR, origColor.fG, origColor.fB, origColor.fA };
            grPaint->setColor4f(origColorAsPM);
            grPaint->addColorFragmentProcessor(std::move(shaderFP));
        }
    } else {
        if (primColorMode) {
            // There is a blend between the primitive color and the paint color. The blend considers
            // the opaque paint color. The paint's alpha is applied to the post-blended color.
            SkPMColor4f opaqueColor = origColor.makeOpaque().premul();
            auto processor = GrConstColorProcessor::Make(opaqueColor,
                                                         GrConstColorProcessor::InputMode::kIgnore);
            processor = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(processor),
                                                                          *primColorMode);
            if (processor) {
                grPaint->addColorFragmentProcessor(std::move(processor));
            }

            grPaint->setColor4f(opaqueColor);

            // We can ignore origColor here - alpha is unchanged by gamma
            float paintAlpha = skPaint.getColor4f().fA;
            if (1.0f != paintAlpha) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                grPaint->addColorFragmentProcessor(GrConstColorProcessor::Make(
                    { paintAlpha, paintAlpha, paintAlpha, paintAlpha },
                    GrConstColorProcessor::InputMode::kModulateRGBA));
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
            auto cfFP = colorFilter->asFragmentProcessor(context, dstColorInfo);
            if (cfFP) {
                grPaint->addColorFragmentProcessor(std::move(cfFP));
            } else {
                return false;
            }
        }
    }

    SkMaskFilterBase* maskFilter = as_MFB(skPaint.getMaskFilter());
    if (maskFilter) {
        // We may have set this before passing to the SkShader.
        fpArgs.fInputColorIsOpaque = false;
        if (auto mfFP = maskFilter->asFragmentProcessor(fpArgs)) {
            grPaint->addCoverageFragmentProcessor(std::move(mfFP));
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
    if (SkPaintPriv::ShouldDither(skPaint, GrColorTypeToSkColorType(ct)) &&
        grPaint->numColorFragmentProcessors() > 0) {
        int32_t ditherRange = dither_range_type_for_config(ct);
        if (ditherRange >= 0) {
            static auto effect = std::get<0>(SkRuntimeEffect::Make(SkString(SKSL_DITHER_SRC)));
            auto ditherFP = GrSkSLFP::Make(context, effect, "Dither",
                                           SkData::MakeWithCopy(&ditherRange, sizeof(ditherRange)));
            if (ditherFP) {
                grPaint->addColorFragmentProcessor(std::move(ditherFP));
            }
        }
    }
#endif
    if (GrColorTypeClampType(dstColorInfo.colorType()) == GrClampType::kManual) {
        if (grPaint->numColorFragmentProcessors()) {
            grPaint->addColorFragmentProcessor(GrClampFragmentProcessor::Make(false));
        } else {
            auto color = grPaint->getColor4f();
            grPaint->setColor4f({SkTPin(color.fR, 0.f, 1.f),
                                 SkTPin(color.fG, 0.f, 1.f),
                                 SkTPin(color.fB, 0.f, 1.f),
                                 SkTPin(color.fA, 0.f, 1.f)});
        }
    }
    return true;
}

bool SkPaintToGrPaint(GrRecordingContext* context, const GrColorInfo& dstColorInfo,
                      const SkPaint& skPaint, const SkMatrix& viewM, GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, viewM, nullptr, nullptr,
                                   grPaint);
}

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. */
bool SkPaintToGrPaintReplaceShader(GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo,
                                   const SkPaint& skPaint,
                                   std::unique_ptr<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint) {
    if (!shaderFP) {
        return false;
    }
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, SkMatrix::I(), &shaderFP,
                                   nullptr, grPaint);
}

/** Ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrRecordingContext* context,
                              const GrColorInfo& dstColorInfo,
                              const SkPaint& skPaint,
                              GrPaint* grPaint) {
    // Use a ptr to a nullptr to to indicate that the SkShader is ignored and not replaced.
    std::unique_ptr<GrFragmentProcessor> nullShaderFP(nullptr);
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, SkMatrix::I(), &nullShaderFP,
                                   nullptr, grPaint);
}

/** Blends the SkPaint's shader (or color if no shader) with a per-primitive color which must
be setup as a vertex attribute using the specified SkBlendMode. */
bool SkPaintToGrPaintWithXfermode(GrRecordingContext* context,
                                  const GrColorInfo& dstColorInfo,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkBlendMode primColorMode,
                                  GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, dstColorInfo, skPaint, viewM, nullptr, &primColorMode,
                                   grPaint);
}

bool SkPaintToGrPaintWithTexture(GrRecordingContext* context,
                                 const GrColorInfo& dstColorInfo,
                                 const SkPaint& paint,
                                 const SkMatrix& viewM,
                                 std::unique_ptr<GrFragmentProcessor> fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint) {
    std::unique_ptr<GrFragmentProcessor> shaderFP;
    if (textureIsAlphaOnly) {
        if (const auto* shader = as_SB(paint.getShader())) {
            shaderFP = shader->asFragmentProcessor(
                    GrFPArgs(context, &viewM, paint.getFilterQuality(), &dstColorInfo));
            if (!shaderFP) {
                return false;
            }
            std::unique_ptr<GrFragmentProcessor> fpSeries[] = { std::move(shaderFP), std::move(fp) };
            shaderFP = GrFragmentProcessor::RunInSeries(fpSeries, 2);
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

    return SkPaintToGrPaintReplaceShader(context, dstColorInfo, paint, std::move(shaderFP),
                                         grPaint);
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrSamplerState::Filter GrSkFilterQualityToGrFilterMode(int imageWidth, int imageHeight,
                                                       SkFilterQuality paintFilterQuality,
                                                       const SkMatrix& viewM,
                                                       const SkMatrix& localM,
                                                       bool sharpenMipmappedTextures,
                                                       bool* doBicubic) {
    *doBicubic = false;
    if (imageWidth <= 1 && imageHeight <= 1) {
        return GrSamplerState::Filter::kNearest;
    }
    switch (paintFilterQuality) {
        case kNone_SkFilterQuality:
            return GrSamplerState::Filter::kNearest;
        case kLow_SkFilterQuality:
            return GrSamplerState::Filter::kBilerp;
        case kMedium_SkFilterQuality: {
            SkMatrix matrix;
            matrix.setConcat(viewM, localM);
            // With sharp mips, we bias lookups by -0.5. That means our final LOD is >= 0 until the
            // computed LOD is >= 0.5. At what scale factor does a texture get an LOD of 0.5?
            //
            // Want:  0       = log2(1/s) - 0.5
            //        0.5     = log2(1/s)
            //        2^0.5   = 1/s
            //        1/2^0.5 = s
            //        2^0.5/2 = s
            SkScalar mipScale = sharpenMipmappedTextures ? SK_ScalarRoot2Over2 : SK_Scalar1;
            if (matrix.getMinScale() < mipScale) {
                return GrSamplerState::Filter::kMipMap;
            } else {
                // Don't trigger MIP level generation unnecessarily.
                return GrSamplerState::Filter::kBilerp;
            }
        }
        case kHigh_SkFilterQuality: {
            SkMatrix matrix;
            matrix.setConcat(viewM, localM);
            GrSamplerState::Filter textureFilterMode;
            *doBicubic = GrBicubicEffect::ShouldUseBicubic(matrix, &textureFilterMode);
            return textureFilterMode;
        }
    }
    SkUNREACHABLE;
}

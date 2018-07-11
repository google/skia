/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGr.h"
#include "GrBitmapTextureMaker.h"
#include "GrCaps.h"
#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResourcePriv.h"
#include "GrPaint.h"
#include "GrProxyProvider.h"
#include "GrTextureProxy.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkAutoMalloc.h"
#include "SkBlendModePriv.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkData.h"
#include "SkImage_Base.h"
#include "SkImageInfoPriv.h"
#include "SkImagePriv.h"
#include "SkMaskFilterBase.h"
#include "SkMessageBus.h"
#include "SkMipMap.h"
#include "SkPM4fPriv.h"
#include "SkPaintPriv.h"
#include "SkPixelRef.h"
#include "SkResourceCache.h"
#include "SkShaderBase.h"
#include "SkTemplates.h"
#include "SkTraceEvent.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrConstColorProcessor.h"
#include "effects/GrDitherEffect.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

GrSurfaceDesc GrImageInfoToSurfaceDesc(const SkImageInfo& info) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = 1;
    return desc;
}

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

//////////////////////////////////////////////////////////////////////////////
sk_sp<GrTextureProxy> GrUploadBitmapToTextureProxy(GrProxyProvider* proxyProvider,
                                                   const SkBitmap& bitmap) {
    if (!bitmap.peekPixels(nullptr)) {
        return nullptr;
    }

    if (!SkImageInfoIsValid(bitmap.info())) {
        return nullptr;
    }

    // In non-ddl we will always instantiate right away. Thus we never want to copy the SkBitmap
    // even if it's mutable. In ddl, if the bitmap is mutable then we must make a copy since the
    // upload of the data to the gpu can happen at anytime and the bitmap may change by then.
    SkCopyPixelsMode cpyMode = proxyProvider->recordingDDL() ? kIfMutable_SkCopyPixelsMode
                                                             : kNever_SkCopyPixelsMode;
    sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(bitmap, cpyMode);

    return proxyProvider->createTextureProxy(std::move(image), kNone_GrSurfaceFlags, 1,
                                             SkBudgeted::kYes, SkBackingFit::kExact);
}

////////////////////////////////////////////////////////////////////////////////

void GrInstallBitmapUniqueKeyInvalidator(const GrUniqueKey& key, uint32_t contextUniqueID,
                                         SkPixelRef* pixelRef) {
    class Invalidator : public SkPixelRef::GenIDChangeListener {
    public:
        explicit Invalidator(const GrUniqueKey& key, uint32_t contextUniqueID)
                : fMsg(key, contextUniqueID) {}

    private:
        GrUniqueKeyInvalidatedMessage fMsg;

        void onChange() override { SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg); }
    };

    pixelRef->addGenIDChangeListener(new Invalidator(key, contextUniqueID));
}

sk_sp<GrTextureProxy> GrCopyBaseMipMapToTextureProxy(GrContext* ctx, GrTextureProxy* baseProxy) {
    SkASSERT(baseProxy);

    if (!ctx->contextPriv().caps()->isConfigCopyable(baseProxy->config())) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = baseProxy->width();
    desc.fHeight = baseProxy->height();
    desc.fConfig = baseProxy->config();
    desc.fSampleCnt = 1;

    sk_sp<GrTextureProxy> proxy =
            proxyProvider->createMipMapProxy(desc, baseProxy->origin(), SkBudgeted::kYes);
    if (!proxy) {
        return nullptr;
    }

    // Copy the base layer to our proxy
    sk_sp<GrSurfaceContext> sContext =
            ctx->contextPriv().makeWrappedSurfaceContext(proxy);
    SkASSERT(sContext);
    SkAssertResult(sContext->copy(baseProxy));

    return proxy;
}

sk_sp<GrTextureProxy> GrRefCachedBitmapTextureProxy(GrContext* ctx,
                                                    const SkBitmap& bitmap,
                                                    const GrSamplerState& params,
                                                    SkScalar scaleAdjust[2]) {
    // Caller doesn't care about the texture's color space (they can always get it from the bitmap)
    return GrBitmapTextureMaker(ctx, bitmap).refTextureProxyForParams(params, nullptr,
                                                                      nullptr, scaleAdjust);
}

sk_sp<GrTextureProxy> GrMakeCachedBitmapProxy(GrProxyProvider* proxyProvider,
                                              const SkBitmap& bitmap,
                                              SkBackingFit fit) {
    if (!bitmap.peekPixels(nullptr)) {
        return nullptr;
    }

    // In non-ddl we will always instantiate right away. Thus we never want to copy the SkBitmap
    // even if its mutable. In ddl, if the bitmap is mutable then we must make a copy since the
    // upload of the data to the gpu can happen at anytime and the bitmap may change by then.
    SkCopyPixelsMode cpyMode = proxyProvider->recordingDDL() ? kIfMutable_SkCopyPixelsMode
                                                             : kNever_SkCopyPixelsMode;
    sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(bitmap, cpyMode);

    if (!image) {
        return nullptr;
    }

    return GrMakeCachedImageProxy(proxyProvider, std::move(image), fit);
}

static void create_unique_key_for_image(const SkImage* image, GrUniqueKey* result) {
    if (!image) {
        result->reset(); // will be invalid
        return;
    }

    if (const SkBitmap* bm = as_IB(image)->onPeekBitmap()) {
        if (!bm->isVolatile()) {
            SkIPoint origin = bm->pixelRefOrigin();
            SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, bm->width(), bm->height());
            GrMakeKeyFromImageID(result, bm->getGenerationID(), subset);
        }
        return;
    }

    GrMakeKeyFromImageID(result, image->uniqueID(), image->bounds());
}

sk_sp<GrTextureProxy> GrMakeCachedImageProxy(GrProxyProvider* proxyProvider,
                                             sk_sp<SkImage> srcImage,
                                             SkBackingFit fit) {
    sk_sp<GrTextureProxy> proxy;
    GrUniqueKey originalKey;

    create_unique_key_for_image(srcImage.get(), &originalKey);

    if (originalKey.isValid()) {
        proxy = proxyProvider->findOrCreateProxyByUniqueKey(originalKey, kTopLeft_GrSurfaceOrigin);
    }
    if (!proxy) {
        proxy = proxyProvider->createTextureProxy(srcImage, kNone_GrSurfaceFlags, 1,
                                                  SkBudgeted::kYes, fit);
        if (proxy && originalKey.isValid()) {
            proxyProvider->assignUniqueKeyToProxy(originalKey, proxy.get());
            const SkBitmap* bm = as_IB(srcImage.get())->onPeekBitmap();
            // When recording DDLs we do not want to install change listeners because doing
            // so isn't threadsafe.
            if (bm && !proxyProvider->recordingDDL()) {
                GrInstallBitmapUniqueKeyInvalidator(originalKey, proxyProvider->contextUniqueID(),
                                                    bm->pixelRef());
            }
        }
    }

    return proxy;
}

///////////////////////////////////////////////////////////////////////////////

GrColor4f SkColorToPremulGrColor4f(SkColor c, const GrColorSpaceInfo& colorSpaceInfo) {
    // We want to premultiply after color space conversion, so this is easy:
    return SkColorToUnpremulGrColor4f(c, colorSpaceInfo).premul();
}

GrColor4f SkColorToPremulGrColor4fLegacy(SkColor c) {
    return GrColor4f::FromGrColor(SkColorToUnpremulGrColor(c)).premul();
}

GrColor4f SkColorToUnpremulGrColor4f(SkColor c, const GrColorSpaceInfo& colorSpaceInfo) {
    GrColor4f color = GrColor4f::FromGrColor(SkColorToUnpremulGrColor(c));
    if (auto* xform = colorSpaceInfo.colorSpaceXformFromSRGB()) {
        color = xform->apply(color);
    }
    return color;
}

///////////////////////////////////////////////////////////////////////////////

GrPixelConfig SkColorType2GrPixelConfig(const SkColorType type) {
    switch (type) {
        case kUnknown_SkColorType:
            return kUnknown_GrPixelConfig;
        case kAlpha_8_SkColorType:
            return kAlpha_8_GrPixelConfig;
        case kRGB_565_SkColorType:
            return kRGB_565_GrPixelConfig;
        case kARGB_4444_SkColorType:
            return kRGBA_4444_GrPixelConfig;
        case kRGBA_8888_SkColorType:
            return kRGBA_8888_GrPixelConfig;
        case kRGB_888x_SkColorType:
            return kRGB_888_GrPixelConfig;
        case kBGRA_8888_SkColorType:
            return kBGRA_8888_GrPixelConfig;
        case kRGBA_1010102_SkColorType:
            return kRGBA_1010102_GrPixelConfig;
        case kRGB_101010x_SkColorType:
            return kUnknown_GrPixelConfig;
        case kGray_8_SkColorType:
            return kGray_8_GrPixelConfig;
        case kRGBA_F16_SkColorType:
            return kRGBA_half_GrPixelConfig;
        case kRGBA_F32_SkColorType:
            return kRGBA_float_GrPixelConfig;
    }
    SkASSERT(0);    // shouldn't get here
    return kUnknown_GrPixelConfig;
}

GrPixelConfig SkImageInfo2GrPixelConfig(const SkImageInfo& info) {
    return SkColorType2GrPixelConfig(info.colorType());
}

bool GrPixelConfigToColorType(GrPixelConfig config, SkColorType* ctOut) {
    SkColorType ct = GrColorTypeToSkColorType(GrPixelConfigToColorType(config));
    if (kUnknown_SkColorType != ct) {
        if (ctOut) {
            *ctOut = ct;
        }
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool blend_requires_shader(const SkBlendMode mode) {
    return SkBlendMode::kDst != mode;
}

static inline bool skpaint_to_grpaint_impl(GrContext* context,
                                           const GrColorSpaceInfo& colorSpaceInfo,
                                           const SkPaint& skPaint,
                                           const SkMatrix& viewM,
                                           std::unique_ptr<GrFragmentProcessor>* shaderProcessor,
                                           SkBlendMode* primColorMode,
                                           GrPaint* grPaint) {
    // Convert SkPaint color to 4f format in the destination color space
    GrColor4f origColor = SkColorToUnpremulGrColor4f(skPaint.getColor(), colorSpaceInfo);

    const GrFPArgs fpArgs(context, &viewM, skPaint.getFilterQuality(), &colorSpaceInfo);

    // Setup the initial color considering the shader, the SkPaint color, and the presence or not
    // of per-vertex colors.
    std::unique_ptr<GrFragmentProcessor> shaderFP;
    if (!primColorMode || blend_requires_shader(*primColorMode)) {
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

            GrColor4f shaderInput = origColor.opaque();
            shaderFP = GrFragmentProcessor::OverrideInput(std::move(shaderFP), shaderInput);
            shaderFP = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(shaderFP),
                                                                         *primColorMode);

            // The above may return null if compose results in a pass through of the prim color.
            if (shaderFP) {
                grPaint->addColorFragmentProcessor(std::move(shaderFP));
            }

            // We can ignore origColor here - alpha is unchanged by gamma
            GrColor paintAlpha = SkColorAlphaToGrColor(skPaint.getColor());
            if (GrColor_WHITE != paintAlpha) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                grPaint->addColorFragmentProcessor(GrConstColorProcessor::Make(
                    GrColor4f::FromGrColor(paintAlpha),
                    GrConstColorProcessor::InputMode::kModulateRGBA));
            }
        } else {
            // The shader's FP sees the paint unpremul color
            grPaint->setColor4f(origColor);
            grPaint->addColorFragmentProcessor(std::move(shaderFP));
        }
    } else {
        if (primColorMode) {
            // There is a blend between the primitive color and the paint color. The blend considers
            // the opaque paint color. The paint's alpha is applied to the post-blended color.
            auto processor = GrConstColorProcessor::Make(origColor.opaque(),
                                                         GrConstColorProcessor::InputMode::kIgnore);
            processor = GrXfermodeFragmentProcessor::MakeFromSrcProcessor(std::move(processor),
                                                                          *primColorMode);
            if (processor) {
                grPaint->addColorFragmentProcessor(std::move(processor));
            }

            grPaint->setColor4f(origColor.opaque());

            // We can ignore origColor here - alpha is unchanged by gamma
            GrColor paintAlpha = SkColorAlphaToGrColor(skPaint.getColor());
            if (GrColor_WHITE != paintAlpha) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                grPaint->addColorFragmentProcessor(GrConstColorProcessor::Make(
                    GrColor4f::FromGrColor(paintAlpha),
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
            grPaint->setColor4f(GrColor4f::FromSkColor4f(
                    colorFilter->filterColor4f(origColor.toSkColor4f())).premul());
        } else {
            auto cfFP = colorFilter->asFragmentProcessor(context, colorSpaceInfo);
            if (cfFP) {
                grPaint->addColorFragmentProcessor(std::move(cfFP));
            } else {
                return false;
            }
        }
    }

    SkMaskFilterBase* maskFilter = as_MFB(skPaint.getMaskFilter());
    if (maskFilter) {
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
    // Conservative default, in case GrPixelConfigToColorType() fails.
    SkColorType ct = SkColorType::kRGB_565_SkColorType;
    GrPixelConfigToColorType(colorSpaceInfo.config(), &ct);
    if (SkPaintPriv::ShouldDither(skPaint, ct) && grPaint->numColorFragmentProcessors() > 0) {
        auto ditherFP = GrDitherEffect::Make(colorSpaceInfo.config());
        if (ditherFP) {
            grPaint->addColorFragmentProcessor(std::move(ditherFP));
        }
    }
#endif
    return true;
}

bool SkPaintToGrPaint(GrContext* context, const GrColorSpaceInfo& colorSpaceInfo,
                      const SkPaint& skPaint, const SkMatrix& viewM, GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, colorSpaceInfo, skPaint, viewM, nullptr, nullptr,
                                   grPaint);
}

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. */
bool SkPaintToGrPaintReplaceShader(GrContext* context,
                                   const GrColorSpaceInfo& colorSpaceInfo,
                                   const SkPaint& skPaint,
                                   std::unique_ptr<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint) {
    if (!shaderFP) {
        return false;
    }
    return skpaint_to_grpaint_impl(context, colorSpaceInfo, skPaint, SkMatrix::I(), &shaderFP,
                                   nullptr, grPaint);
}

/** Ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrContext* context,
                              const GrColorSpaceInfo& colorSpaceInfo,
                              const SkPaint& skPaint,
                              GrPaint* grPaint) {
    // Use a ptr to a nullptr to to indicate that the SkShader is ignored and not replaced.
    std::unique_ptr<GrFragmentProcessor> nullShaderFP(nullptr);
    return skpaint_to_grpaint_impl(context, colorSpaceInfo, skPaint, SkMatrix::I(), &nullShaderFP,
                                   nullptr, grPaint);
}

/** Blends the SkPaint's shader (or color if no shader) with a per-primitive color which must
be setup as a vertex attribute using the specified SkBlendMode. */
bool SkPaintToGrPaintWithXfermode(GrContext* context,
                                  const GrColorSpaceInfo& colorSpaceInfo,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkBlendMode primColorMode,
                                  GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, colorSpaceInfo, skPaint, viewM, nullptr, &primColorMode,
                                   grPaint);
}

bool SkPaintToGrPaintWithTexture(GrContext* context,
                                 const GrColorSpaceInfo& colorSpaceInfo,
                                 const SkPaint& paint,
                                 const SkMatrix& viewM,
                                 std::unique_ptr<GrFragmentProcessor> fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint) {
    std::unique_ptr<GrFragmentProcessor> shaderFP;
    if (textureIsAlphaOnly) {
        if (const auto* shader = as_SB(paint.getShader())) {
            shaderFP = shader->asFragmentProcessor(GrFPArgs(
                    context, &viewM, paint.getFilterQuality(), &colorSpaceInfo));
            if (!shaderFP) {
                return false;
            }
            std::unique_ptr<GrFragmentProcessor> fpSeries[] = { std::move(shaderFP), std::move(fp) };
            shaderFP = GrFragmentProcessor::RunInSeries(fpSeries, 2);
        } else {
            shaderFP = GrFragmentProcessor::MakeInputPremulAndMulByOutput(std::move(fp));
        }
    } else {
        shaderFP = GrFragmentProcessor::MulChildByInputAlpha(std::move(fp));
    }

    return SkPaintToGrPaintReplaceShader(context, colorSpaceInfo, paint, std::move(shaderFP),
                                         grPaint);
}


////////////////////////////////////////////////////////////////////////////////////////////////

GrSamplerState::Filter GrSkFilterQualityToGrFilterMode(SkFilterQuality paintFilterQuality,
                                                       const SkMatrix& viewM,
                                                       const SkMatrix& localM,
                                                       bool sharpenMipmappedTextures,
                                                       bool* doBicubic) {
    *doBicubic = false;
    GrSamplerState::Filter textureFilterMode;
    switch (paintFilterQuality) {
        case kNone_SkFilterQuality:
            textureFilterMode = GrSamplerState::Filter::kNearest;
            break;
        case kLow_SkFilterQuality:
            textureFilterMode = GrSamplerState::Filter::kBilerp;
            break;
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
                textureFilterMode = GrSamplerState::Filter::kMipMap;
            } else {
                // Don't trigger MIP level generation unnecessarily.
                textureFilterMode = GrSamplerState::Filter::kBilerp;
            }
            break;
        }
        case kHigh_SkFilterQuality: {
            SkMatrix matrix;
            matrix.setConcat(viewM, localM);
            *doBicubic = GrBicubicEffect::ShouldUseBicubic(matrix, &textureFilterMode);
            break;
        }
        default:
            // Should be unreachable.  If not, fall back to mipmaps.
            textureFilterMode = GrSamplerState::Filter::kMipMap;
            break;

    }
    return textureFilterMode;
}

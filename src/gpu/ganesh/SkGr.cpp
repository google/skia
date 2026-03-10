/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/SkGr.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/base/SkTPin.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/DitherUtils.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/shaders/SkShaderBase.h"

#include <optional>
#include <utility>

class SkBlender;
class SkColorSpace;
enum SkColorType : int;

void GrMakeKeyFromImageID(skgpu::UniqueKey* key, uint32_t imageID, const SkIRect& imageBounds) {
    SkASSERT(key);
    SkASSERT(imageID);
    SkASSERT(!imageBounds.isEmpty());
    static const skgpu::UniqueKey::Domain kImageIDDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey::Builder builder(key, kImageIDDomain, 5, "Image");
    builder[0] = imageID;
    builder[1] = imageBounds.fLeft;
    builder[2] = imageBounds.fTop;
    builder[3] = imageBounds.fRight;
    builder[4] = imageBounds.fBottom;
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkIDChangeListener> GrMakeUniqueKeyInvalidationListener(skgpu::UniqueKey* key,
                                                              uint32_t contextID) {
    class Listener : public SkIDChangeListener {
    public:
        Listener(const skgpu::UniqueKey& key, uint32_t contextUniqueID)
                : fMsg(key, contextUniqueID) {}

        void changed() override {
            SkMessageBus<skgpu::UniqueKeyInvalidatedMessage, uint32_t>::Post(fMsg);
        }

    private:
        skgpu::UniqueKeyInvalidatedMessage fMsg;
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
    return listener;
}

sk_sp<GrSurfaceProxy> GrCopyBaseMipMapToTextureProxy(GrRecordingContext* ctx,
                                                     sk_sp<GrSurfaceProxy> baseProxy,
                                                     GrSurfaceOrigin origin,
                                                     std::string_view label,
                                                     skgpu::Budgeted budgeted) {
    SkASSERT(baseProxy);

    // We don't allow this for promise proxies i.e. if they need mips they need to give them
    // to us upfront.
    if (baseProxy->isPromiseProxy()) {
        return nullptr;
    }
    if (!ctx->priv().caps()->isFormatCopyable(baseProxy->backendFormat())) {
        return nullptr;
    }
    auto copy = GrSurfaceProxy::Copy(ctx,
                                     std::move(baseProxy),
                                     origin,
                                     skgpu::Mipmapped::kYes,
                                     SkBackingFit::kExact,
                                     budgeted,
                                     label);
    if (!copy) {
        return nullptr;
    }
    SkASSERT(copy->asTextureProxy());
    return copy;
}

GrSurfaceProxyView GrCopyBaseMipMapToView(GrRecordingContext* context,
                                          GrSurfaceProxyView src,
                                          skgpu::Budgeted budgeted) {
    auto origin = src.origin();
    auto swizzle = src.swizzle();
    auto proxy = src.refProxy();
    return {GrCopyBaseMipMapToTextureProxy(
                    context, proxy, origin, /*label=*/"CopyBaseMipMapToView", budgeted),
            origin,
            swizzle};
}

static skgpu::Mipmapped adjust_mipmapped(skgpu::Mipmapped mipmapped,
                                         const SkBitmap& bitmap,
                                         const GrCaps* caps) {
    if (!caps->mipmapSupport() || bitmap.dimensions().area() <= 1) {
        return skgpu::Mipmapped::kNo;
    }
    return mipmapped;
}

static GrColorType choose_bmp_texture_colortype(const GrCaps* caps, const SkBitmap& bitmap) {
    GrColorType ct = SkColorTypeToGrColorType(bitmap.info().colorType());
    if (caps->getDefaultBackendFormat(ct, GrRenderable::kNo).isValid()) {
        return ct;
    }
    return GrColorType::kRGBA_8888;
}

static sk_sp<GrTextureProxy> make_bmp_proxy(GrProxyProvider* proxyProvider,
                                            const SkBitmap& bitmap,
                                            GrColorType ct,
                                            skgpu::Mipmapped mipmapped,
                                            SkBackingFit fit,
                                            skgpu::Budgeted budgeted) {
    SkBitmap bmpToUpload;
    if (ct != SkColorTypeToGrColorType(bitmap.info().colorType())) {
        SkColorType skCT = GrColorTypeToSkColorType(ct);
        if (!bmpToUpload.tryAllocPixels(bitmap.info().makeColorType(skCT)) ||
            !bitmap.readPixels(bmpToUpload.pixmap())) {
            return {};
        }
        bmpToUpload.setImmutable();
    } else {
        bmpToUpload = bitmap;
    }
    auto proxy = proxyProvider->createProxyFromBitmap(bmpToUpload, mipmapped, fit, budgeted);
    SkASSERT(!proxy || mipmapped == skgpu::Mipmapped::kNo ||
             proxy->mipmapped() == skgpu::Mipmapped::kYes);
    return proxy;
}

std::tuple<GrSurfaceProxyView, GrColorType> GrMakeCachedBitmapProxyView(
        GrRecordingContext* rContext,
        const SkBitmap& bitmap,
        std::string_view label,
        skgpu::Mipmapped mipmapped) {
    if (!bitmap.peekPixels(nullptr)) {
        return {};
    }

    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    const GrCaps* caps = rContext->priv().caps();

    skgpu::UniqueKey key;
    SkIPoint origin = bitmap.pixelRefOrigin();
    SkIRect subset = SkIRect::MakePtSize(origin, bitmap.dimensions());
    GrMakeKeyFromImageID(&key, bitmap.pixelRef()->getGenerationID(), subset);

    mipmapped = adjust_mipmapped(mipmapped, bitmap, caps);
    GrColorType ct = choose_bmp_texture_colortype(caps, bitmap);

    auto installKey = [&](GrTextureProxy* proxy) {
        auto listener = GrMakeUniqueKeyInvalidationListener(&key, proxyProvider->contextID());
        bitmap.pixelRef()->addGenIDChangeListener(std::move(listener));
        proxyProvider->assignUniqueKeyToProxy(key, proxy);
    };

    sk_sp<GrTextureProxy> proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
    if (!proxy) {
        proxy = make_bmp_proxy(
                proxyProvider, bitmap, ct, mipmapped, SkBackingFit::kExact, skgpu::Budgeted::kYes);
        if (!proxy) {
            return {};
        }
        SkASSERT(mipmapped == skgpu::Mipmapped::kNo ||
                 proxy->mipmapped() == skgpu::Mipmapped::kYes);
        installKey(proxy.get());
    }

    skgpu::Swizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), ct);
    if (mipmapped == skgpu::Mipmapped::kNo || proxy->mipmapped() == skgpu::Mipmapped::kYes) {
        return {{std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle}, ct};
    }

    // We need a mipped proxy, but we found a proxy earlier that wasn't mipped. Thus we generate
    // a new mipped surface and copy the original proxy into the base layer. We will then let
    // the gpu generate the rest of the mips.
    auto mippedProxy = GrCopyBaseMipMapToTextureProxy(
            rContext, proxy, kTopLeft_GrSurfaceOrigin, /*label=*/"MakeCachedBitmapProxyView");
    if (!mippedProxy) {
        // We failed to make a mipped proxy with the base copied into it. This could have
        // been from failure to make the proxy or failure to do the copy. Thus we will fall
        // back to just using the non mipped proxy; See skbug.com/40038328.
        return {{std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle}, ct};
    }
    // In this case we are stealing the key from the original proxy which should only happen
    // when we have just generated mipmaps for an originally unmipped proxy/texture. This
    // means that all future uses of the key will access the mipmapped version. The texture
    // backing the unmipped version will remain in the resource cache until the last texture
    // proxy referencing it is deleted at which time it too will be deleted or recycled.
    SkASSERT(proxy->getUniqueKey() == key);
    proxyProvider->removeUniqueKeyFromProxy(proxy.get());
    installKey(mippedProxy->asTextureProxy());
    return {{std::move(mippedProxy), kTopLeft_GrSurfaceOrigin, swizzle}, ct};
}

std::tuple<GrSurfaceProxyView, GrColorType> GrMakeUncachedBitmapProxyView(
        GrRecordingContext* rContext,
        const SkBitmap& bitmap,
        skgpu::Mipmapped mipmapped,
        SkBackingFit fit,
        skgpu::Budgeted budgeted) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    const GrCaps* caps = rContext->priv().caps();

    mipmapped = adjust_mipmapped(mipmapped, bitmap, caps);
    GrColorType ct = choose_bmp_texture_colortype(caps, bitmap);

    if (auto proxy = make_bmp_proxy(proxyProvider, bitmap, ct, mipmapped, fit, budgeted)) {
        skgpu::Swizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), ct);
        SkASSERT(mipmapped == skgpu::Mipmapped::kNo ||
                 proxy->mipmapped() == skgpu::Mipmapped::kYes);
        return {{std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle}, ct};
    }
    return {};
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

static inline bool blender_requires_shader(const SkBlender* blender) {
    SkASSERT(blender);
    std::optional<SkBlendMode> mode = as_BB(blender)->asBlendMode();
    return !mode.has_value() || *mode != SkBlendMode::kDst;
}


#ifndef SK_IGNORE_GPU_DITHER
static std::unique_ptr<GrFragmentProcessor> make_dither_effect(
        GrRecordingContext* rContext,
        std::unique_ptr<GrFragmentProcessor> inputFP,
        float range,
        const GrCaps* caps) {
    if (range == 0 || inputFP == nullptr) {
        return inputFP;
    }

    if (caps->avoidDithering()) {
        return inputFP;
    }

    // We used to use integer math on sk_FragCoord, when supported, and a fallback using floating
    // point (on a 4x4 rather than 8x8 grid). Now we precompute a 8x8 table in a texture because
    // it was shown to be significantly faster on several devices. Test was done with the following
    // running in viewer with the stats layer enabled and looking at total frame time:
    //      SkRandom r;
    //      for (int i = 0; i < N; ++i) {
    //          SkColor c[2] = {r.nextU(), r.nextU()};
    //          SkPoint pts[2] = {{r.nextRangeScalar(0, 500), r.nextRangeScalar(0, 500)},
    //                            {r.nextRangeScalar(0, 500), r.nextRangeScalar(0, 500)}};
    //          SkPaint p;
    //          p.setDither(true);
    //          p.setShader(SkGradientShader::MakeLinear(pts, c, nullptr, 2, SkTileMode::kRepeat));
    //          canvas->drawPaint(p);
    //      }
    // Device            GPU             N      no dither    int math dither   table dither
    // Linux desktop     QuadroP1000     5000   304ms        400ms (1.31x)     383ms (1.26x)
    // TecnoSpark3Pro    PowerVRGE8320   200    299ms        820ms (2.74x)     592ms (1.98x)
    // Pixel 4           Adreno640       500    110ms        221ms (2.01x)     214ms (1.95x)
    // Galaxy S20 FE     Mali-G77 MP11   600    165ms        360ms (2.18x)     260ms (1.58x)
    static const SkBitmap gLUT = skgpu::MakeDitherLUT();
    auto [tex, ct] = GrMakeCachedBitmapProxyView(
            rContext, gLUT, /*label=*/"MakeDitherEffect", skgpu::Mipmapped::kNo);
    if (!tex) {
        return inputFP;
    }
    SkASSERT(ct == GrColorType::kAlpha_8);
    GrSamplerState sampler(GrSamplerState::WrapMode::kRepeat, SkFilterMode::kNearest);
    auto te = GrTextureEffect::Make(
            std::move(tex), kPremul_SkAlphaType, SkMatrix::I(), sampler, *caps);
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform half range;"
        "uniform shader inputFP;"
        "uniform shader table;"
        "half4 main(float2 xy) {"
            "half4 color = inputFP.eval(xy);"
            "half value = table.eval(sk_FragCoord.xy).a - 0.5;" // undo the bias in the table
            // For each color channel, add the random offset to the channel value and then clamp
            // between 0 and alpha to keep the color premultiplied.
            "return half4(clamp(color.rgb + value * range, 0.0, color.a), color.a);"
        "}"
    );
    return GrSkSLFP::Make(effect, "Dither", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput,
                          "range", range,
                          "inputFP", std::move(inputFP),
                          "table", GrSkSLFP::IgnoreOptFlags(std::move(te)));
}
#endif

static inline bool skpaint_to_grpaint_impl(
        skgpu::ganesh::SurfaceDrawContext* sdc,
        const SkPaint& skPaint,
        const SkMatrix& ctm,
        std::optional<std::unique_ptr<GrFragmentProcessor>> shaderFP,
        SkBlender* primColorBlender,
        GrPaint* grPaint) {
    const GrColorInfo& dstColorInfo = sdc->colorInfo();
    const SkSurfaceProps& surfaceProps = sdc->surfaceProps();
    // Convert SkPaint color to 4f format in the destination color space
    SkColor4f origColor = SkColor4fPrepForDst(skPaint.getColor4f(), dstColorInfo);

    GrFPArgs fpArgs(sdc, &dstColorInfo, surfaceProps, GrFPArgs::Scope::kDefault);

    // Setup the initial color considering the shader, the SkPaint color, and the presence or not
    // of per-vertex colors.
    std::unique_ptr<GrFragmentProcessor> paintFP;
    const bool gpProvidesShader = shaderFP.has_value() && !*shaderFP;
    if (!primColorBlender || blender_requires_shader(primColorBlender)) {
        if (shaderFP.has_value()) {
            paintFP = std::move(*shaderFP);
        } else {
            if (const SkShaderBase* shader = as_SB(skPaint.getShader())) {
                paintFP = GrFragmentProcessors::Make(shader, fpArgs, ctm);
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
        if (primColorBlender) {
            // There is a blend between the primitive color and the shader color. The shader sees
            // the opaque paint color. The shader's output is blended using the provided mode by
            // the primitive color. The blended color is then modulated by the paint's alpha.

            // The geometry processor will insert the primitive color to start the color chain, so
            // the GrPaint color will be ignored.

            SkPMColor4f shaderInput = origColor.makeOpaque().premul();
            paintFP = GrFragmentProcessor::OverrideInput(std::move(paintFP), shaderInput);
            paintFP = GrFragmentProcessors::Make(as_BB(primColorBlender),
                                                 /*srcFP=*/std::move(paintFP),
                                                 /*dstFP=*/nullptr,
                                                 fpArgs);
            if (!paintFP) {
                return false;
            }

            // We can ignore origColor here - alpha is unchanged by gamma
            float paintAlpha = skPaint.getColor4f().fA;
            if (1.0f != paintAlpha) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                paintFP = GrFragmentProcessor::ModulateRGBA(
                        std::move(paintFP), {paintAlpha, paintAlpha, paintAlpha, paintAlpha});
            }
        } else {
            float paintAlpha = skPaint.getColor4f().fA;
            if (paintAlpha != 1.0f) {
                // This invokes the shader's FP tree with an opaque version of the paint color,
                // then multiplies the final result by the incoming (paint) alpha.
                // We're actually putting the *unpremul* paint color on the GrPaint. This is okay,
                // because the shader is supposed to see the original (opaque) RGB from the paint.
                // ApplyPaintAlpha then creates a valid premul color by applying the paint alpha.
                // Think of this as equivalent to (but faster than) putting origColor.premul() on
                // the GrPaint, and ApplyPaintAlpha unpremuling it before passing it to the child.
                paintFP = GrFragmentProcessor::ApplyPaintAlpha(std::move(paintFP));
                grPaint->setColor4f({origColor.fR, origColor.fG, origColor.fB, origColor.fA});
            } else {
                // paintFP will ignore its input color, so we must disable coverage-as-alpha.
                // TODO(skbug.com/40043035): The alternative would be to always use ApplyPaintAlpha, but
                // we'd need to measure the cost of that shader math against the CAA benefit.
                paintFP = GrFragmentProcessor::DisableCoverageAsAlpha(std::move(paintFP));
                grPaint->setColor4f(origColor.premul());
            }
        }
    } else {
        if (primColorBlender) {
            // The primitive itself has color (e.g. interpolated vertex color) and this is what
            // the GP will output. Thus, we must get the paint color in separately below as a color
            // FP. This could be made more efficient if the relevant GPs used GrPaint color and
            // took the SkBlender to apply with primitive color. As it stands changing the SkPaint
            // color will break batches.
            grPaint->setColor4f(SK_PMColor4fWHITE);  // won't be used.
            if (blender_requires_shader(primColorBlender)) {
                paintFP = GrFragmentProcessor::MakeColor(origColor.makeOpaque().premul());
                paintFP = GrFragmentProcessors::Make(as_BB(primColorBlender),
                                                     /*srcFP=*/std::move(paintFP),
                                                     /*dstFP=*/nullptr,
                                                     fpArgs);
                if (!paintFP) {
                    return false;
                }
            }

            // The paint's *alpha* is applied after the paint/primitive color blend:
            // We can ignore origColor here - alpha is unchanged by gamma
            float paintAlpha = skPaint.getColor4f().fA;
            if (paintAlpha != 1.0f) {
                // No gamut conversion - paintAlpha is a (linear) alpha value, splatted to all
                // color channels. It's value should be treated as the same in ANY color space.
                paintFP = GrFragmentProcessor::ModulateRGBA(
                        std::move(paintFP), {paintAlpha, paintAlpha, paintAlpha, paintAlpha});
            }
        } else {
            // No shader, no primitive color.
            grPaint->setColor4f(origColor.premul());
            // We can do this if there isn't a GP that is acting as the shader.
            applyColorFilterToPaintColor = !gpProvidesShader;
        }
    }

    SkColorFilter* colorFilter = skPaint.getColorFilter();
    if (colorFilter) {
        if (applyColorFilterToPaintColor) {
            SkColorSpace* dstCS = dstColorInfo.colorSpace();
            grPaint->setColor4f(colorFilter->filterColor4f(origColor, dstCS, dstCS).premul());
        } else {
            auto [success, fp] = GrFragmentProcessors::Make(
                    sdc, colorFilter, std::move(paintFP), dstColorInfo, surfaceProps);
            if (!success) {
                return false;
            }
            paintFP = std::move(fp);
        }
    }

    if (auto maskFilter = skPaint.getMaskFilter()) {
        if (auto mfFP = GrFragmentProcessors::Make(maskFilter, fpArgs, ctm)) {
            grPaint->setCoverageFragmentProcessor(std::move(mfFP));
        }
    }

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = GrColorTypeToSkColorType(dstColorInfo.colorType());
    if (paintFP != nullptr && (
            surfaceProps.isAlwaysDither() || SkPaintPriv::ShouldDither(skPaint, ct))) {
        float ditherRange = skgpu::DitherRangeForConfig(ct);
        GrRecordingContext* context = sdc->recordingContext();
        paintFP = make_dither_effect(
                context, std::move(paintFP), ditherRange, context->priv().caps());
    }
#endif

    // Note that for the final blend onto the canvas, we should prefer to use the GrXferProcessor
    // instead of a SkBlendModeBlender to perform the blend. The Xfer processor is able to perform
    // coefficient-based blends directly, without readback. This will be much more efficient.
    if (auto bm = skPaint.asBlendMode()) {
        // When the xfermode is null on the SkPaint (meaning kSrcOver) we need the XPFactory field
        // on the GrPaint to also be null (also kSrcOver).
        SkASSERT(!grPaint->getXPFactory());
        if (bm.value() != SkBlendMode::kSrcOver) {
            grPaint->setXPFactory(GrXPFactory::FromBlendMode(bm.value()));
        }
    } else {
        // Apply a custom blend against the surface color, and force the XP to kSrc so that the
        // computed result is applied directly to the canvas while still honoring the alpha.
        paintFP = GrFragmentProcessors::Make(as_BB(skPaint.getBlender()),
                                             std::move(paintFP),
                                             GrFragmentProcessor::SurfaceColor(),
                                             fpArgs);
        if (!paintFP) {
            return false;
        }
        grPaint->setXPFactory(GrXPFactory::FromBlendMode(SkBlendMode::kSrc));
    }

    if (GrColorTypeClampType(dstColorInfo.colorType()) == GrClampType::kManual) {
        if (paintFP != nullptr) {
            paintFP = GrFragmentProcessor::ClampOutput(std::move(paintFP));
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

bool SkPaintToGrPaint(skgpu::ganesh::SurfaceDrawContext* sdc,
                      const SkPaint& skPaint,
                      const SkMatrix& ctm,
                      GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(sdc,
                                   skPaint,
                                   ctm,
                                   /*shaderFP=*/std::nullopt,
                                   /*primColorBlender=*/nullptr,
                                   grPaint);
}

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. */
bool SkPaintToGrPaintReplaceShader(skgpu::ganesh::SurfaceDrawContext* sdc,
                                   const SkPaint& skPaint,
                                   const SkMatrix& ctm,
                                   std::unique_ptr<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(sdc,
                                   skPaint,
                                   ctm,
                                   std::move(shaderFP),
                                   /*primColorBlender=*/nullptr,
                                   grPaint);
}

/** Blends the SkPaint's shader (or color if no shader) with a per-primitive color which must
    be setup as a vertex attribute using the specified SkBlender. */
bool SkPaintToGrPaintWithBlend(skgpu::ganesh::SurfaceDrawContext* sdc,
                               const SkPaint& skPaint,
                               const SkMatrix& ctm,
                               SkBlender* primColorBlender,
                               GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(sdc,
                                   skPaint,
                                   ctm,
                                   /*shaderFP=*/std::nullopt,
                                   primColorBlender,
                                   grPaint);
}

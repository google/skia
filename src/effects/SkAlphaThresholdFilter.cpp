/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAlphaThresholdFilter.h"

#include "SkBitmap.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkRegion.h"
#if SK_SUPPORT_GPU
#include "GrDrawContext.h"
#endif

class SK_API SkAlphaThresholdFilterImpl : public SkImageFilter {
public:
    SkAlphaThresholdFilterImpl(const SkRegion& region, SkScalar innerThreshold,
                               SkScalar outerThreshold, sk_sp<SkImageFilter> input);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkAlphaThresholdFilterImpl)
    friend void SkAlphaThresholdFilter::InitializeFlattenables();

protected:
    void flatten(SkWriteBuffer&) const override;

    bool onFilterImageDeprecated(Proxy*, const SkBitmap& src, const Context&,
                                 SkBitmap* result, SkIPoint* offset) const override;
#if SK_SUPPORT_GPU
    bool asFragmentProcessor(GrFragmentProcessor**, GrTexture*, const SkMatrix&,
                             const SkIRect& bounds) const override;
#endif

private:
    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;
    typedef SkImageFilter INHERITED;
};

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkAlphaThresholdFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkAlphaThresholdFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

static SkScalar pin_0_1(SkScalar x) {
    return SkMinScalar(SkMaxScalar(x, 0), 1);
}

sk_sp<SkImageFilter> SkAlphaThresholdFilter::Make(const SkRegion& region,
                                                  SkScalar innerThreshold,
                                                  SkScalar outerThreshold,
                                                  sk_sp<SkImageFilter> input) {
    innerThreshold = pin_0_1(innerThreshold);
    outerThreshold = pin_0_1(outerThreshold);
    if (!SkScalarIsFinite(innerThreshold) || !SkScalarIsFinite(outerThreshold)) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkAlphaThresholdFilterImpl(region, innerThreshold,
                                                               outerThreshold, std::move(input)));
}

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrTextureAccess.h"
#include "effects/GrPorterDuffXferProcessor.h"

#include "SkGr.h"

#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

namespace {

SkMatrix make_div_and_translate_matrix(GrTexture* texture, int x, int y) {
    SkMatrix matrix = GrCoordTransform::MakeDivByTextureWHMatrix(texture);
    matrix.preTranslate(SkIntToScalar(x), SkIntToScalar(y));
    return matrix;
}

};

class AlphaThresholdEffect : public GrFragmentProcessor {

public:
    static GrFragmentProcessor* Create(GrTexture* texture,
                                       GrTexture* maskTexture,
                                       float innerThreshold,
                                       float outerThreshold,
                                       const SkIRect& bounds) {
        return new AlphaThresholdEffect(texture, maskTexture, innerThreshold, outerThreshold,
                                        bounds);
    }

    virtual ~AlphaThresholdEffect() {};

    const char* name() const override { return "Alpha Threshold"; }

    float innerThreshold() const { return fInnerThreshold; }
    float outerThreshold() const { return fOuterThreshold; }

private:
    AlphaThresholdEffect(GrTexture* texture,
                         GrTexture* maskTexture,
                         float innerThreshold,
                         float outerThreshold,
                         const SkIRect& bounds)
        : fInnerThreshold(innerThreshold)
        , fOuterThreshold(outerThreshold)
        , fImageCoordTransform(kLocal_GrCoordSet,
                               GrCoordTransform::MakeDivByTextureWHMatrix(texture), texture,
                               GrTextureParams::kNone_FilterMode)
        , fImageTextureAccess(texture)
        , fMaskCoordTransform(kLocal_GrCoordSet,
                              make_div_and_translate_matrix(maskTexture, -bounds.x(), -bounds.y()),
                              maskTexture,
                              GrTextureParams::kNone_FilterMode)
        , fMaskTextureAccess(maskTexture) {
        this->initClassID<AlphaThresholdEffect>();
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureAccess(&fImageTextureAccess);
        this->addCoordTransform(&fMaskCoordTransform);
        this->addTextureAccess(&fMaskTextureAccess);
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    float fInnerThreshold;
    float fOuterThreshold;
    GrCoordTransform fImageCoordTransform;
    GrTextureAccess  fImageTextureAccess;
    GrCoordTransform fMaskCoordTransform;
    GrTextureAccess  fMaskTextureAccess;

    typedef GrFragmentProcessor INHERITED;
};

class GrGLAlphaThresholdEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fInnerThresholdVar;
    GrGLSLProgramDataManager::UniformHandle fOuterThresholdVar;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLAlphaThresholdEffect::emitCode(EmitArgs& args) {
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fInnerThresholdVar = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    "inner_threshold");
    fOuterThresholdVar = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    "outer_threshold");

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    SkString maskCoords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 1);

    fragBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    fragBuilder->codeAppendf("\t\tvec2 mask_coord = %s;\n", maskCoords2D.c_str());
    fragBuilder->codeAppend("\t\tvec4 input_color = ");
    fragBuilder->appendTextureLookup(args.fSamplers[0], "coord");
    fragBuilder->codeAppend(";\n");
    fragBuilder->codeAppend("\t\tvec4 mask_color = ");
    fragBuilder->appendTextureLookup(args.fSamplers[1], "mask_coord");
    fragBuilder->codeAppend(";\n");

    fragBuilder->codeAppendf("\t\tfloat inner_thresh = %s;\n",
                             uniformHandler->getUniformCStr(fInnerThresholdVar));
    fragBuilder->codeAppendf("\t\tfloat outer_thresh = %s;\n",
                             uniformHandler->getUniformCStr(fOuterThresholdVar));
    fragBuilder->codeAppend("\t\tfloat mask = mask_color.a;\n");

    fragBuilder->codeAppend("vec4 color = input_color;\n");
    fragBuilder->codeAppend("\t\tif (mask < 0.5) {\n"
                            "\t\t\tif (color.a > outer_thresh) {\n"
                            "\t\t\t\tfloat scale = outer_thresh / color.a;\n"
                            "\t\t\t\tcolor.rgb *= scale;\n"
                            "\t\t\t\tcolor.a = outer_thresh;\n"
                            "\t\t\t}\n"
                            "\t\t} else if (color.a < inner_thresh) {\n"
                            "\t\t\tfloat scale = inner_thresh / max(0.001, color.a);\n"
                            "\t\t\tcolor.rgb *= scale;\n"
                            "\t\t\tcolor.a = inner_thresh;\n"
                            "\t\t}\n");

    fragBuilder->codeAppendf("%s = %s;\n", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr4("color")).c_str());
}

void GrGLAlphaThresholdEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                       const GrProcessor& proc) {
    const AlphaThresholdEffect& alpha_threshold = proc.cast<AlphaThresholdEffect>();
    pdman.set1f(fInnerThresholdVar, alpha_threshold.innerThreshold());
    pdman.set1f(fOuterThresholdVar, alpha_threshold.outerThreshold());
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(AlphaThresholdEffect);

const GrFragmentProcessor* AlphaThresholdEffect::TestCreate(GrProcessorTestData* d) {
    GrTexture* bmpTex = d->fTextures[GrProcessorUnitTest::kSkiaPMTextureIdx];
    GrTexture* maskTex = d->fTextures[GrProcessorUnitTest::kAlphaTextureIdx];
    float innerThresh = d->fRandom->nextUScalar1();
    float outerThresh = d->fRandom->nextUScalar1();
    const int kMaxWidth = 1000;
    const int kMaxHeight = 1000;
    uint32_t width = d->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = d->fRandom->nextULessThan(kMaxHeight);
    uint32_t x = d->fRandom->nextULessThan(kMaxWidth - width);
    uint32_t y = d->fRandom->nextULessThan(kMaxHeight - height);
    SkIRect bounds = SkIRect::MakeXYWH(x, y, width, height);
    return AlphaThresholdEffect::Create(bmpTex, maskTex, innerThresh, outerThresh, bounds);
}

///////////////////////////////////////////////////////////////////////////////

void AlphaThresholdEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                 GrProcessorKeyBuilder* b) const {
    GrGLAlphaThresholdEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* AlphaThresholdEffect::onCreateGLSLInstance() const {
    return new GrGLAlphaThresholdEffect;
}

bool AlphaThresholdEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const AlphaThresholdEffect& s = sBase.cast<AlphaThresholdEffect>();
    return (this->fInnerThreshold == s.fInnerThreshold &&
            this->fOuterThreshold == s.fOuterThreshold);
}

void AlphaThresholdEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (GrPixelConfigIsAlphaOnly(this->texture(0)->config())) {
        inout->mulByUnknownSingleComponent();
    } else if (GrPixelConfigIsOpaque(this->texture(0)->config()) && fOuterThreshold >= 1.f) {
        inout->mulByUnknownOpaqueFourComponents();
    } else {
        inout->mulByUnknownFourComponents();
    }
}

#endif

sk_sp<SkFlattenable> SkAlphaThresholdFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar inner = buffer.readScalar();
    SkScalar outer = buffer.readScalar();
    SkRegion rgn;
    buffer.readRegion(&rgn);
    return SkAlphaThresholdFilter::Make(rgn, inner, outer, common.getInput(0));
}

SkAlphaThresholdFilterImpl::SkAlphaThresholdFilterImpl(const SkRegion& region,
                                                       SkScalar innerThreshold,
                                                       SkScalar outerThreshold,
                                                       sk_sp<SkImageFilter> input)
    : INHERITED(&input, 1, nullptr)
    , fRegion(region)
    , fInnerThreshold(innerThreshold)
    , fOuterThreshold(outerThreshold) {
}

#if SK_SUPPORT_GPU
bool SkAlphaThresholdFilterImpl::asFragmentProcessor(GrFragmentProcessor** fp,
                                                     GrTexture* texture,
                                                     const SkMatrix& inMatrix,
                                                     const SkIRect& bounds) const {
    if (fp) {
        GrContext* context = texture->getContext();
        GrSurfaceDesc maskDesc;
        if (context->caps()->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
            maskDesc.fConfig = kAlpha_8_GrPixelConfig;
        } else {
            maskDesc.fConfig = kRGBA_8888_GrPixelConfig;
        }
        maskDesc.fFlags = kRenderTarget_GrSurfaceFlag;
        // Add one pixel of border to ensure that clamp mode will be all zeros
        // the outside.
        maskDesc.fWidth = bounds.width();
        maskDesc.fHeight = bounds.height();
        SkAutoTUnref<GrTexture> maskTexture(
            context->textureProvider()->createApproxTexture(maskDesc));
        if (!maskTexture) {
            return false;
        }

        SkAutoTUnref<GrDrawContext> drawContext(
                                            context->drawContext(maskTexture->asRenderTarget()));
        if (drawContext) {
            GrPaint grPaint;
            grPaint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
            SkRegion::Iterator iter(fRegion);
            drawContext->clear(nullptr, 0x0, true);

            GrClip clip(SkRect::Make(SkIRect::MakeWH(bounds.width(), bounds.height())));
            while (!iter.done()) {
                SkRect rect = SkRect::Make(iter.rect());
                drawContext->drawRect(clip, grPaint, inMatrix, rect);
                iter.next();
            }
        }

        *fp = AlphaThresholdEffect::Create(texture,
                                           maskTexture,
                                           fInnerThreshold,
                                           fOuterThreshold,
                                           bounds);
    }
    return true;
}
#endif

void SkAlphaThresholdFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fInnerThreshold);
    buffer.writeScalar(fOuterThreshold);
    buffer.writeRegion(fRegion);
}

bool SkAlphaThresholdFilterImpl::onFilterImageDeprecated(Proxy* proxy, const SkBitmap& src,
                                                         const Context& ctx, SkBitmap* dst,
                                                         SkIPoint* offset) const {

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkMatrix localInverse;
    if (!ctx.ctm().invert(&localInverse)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    SkASSERT(src.getPixels());
    if (!src.getPixels() || src.width() <= 0 || src.height() <= 0) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(src.width(), src.height()));
    if (!device) {
        return false;
    }
    *dst = device->accessBitmap(false);
    SkAutoLockPixels alp_dst(*dst);

    U8CPU innerThreshold = (U8CPU)(fInnerThreshold * 0xFF);
    U8CPU outerThreshold = (U8CPU)(fOuterThreshold * 0xFF);
    SkColor* sptr = src.getAddr32(0, 0);
    SkColor* dptr = dst->getAddr32(0, 0);
    int width = src.width(), height = src.height();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const SkColor& source = sptr[y * width + x];
            SkColor output_color(source);
            SkPoint position;
            localInverse.mapXY((SkScalar)x, (SkScalar)y, &position);
            if (fRegion.contains((int32_t)position.x(), (int32_t)position.y())) {
                if (SkColorGetA(source) < innerThreshold) {
                    U8CPU alpha = SkColorGetA(source);
                    if (alpha == 0)
                        alpha = 1;
                    float scale = (float)innerThreshold / alpha;
                    output_color = SkColorSetARGB(innerThreshold,
                                                  (U8CPU)(SkColorGetR(source) * scale),
                                                  (U8CPU)(SkColorGetG(source) * scale),
                                                  (U8CPU)(SkColorGetB(source) * scale));
                }
            } else {
                if (SkColorGetA(source) > outerThreshold) {
                    float scale = (float)outerThreshold / SkColorGetA(source);
                    output_color = SkColorSetARGB(outerThreshold,
                                                  (U8CPU)(SkColorGetR(source) * scale),
                                                  (U8CPU)(SkColorGetG(source) * scale),
                                                  (U8CPU)(SkColorGetB(source) * scale));
                }
            }
            dptr[y * dst->width() + x] = output_color;
        }
    }

    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkAlphaThresholdFilterImpl::toString(SkString* str) const {
    str->appendf("SkAlphaThresholdImageFilter: (");
    str->appendf("inner: %f outer: %f", fInnerThreshold, fOuterThreshold);
    str->append(")");
}
#endif

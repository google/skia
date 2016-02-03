/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkMagnifierImageFilter.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkValidationUtils.h"

////////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "GrInvariantOutput.h"
#include "effects/GrSingleTextureEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class GrMagnifierEffect : public GrSingleTextureEffect {

public:
    static GrFragmentProcessor* Create(GrTexture* texture,
                                       const SkRect& bounds,
                                       float xOffset,
                                       float yOffset,
                                       float xInvZoom,
                                       float yInvZoom,
                                       float xInvInset,
                                       float yInvInset) {
        return new GrMagnifierEffect(texture, bounds, xOffset, yOffset, xInvZoom, yInvZoom, xInvInset,
                                     yInvInset);
    }

    virtual ~GrMagnifierEffect() {};

    const char* name() const override { return "Magnifier"; }

    const SkRect& bounds() const { return fBounds; }    // Bounds of source image.
    // Offset to apply to zoomed pixels, (srcRect position / texture size).
    float x_offset() const { return fXOffset; }
    float y_offset() const { return fYOffset; }

    // Scale to apply to zoomed pixels (srcRect size / bounds size).
    float x_inv_zoom() const { return fXInvZoom; }
    float y_inv_zoom() const { return fYInvZoom; }

    // 1/radius over which to transition from unzoomed to zoomed pixels (bounds size / inset).
    float x_inv_inset() const { return fXInvInset; }
    float y_inv_inset() const { return fYInvInset; }

private:
    GrMagnifierEffect(GrTexture* texture,
                      const SkRect& bounds,
                      float xOffset,
                      float yOffset,
                      float xInvZoom,
                      float yInvZoom,
                      float xInvInset,
                      float yInvInset)
        : INHERITED(texture, GrCoordTransform::MakeDivByTextureWHMatrix(texture))
        , fBounds(bounds)
        , fXOffset(xOffset)
        , fYOffset(yOffset)
        , fXInvZoom(xInvZoom)
        , fYInvZoom(yInvZoom)
        , fXInvInset(xInvInset)
        , fYInvInset(yInvInset) {
        this->initClassID<GrMagnifierEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkRect fBounds;
    float fXOffset;
    float fYOffset;
    float fXInvZoom;
    float fYInvZoom;
    float fXInvInset;
    float fYInvInset;

    typedef GrSingleTextureEffect INHERITED;
};

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

class GrGLMagnifierEffect : public GrGLSLFragmentProcessor {
public:
    GrGLMagnifierEffect(const GrProcessor&);

    virtual void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    UniformHandle       fOffsetVar;
    UniformHandle       fInvZoomVar;
    UniformHandle       fInvInsetVar;
    UniformHandle       fBoundsVar;

    typedef GrGLSLFragmentProcessor INHERITED;
};

GrGLMagnifierEffect::GrGLMagnifierEffect(const GrProcessor&) {
}

void GrGLMagnifierEffect::emitCode(EmitArgs& args) {
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fOffsetVar = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                            kVec2f_GrSLType, kDefault_GrSLPrecision,
                                            "Offset");
    fInvZoomVar = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                             kVec2f_GrSLType, kDefault_GrSLPrecision,
                                             "InvZoom");
    fInvInsetVar = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                              kVec2f_GrSLType, kDefault_GrSLPrecision,
                                              "InvInset");
    fBoundsVar = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                            kVec4f_GrSLType, kDefault_GrSLPrecision,
                                            "Bounds");

    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    fragBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    fragBuilder->codeAppendf("\t\tvec2 zoom_coord = %s + %s * %s;\n",
                             uniformHandler->getUniformCStr(fOffsetVar),
                             coords2D.c_str(),
                             uniformHandler->getUniformCStr(fInvZoomVar));
    const char* bounds = uniformHandler->getUniformCStr(fBoundsVar);
    fragBuilder->codeAppendf("\t\tvec2 delta = (coord - %s.xy) * %s.zw;\n", bounds, bounds);
    fragBuilder->codeAppendf("\t\tdelta = min(delta, vec2(1.0, 1.0) - delta);\n");
    fragBuilder->codeAppendf("\t\tdelta = delta * %s;\n",
                             uniformHandler->getUniformCStr(fInvInsetVar));

    fragBuilder->codeAppend("\t\tfloat weight = 0.0;\n");
    fragBuilder->codeAppend("\t\tif (delta.s < 2.0 && delta.t < 2.0) {\n");
    fragBuilder->codeAppend("\t\t\tdelta = vec2(2.0, 2.0) - delta;\n");
    fragBuilder->codeAppend("\t\t\tfloat dist = length(delta);\n");
    fragBuilder->codeAppend("\t\t\tdist = max(2.0 - dist, 0.0);\n");
    fragBuilder->codeAppend("\t\t\tweight = min(dist * dist, 1.0);\n");
    fragBuilder->codeAppend("\t\t} else {\n");
    fragBuilder->codeAppend("\t\t\tvec2 delta_squared = delta * delta;\n");
    fragBuilder->codeAppend("\t\t\tweight = min(min(delta_squared.x, delta_squared.y), 1.0);\n");
    fragBuilder->codeAppend("\t\t}\n");

    fragBuilder->codeAppend("\t\tvec2 mix_coord = mix(coord, zoom_coord, weight);\n");
    fragBuilder->codeAppend("\t\tvec4 output_color = ");
    fragBuilder->appendTextureLookup(args.fSamplers[0], "mix_coord");
    fragBuilder->codeAppend(";\n");

    fragBuilder->codeAppendf("\t\t%s = output_color;", args.fOutputColor);
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    fragBuilder->codeAppend(modulate.c_str());
}

void GrGLMagnifierEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                    const GrProcessor& effect) {
    const GrMagnifierEffect& zoom = effect.cast<GrMagnifierEffect>();
    pdman.set2f(fOffsetVar, zoom.x_offset(), zoom.y_offset());
    pdman.set2f(fInvZoomVar, zoom.x_inv_zoom(), zoom.y_inv_zoom());
    pdman.set2f(fInvInsetVar, zoom.x_inv_inset(), zoom.y_inv_inset());
    pdman.set4f(fBoundsVar, zoom.bounds().x(), zoom.bounds().y(),
                            zoom.bounds().width(), zoom.bounds().height());
}

/////////////////////////////////////////////////////////////////////

void GrMagnifierEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                              GrProcessorKeyBuilder* b) const {
    GrGLMagnifierEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrMagnifierEffect::onCreateGLSLInstance() const {
    return new GrGLMagnifierEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMagnifierEffect);

const GrFragmentProcessor* GrMagnifierEffect::TestCreate(GrProcessorTestData* d) {
    GrTexture* texture = d->fTextures[0];
    const int kMaxWidth = 200;
    const int kMaxHeight = 200;
    const int kMaxInset = 20;
    uint32_t width = d->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = d->fRandom->nextULessThan(kMaxHeight);
    uint32_t x = d->fRandom->nextULessThan(kMaxWidth - width);
    uint32_t y = d->fRandom->nextULessThan(kMaxHeight - height);
    uint32_t inset = d->fRandom->nextULessThan(kMaxInset);

    GrFragmentProcessor* effect = GrMagnifierEffect::Create(
        texture,
        SkRect::MakeWH(SkIntToScalar(kMaxWidth), SkIntToScalar(kMaxHeight)),
        (float) width / texture->width(),
        (float) height / texture->height(),
        texture->width() / (float) x,
        texture->height() / (float) y,
        (float) inset / texture->width(),
        (float) inset / texture->height());
    SkASSERT(effect);
    return effect;
}

///////////////////////////////////////////////////////////////////////////////

bool GrMagnifierEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrMagnifierEffect& s = sBase.cast<GrMagnifierEffect>();
    return (this->fBounds == s.fBounds &&
            this->fXOffset == s.fXOffset &&
            this->fYOffset == s.fYOffset &&
            this->fXInvZoom == s.fXInvZoom &&
            this->fYInvZoom == s.fYInvZoom &&
            this->fXInvInset == s.fXInvInset &&
            this->fYInvInset == s.fYInvInset);
}

void GrMagnifierEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    this->updateInvariantOutputForModulation(inout);
}

#endif

////////////////////////////////////////////////////////////////////////////////

SkImageFilter* SkMagnifierImageFilter::Create(const SkRect& srcRect, SkScalar inset,
                                              SkImageFilter* input) {

    if (!SkScalarIsFinite(inset) || !SkIsValidRect(srcRect)) {
        return nullptr;
    }
    // Negative numbers in src rect are not supported
    if (srcRect.fLeft < 0 || srcRect.fTop < 0) {
        return nullptr;
    }
    return new SkMagnifierImageFilter(srcRect, inset, input);
}


SkMagnifierImageFilter::SkMagnifierImageFilter(const SkRect& srcRect, SkScalar inset,
                                               SkImageFilter* input)
    : INHERITED(1, &input), fSrcRect(srcRect), fInset(inset) {
    SkASSERT(srcRect.x() >= 0 && srcRect.y() >= 0 && inset >= 0);
}

#if SK_SUPPORT_GPU
bool SkMagnifierImageFilter::asFragmentProcessor(GrFragmentProcessor** fp,
                                                 GrTexture* texture, const SkMatrix&,
                                                 const SkIRect&bounds) const {
    if (fp) {
        SkScalar yOffset = texture->origin() == kTopLeft_GrSurfaceOrigin ? fSrcRect.y() :
           texture->height() - fSrcRect.height() * texture->height() / bounds.height()
                             - fSrcRect.y();
        int boundsY = (texture->origin() == kTopLeft_GrSurfaceOrigin) ? bounds.y() :
                      (texture->height() - bounds.height());
        SkRect effectBounds = SkRect::MakeXYWH(
            SkIntToScalar(bounds.x()) / texture->width(),
            SkIntToScalar(boundsY) / texture->height(),
            SkIntToScalar(texture->width()) / bounds.width(),
            SkIntToScalar(texture->height()) / bounds.height());
        SkScalar invInset = fInset > 0 ? SkScalarInvert(fInset) : SK_Scalar1;
        *fp = GrMagnifierEffect::Create(texture,
                                        effectBounds,
                                        fSrcRect.x() / texture->width(),
                                        yOffset / texture->height(),
                                        fSrcRect.width() / bounds.width(),
                                        fSrcRect.height() / bounds.height(),
                                        bounds.width() * invInset,
                                        bounds.height() * invInset);
    }
    return true;
}
#endif

SkFlattenable* SkMagnifierImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect src;
    buffer.readRect(&src);
    return Create(src, buffer.readScalar(), common.getInput(0));
}

void SkMagnifierImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeScalar(fInset);
}

bool SkMagnifierImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                           const Context&, SkBitmap* dst,
                                           SkIPoint* offset) const {
    if ((src.colorType() != kN32_SkColorType) ||
        (fSrcRect.width() >= src.width()) ||
        (fSrcRect.height() >= src.height())) {
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

    SkScalar inv_inset = fInset > 0 ? SkScalarInvert(fInset) : SK_Scalar1;

    SkScalar inv_x_zoom = fSrcRect.width() / src.width();
    SkScalar inv_y_zoom = fSrcRect.height() / src.height();

    SkColor* sptr = src.getAddr32(0, 0);
    SkColor* dptr = dst->getAddr32(0, 0);
    int width = src.width(), height = src.height();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            SkScalar x_dist = SkMin32(x, width - x - 1) * inv_inset;
            SkScalar y_dist = SkMin32(y, height - y - 1) * inv_inset;
            SkScalar weight = 0;

            static const SkScalar kScalar2 = SkScalar(2);

            // To create a smooth curve at the corners, we need to work on
            // a square twice the size of the inset.
            if (x_dist < kScalar2 && y_dist < kScalar2) {
                x_dist = kScalar2 - x_dist;
                y_dist = kScalar2 - y_dist;

                SkScalar dist = SkScalarSqrt(SkScalarSquare(x_dist) +
                                             SkScalarSquare(y_dist));
                dist = SkMaxScalar(kScalar2 - dist, 0);
                weight = SkMinScalar(SkScalarSquare(dist), SK_Scalar1);
            } else {
                SkScalar sqDist = SkMinScalar(SkScalarSquare(x_dist),
                                              SkScalarSquare(y_dist));
                weight = SkMinScalar(sqDist, SK_Scalar1);
            }

            SkScalar x_interp = SkScalarMul(weight, (fSrcRect.x() + x * inv_x_zoom)) +
                           (SK_Scalar1 - weight) * x;
            SkScalar y_interp = SkScalarMul(weight, (fSrcRect.y() + y * inv_y_zoom)) +
                           (SK_Scalar1 - weight) * y;

            int x_val = SkTPin(SkScalarFloorToInt(x_interp), 0, width - 1);
            int y_val = SkTPin(SkScalarFloorToInt(y_interp), 0, height - 1);

            *dptr = sptr[y_val * width + x_val];
            dptr++;
        }
    }
    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkMagnifierImageFilter::toString(SkString* str) const {
    str->appendf("SkMagnifierImageFilter: (");
    str->appendf("src: (%f,%f,%f,%f) ",
                 fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight, fSrcRect.fBottom);
    str->appendf("inset: %f", fInset);
    str->append(")");
}
#endif

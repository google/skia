/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLightingImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkTypes.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrPaint.h"
#include "SkGr.h"
#include "effects/GrSingleTextureEffect.h"
#include "effects/GrTextureDomain.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class GrGLDiffuseLightingEffect;
class GrGLSpecularLightingEffect;

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
#endif

namespace {

const SkScalar gOneThird = SkIntToScalar(1) / 3;
const SkScalar gTwoThirds = SkIntToScalar(2) / 3;
const SkScalar gOneHalf = 0.5f;
const SkScalar gOneQuarter = 0.25f;

#if SK_SUPPORT_GPU
void setUniformPoint3(const GrGLSLProgramDataManager& pdman, UniformHandle uni,
                      const SkPoint3& point) {
    GR_STATIC_ASSERT(sizeof(SkPoint3) == 3 * sizeof(float));
    pdman.set3fv(uni, 1, &point.fX);
}

void setUniformNormal3(const GrGLSLProgramDataManager& pdman, UniformHandle uni,
                       const SkPoint3& point) {
    setUniformPoint3(pdman, uni, point);
}
#endif

// Shift matrix components to the left, as we advance pixels to the right.
inline void shiftMatrixLeft(int m[9]) {
    m[0] = m[1];
    m[3] = m[4];
    m[6] = m[7];
    m[1] = m[2];
    m[4] = m[5];
    m[7] = m[8];
}

static inline void fast_normalize(SkPoint3* vector) {
    // add a tiny bit so we don't have to worry about divide-by-zero
    SkScalar magSq = vector->dot(*vector) + SK_ScalarNearlyZero;
    SkScalar scale = sk_float_rsqrt(magSq);
    vector->fX *= scale;
    vector->fY *= scale;
    vector->fZ *= scale;
}

class DiffuseLightingType {
public:
    DiffuseLightingType(SkScalar kd)
        : fKD(kd) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight,
                    const SkPoint3& lightColor) const {
        SkScalar colorScale = SkScalarMul(fKD, normal.dot(surfaceTolight));
        colorScale = SkScalarClampMax(colorScale, SK_Scalar1);
        SkPoint3 color = lightColor.makeScale(colorScale);
        return SkPackARGB32(255,
                            SkClampMax(SkScalarRoundToInt(color.fX), 255),
                            SkClampMax(SkScalarRoundToInt(color.fY), 255),
                            SkClampMax(SkScalarRoundToInt(color.fZ), 255));
    }
private:
    SkScalar fKD;
};

static SkScalar max_component(const SkPoint3& p) {
    return p.x() > p.y() ? (p.x() > p.z() ? p.x() : p.z()) : (p.y() > p.z() ? p.y() : p.z());
}

class SpecularLightingType {
public:
    SpecularLightingType(SkScalar ks, SkScalar shininess)
        : fKS(ks), fShininess(shininess) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight,
                    const SkPoint3& lightColor) const {
        SkPoint3 halfDir(surfaceTolight);
        halfDir.fZ += SK_Scalar1;        // eye position is always (0, 0, 1)
        fast_normalize(&halfDir);
        SkScalar colorScale = SkScalarMul(fKS,
            SkScalarPow(normal.dot(halfDir), fShininess));
        colorScale = SkScalarClampMax(colorScale, SK_Scalar1);
        SkPoint3 color = lightColor.makeScale(colorScale);
        return SkPackARGB32(SkClampMax(SkScalarRoundToInt(max_component(color)), 255),
                            SkClampMax(SkScalarRoundToInt(color.fX), 255),
                            SkClampMax(SkScalarRoundToInt(color.fY), 255),
                            SkClampMax(SkScalarRoundToInt(color.fZ), 255));
    }
private:
    SkScalar fKS;
    SkScalar fShininess;
};

inline SkScalar sobel(int a, int b, int c, int d, int e, int f, SkScalar scale) {
    return SkScalarMul(SkIntToScalar(-a + b - 2 * c + 2 * d -e + f), scale);
}

inline SkPoint3 pointToNormal(SkScalar x, SkScalar y, SkScalar surfaceScale) {
    SkPoint3 vector = SkPoint3::Make(SkScalarMul(-x, surfaceScale),
                                     SkScalarMul(-y, surfaceScale),
                                     SK_Scalar1);
    fast_normalize(&vector);
    return vector;
}

inline SkPoint3 topLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(0, 0, m[4], m[5], m[7], m[8], gTwoThirds),
                         sobel(0, 0, m[4], m[7], m[5], m[8], gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 topNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[5], m[6], m[8], gOneThird),
                         sobel(m[3], m[6], m[4], m[7], m[5], m[8], gOneHalf),
                         surfaceScale);
}

inline SkPoint3 topRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[4], m[6], m[7], gTwoThirds),
                         sobel(m[3], m[6], m[4], m[7],    0,    0, gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 leftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5], m[7], m[8], gOneHalf),
                         sobel(   0,    0, m[1], m[7], m[2], m[8], gOneThird),
                         surfaceScale);
}


inline SkPoint3 interiorNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5], m[6], m[8], gOneQuarter),
                         sobel(m[0], m[6], m[1], m[7], m[2], m[8], gOneQuarter),
                         surfaceScale);
}

inline SkPoint3 rightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], m[6], m[7], gOneHalf),
                         sobel(m[0], m[6], m[1], m[7],    0,    0, gOneThird),
                         surfaceScale);
}

inline SkPoint3 bottomLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5],    0,    0, gTwoThirds),
                         sobel(   0,    0, m[1], m[4], m[2], m[5], gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 bottomNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5],    0,    0, gOneThird),
                         sobel(m[0], m[3], m[1], m[4], m[2], m[5], gOneHalf),
                         surfaceScale);
}

inline SkPoint3 bottomRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], 0,  0, gTwoThirds),
                         sobel(m[0], m[3], m[1], m[4], 0,  0, gTwoThirds),
                         surfaceScale);
}


class UncheckedPixelFetcher {
public:
    static inline uint32_t Fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        return SkGetPackedA32(*src.getAddr32(x, y));
    }
};

// The DecalPixelFetcher is used when the destination crop rect exceeds the input bitmap bounds.
class DecalPixelFetcher {
public:
    static inline uint32_t Fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        if (x < bounds.fLeft || x >= bounds.fRight || y < bounds.fTop || y >= bounds.fBottom) {
            return 0;
        } else {
            return SkGetPackedA32(*src.getAddr32(x, y));
        }
    }
};

template <class LightingType, class LightType, class PixelFetcher>
void lightBitmap(const LightingType& lightingType,
                 const SkImageFilterLight* light,
                 const SkBitmap& src,
                 SkBitmap* dst,
                 SkScalar surfaceScale,
                 const SkIRect& bounds) {
    SkASSERT(dst->width() == bounds.width() && dst->height() == bounds.height());
    const LightType* l = static_cast<const LightType*>(light);
    int left = bounds.left(), right = bounds.right();
    int bottom = bounds.bottom();
    int y = bounds.top();
    SkIRect srcBounds = src.bounds();
    SkPMColor* dptr = dst->getAddr32(0, 0);
    {
        int x = left;
        int m[9];
        m[4] = PixelFetcher::Fetch(src, x,     y,     srcBounds);
        m[5] = PixelFetcher::Fetch(src, x + 1, y,     srcBounds);
        m[7] = PixelFetcher::Fetch(src, x,     y + 1, srcBounds);
        m[8] = PixelFetcher::Fetch(src, x + 1, y + 1, srcBounds);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topLeftNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x)
        {
            shiftMatrixLeft(m);
            m[5] = PixelFetcher::Fetch(src, x + 1, y,     srcBounds);
            m[8] = PixelFetcher::Fetch(src, x + 1, y + 1, srcBounds);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(topNormal(m, surfaceScale), surfaceToLight,
                                         l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topRightNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
    }

    for (++y; y < bottom - 1; ++y) {
        int x = left;
        int m[9];
        m[1] = PixelFetcher::Fetch(src, x,     y - 1, srcBounds);
        m[2] = PixelFetcher::Fetch(src, x + 1, y - 1, srcBounds);
        m[4] = PixelFetcher::Fetch(src, x,     y,     srcBounds);
        m[5] = PixelFetcher::Fetch(src, x + 1, y,     srcBounds);
        m[7] = PixelFetcher::Fetch(src, x,     y + 1, srcBounds);
        m[8] = PixelFetcher::Fetch(src, x + 1, y + 1, srcBounds);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(leftNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x) {
            shiftMatrixLeft(m);
            m[2] = PixelFetcher::Fetch(src, x + 1, y - 1, srcBounds);
            m[5] = PixelFetcher::Fetch(src, x + 1, y,     srcBounds);
            m[8] = PixelFetcher::Fetch(src, x + 1, y + 1, srcBounds);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(interiorNormal(m, surfaceScale), surfaceToLight,
                                         l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(rightNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
    }

    {
        int x = left;
        int m[9];
        m[1] = PixelFetcher::Fetch(src, x,     bottom - 2, srcBounds);
        m[2] = PixelFetcher::Fetch(src, x + 1, bottom - 2, srcBounds);
        m[4] = PixelFetcher::Fetch(src, x,     bottom - 1, srcBounds);
        m[5] = PixelFetcher::Fetch(src, x + 1, bottom - 1, srcBounds);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(bottomLeftNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x)
        {
            shiftMatrixLeft(m);
            m[2] = PixelFetcher::Fetch(src, x + 1, bottom - 2, srcBounds);
            m[5] = PixelFetcher::Fetch(src, x + 1, bottom - 1, srcBounds);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(bottomNormal(m, surfaceScale), surfaceToLight,
                                         l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(bottomRightNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
    }
}

template <class LightingType, class LightType>
void lightBitmap(const LightingType& lightingType,
                 const SkImageFilterLight* light,
                 const SkBitmap& src,
                 SkBitmap* dst,
                 SkScalar surfaceScale,
                 const SkIRect& bounds) {
    if (src.bounds().contains(bounds)) {
        lightBitmap<LightingType, LightType, UncheckedPixelFetcher>(
            lightingType, light, src, dst, surfaceScale, bounds);
    } else {
        lightBitmap<LightingType, LightType, DecalPixelFetcher>(
            lightingType, light, src, dst, surfaceScale, bounds);
    }
}

SkPoint3 readPoint3(SkReadBuffer& buffer) {
    SkPoint3 point;
    point.fX = buffer.readScalar();
    point.fY = buffer.readScalar();
    point.fZ = buffer.readScalar();
    buffer.validate(SkScalarIsFinite(point.fX) &&
                    SkScalarIsFinite(point.fY) &&
                    SkScalarIsFinite(point.fZ));
    return point;
};

void writePoint3(const SkPoint3& point, SkWriteBuffer& buffer) {
    buffer.writeScalar(point.fX);
    buffer.writeScalar(point.fY);
    buffer.writeScalar(point.fZ);
};

enum BoundaryMode {
    kTopLeft_BoundaryMode,
    kTop_BoundaryMode,
    kTopRight_BoundaryMode,
    kLeft_BoundaryMode,
    kInterior_BoundaryMode,
    kRight_BoundaryMode,
    kBottomLeft_BoundaryMode,
    kBottom_BoundaryMode,
    kBottomRight_BoundaryMode,

    kBoundaryModeCount,
};

class SkLightingImageFilterInternal : public SkLightingImageFilter {
protected:
    SkLightingImageFilterInternal(sk_sp<SkImageFilterLight> light,
                                  SkScalar surfaceScale,
                                  sk_sp<SkImageFilter> input,
                                  const CropRect* cropRect)
        : INHERITED(std::move(light), surfaceScale, std::move(input), cropRect) {
    }

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> filterImageGPU(SkSpecialImage* source, 
                                         SkSpecialImage* input,
                                         const SkIRect& bounds,
                                         const SkMatrix& matrix) const;
    virtual sk_sp<GrFragmentProcessor> makeFragmentProcessor(GrTexture*,
                                                             const SkMatrix&,
                                                             const SkIRect* srcBounds,
                                                             BoundaryMode boundaryMode) const = 0;
#endif
private:
#if SK_SUPPORT_GPU
    void drawRect(GrDrawContext* drawContext,
                  GrTexture* src,
                  const SkMatrix& matrix,
                  const GrClip& clip,
                  const SkRect& dstRect,
                  BoundaryMode boundaryMode,
                  const SkIRect* srcBounds,
                  const SkIRect& bounds) const;
#endif
    typedef SkLightingImageFilter INHERITED;
};

#if SK_SUPPORT_GPU
void SkLightingImageFilterInternal::drawRect(GrDrawContext* drawContext,
                                             GrTexture* src,
                                             const SkMatrix& matrix,
                                             const GrClip& clip,
                                             const SkRect& dstRect,
                                             BoundaryMode boundaryMode,
                                             const SkIRect* srcBounds,
                                             const SkIRect& bounds) const {
    SkRect srcRect = dstRect.makeOffset(SkIntToScalar(bounds.x()), SkIntToScalar(bounds.y()));
    GrPaint paint;
    // SRGBTODO: AllowSRGBInputs?
    sk_sp<GrFragmentProcessor> fp(this->makeFragmentProcessor(src, matrix, srcBounds,
                                                              boundaryMode));
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    drawContext->fillRectToRect(clip, paint, SkMatrix::I(), dstRect, srcRect);
}

sk_sp<SkSpecialImage> SkLightingImageFilterInternal::filterImageGPU(SkSpecialImage* source,
                                                                    SkSpecialImage* input,
                                                                    const SkIRect& offsetBounds,
                                                                    const SkMatrix& matrix) const {
    SkASSERT(source->isTextureBacked());

    GrContext* context = source->getContext();

    sk_sp<GrTexture> inputTexture(input->asTextureRef(context));
    SkASSERT(inputTexture);

    sk_sp<GrDrawContext> drawContext(context->newDrawContext(SkBackingFit::kApprox,
                                                             offsetBounds.width(),
                                                             offsetBounds.height(),
                                                             kRGBA_8888_GrPixelConfig));
    if (!drawContext) {
        return nullptr;
    }

    SkIRect dstIRect = SkIRect::MakeWH(offsetBounds.width(), offsetBounds.height());
    SkRect dstRect = SkRect::Make(dstIRect);

    // setup new clip
    GrFixedClip clip(dstIRect);

    const SkIRect inputBounds = SkIRect::MakeWH(input->width(), input->height());
    SkRect topLeft = SkRect::MakeXYWH(0, 0, 1, 1);
    SkRect top = SkRect::MakeXYWH(1, 0, dstRect.width() - 2, 1);
    SkRect topRight = SkRect::MakeXYWH(dstRect.width() - 1, 0, 1, 1);
    SkRect left = SkRect::MakeXYWH(0, 1, 1, dstRect.height() - 2);
    SkRect interior = dstRect.makeInset(1, 1);
    SkRect right = SkRect::MakeXYWH(dstRect.width() - 1, 1, 1, dstRect.height() - 2);
    SkRect bottomLeft = SkRect::MakeXYWH(0, dstRect.height() - 1, 1, 1);
    SkRect bottom = SkRect::MakeXYWH(1, dstRect.height() - 1, dstRect.width() - 2, 1);
    SkRect bottomRight = SkRect::MakeXYWH(dstRect.width() - 1, dstRect.height() - 1, 1, 1);

    const SkIRect* pSrcBounds = inputBounds.contains(offsetBounds) ? nullptr : &inputBounds;
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, topLeft,
                   kTopLeft_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, top, kTop_BoundaryMode,
                   pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, topRight,
                   kTopRight_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, left, kLeft_BoundaryMode,
                   pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, interior,
                   kInterior_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, right, kRight_BoundaryMode,
                   pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, bottomLeft,
                   kBottomLeft_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, bottom,
                   kBottom_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(drawContext.get(), inputTexture.get(), matrix, clip, bottomRight,
                   kBottomRight_BoundaryMode, pSrcBounds, offsetBounds);

    return SkSpecialImage::MakeFromGpu(SkIRect::MakeWH(offsetBounds.width(), offsetBounds.height()),
                                       kNeedNewImageUniqueID_SpecialImage,
                                       drawContext->asTexture());
}
#endif

class SkDiffuseLightingImageFilter : public SkLightingImageFilterInternal {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilterLight> light,
                                     SkScalar surfaceScale,
                                     SkScalar kd,
                                     sk_sp<SkImageFilter>,
                                     const CropRect*);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDiffuseLightingImageFilter)
    SkScalar kd() const { return fKD; }

protected:
    SkDiffuseLightingImageFilter(sk_sp<SkImageFilterLight> light, SkScalar surfaceScale,
                                 SkScalar kd,
                                 sk_sp<SkImageFilter> input, const CropRect* cropRect);
    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> makeFragmentProcessor(GrTexture*, const SkMatrix&,
                                                     const SkIRect* bounds,
                                                     BoundaryMode) const override;
#endif

private:
    friend class SkLightingImageFilter;
    SkScalar fKD;

    typedef SkLightingImageFilterInternal INHERITED;
};

class SkSpecularLightingImageFilter : public SkLightingImageFilterInternal {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilterLight> light,
                                     SkScalar surfaceScale,
                                     SkScalar ks, SkScalar shininess,
                                     sk_sp<SkImageFilter>, const CropRect*);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSpecularLightingImageFilter)

    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

protected:
    SkSpecularLightingImageFilter(sk_sp<SkImageFilterLight> light,
                                  SkScalar surfaceScale, SkScalar ks,
                                  SkScalar shininess,
                                  sk_sp<SkImageFilter> input, const CropRect*);
    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> makeFragmentProcessor(GrTexture*, const SkMatrix&,
                                                     const SkIRect* bounds,
                                                     BoundaryMode) const override;
#endif

private:
    SkScalar fKS;
    SkScalar fShininess;
    friend class SkLightingImageFilter;
    typedef SkLightingImageFilterInternal INHERITED;
};

#if SK_SUPPORT_GPU

class GrLightingEffect : public GrSingleTextureEffect {
public:
    GrLightingEffect(GrTexture* texture, const SkImageFilterLight* light, SkScalar surfaceScale,
                     const SkMatrix& matrix, BoundaryMode boundaryMode, const SkIRect* srcBounds);
    ~GrLightingEffect() override;

    const SkImageFilterLight* light() const { return fLight; }
    SkScalar surfaceScale() const { return fSurfaceScale; }
    const SkMatrix& filterMatrix() const { return fFilterMatrix; }
    BoundaryMode boundaryMode() const { return fBoundaryMode; }
    const GrTextureDomain& domain() const { return fDomain; }

protected:
    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        // lighting shaders are complicated. We just throw up our hands.
        inout->mulByUnknownFourComponents();
    }

private:
    const SkImageFilterLight* fLight;
    SkScalar fSurfaceScale;
    SkMatrix fFilterMatrix;
    BoundaryMode fBoundaryMode;
    GrTextureDomain fDomain;

    typedef GrSingleTextureEffect INHERITED;
};

class GrDiffuseLightingEffect : public GrLightingEffect {
public:
    static sk_sp<GrFragmentProcessor> Make(GrTexture* texture,
                                           const SkImageFilterLight* light,
                                           SkScalar surfaceScale,
                                           const SkMatrix& matrix,
                                           SkScalar kd,
                                           BoundaryMode boundaryMode,
                                           const SkIRect* srcBounds) {
        return sk_sp<GrFragmentProcessor>(
            new GrDiffuseLightingEffect(texture, light, surfaceScale, matrix, kd, boundaryMode,
                                         srcBounds));
    }

    const char* name() const override { return "DiffuseLighting"; }

    SkScalar kd() const { return fKD; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrDiffuseLightingEffect(GrTexture* texture,
                            const SkImageFilterLight* light,
                            SkScalar surfaceScale,
                            const SkMatrix& matrix,
                            SkScalar kd,
                            BoundaryMode boundaryMode,
                            const SkIRect* srcBounds);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrLightingEffect INHERITED;
    SkScalar fKD;
};

class GrSpecularLightingEffect : public GrLightingEffect {
public:
    static sk_sp<GrFragmentProcessor> Make(GrTexture* texture,
                                           const SkImageFilterLight* light,
                                           SkScalar surfaceScale,
                                           const SkMatrix& matrix,
                                           SkScalar ks,
                                           SkScalar shininess,
                                           BoundaryMode boundaryMode,
                                           const SkIRect* srcBounds) {
        return sk_sp<GrFragmentProcessor>(
            new GrSpecularLightingEffect(texture, light, surfaceScale, matrix, ks, shininess,
                                         boundaryMode, srcBounds));
    }

    const char* name() const override { return "SpecularLighting"; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

private:
    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrSpecularLightingEffect(GrTexture* texture,
                             const SkImageFilterLight* light,
                             SkScalar surfaceScale,
                             const SkMatrix& matrix,
                             SkScalar ks,
                             SkScalar shininess,
                             BoundaryMode boundaryMode,
                             const SkIRect* srcBounds);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrLightingEffect INHERITED;
    SkScalar fKS;
    SkScalar fShininess;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLLight {
public:
    virtual ~GrGLLight() {}

    /**
     * This is called by GrGLLightingEffect::emitCode() before either of the two virtual functions
     * below. It adds a vec3f uniform visible in the FS that represents the constant light color.
     */
    void emitLightColorUniform(GrGLSLUniformHandler*);

    /**
     * These two functions are called from GrGLLightingEffect's emitCode() function.
     * emitSurfaceToLight places an expression in param out that is the vector from the surface to
     * the light. The expression will be used in the FS. emitLightColor writes an expression into
     * the FS that is the color of the light. Either function may add functions and/or uniforms to
     * the FS. The default of emitLightColor appends the name of the constant light color uniform
     * and so this function only needs to be overridden if the light color varies spatially.
     */
    virtual void emitSurfaceToLight(GrGLSLUniformHandler*,
                                    GrGLSLFPFragmentBuilder*,
                                    const char* z) = 0;
    virtual void emitLightColor(GrGLSLUniformHandler*,
                                GrGLSLFPFragmentBuilder*,
                                const char *surfaceToLight);

    // This is called from GrGLLightingEffect's setData(). Subclasses of GrGLLight must call
    // INHERITED::setData().
    virtual void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const;

protected:
    /**
     * Gets the constant light color uniform. Subclasses can use this in their emitLightColor
     * function.
     */
    UniformHandle lightColorUni() const { return fColorUni; }

private:
    UniformHandle fColorUni;

    typedef SkRefCnt INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDistantLight : public GrGLLight {
public:
    virtual ~GrGLDistantLight() {}
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(GrGLSLUniformHandler*, GrGLSLFPFragmentBuilder*, const char* z) override;

private:
    typedef GrGLLight INHERITED;
    UniformHandle fDirectionUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLPointLight : public GrGLLight {
public:
    virtual ~GrGLPointLight() {}
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(GrGLSLUniformHandler*, GrGLSLFPFragmentBuilder*, const char* z) override;

private:
    typedef GrGLLight INHERITED;
    UniformHandle fLocationUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpotLight : public GrGLLight {
public:
    virtual ~GrGLSpotLight() {}
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(GrGLSLUniformHandler*, GrGLSLFPFragmentBuilder*, const char* z) override;
    void emitLightColor(GrGLSLUniformHandler*,
                        GrGLSLFPFragmentBuilder*,
                        const char *surfaceToLight) override;

private:
    typedef GrGLLight INHERITED;

    SkString        fLightColorFunc;
    UniformHandle   fLocationUni;
    UniformHandle   fExponentUni;
    UniformHandle   fCosOuterConeAngleUni;
    UniformHandle   fCosInnerConeAngleUni;
    UniformHandle   fConeScaleUni;
    UniformHandle   fSUni;
};
#else

class GrGLLight;

#endif

};

///////////////////////////////////////////////////////////////////////////////

class SkImageFilterLight : public SkRefCnt {
public:


    enum LightType {
        kDistant_LightType,
        kPoint_LightType,
        kSpot_LightType,
    };
    virtual LightType type() const = 0;
    const SkPoint3& color() const { return fColor; }
    virtual GrGLLight* createGLLight() const = 0;
    virtual bool isEqual(const SkImageFilterLight& other) const {
        return fColor == other.fColor;
    }
    // Called to know whether the generated GrGLLight will require access to the fragment position.
    virtual bool requiresFragmentPosition() const = 0;
    virtual SkImageFilterLight* transform(const SkMatrix& matrix) const = 0;

    // Defined below SkLight's subclasses.
    void flattenLight(SkWriteBuffer& buffer) const;
    static SkImageFilterLight* UnflattenLight(SkReadBuffer& buffer);

protected:
    SkImageFilterLight(SkColor color) {
        fColor = SkPoint3::Make(SkIntToScalar(SkColorGetR(color)),
                                SkIntToScalar(SkColorGetG(color)),
                                SkIntToScalar(SkColorGetB(color)));
    }
    SkImageFilterLight(const SkPoint3& color)
      : fColor(color) {}
    SkImageFilterLight(SkReadBuffer& buffer) {
        fColor = readPoint3(buffer);
    }

    virtual void onFlattenLight(SkWriteBuffer& buffer) const = 0;


private:
    typedef SkRefCnt INHERITED;
    SkPoint3 fColor;
};

///////////////////////////////////////////////////////////////////////////////

class SkDistantLight : public SkImageFilterLight {
public:
    SkDistantLight(const SkPoint3& direction, SkColor color)
      : INHERITED(color), fDirection(direction) {
    }

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        return fDirection;
    };
    const SkPoint3& lightColor(const SkPoint3&) const { return this->color(); }
    LightType type() const override { return kDistant_LightType; }
    const SkPoint3& direction() const { return fDirection; }
    GrGLLight* createGLLight() const override {
#if SK_SUPPORT_GPU
        return new GrGLDistantLight;
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }
    bool requiresFragmentPosition() const override { return false; }

    bool isEqual(const SkImageFilterLight& other) const override {
        if (other.type() != kDistant_LightType) {
            return false;
        }

        const SkDistantLight& o = static_cast<const SkDistantLight&>(other);
        return INHERITED::isEqual(other) &&
               fDirection == o.fDirection;
    }

    SkDistantLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fDirection = readPoint3(buffer);
    }

protected:
    SkDistantLight(const SkPoint3& direction, const SkPoint3& color)
      : INHERITED(color), fDirection(direction) {
    }
    SkImageFilterLight* transform(const SkMatrix& matrix) const override {
        return new SkDistantLight(direction(), color());
    }
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        writePoint3(fDirection, buffer);
    }

private:
    SkPoint3 fDirection;

    typedef SkImageFilterLight INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkPointLight : public SkImageFilterLight {
public:
    SkPointLight(const SkPoint3& location, SkColor color)
     : INHERITED(color), fLocation(location) {}

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        SkPoint3 direction = SkPoint3::Make(fLocation.fX - SkIntToScalar(x),
                                            fLocation.fY - SkIntToScalar(y),
                                            fLocation.fZ - SkScalarMul(SkIntToScalar(z),
                                                                       surfaceScale));
        fast_normalize(&direction);
        return direction;
    };
    const SkPoint3& lightColor(const SkPoint3&) const { return this->color(); }
    LightType type() const override { return kPoint_LightType; }
    const SkPoint3& location() const { return fLocation; }
    GrGLLight* createGLLight() const override {
#if SK_SUPPORT_GPU
        return new GrGLPointLight;
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }
    bool requiresFragmentPosition() const override { return true; }
    bool isEqual(const SkImageFilterLight& other) const override {
        if (other.type() != kPoint_LightType) {
            return false;
        }
        const SkPointLight& o = static_cast<const SkPointLight&>(other);
        return INHERITED::isEqual(other) &&
               fLocation == o.fLocation;
    }
    SkImageFilterLight* transform(const SkMatrix& matrix) const override {
        SkPoint location2 = SkPoint::Make(fLocation.fX, fLocation.fY);
        matrix.mapPoints(&location2, 1);
        // Use X scale and Y scale on Z and average the result
        SkPoint locationZ = SkPoint::Make(fLocation.fZ, fLocation.fZ);
        matrix.mapVectors(&locationZ, 1);
        SkPoint3 location = SkPoint3::Make(location2.fX,
                                           location2.fY,
                                           SkScalarAve(locationZ.fX, locationZ.fY));
        return new SkPointLight(location, color());
    }

    SkPointLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
    }

protected:
    SkPointLight(const SkPoint3& location, const SkPoint3& color)
     : INHERITED(color), fLocation(location) {}
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        writePoint3(fLocation, buffer);
    }

private:
    SkPoint3 fLocation;

    typedef SkImageFilterLight INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkSpotLight : public SkImageFilterLight {
public:
    SkSpotLight(const SkPoint3& location,
                const SkPoint3& target,
                SkScalar specularExponent,
                SkScalar cutoffAngle,
                SkColor color)
     : INHERITED(color),
       fLocation(location),
       fTarget(target),
       fSpecularExponent(SkScalarPin(specularExponent, kSpecularExponentMin, kSpecularExponentMax))
    {
       fS = target - location;
       fast_normalize(&fS);
       fCosOuterConeAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
       const SkScalar antiAliasThreshold = 0.016f;
       fCosInnerConeAngle = fCosOuterConeAngle + antiAliasThreshold;
       fConeScale = SkScalarInvert(antiAliasThreshold);
    }

    SkImageFilterLight* transform(const SkMatrix& matrix) const override {
        SkPoint location2 = SkPoint::Make(fLocation.fX, fLocation.fY);
        matrix.mapPoints(&location2, 1);
        // Use X scale and Y scale on Z and average the result
        SkPoint locationZ = SkPoint::Make(fLocation.fZ, fLocation.fZ);
        matrix.mapVectors(&locationZ, 1);
        SkPoint3 location = SkPoint3::Make(location2.fX, location2.fY,
                                           SkScalarAve(locationZ.fX, locationZ.fY));
        SkPoint target2 = SkPoint::Make(fTarget.fX, fTarget.fY);
        matrix.mapPoints(&target2, 1);
        SkPoint targetZ = SkPoint::Make(fTarget.fZ, fTarget.fZ);
        matrix.mapVectors(&targetZ, 1);
        SkPoint3 target = SkPoint3::Make(target2.fX, target2.fY,
                                         SkScalarAve(targetZ.fX, targetZ.fY));
        SkPoint3 s = target - location;
        fast_normalize(&s);
        return new SkSpotLight(location,
                               target,
                               fSpecularExponent,
                               fCosOuterConeAngle,
                               fCosInnerConeAngle,
                               fConeScale,
                               s,
                               color());
    }

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        SkPoint3 direction = SkPoint3::Make(fLocation.fX - SkIntToScalar(x),
                                            fLocation.fY - SkIntToScalar(y),
                                            fLocation.fZ - SkScalarMul(SkIntToScalar(z),
                                                                       surfaceScale));
        fast_normalize(&direction);
        return direction;
    };
    SkPoint3 lightColor(const SkPoint3& surfaceToLight) const {
        SkScalar cosAngle = -surfaceToLight.dot(fS);
        SkScalar scale = 0;
        if (cosAngle >= fCosOuterConeAngle) {
            scale = SkScalarPow(cosAngle, fSpecularExponent);
            if (cosAngle < fCosInnerConeAngle) {
                scale = SkScalarMul(scale, cosAngle - fCosOuterConeAngle);
                scale *= fConeScale;
            }
        }
        return this->color().makeScale(scale);
    }
    GrGLLight* createGLLight() const override {
#if SK_SUPPORT_GPU
        return new GrGLSpotLight;
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }
    bool requiresFragmentPosition() const override { return true; }
    LightType type() const override { return kSpot_LightType; }
    const SkPoint3& location() const { return fLocation; }
    const SkPoint3& target() const { return fTarget; }
    SkScalar specularExponent() const { return fSpecularExponent; }
    SkScalar cosInnerConeAngle() const { return fCosInnerConeAngle; }
    SkScalar cosOuterConeAngle() const { return fCosOuterConeAngle; }
    SkScalar coneScale() const { return fConeScale; }
    const SkPoint3& s() const { return fS; }

    SkSpotLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
        fTarget = readPoint3(buffer);
        fSpecularExponent = buffer.readScalar();
        fCosOuterConeAngle = buffer.readScalar();
        fCosInnerConeAngle = buffer.readScalar();
        fConeScale = buffer.readScalar();
        fS = readPoint3(buffer);
        buffer.validate(SkScalarIsFinite(fSpecularExponent) &&
                        SkScalarIsFinite(fCosOuterConeAngle) &&
                        SkScalarIsFinite(fCosInnerConeAngle) &&
                        SkScalarIsFinite(fConeScale));
    }
protected:
    SkSpotLight(const SkPoint3& location,
                const SkPoint3& target,
                SkScalar specularExponent,
                SkScalar cosOuterConeAngle,
                SkScalar cosInnerConeAngle,
                SkScalar coneScale,
                const SkPoint3& s,
                const SkPoint3& color)
     : INHERITED(color),
       fLocation(location),
       fTarget(target),
       fSpecularExponent(specularExponent),
       fCosOuterConeAngle(cosOuterConeAngle),
       fCosInnerConeAngle(cosInnerConeAngle),
       fConeScale(coneScale),
       fS(s)
    {
    }
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        writePoint3(fLocation, buffer);
        writePoint3(fTarget, buffer);
        buffer.writeScalar(fSpecularExponent);
        buffer.writeScalar(fCosOuterConeAngle);
        buffer.writeScalar(fCosInnerConeAngle);
        buffer.writeScalar(fConeScale);
        writePoint3(fS, buffer);
    }

    bool isEqual(const SkImageFilterLight& other) const override {
        if (other.type() != kSpot_LightType) {
            return false;
        }

        const SkSpotLight& o = static_cast<const SkSpotLight&>(other);
        return INHERITED::isEqual(other) &&
               fLocation == o.fLocation &&
               fTarget == o.fTarget &&
               fSpecularExponent == o.fSpecularExponent &&
               fCosOuterConeAngle == o.fCosOuterConeAngle;
    }

private:
    static const SkScalar kSpecularExponentMin;
    static const SkScalar kSpecularExponentMax;

    SkPoint3 fLocation;
    SkPoint3 fTarget;
    SkScalar fSpecularExponent;
    SkScalar fCosOuterConeAngle;
    SkScalar fCosInnerConeAngle;
    SkScalar fConeScale;
    SkPoint3 fS;

    typedef SkImageFilterLight INHERITED;
};

// According to the spec, the specular term should be in the range [1, 128] :
// http://www.w3.org/TR/SVG/filters.html#feSpecularLightingSpecularExponentAttribute
const SkScalar SkSpotLight::kSpecularExponentMin = 1.0f;
const SkScalar SkSpotLight::kSpecularExponentMax = 128.0f;

///////////////////////////////////////////////////////////////////////////////

void SkImageFilterLight::flattenLight(SkWriteBuffer& buffer) const {
    // Write type first, then baseclass, then subclass.
    buffer.writeInt(this->type());
    writePoint3(fColor, buffer);
    this->onFlattenLight(buffer);
}

/*static*/ SkImageFilterLight* SkImageFilterLight::UnflattenLight(SkReadBuffer& buffer) {
    // Read type first.
    const SkImageFilterLight::LightType type = (SkImageFilterLight::LightType)buffer.readInt();
    switch (type) {
        // Each of these constructors must first call SkLight's, so we'll read the baseclass
        // then subclass, same order as flattenLight.
        case SkImageFilterLight::kDistant_LightType:
            return new SkDistantLight(buffer);
        case SkImageFilterLight::kPoint_LightType:
            return new SkPointLight(buffer);
        case SkImageFilterLight::kSpot_LightType:
            return new SkSpotLight(buffer);
        default:
            SkDEBUGFAIL("Unknown LightType.");
            buffer.validate(false);
            return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////

SkLightingImageFilter::SkLightingImageFilter(sk_sp<SkImageFilterLight> light,
                                             SkScalar surfaceScale,
                                             sk_sp<SkImageFilter> input, const CropRect* cropRect)
    : INHERITED(&input, 1, cropRect)
    , fLight(std::move(light))
    , fSurfaceScale(surfaceScale / 255) {
}

SkLightingImageFilter::~SkLightingImageFilter() {}

sk_sp<SkImageFilter> SkLightingImageFilter::MakeDistantLitDiffuse(const SkPoint3& direction,
                                                                  SkColor lightColor,
                                                                  SkScalar surfaceScale,
                                                                  SkScalar kd,
                                                                  sk_sp<SkImageFilter> input,
                                                                  const CropRect* cropRect) {
    sk_sp<SkImageFilterLight> light(new SkDistantLight(direction, lightColor));
    return SkDiffuseLightingImageFilter::Make(std::move(light), surfaceScale, kd, 
                                              std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkLightingImageFilter::MakePointLitDiffuse(const SkPoint3& location,
                                                                SkColor lightColor,
                                                                SkScalar surfaceScale,
                                                                SkScalar kd,
                                                                sk_sp<SkImageFilter> input,
                                                                const CropRect* cropRect) {
    sk_sp<SkImageFilterLight> light(new SkPointLight(location, lightColor));
    return SkDiffuseLightingImageFilter::Make(std::move(light), surfaceScale, kd,
                                              std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkLightingImageFilter::MakeSpotLitDiffuse(const SkPoint3& location,
                                                               const SkPoint3& target,
                                                               SkScalar specularExponent,
                                                               SkScalar cutoffAngle,
                                                               SkColor lightColor,
                                                               SkScalar surfaceScale,
                                                               SkScalar kd,
                                                               sk_sp<SkImageFilter> input,
                                                               const CropRect* cropRect) {
    sk_sp<SkImageFilterLight> light(
            new SkSpotLight(location, target, specularExponent, cutoffAngle, lightColor));
    return SkDiffuseLightingImageFilter::Make(std::move(light), surfaceScale, kd,
                                              std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkLightingImageFilter::MakeDistantLitSpecular(const SkPoint3& direction,
                                                                   SkColor lightColor,
                                                                   SkScalar surfaceScale,
                                                                   SkScalar ks,
                                                                   SkScalar shine,
                                                                   sk_sp<SkImageFilter> input,
                                                                   const CropRect* cropRect) {
    sk_sp<SkImageFilterLight> light(new SkDistantLight(direction, lightColor));
    return SkSpecularLightingImageFilter::Make(std::move(light), surfaceScale, ks, shine,
                                               std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkLightingImageFilter::MakePointLitSpecular(const SkPoint3& location,
                                                                 SkColor lightColor,
                                                                 SkScalar surfaceScale,
                                                                 SkScalar ks,
                                                                 SkScalar shine,
                                                                 sk_sp<SkImageFilter> input,
                                                                 const CropRect* cropRect) {
    sk_sp<SkImageFilterLight> light(new SkPointLight(location, lightColor));
    return SkSpecularLightingImageFilter::Make(std::move(light), surfaceScale, ks, shine,
                                               std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkLightingImageFilter::MakeSpotLitSpecular(const SkPoint3& location,
                                                                const SkPoint3& target,
                                                                SkScalar specularExponent,
                                                                SkScalar cutoffAngle,
                                                                SkColor lightColor,
                                                                SkScalar surfaceScale,
                                                                SkScalar ks,
                                                                SkScalar shine,
                                                                sk_sp<SkImageFilter> input,
                                                                const CropRect* cropRect) {
    sk_sp<SkImageFilterLight> light(
            new SkSpotLight(location, target, specularExponent, cutoffAngle, lightColor));
    return SkSpecularLightingImageFilter::Make(std::move(light), surfaceScale, ks, shine,
                                               std::move(input), cropRect);
}

void SkLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    fLight->flattenLight(buffer);
    buffer.writeScalar(fSurfaceScale * 255);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkDiffuseLightingImageFilter::Make(sk_sp<SkImageFilterLight> light,
                                                        SkScalar surfaceScale,
                                                        SkScalar kd,
                                                        sk_sp<SkImageFilter> input,
                                                        const CropRect* cropRect) {
    if (!light) {
        return nullptr;
    }
    if (!SkScalarIsFinite(surfaceScale) || !SkScalarIsFinite(kd)) {
        return nullptr;
    }
    // According to the spec, kd can be any non-negative number :
    // http://www.w3.org/TR/SVG/filters.html#feDiffuseLightingElement
    if (kd < 0) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkDiffuseLightingImageFilter(std::move(light), surfaceScale, 
                                                                 kd, std::move(input), cropRect));
}

SkDiffuseLightingImageFilter::SkDiffuseLightingImageFilter(sk_sp<SkImageFilterLight> light,
                                                           SkScalar surfaceScale,
                                                           SkScalar kd,
                                                           sk_sp<SkImageFilter> input,
                                                           const CropRect* cropRect)
    : INHERITED(std::move(light), surfaceScale, std::move(input), cropRect)
    , fKD(kd) {
}

sk_sp<SkFlattenable> SkDiffuseLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    sk_sp<SkImageFilterLight> light(SkImageFilterLight::UnflattenLight(buffer));
    SkScalar surfaceScale = buffer.readScalar();
    SkScalar kd = buffer.readScalar();
    return Make(std::move(light), surfaceScale, kd, common.getInput(0), &common.cropRect());
}

void SkDiffuseLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKD);
}

sk_sp<SkSpecialImage> SkDiffuseLightingImageFilter::onFilterImage(SkSpecialImage* source,
                                                                  const Context& ctx,
                                                                  SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    const SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                  input->width(), input->height());
    SkIRect bounds;
    if (!this->applyCropRect(ctx, inputBounds, &bounds)) {
        return nullptr;
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    bounds.offset(-inputOffset);

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-offset->fX), SkIntToScalar(-offset->fY));

        return this->filterImageGPU(source, input.get(), bounds, matrix);
    }
#endif

    if (bounds.width() < 2 || bounds.height() < 2) {
        return nullptr;
    }

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkAutoLockPixels alp(inputBM);
    if (!inputBM.getPixels()) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(bounds.width(), bounds.height());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    SkAutoLockPixels dstLock(dst);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-inputOffset.x()), SkIntToScalar(-inputOffset.y()));

    sk_sp<SkImageFilterLight> transformedLight(light()->transform(matrix));

    DiffuseLightingType lightingType(fKD);
    switch (transformedLight->type()) {
        case SkImageFilterLight::kDistant_LightType:
            lightBitmap<DiffuseLightingType, SkDistantLight>(lightingType,
                                                             transformedLight.get(),
                                                             inputBM,
                                                             &dst,
                                                             surfaceScale(),
                                                             bounds);
            break;
        case SkImageFilterLight::kPoint_LightType:
            lightBitmap<DiffuseLightingType, SkPointLight>(lightingType,
                                                           transformedLight.get(),
                                                           inputBM,
                                                           &dst,
                                                           surfaceScale(),
                                                           bounds);
            break;
        case SkImageFilterLight::kSpot_LightType:
            lightBitmap<DiffuseLightingType, SkSpotLight>(lightingType,
                                                          transformedLight.get(),
                                                          inputBM,
                                                          &dst,
                                                          surfaceScale(),
                                                          bounds);
            break;
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst);
}

#ifndef SK_IGNORE_TO_STRING
void SkDiffuseLightingImageFilter::toString(SkString* str) const {
    str->appendf("SkDiffuseLightingImageFilter: (");
    str->appendf("kD: %f\n", fKD);
    str->append(")");
}
#endif

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkDiffuseLightingImageFilter::makeFragmentProcessor(
                                                   GrTexture* texture,
                                                   const SkMatrix& matrix,
                                                   const SkIRect* srcBounds,
                                                   BoundaryMode boundaryMode) const {
    SkScalar scale = SkScalarMul(this->surfaceScale(), SkIntToScalar(255));
    return GrDiffuseLightingEffect::Make(texture, this->light(), scale, matrix, this->kd(),
                                         boundaryMode, srcBounds);
}
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkSpecularLightingImageFilter::Make(sk_sp<SkImageFilterLight> light,
                                                         SkScalar surfaceScale,
                                                         SkScalar ks,
                                                         SkScalar shininess,
                                                         sk_sp<SkImageFilter> input,
                                                         const CropRect* cropRect) {
    if (!light) {
        return nullptr;
    }
    if (!SkScalarIsFinite(surfaceScale) || !SkScalarIsFinite(ks) || !SkScalarIsFinite(shininess)) {
        return nullptr;
    }
    // According to the spec, ks can be any non-negative number :
    // http://www.w3.org/TR/SVG/filters.html#feSpecularLightingElement
    if (ks < 0) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkSpecularLightingImageFilter(std::move(light), surfaceScale,
                                                                  ks, shininess,
                                                                  std::move(input), cropRect));
}

SkSpecularLightingImageFilter::SkSpecularLightingImageFilter(sk_sp<SkImageFilterLight> light,
                                                             SkScalar surfaceScale,
                                                             SkScalar ks,
                                                             SkScalar shininess,
                                                             sk_sp<SkImageFilter> input,
                                                             const CropRect* cropRect)
    : INHERITED(std::move(light), surfaceScale, std::move(input), cropRect)
    , fKS(ks)
    , fShininess(shininess) {
}

sk_sp<SkFlattenable> SkSpecularLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    sk_sp<SkImageFilterLight> light(SkImageFilterLight::UnflattenLight(buffer));
    SkScalar surfaceScale = buffer.readScalar();
    SkScalar ks = buffer.readScalar();
    SkScalar shine = buffer.readScalar();
    return Make(std::move(light), surfaceScale, ks, shine, common.getInput(0),
                &common.cropRect());
}

void SkSpecularLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKS);
    buffer.writeScalar(fShininess);
}

sk_sp<SkSpecialImage> SkSpecularLightingImageFilter::onFilterImage(SkSpecialImage* source,
                                                                   const Context& ctx,
                                                                   SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    const SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                  input->width(), input->height());
    SkIRect bounds;
    if (!this->applyCropRect(ctx, inputBounds, &bounds)) {
        return nullptr;
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    bounds.offset(-inputOffset);

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-offset->fX), SkIntToScalar(-offset->fY));

        return this->filterImageGPU(source, input.get(), bounds, matrix);
    }
#endif

    if (bounds.width() < 2 || bounds.height() < 2) {
        return nullptr;
    }

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkAutoLockPixels alp(inputBM);
    if (!inputBM.getPixels()) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(bounds.width(), bounds.height());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    SkAutoLockPixels dstLock(dst);

    SpecularLightingType lightingType(fKS, fShininess);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-inputOffset.x()), SkIntToScalar(-inputOffset.y()));

    sk_sp<SkImageFilterLight> transformedLight(light()->transform(matrix));

    switch (transformedLight->type()) {
        case SkImageFilterLight::kDistant_LightType:
            lightBitmap<SpecularLightingType, SkDistantLight>(lightingType,
                                                              transformedLight.get(),
                                                              inputBM,
                                                              &dst,
                                                              surfaceScale(),
                                                              bounds);
            break;
        case SkImageFilterLight::kPoint_LightType:
            lightBitmap<SpecularLightingType, SkPointLight>(lightingType,
                                                            transformedLight.get(),
                                                            inputBM,
                                                            &dst,
                                                            surfaceScale(),
                                                            bounds);
            break;
        case SkImageFilterLight::kSpot_LightType:
            lightBitmap<SpecularLightingType, SkSpotLight>(lightingType,
                                                           transformedLight.get(),
                                                           inputBM,
                                                           &dst,
                                                           surfaceScale(),
                                                           bounds);
            break;
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()), dst);
}

#ifndef SK_IGNORE_TO_STRING
void SkSpecularLightingImageFilter::toString(SkString* str) const {
    str->appendf("SkSpecularLightingImageFilter: (");
    str->appendf("kS: %f shininess: %f", fKS, fShininess);
    str->append(")");
}
#endif

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkSpecularLightingImageFilter::makeFragmentProcessor(
                                                    GrTexture* texture,
                                                    const SkMatrix& matrix,
                                                    const SkIRect* srcBounds,
                                                    BoundaryMode boundaryMode) const {
    SkScalar scale = SkScalarMul(this->surfaceScale(), SkIntToScalar(255));
    return GrSpecularLightingEffect::Make(texture, this->light(), scale, matrix, this->ks(),
                                          this->shininess(), boundaryMode, srcBounds);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

namespace {
SkPoint3 random_point3(SkRandom* random) {
    return SkPoint3::Make(SkScalarToFloat(random->nextSScalar1()),
                          SkScalarToFloat(random->nextSScalar1()),
                          SkScalarToFloat(random->nextSScalar1()));
}

SkImageFilterLight* create_random_light(SkRandom* random) {
    int type = random->nextULessThan(3);
    switch (type) {
        case 0: {
            return new SkDistantLight(random_point3(random), random->nextU());
        }
        case 1: {
            return new SkPointLight(random_point3(random), random->nextU());
        }
        case 2: {
            return new SkSpotLight(random_point3(random), random_point3(random),
                                   random->nextUScalar1(), random->nextUScalar1(), random->nextU());
        }
        default:
            SkFAIL("Unexpected value.");
            return nullptr;
    }
}

SkString emitNormalFunc(BoundaryMode mode,
                        const char* pointToNormalName,
                        const char* sobelFuncName) {
    SkString result;
    switch (mode) {
    case kTopLeft_BoundaryMode:
        result.printf("\treturn %s(%s(0.0, 0.0, m[4], m[5], m[7], m[8], %g),\n"
                      "\t          %s(0.0, 0.0, m[4], m[7], m[5], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kTop_BoundaryMode:
        result.printf("\treturn %s(%s(0.0, 0.0, m[3], m[5], m[6], m[8], %g),\n"
                      "\t          %s(0.0, 0.0, m[4], m[7], m[5], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneThird,
                                         sobelFuncName, gOneHalf);
        break;
    case kTopRight_BoundaryMode:
        result.printf("\treturn %s(%s( 0.0,  0.0, m[3], m[4], m[6], m[7], %g),\n"
                      "\t          %s(m[3], m[6], m[4], m[7],  0.0,  0.0, %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kLeft_BoundaryMode:
        result.printf("\treturn %s(%s(m[1], m[2], m[4], m[5], m[7], m[8], %g),\n"
                      "\t          %s( 0.0,  0.0, m[1], m[7], m[2], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneHalf,
                                         sobelFuncName, gOneThird);
        break;
    case kInterior_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[2], m[3], m[5], m[6], m[8], %g),\n"
                      "\t          %s(m[0], m[6], m[1], m[7], m[2], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneQuarter,
                                         sobelFuncName, gOneQuarter);
        break;
    case kRight_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[1], m[3], m[4], m[6], m[7], %g),\n"
                      "\t          %s(m[0], m[6], m[1], m[7],  0.0,  0.0, %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneHalf,
                                         sobelFuncName, gOneThird);
        break;
    case kBottomLeft_BoundaryMode:
        result.printf("\treturn %s(%s(m[1], m[2], m[4], m[5],  0.0,  0.0, %g),\n"
                      "\t          %s( 0.0,  0.0, m[1], m[4], m[2], m[5], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kBottom_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[2], m[3], m[5],  0.0,  0.0, %g),\n"
                      "\t          %s(m[0], m[3], m[1], m[4], m[2], m[5], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneThird,
                                         sobelFuncName, gOneHalf);
        break;
    case kBottomRight_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[1], m[3], m[4],  0.0,  0.0, %g),\n"
                      "\t          %s(m[0], m[3], m[1], m[4],  0.0,  0.0, %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    default:
        SkASSERT(false);
        break;
    }
    return result;
}

}

class GrGLLightingEffect  : public GrGLSLFragmentProcessor {
public:
    GrGLLightingEffect() : fLight(nullptr) { }
    virtual ~GrGLLightingEffect() { delete fLight; }

    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b);

protected:
    /**
     * Subclasses of GrGLLightingEffect must call INHERITED::onSetData();
     */
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    virtual void emitLightFunc(GrGLSLUniformHandler*,
                               GrGLSLFPFragmentBuilder*,
                               SkString* funcName) = 0;

private:
    typedef GrGLSLFragmentProcessor INHERITED;

    UniformHandle              fImageIncrementUni;
    UniformHandle              fSurfaceScaleUni;
    GrTextureDomain::GLDomain  fDomain;
    GrGLLight*                 fLight;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDiffuseLightingEffect  : public GrGLLightingEffect {
public:
    void emitLightFunc(GrGLSLUniformHandler*, GrGLSLFPFragmentBuilder*, SkString* funcName) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    typedef GrGLLightingEffect INHERITED;

    UniformHandle   fKDUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpecularLightingEffect  : public GrGLLightingEffect {
public:
    void emitLightFunc(GrGLSLUniformHandler*, GrGLSLFPFragmentBuilder*, SkString* funcName) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    typedef GrGLLightingEffect INHERITED;

    UniformHandle   fKSUni;
    UniformHandle   fShininessUni;
};

///////////////////////////////////////////////////////////////////////////////

namespace {

GrTextureDomain create_domain(GrTexture* texture, const SkIRect* srcBounds,
                              GrTextureDomain::Mode mode) {
    if (srcBounds) {
        SkRect texelDomain = GrTextureDomain::MakeTexelDomainForMode(texture, *srcBounds, mode);
        return GrTextureDomain(texelDomain, mode);
    } else {
        return GrTextureDomain(SkRect::MakeEmpty(), GrTextureDomain::kIgnore_Mode);
    }
}

};

GrLightingEffect::GrLightingEffect(GrTexture* texture,
                                   const SkImageFilterLight* light,
                                   SkScalar surfaceScale,
                                   const SkMatrix& matrix,
                                   BoundaryMode boundaryMode,
                                   const SkIRect* srcBounds)
    : INHERITED(texture, GrCoordTransform::MakeDivByTextureWHMatrix(texture))
    , fLight(light)
    , fSurfaceScale(surfaceScale)
    , fFilterMatrix(matrix)
    , fBoundaryMode(boundaryMode)
    , fDomain(create_domain(texture, srcBounds, GrTextureDomain::kDecal_Mode)) {
    fLight->ref();
    if (light->requiresFragmentPosition()) {
        this->setWillReadFragmentPosition();
    }
}

GrLightingEffect::~GrLightingEffect() {
    fLight->unref();
}

bool GrLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrLightingEffect& s = sBase.cast<GrLightingEffect>();
    return fLight->isEqual(*s.fLight) &&
           fSurfaceScale == s.fSurfaceScale &&
           fBoundaryMode == s.fBoundaryMode;
}

///////////////////////////////////////////////////////////////////////////////

GrDiffuseLightingEffect::GrDiffuseLightingEffect(GrTexture* texture,
                                                 const SkImageFilterLight* light,
                                                 SkScalar surfaceScale,
                                                 const SkMatrix& matrix,
                                                 SkScalar kd,
                                                 BoundaryMode boundaryMode,
                                                 const SkIRect* srcBounds)
    : INHERITED(texture, light, surfaceScale, matrix, boundaryMode, srcBounds), fKD(kd) {
    this->initClassID<GrDiffuseLightingEffect>();
}

bool GrDiffuseLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrDiffuseLightingEffect& s = sBase.cast<GrDiffuseLightingEffect>();
    return INHERITED::onIsEqual(sBase) && this->kd() == s.kd();
}

void GrDiffuseLightingEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                    GrProcessorKeyBuilder* b) const {
    GrGLDiffuseLightingEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrDiffuseLightingEffect::onCreateGLSLInstance() const {
    return new GrGLDiffuseLightingEffect;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDiffuseLightingEffect);

sk_sp<GrFragmentProcessor> GrDiffuseLightingEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    GrTexture* tex = d->fTextures[texIdx];
    SkScalar surfaceScale = d->fRandom->nextSScalar1();
    SkScalar kd = d->fRandom->nextUScalar1();
    SkAutoTUnref<SkImageFilterLight> light(create_random_light(d->fRandom));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = d->fRandom->nextUScalar1();
    }
    SkIRect srcBounds = SkIRect::MakeXYWH(d->fRandom->nextRangeU(0, tex->width()),
                                          d->fRandom->nextRangeU(0, tex->height()),
                                          d->fRandom->nextRangeU(0, tex->width()),
                                          d->fRandom->nextRangeU(0, tex->height()));
    BoundaryMode mode = static_cast<BoundaryMode>(d->fRandom->nextU() % kBoundaryModeCount);
    return GrDiffuseLightingEffect::Make(tex, light, surfaceScale, matrix, kd, mode, &srcBounds);
}


///////////////////////////////////////////////////////////////////////////////

void GrGLLightingEffect::emitCode(EmitArgs& args) {
    const GrLightingEffect& le = args.fFp.cast<GrLightingEffect>();
    if (!fLight) {
        fLight = le.light()->createGLLight();
    }

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fImageIncrementUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                    "ImageIncrement");
    fSurfaceScaleUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                  kFloat_GrSLType, kDefault_GrSLPrecision,
                                                  "SurfaceScale");
    fLight->emitLightColorUniform(uniformHandler);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString lightFunc;
    this->emitLightFunc(uniformHandler, fragBuilder, &lightFunc);
    static const GrGLSLShaderVar gSobelArgs[] =  {
        GrGLSLShaderVar("a", kFloat_GrSLType),
        GrGLSLShaderVar("b", kFloat_GrSLType),
        GrGLSLShaderVar("c", kFloat_GrSLType),
        GrGLSLShaderVar("d", kFloat_GrSLType),
        GrGLSLShaderVar("e", kFloat_GrSLType),
        GrGLSLShaderVar("f", kFloat_GrSLType),
        GrGLSLShaderVar("scale", kFloat_GrSLType),
    };
    SkString sobelFuncName;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);

    fragBuilder->emitFunction(kFloat_GrSLType,
                              "sobel",
                              SK_ARRAY_COUNT(gSobelArgs),
                              gSobelArgs,
                              "\treturn (-a + b - 2.0 * c + 2.0 * d -e + f) * scale;\n",
                              &sobelFuncName);
    static const GrGLSLShaderVar gPointToNormalArgs[] =  {
        GrGLSLShaderVar("x", kFloat_GrSLType),
        GrGLSLShaderVar("y", kFloat_GrSLType),
        GrGLSLShaderVar("scale", kFloat_GrSLType),
    };
    SkString pointToNormalName;
    fragBuilder->emitFunction(kVec3f_GrSLType,
                              "pointToNormal",
                              SK_ARRAY_COUNT(gPointToNormalArgs),
                              gPointToNormalArgs,
                              "\treturn normalize(vec3(-x * scale, -y * scale, 1));\n",
                              &pointToNormalName);

    static const GrGLSLShaderVar gInteriorNormalArgs[] =  {
        GrGLSLShaderVar("m", kFloat_GrSLType, 9),
        GrGLSLShaderVar("surfaceScale", kFloat_GrSLType),
    };
    SkString normalBody = emitNormalFunc(le.boundaryMode(),
                                         pointToNormalName.c_str(),
                                         sobelFuncName.c_str());
    SkString normalName;
    fragBuilder->emitFunction(kVec3f_GrSLType,
                              "normal",
                              SK_ARRAY_COUNT(gInteriorNormalArgs),
                              gInteriorNormalArgs,
                              normalBody.c_str(),
                              &normalName);

    fragBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    fragBuilder->codeAppend("\t\tfloat m[9];\n");

    const char* imgInc = uniformHandler->getUniformCStr(fImageIncrementUni);
    const char* surfScale = uniformHandler->getUniformCStr(fSurfaceScaleUni);

    int index = 0;
    for (int dy = 1; dy >= -1; dy--) {
        for (int dx = -1; dx <= 1; dx++) {
            SkString texCoords;
            texCoords.appendf("coord + vec2(%d, %d) * %s", dx, dy, imgInc);
            SkString temp;
            temp.appendf("temp%d", index);
            fragBuilder->codeAppendf("vec4 %s;", temp.c_str());
            fDomain.sampleTexture(fragBuilder,
                                  args.fUniformHandler,
                                  args.fGLSLCaps,
                                  le.domain(),
                                  temp.c_str(),
                                  texCoords,
                                  args.fTexSamplers[0]);
            fragBuilder->codeAppendf("m[%d] = %s.a;", index, temp.c_str());
            index++;
        }
    }
    fragBuilder->codeAppend("\t\tvec3 surfaceToLight = ");
    SkString arg;
    arg.appendf("%s * m[4]", surfScale);
    fLight->emitSurfaceToLight(uniformHandler, fragBuilder, arg.c_str());
    fragBuilder->codeAppend(";\n");
    fragBuilder->codeAppendf("\t\t%s = %s(%s(m, %s), surfaceToLight, ",
                             args.fOutputColor, lightFunc.c_str(), normalName.c_str(), surfScale);
    fLight->emitLightColor(uniformHandler, fragBuilder, "surfaceToLight");
    fragBuilder->codeAppend(");\n");
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    fragBuilder->codeAppend(modulate.c_str());
}

void GrGLLightingEffect::GenKey(const GrProcessor& proc,
                                const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) {
    const GrLightingEffect& lighting = proc.cast<GrLightingEffect>();
    b->add32(lighting.boundaryMode() << 2 | lighting.light()->type());
    b->add32(GrTextureDomain::GLDomain::DomainKey(lighting.domain()));
}

void GrGLLightingEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                   const GrProcessor& proc) {
    const GrLightingEffect& lighting = proc.cast<GrLightingEffect>();
    if (!fLight) {
        fLight = lighting.light()->createGLLight();
    }

    GrTexture* texture = lighting.texture(0);
    float ySign = texture->origin() == kTopLeft_GrSurfaceOrigin ? -1.0f : 1.0f;
    pdman.set2f(fImageIncrementUni, 1.0f / texture->width(), ySign / texture->height());
    pdman.set1f(fSurfaceScaleUni, lighting.surfaceScale());
    SkAutoTUnref<SkImageFilterLight> transformedLight(
                                            lighting.light()->transform(lighting.filterMatrix()));
    fDomain.setData(pdman, lighting.domain(), texture->origin());
    fLight->setData(pdman, transformedLight);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

void GrGLDiffuseLightingEffect::emitLightFunc(GrGLSLUniformHandler* uniformHandler,
                                              GrGLSLFPFragmentBuilder* fragBuilder,
                                              SkString* funcName) {
    const char* kd;
    fKDUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                 kFloat_GrSLType, kDefault_GrSLPrecision,
                                 "KD", &kd);

    static const GrGLSLShaderVar gLightArgs[] = {
        GrGLSLShaderVar("normal", kVec3f_GrSLType),
        GrGLSLShaderVar("surfaceToLight", kVec3f_GrSLType),
        GrGLSLShaderVar("lightColor", kVec3f_GrSLType)
    };
    SkString lightBody;
    lightBody.appendf("\tfloat colorScale = %s * dot(normal, surfaceToLight);\n", kd);
    lightBody.appendf("\treturn vec4(lightColor * clamp(colorScale, 0.0, 1.0), 1.0);\n");
    fragBuilder->emitFunction(kVec4f_GrSLType,
                              "light",
                              SK_ARRAY_COUNT(gLightArgs),
                              gLightArgs,
                              lightBody.c_str(),
                              funcName);
}

void GrGLDiffuseLightingEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                          const GrProcessor& proc) {
    INHERITED::onSetData(pdman, proc);
    const GrDiffuseLightingEffect& diffuse = proc.cast<GrDiffuseLightingEffect>();
    pdman.set1f(fKDUni, diffuse.kd());
}

///////////////////////////////////////////////////////////////////////////////

GrSpecularLightingEffect::GrSpecularLightingEffect(GrTexture* texture,
                                                   const SkImageFilterLight* light,
                                                   SkScalar surfaceScale,
                                                   const SkMatrix& matrix,
                                                   SkScalar ks,
                                                   SkScalar shininess,
                                                   BoundaryMode boundaryMode,
                                                   const SkIRect* srcBounds)
    : INHERITED(texture, light, surfaceScale, matrix, boundaryMode, srcBounds)
    , fKS(ks)
    , fShininess(shininess) {
    this->initClassID<GrSpecularLightingEffect>();
}

bool GrSpecularLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrSpecularLightingEffect& s = sBase.cast<GrSpecularLightingEffect>();
    return INHERITED::onIsEqual(sBase) &&
           this->ks() == s.ks() &&
           this->shininess() == s.shininess();
}

void GrSpecularLightingEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    GrGLSpecularLightingEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrSpecularLightingEffect::onCreateGLSLInstance() const {
    return new GrGLSpecularLightingEffect;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSpecularLightingEffect);

sk_sp<GrFragmentProcessor> GrSpecularLightingEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    GrTexture* tex = d->fTextures[texIdx];
    SkScalar surfaceScale = d->fRandom->nextSScalar1();
    SkScalar ks = d->fRandom->nextUScalar1();
    SkScalar shininess = d->fRandom->nextUScalar1();
    SkAutoTUnref<SkImageFilterLight> light(create_random_light(d->fRandom));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = d->fRandom->nextUScalar1();
    }
    BoundaryMode mode = static_cast<BoundaryMode>(d->fRandom->nextU() % kBoundaryModeCount);
    SkIRect srcBounds = SkIRect::MakeXYWH(d->fRandom->nextRangeU(0, tex->width()),
                                          d->fRandom->nextRangeU(0, tex->height()),
                                          d->fRandom->nextRangeU(0, tex->width()),
                                          d->fRandom->nextRangeU(0, tex->height()));
    return GrSpecularLightingEffect::Make(d->fTextures[GrProcessorUnitTest::kAlphaTextureIdx],
                                          light, surfaceScale, matrix, ks, shininess, mode,
                                          &srcBounds);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLSpecularLightingEffect::emitLightFunc(GrGLSLUniformHandler* uniformHandler,
                                               GrGLSLFPFragmentBuilder* fragBuilder,
                                               SkString* funcName) {
    const char* ks;
    const char* shininess;

    fKSUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                        kFloat_GrSLType, kDefault_GrSLPrecision, "KS", &ks);
    fShininessUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                               kFloat_GrSLType,
                                               kDefault_GrSLPrecision,
                                               "Shininess",
                                               &shininess);

    static const GrGLSLShaderVar gLightArgs[] = {
        GrGLSLShaderVar("normal", kVec3f_GrSLType),
        GrGLSLShaderVar("surfaceToLight", kVec3f_GrSLType),
        GrGLSLShaderVar("lightColor", kVec3f_GrSLType)
    };
    SkString lightBody;
    lightBody.appendf("\tvec3 halfDir = vec3(normalize(surfaceToLight + vec3(0, 0, 1)));\n");
    lightBody.appendf("\tfloat colorScale = %s * pow(dot(normal, halfDir), %s);\n", ks, shininess);
    lightBody.appendf("\tvec3 color = lightColor * clamp(colorScale, 0.0, 1.0);\n");
    lightBody.appendf("\treturn vec4(color, max(max(color.r, color.g), color.b));\n");
    fragBuilder->emitFunction(kVec4f_GrSLType,
                              "light",
                              SK_ARRAY_COUNT(gLightArgs),
                              gLightArgs,
                              lightBody.c_str(),
                              funcName);
}

void GrGLSpecularLightingEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                           const GrProcessor& effect) {
    INHERITED::onSetData(pdman, effect);
    const GrSpecularLightingEffect& spec = effect.cast<GrSpecularLightingEffect>();
    pdman.set1f(fKSUni, spec.ks());
    pdman.set1f(fShininessUni, spec.shininess());
}

///////////////////////////////////////////////////////////////////////////////
void GrGLLight::emitLightColorUniform(GrGLSLUniformHandler* uniformHandler) {
    fColorUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                           kVec3f_GrSLType, kDefault_GrSLPrecision,
                                           "LightColor");
}

void GrGLLight::emitLightColor(GrGLSLUniformHandler* uniformHandler,
                               GrGLSLFPFragmentBuilder* fragBuilder,
                               const char *surfaceToLight) {
    fragBuilder->codeAppend(uniformHandler->getUniformCStr(this->lightColorUni()));
}

void GrGLLight::setData(const GrGLSLProgramDataManager& pdman,
                        const SkImageFilterLight* light) const {
    setUniformPoint3(pdman, fColorUni,
                     light->color().makeScale(SkScalarInvert(SkIntToScalar(255))));
}

///////////////////////////////////////////////////////////////////////////////

void GrGLDistantLight::setData(const GrGLSLProgramDataManager& pdman,
                               const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kDistant_LightType);
    const SkDistantLight* distantLight = static_cast<const SkDistantLight*>(light);
    setUniformNormal3(pdman, fDirectionUni, distantLight->direction());
}

void GrGLDistantLight::emitSurfaceToLight(GrGLSLUniformHandler* uniformHandler,
                                          GrGLSLFPFragmentBuilder* fragBuilder,
                                          const char* z) {
    const char* dir;
    fDirectionUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                               kVec3f_GrSLType, kDefault_GrSLPrecision,
                                               "LightDirection", &dir);
    fragBuilder->codeAppend(dir);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLPointLight::setData(const GrGLSLProgramDataManager& pdman,
                             const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kPoint_LightType);
    const SkPointLight* pointLight = static_cast<const SkPointLight*>(light);
    setUniformPoint3(pdman, fLocationUni, pointLight->location());
}

void GrGLPointLight::emitSurfaceToLight(GrGLSLUniformHandler* uniformHandler,
                                        GrGLSLFPFragmentBuilder* fragBuilder,
                                        const char* z) {
    const char* loc;
    fLocationUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                              kVec3f_GrSLType, kDefault_GrSLPrecision,
                                              "LightLocation", &loc);
    fragBuilder->codeAppendf("normalize(%s - vec3(%s.xy, %s))",
                             loc, fragBuilder->fragmentPosition(), z);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLSpotLight::setData(const GrGLSLProgramDataManager& pdman,
                            const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kSpot_LightType);
    const SkSpotLight* spotLight = static_cast<const SkSpotLight *>(light);
    setUniformPoint3(pdman, fLocationUni, spotLight->location());
    pdman.set1f(fExponentUni, spotLight->specularExponent());
    pdman.set1f(fCosInnerConeAngleUni, spotLight->cosInnerConeAngle());
    pdman.set1f(fCosOuterConeAngleUni, spotLight->cosOuterConeAngle());
    pdman.set1f(fConeScaleUni, spotLight->coneScale());
    setUniformNormal3(pdman, fSUni, spotLight->s());
}

void GrGLSpotLight::emitSurfaceToLight(GrGLSLUniformHandler* uniformHandler,
                                       GrGLSLFPFragmentBuilder* fragBuilder,
                                       const char* z) {
    const char* location;
    fLocationUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                              kVec3f_GrSLType, kDefault_GrSLPrecision,
                                              "LightLocation", &location);

    fragBuilder->codeAppendf("normalize(%s - vec3(%s.xy, %s))",
                             location, fragBuilder->fragmentPosition(), z);
}

void GrGLSpotLight::emitLightColor(GrGLSLUniformHandler* uniformHandler,
                                   GrGLSLFPFragmentBuilder* fragBuilder,
                                   const char *surfaceToLight) {

    const char* color = uniformHandler->getUniformCStr(this->lightColorUni()); // created by parent class.

    const char* exponent;
    const char* cosInner;
    const char* cosOuter;
    const char* coneScale;
    const char* s;
    fExponentUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                              kFloat_GrSLType, kDefault_GrSLPrecision,
                                              "Exponent", &exponent);
    fCosInnerConeAngleUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kFloat_GrSLType, kDefault_GrSLPrecision,
                                                       "CosInnerConeAngle", &cosInner);
    fCosOuterConeAngleUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kFloat_GrSLType, kDefault_GrSLPrecision,
                                                       "CosOuterConeAngle", &cosOuter);
    fConeScaleUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                               kFloat_GrSLType, kDefault_GrSLPrecision,
                                               "ConeScale", &coneScale);
    fSUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                       kVec3f_GrSLType, kDefault_GrSLPrecision, "S", &s);

    static const GrGLSLShaderVar gLightColorArgs[] = {
        GrGLSLShaderVar("surfaceToLight", kVec3f_GrSLType)
    };
    SkString lightColorBody;
    lightColorBody.appendf("\tfloat cosAngle = -dot(surfaceToLight, %s);\n", s);
    lightColorBody.appendf("\tif (cosAngle < %s) {\n", cosOuter);
    lightColorBody.appendf("\t\treturn vec3(0);\n");
    lightColorBody.appendf("\t}\n");
    lightColorBody.appendf("\tfloat scale = pow(cosAngle, %s);\n", exponent);
    lightColorBody.appendf("\tif (cosAngle < %s) {\n", cosInner);
    lightColorBody.appendf("\t\treturn %s * scale * (cosAngle - %s) * %s;\n",
                           color, cosOuter, coneScale);
    lightColorBody.appendf("\t}\n");
    lightColorBody.appendf("\treturn %s;\n", color);
    fragBuilder->emitFunction(kVec3f_GrSLType,
                              "lightColor",
                              SK_ARRAY_COUNT(gLightColorArgs),
                              gLightColorArgs,
                              lightColorBody.c_str(),
                              &fLightColorFunc);

    fragBuilder->codeAppendf("%s(%s)", fLightColorFunc.c_str(), surfaceToLight);
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiffuseLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpecularLightingImageFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

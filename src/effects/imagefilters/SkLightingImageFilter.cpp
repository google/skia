/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#if defined(SK_USE_LEGACY_LIGHTING_IMAGEFILTER)

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

struct GrShaderCaps;

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
#endif

#if GR_TEST_UTILS
#include "src/base/SkRandom.h"
#endif

const SkScalar gOneThird = SkIntToScalar(1) / 3;
const SkScalar gTwoThirds = SkIntToScalar(2) / 3;
const SkScalar gOneHalf = 0.5f;
const SkScalar gOneQuarter = 0.25f;

#if defined(SK_GANESH)
static void setUniformPoint3(const GrGLSLProgramDataManager& pdman, UniformHandle uni,
                             const SkPoint3& point) {
    static_assert(sizeof(SkPoint3) == 3 * sizeof(float));
    pdman.set3fv(uni, 1, &point.fX);
}

static void setUniformNormal3(const GrGLSLProgramDataManager& pdman, UniformHandle uni,
                              const SkPoint3& point) {
    setUniformPoint3(pdman, uni, point);
}
#endif

// Shift matrix components to the left, as we advance pixels to the right.
static inline void shiftMatrixLeft(int m[9]) {
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
#if defined(_MSC_VER) && _MSC_VER >= 1920
    // Visual Studio 2019 has some kind of code-generation bug in release builds involving the
    // lighting math in this file. Using the portable rsqrt avoids the issue. This issue appears
    // to be specific to the collection of (inline) functions in this file that call into this
    // function, not with sk_float_rsqrt itself.
    SkScalar scale = sk_float_rsqrt_portable(magSq);
#else
    SkScalar scale = sk_float_rsqrt(magSq);
#endif
    vector->fX *= scale;
    vector->fY *= scale;
    vector->fZ *= scale;
}

static SkPoint3 read_point3(SkReadBuffer& buffer) {
    SkPoint3 point;
    point.fX = buffer.readScalar();
    point.fY = buffer.readScalar();
    point.fZ = buffer.readScalar();
    buffer.validate(SkScalarIsFinite(point.fX) &&
                    SkScalarIsFinite(point.fY) &&
                    SkScalarIsFinite(point.fZ));
    return point;
}

static void write_point3(const SkPoint3& point, SkWriteBuffer& buffer) {
    buffer.writeScalar(point.fX);
    buffer.writeScalar(point.fY);
    buffer.writeScalar(point.fZ);
}

namespace {
class GpuLight;
class SkImageFilterLight : public SkRefCnt {
public:
    enum LightType {
        kDistant_LightType,
        kPoint_LightType,
        kSpot_LightType,

        kLast_LightType = kSpot_LightType
    };
    virtual LightType type() const = 0;
    const SkPoint3& color() const { return fColor; }
    virtual std::unique_ptr<GpuLight> createGpuLight() const = 0;
    virtual bool isEqual(const SkImageFilterLight& other) const {
        return fColor == other.fColor;
    }
    virtual SkImageFilterLight* transform(const SkMatrix& matrix) const = 0;

    // Defined below SkLight's subclasses.
    void flattenLight(SkWriteBuffer& buffer) const;
    static SkImageFilterLight* UnflattenLight(SkReadBuffer& buffer);

    virtual SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const = 0;
    virtual SkPoint3 lightColor(const SkPoint3& surfaceToLight) const = 0;

protected:
    SkImageFilterLight(SkColor color) {
        fColor = SkPoint3::Make(SkIntToScalar(SkColorGetR(color)),
                                SkIntToScalar(SkColorGetG(color)),
                                SkIntToScalar(SkColorGetB(color)));
    }
    SkImageFilterLight(const SkPoint3& color) : fColor(color) {}

    SkImageFilterLight(SkReadBuffer& buffer) {
        fColor = read_point3(buffer);
    }

    virtual void onFlattenLight(SkWriteBuffer& buffer) const = 0;


private:
    using INHERITED = SkRefCnt;
    SkPoint3 fColor;
};

class BaseLightingType {
public:
    BaseLightingType() {}
    virtual ~BaseLightingType() {}

    virtual SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight,
                            const SkPoint3& lightColor) const= 0;
};

class DiffuseLightingType : public BaseLightingType {
public:
    DiffuseLightingType(SkScalar kd)
        : fKD(kd) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight,
                    const SkPoint3& lightColor) const override {
        SkScalar colorScale = fKD * normal.dot(surfaceTolight);
        SkPoint3 color = lightColor.makeScale(colorScale);
        return SkPackARGB32(255,
                            SkTPin(SkScalarRoundToInt(color.fX), 0, 255),
                            SkTPin(SkScalarRoundToInt(color.fY), 0, 255),
                            SkTPin(SkScalarRoundToInt(color.fZ), 0, 255));
    }
private:
    SkScalar fKD;
};

static SkScalar max_component(const SkPoint3& p) {
    return p.x() > p.y() ? (p.x() > p.z() ? p.x() : p.z()) : (p.y() > p.z() ? p.y() : p.z());
}

class SpecularLightingType : public BaseLightingType {
public:
    SpecularLightingType(SkScalar ks, SkScalar shininess)
        : fKS(ks), fShininess(shininess) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight,
                    const SkPoint3& lightColor) const override {
        SkPoint3 halfDir(surfaceTolight);
        halfDir.fZ += SK_Scalar1;        // eye position is always (0, 0, 1)
        fast_normalize(&halfDir);
        SkScalar colorScale = fKS * SkScalarPow(normal.dot(halfDir), fShininess);
        SkPoint3 color = lightColor.makeScale(colorScale);
        return SkPackARGB32(SkTPin(SkScalarRoundToInt(max_component(color)), 0, 255),
                            SkTPin(SkScalarRoundToInt(color.fX), 0, 255),
                            SkTPin(SkScalarRoundToInt(color.fY), 0, 255),
                            SkTPin(SkScalarRoundToInt(color.fZ), 0, 255));
    }
private:
    SkScalar fKS;
    SkScalar fShininess;
};
}  // anonymous namespace

static inline SkScalar sobel(int a, int b, int c, int d, int e, int f, SkScalar scale) {
    return (-a + b - 2 * c + 2 * d -e + f) * scale;
}

static inline SkPoint3 pointToNormal(SkScalar x, SkScalar y, SkScalar surfaceScale) {
    SkPoint3 vector = SkPoint3::Make(-x * surfaceScale, -y * surfaceScale, 1);
    fast_normalize(&vector);
    return vector;
}

static inline SkPoint3 topLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(0, 0, m[4], m[5], m[7], m[8], gTwoThirds),
                         sobel(0, 0, m[4], m[7], m[5], m[8], gTwoThirds),
                         surfaceScale);
}

static inline SkPoint3 topNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[5], m[6], m[8], gOneThird),
                         sobel(m[3], m[6], m[4], m[7], m[5], m[8], gOneHalf),
                         surfaceScale);
}

static inline SkPoint3 topRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[4], m[6], m[7], gTwoThirds),
                         sobel(m[3], m[6], m[4], m[7],    0,    0, gTwoThirds),
                         surfaceScale);
}

static inline SkPoint3 leftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5], m[7], m[8], gOneHalf),
                         sobel(   0,    0, m[1], m[7], m[2], m[8], gOneThird),
                         surfaceScale);
}


static inline SkPoint3 interiorNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5], m[6], m[8], gOneQuarter),
                         sobel(m[0], m[6], m[1], m[7], m[2], m[8], gOneQuarter),
                         surfaceScale);
}

static inline SkPoint3 rightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], m[6], m[7], gOneHalf),
                         sobel(m[0], m[6], m[1], m[7],    0,    0, gOneThird),
                         surfaceScale);
}

static inline SkPoint3 bottomLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5],    0,    0, gTwoThirds),
                         sobel(   0,    0, m[1], m[4], m[2], m[5], gTwoThirds),
                         surfaceScale);
}

static inline SkPoint3 bottomNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5],    0,    0, gOneThird),
                         sobel(m[0], m[3], m[1], m[4], m[2], m[5], gOneHalf),
                         surfaceScale);
}

static inline SkPoint3 bottomRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], 0,  0, gTwoThirds),
                         sobel(m[0], m[3], m[1], m[4], 0,  0, gTwoThirds),
                         surfaceScale);
}

namespace {
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
}  // anonymous namespace

template <class PixelFetcher>
static void lightBitmap(const BaseLightingType& lightingType,
                 const SkImageFilterLight* l,
                 const SkBitmap& src,
                 SkBitmap* dst,
                 SkScalar surfaceScale,
                 const SkIRect& bounds) {
    SkASSERT(dst->width() == bounds.width() && dst->height() == bounds.height());
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

static void lightBitmap(const BaseLightingType& lightingType,
                 const SkImageFilterLight* light,
                 const SkBitmap& src,
                 SkBitmap* dst,
                 SkScalar surfaceScale,
                 const SkIRect& bounds) {
    if (src.bounds().contains(bounds)) {
        lightBitmap<UncheckedPixelFetcher>(
            lightingType, light, src, dst, surfaceScale, bounds);
    } else {
        lightBitmap<DecalPixelFetcher>(
            lightingType, light, src, dst, surfaceScale, bounds);
    }
}

namespace {
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

class SkLightingImageFilterInternal : public SkImageFilter_Base {
protected:
    SkLightingImageFilterInternal(sk_sp<SkImageFilterLight> light,
                                  SkScalar surfaceScale,
                                  sk_sp<SkImageFilter> input,
                                  const SkRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fLight(std::move(light))
            , fSurfaceScale(surfaceScale / 255) {}

    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        fLight->flattenLight(buffer);
        buffer.writeScalar(fSurfaceScale * 255);
    }

    bool onAffectsTransparentBlack() const override { return true; }

    const SkImageFilterLight* light() const { return fLight.get(); }
    inline sk_sp<const SkImageFilterLight> refLight() const { return fLight; }
    SkScalar surfaceScale() const { return fSurfaceScale; }

#if defined(SK_GANESH)
    sk_sp<SkSpecialImage> filterImageGPU(const Context& ctx,
                                         SkSpecialImage* input,
                                         const SkIRect& bounds,
                                         const SkMatrix& matrix) const;
    virtual std::unique_ptr<GrFragmentProcessor> makeFragmentProcessor(GrSurfaceProxyView,
                                                                       const SkIPoint& viewOffset,
                                                                       const SkMatrix&,
                                                                       const SkIRect* srcBounds,
                                                                       BoundaryMode boundaryMode,
                                                                       const GrCaps&) const = 0;
#endif

private:
#if defined(SK_GANESH)
    void drawRect(skgpu::ganesh::SurfaceFillContext*,
                  GrSurfaceProxyView srcView,
                  const SkIPoint& viewOffset,
                  const SkMatrix& matrix,
                  const SkIRect& dstRect,
                  BoundaryMode boundaryMode,
                  const SkIRect* srcBounds,
                  const SkIRect& bounds) const;
#endif

    sk_sp<SkImageFilterLight> fLight;
    SkScalar fSurfaceScale;

    using INHERITED = SkImageFilter_Base;
};
}  // anonymous namespace

#if defined(SK_GANESH)
void SkLightingImageFilterInternal::drawRect(skgpu::ganesh::SurfaceFillContext* sfc,
                                             GrSurfaceProxyView srcView,
                                             const SkIPoint& viewOffset,
                                             const SkMatrix& matrix,
                                             const SkIRect& dstRect,
                                             BoundaryMode boundaryMode,
                                             const SkIRect* srcBounds,
                                             const SkIRect& bounds) const {
    SkIRect srcRect = dstRect.makeOffset(bounds.topLeft());
    auto fp = this->makeFragmentProcessor(std::move(srcView), viewOffset, matrix, srcBounds,
                                          boundaryMode, *sfc->caps());
    sfc->fillRectToRectWithFP(srcRect, dstRect, std::move(fp));
}

sk_sp<SkSpecialImage> SkLightingImageFilterInternal::filterImageGPU(
                                                   const Context& ctx,
                                                   SkSpecialImage* input,
                                                   const SkIRect& offsetBounds,
                                                   const SkMatrix& matrix) const {
    SkASSERT(ctx.gpuBacked());

    auto rContext = ctx.getContext();

    GrSurfaceProxyView inputView = input->view(rContext);
    SkASSERT(inputView.asTextureProxy());

    GrImageInfo info(ctx.grColorType(),
                     kPremul_SkAlphaType,
                     ctx.refColorSpace(),
                     offsetBounds.size());
    auto sfc = rContext->priv().makeSFC(info,
                                        "LightingImageFilterInternal_FilterImageGPU",
                                        SkBackingFit::kApprox,
                                        1,
                                        skgpu::Mipmapped::kNo,
                                        inputView.proxy()->isProtected(),
                                        kBottomLeft_GrSurfaceOrigin);
    if (!sfc) {
        return nullptr;
    }

    SkIRect dstRect = SkIRect::MakeWH(offsetBounds.width(), offsetBounds.height());

    const SkIRect inputBounds = SkIRect::MakeWH(input->width(), input->height());
    SkIRect topLeft = SkIRect::MakeXYWH(0, 0, 1, 1);
    SkIRect top = SkIRect::MakeXYWH(1, 0, dstRect.width() - 2, 1);
    SkIRect topRight = SkIRect::MakeXYWH(dstRect.width() - 1, 0, 1, 1);
    SkIRect left = SkIRect::MakeXYWH(0, 1, 1, dstRect.height() - 2);
    SkIRect interior = dstRect.makeInset(1, 1);
    SkIRect right = SkIRect::MakeXYWH(dstRect.width() - 1, 1, 1, dstRect.height() - 2);
    SkIRect bottomLeft = SkIRect::MakeXYWH(0, dstRect.height() - 1, 1, 1);
    SkIRect bottom = SkIRect::MakeXYWH(1, dstRect.height() - 1, dstRect.width() - 2, 1);
    SkIRect bottomRight = SkIRect::MakeXYWH(dstRect.width() - 1, dstRect.height() - 1, 1, 1);

    const SkIRect* pSrcBounds = inputBounds.contains(offsetBounds) ? nullptr : &inputBounds;
    const SkIPoint inputViewOffset = input->subset().topLeft();

    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, topLeft,
                   kTopLeft_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, top,
                   kTop_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, topRight,
                   kTopRight_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, left,
                   kLeft_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, interior,
                   kInterior_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, right,
                   kRight_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, bottomLeft,
                   kBottomLeft_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), inputView, inputViewOffset, matrix, bottom,
                   kBottom_BoundaryMode, pSrcBounds, offsetBounds);
    this->drawRect(sfc.get(), std::move(inputView), inputViewOffset, matrix, bottomRight,
                   kBottomRight_BoundaryMode, pSrcBounds, offsetBounds);

    return SkSpecialImage::MakeDeferredFromGpu(
            rContext,
            SkIRect::MakeWH(offsetBounds.width(), offsetBounds.height()),
            kNeedNewImageUniqueID_SpecialImage,
            sfc->readSurfaceView(),
            sfc->colorInfo(),
            ctx.surfaceProps());
}
#endif

namespace {
class SkDiffuseLightingImageFilter : public SkLightingImageFilterInternal {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilterLight> light,
                                     SkScalar surfaceScale,
                                     SkScalar kd,
                                     sk_sp<SkImageFilter>,
                                     const SkRect*);

    SkScalar kd() const { return fKD; }

protected:
    SkDiffuseLightingImageFilter(sk_sp<SkImageFilterLight> light, SkScalar surfaceScale,
                                 SkScalar kd,
                                 sk_sp<SkImageFilter> input, const SkRect* cropRect);
    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> makeFragmentProcessor(GrSurfaceProxyView,
                                                               const SkIPoint& viewOffset,
                                                               const SkMatrix&,
                                                               const SkIRect* bounds,
                                                               BoundaryMode,
                                                               const GrCaps&) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkDiffuseLightingImageFilter)
    friend void ::SkRegisterLightingImageFilterFlattenables();
    SkScalar fKD;

    using INHERITED = SkLightingImageFilterInternal;
};

class SkSpecularLightingImageFilter : public SkLightingImageFilterInternal {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilterLight> light,
                                     SkScalar surfaceScale,
                                     SkScalar ks, SkScalar shininess,
                                     sk_sp<SkImageFilter>, const SkRect*);

    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

protected:
    SkSpecularLightingImageFilter(sk_sp<SkImageFilterLight> light,
                                  SkScalar surfaceScale, SkScalar ks,
                                  SkScalar shininess,
                                  sk_sp<SkImageFilter> input, const SkRect*);
    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> makeFragmentProcessor(GrSurfaceProxyView,
                                                               const SkIPoint& viewOffset,
                                                               const SkMatrix&,
                                                               const SkIRect* bounds,
                                                               BoundaryMode,
                                                               const GrCaps&) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkSpecularLightingImageFilter)
    friend void ::SkRegisterLightingImageFilterFlattenables();

    SkScalar fKS;
    SkScalar fShininess;

    using INHERITED = SkLightingImageFilterInternal;
};

#if defined(SK_GANESH)

class LightingEffect : public GrFragmentProcessor {
public:
    const SkImageFilterLight* light() const { return fLight.get(); }
    SkScalar surfaceScale() const { return fSurfaceScale; }
    const SkMatrix& filterMatrix() const { return fFilterMatrix; }
    BoundaryMode boundaryMode() const { return fBoundaryMode; }

protected:
    class ImplBase;

    LightingEffect(ClassID classID,
                   GrSurfaceProxyView,
                   const SkIPoint& viewOffset,
                   sk_sp<const SkImageFilterLight> light,
                   SkScalar surfaceScale,
                   const SkMatrix& matrix,
                   BoundaryMode boundaryMode,
                   const SkIRect* srcBounds,
                   const GrCaps& caps);

    explicit LightingEffect(const LightingEffect& that);

    bool onIsEqual(const GrFragmentProcessor&) const override;

private:
    void onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const override {
        b->add32(fBoundaryMode << 2 | fLight->type());
    }

    sk_sp<const SkImageFilterLight> fLight;
    SkScalar fSurfaceScale;
    SkMatrix fFilterMatrix;
    BoundaryMode fBoundaryMode;

    using INHERITED = GrFragmentProcessor;
};

class DiffuseLightingEffect : public LightingEffect {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView view,
                                                     const SkIPoint& viewOffset,
                                                     sk_sp<const SkImageFilterLight> light,
                                                     SkScalar surfaceScale,
                                                     const SkMatrix& matrix,
                                                     SkScalar kd,
                                                     BoundaryMode boundaryMode,
                                                     const SkIRect* srcBounds,
                                                     const GrCaps& caps) {
        return std::unique_ptr<GrFragmentProcessor>(new DiffuseLightingEffect(std::move(view),
                                                                              viewOffset,
                                                                              std::move(light),
                                                                              surfaceScale,
                                                                              matrix,
                                                                              kd,
                                                                              boundaryMode,
                                                                              srcBounds,
                                                                              caps));
    }

    const char* name() const override { return "DiffuseLighting"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new DiffuseLightingEffect(*this));
    }

private:
    class Impl;

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    DiffuseLightingEffect(GrSurfaceProxyView view,
                          const SkIPoint& viewOffset,
                          sk_sp<const SkImageFilterLight> light,
                          SkScalar surfaceScale,
                          const SkMatrix& matrix,
                          SkScalar kd,
                          BoundaryMode boundaryMode,
                          const SkIRect* srcBounds,
                          const GrCaps& caps);

    explicit DiffuseLightingEffect(const DiffuseLightingEffect& that);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    SkScalar fKD;

    using INHERITED = LightingEffect;
};

class SpecularLightingEffect : public LightingEffect {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView view,
                                                     const SkIPoint& viewOffset,
                                                     sk_sp<const SkImageFilterLight> light,
                                                     SkScalar surfaceScale,
                                                     const SkMatrix& matrix,
                                                     SkScalar ks,
                                                     SkScalar shininess,
                                                     BoundaryMode boundaryMode,
                                                     const SkIRect* srcBounds,
                                                     const GrCaps& caps) {
        return std::unique_ptr<GrFragmentProcessor>(new SpecularLightingEffect(std::move(view),
                                                                               viewOffset,
                                                                               std::move(light),
                                                                               surfaceScale,
                                                                               matrix,
                                                                               ks,
                                                                               shininess,
                                                                               boundaryMode,
                                                                               srcBounds,
                                                                               caps));
    }

    const char* name() const override { return "SpecularLighting"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new SpecularLightingEffect(*this));
    }

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

private:
    class Impl;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    SpecularLightingEffect(GrSurfaceProxyView,
                           const SkIPoint& viewOffset,
                           sk_sp<const SkImageFilterLight> light,
                           SkScalar surfaceScale,
                           const SkMatrix& matrix,
                           SkScalar ks,
                           SkScalar shininess,
                           BoundaryMode boundaryMode,
                           const SkIRect* srcBounds,
                           const GrCaps&);

    explicit SpecularLightingEffect(const SpecularLightingEffect&);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    SkScalar fKS;
    SkScalar fShininess;

    using INHERITED = LightingEffect;
};

///////////////////////////////////////////////////////////////////////////////

class GpuLight {
public:
    virtual ~GpuLight() = default;

    /**
     * This is called by GrGLLightingEffect::emitCode() before either of the two virtual functions
     * below. It adds a half3 uniform visible in the FS that represents the constant light color.
     */
    void emitLightColorUniform(const GrFragmentProcessor*, GrGLSLUniformHandler*);

    /**
     * These two functions are called from GrGLLightingEffect's emitCode() function.
     * emitSurfaceToLight places an expression in param out that is the vector from the surface to
     * the light. The expression will be used in the FS. emitLightColor writes an expression into
     * the FS that is the color of the light. Either function may add functions and/or uniforms to
     * the FS. The default of emitLightColor appends the name of the constant light color uniform
     * and so this function only needs to be overridden if the light color varies spatially.
     */
    virtual void emitSurfaceToLight(const GrFragmentProcessor*,
                                    GrGLSLUniformHandler*,
                                    GrGLSLFPFragmentBuilder*,
                                    const char* z) = 0;
    virtual void emitLightColor(const GrFragmentProcessor*,
                                GrGLSLUniformHandler*,
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

    using INHERITED = SkRefCnt;
};

///////////////////////////////////////////////////////////////////////////////

class GpuDistantLight : public GpuLight {
public:
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(const GrFragmentProcessor*, GrGLSLUniformHandler*,
                            GrGLSLFPFragmentBuilder*, const char* z) override;

private:
    using INHERITED = GpuLight;
    UniformHandle fDirectionUni;
};

///////////////////////////////////////////////////////////////////////////////

class GpuPointLight : public GpuLight {
public:
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(const GrFragmentProcessor*, GrGLSLUniformHandler*,
                            GrGLSLFPFragmentBuilder*, const char* z) override;

private:
    using INHERITED = GpuLight;
    UniformHandle fLocationUni;
};

///////////////////////////////////////////////////////////////////////////////

class GpuSpotLight : public GpuLight {
public:
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(const GrFragmentProcessor*, GrGLSLUniformHandler*,
                            GrGLSLFPFragmentBuilder*, const char* z) override;
    void emitLightColor(const GrFragmentProcessor*,
                        GrGLSLUniformHandler*,
                        GrGLSLFPFragmentBuilder*,
                        const char *surfaceToLight) override;

private:
    using INHERITED = GpuLight;

    SkString        fLightColorFunc;
    UniformHandle   fLocationUni;
    UniformHandle   fExponentUni;
    UniformHandle   fCosOuterConeAngleUni;
    UniformHandle   fCosInnerConeAngleUni;
    UniformHandle   fConeScaleUni;
    UniformHandle   fSUni;
};

#else

class GpuLight {};

#endif

///////////////////////////////////////////////////////////////////////////////

class SkDistantLight : public SkImageFilterLight {
public:
    SkDistantLight(const SkPoint3& direction, SkColor color)
      : INHERITED(color), fDirection(direction) {
    }

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const override {
        return fDirection;
    }
    SkPoint3 lightColor(const SkPoint3&) const override { return this->color(); }
    LightType type() const override { return kDistant_LightType; }
    const SkPoint3& direction() const { return fDirection; }
    std::unique_ptr<GpuLight> createGpuLight() const override {
#if defined(SK_GANESH)
        return std::make_unique<GpuDistantLight>();
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }

    bool isEqual(const SkImageFilterLight& other) const override {
        if (other.type() != kDistant_LightType) {
            return false;
        }

        const SkDistantLight& o = static_cast<const SkDistantLight&>(other);
        return INHERITED::isEqual(other) &&
               fDirection == o.fDirection;
    }

    SkDistantLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fDirection = read_point3(buffer);
    }

protected:
    SkDistantLight(const SkPoint3& direction, const SkPoint3& color)
      : INHERITED(color), fDirection(direction) {
    }
    SkImageFilterLight* transform(const SkMatrix& matrix) const override {
        return new SkDistantLight(direction(), color());
    }
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        write_point3(fDirection, buffer);
    }

private:
    SkPoint3 fDirection;

    using INHERITED = SkImageFilterLight;
};

///////////////////////////////////////////////////////////////////////////////

class SkPointLight : public SkImageFilterLight {
public:
    SkPointLight(const SkPoint3& location, SkColor color)
     : INHERITED(color), fLocation(location) {}

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const override {
        SkPoint3 direction = SkPoint3::Make(fLocation.fX - SkIntToScalar(x),
                                            fLocation.fY - SkIntToScalar(y),
                                            fLocation.fZ - SkIntToScalar(z) * surfaceScale);
        fast_normalize(&direction);
        return direction;
    }
    SkPoint3 lightColor(const SkPoint3&) const override { return this->color(); }
    LightType type() const override { return kPoint_LightType; }
    const SkPoint3& location() const { return fLocation; }
    std::unique_ptr<GpuLight> createGpuLight() const override {
#if defined(SK_GANESH)
        return std::make_unique<GpuPointLight>();
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }

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
        fLocation = read_point3(buffer);
    }

protected:
    SkPointLight(const SkPoint3& location, const SkPoint3& color)
     : INHERITED(color), fLocation(location) {}
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        write_point3(fLocation, buffer);
    }

private:
    SkPoint3 fLocation;

    using INHERITED = SkImageFilterLight;
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
       fSpecularExponent(specularExponent)
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

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const override {
        SkPoint3 direction = SkPoint3::Make(fLocation.fX - SkIntToScalar(x),
                                            fLocation.fY - SkIntToScalar(y),
                                            fLocation.fZ - SkIntToScalar(z) * surfaceScale);
        fast_normalize(&direction);
        return direction;
    }
    SkPoint3 lightColor(const SkPoint3& surfaceToLight) const override {
        SkScalar cosAngle = -surfaceToLight.dot(fS);
        SkScalar scale = 0;
        if (cosAngle >= fCosOuterConeAngle) {
            scale = SkScalarPow(cosAngle, fSpecularExponent);
            if (cosAngle < fCosInnerConeAngle) {
                scale *= (cosAngle - fCosOuterConeAngle) * fConeScale;
            }
        }
        return this->color().makeScale(scale);
    }
    std::unique_ptr<GpuLight> createGpuLight() const override {
#if defined(SK_GANESH)
        return std::make_unique<GpuSpotLight>();
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }
    LightType type() const override { return kSpot_LightType; }
    const SkPoint3& location() const { return fLocation; }
    const SkPoint3& target() const { return fTarget; }
    SkScalar specularExponent() const { return fSpecularExponent; }
    SkScalar cosInnerConeAngle() const { return fCosInnerConeAngle; }
    SkScalar cosOuterConeAngle() const { return fCosOuterConeAngle; }
    SkScalar coneScale() const { return fConeScale; }
    const SkPoint3& s() const { return fS; }

    SkSpotLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = read_point3(buffer);
        fTarget = read_point3(buffer);
        fSpecularExponent = buffer.readScalar();
        fCosOuterConeAngle = buffer.readScalar();
        fCosInnerConeAngle = buffer.readScalar();
        fConeScale = buffer.readScalar();
        fS = read_point3(buffer);
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
        write_point3(fLocation, buffer);
        write_point3(fTarget, buffer);
        buffer.writeScalar(fSpecularExponent);
        buffer.writeScalar(fCosOuterConeAngle);
        buffer.writeScalar(fCosInnerConeAngle);
        buffer.writeScalar(fConeScale);
        write_point3(fS, buffer);
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
    SkPoint3 fLocation;
    SkPoint3 fTarget;
    SkScalar fSpecularExponent;
    SkScalar fCosOuterConeAngle;
    SkScalar fCosInnerConeAngle;
    SkScalar fConeScale;
    SkPoint3 fS;

    using INHERITED = SkImageFilterLight;
};
}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

void SkImageFilterLight::flattenLight(SkWriteBuffer& buffer) const {
    // Write type first, then baseclass, then subclass.
    buffer.writeInt(this->type());
    write_point3(fColor, buffer);
    this->onFlattenLight(buffer);
}

/*static*/ SkImageFilterLight* SkImageFilterLight::UnflattenLight(SkReadBuffer& buffer) {
    SkImageFilterLight::LightType type = buffer.read32LE(SkImageFilterLight::kLast_LightType);

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
            // Should never get here due to prior check of SkSafeRange
            SkDEBUGFAIL("Unknown LightType.");
            return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::DistantLitDiffuse(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    sk_sp<SkImageFilterLight> light(new SkDistantLight(direction, lightColor));
    return SkDiffuseLightingImageFilter::Make(std::move(light), surfaceScale, kd,
                                              std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitDiffuse(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    sk_sp<SkImageFilterLight> light(new SkPointLight(location, lightColor));
    return SkDiffuseLightingImageFilter::Make(std::move(light), surfaceScale, kd,
                                              std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitDiffuse(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    sk_sp<SkImageFilterLight> light(new SkSpotLight(location, target, falloffExponent,
                                                    cutoffAngle, lightColor));
    return SkDiffuseLightingImageFilter::Make(std::move(light), surfaceScale, kd,
                                              std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DistantLitSpecular(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    sk_sp<SkImageFilterLight> light(new SkDistantLight(direction, lightColor));
    return SkSpecularLightingImageFilter::Make(std::move(light), surfaceScale, ks, shininess,
                                               std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitSpecular(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    sk_sp<SkImageFilterLight> light(new SkPointLight(location, lightColor));
    return SkSpecularLightingImageFilter::Make(std::move(light), surfaceScale, ks, shininess,
                                               std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitSpecular(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    sk_sp<SkImageFilterLight> light(new SkSpotLight(location, target, falloffExponent,
                                                    cutoffAngle, lightColor));
    return SkSpecularLightingImageFilter::Make(std::move(light), surfaceScale, ks, shininess,
                                               std::move(input), cropRect);
}

void SkRegisterLightingImageFilterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkDiffuseLightingImageFilter);
    SK_REGISTER_FLATTENABLE(SkSpecularLightingImageFilter);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkDiffuseLightingImageFilter::Make(sk_sp<SkImageFilterLight> light,
                                                        SkScalar surfaceScale,
                                                        SkScalar kd,
                                                        sk_sp<SkImageFilter> input,
                                                        const SkRect* cropRect) {
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
                                                           const SkRect* cropRect)
    : INHERITED(std::move(light), surfaceScale, std::move(input), cropRect)
    , fKD(kd) {
}

sk_sp<SkFlattenable> SkDiffuseLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    sk_sp<SkImageFilterLight> light(SkImageFilterLight::UnflattenLight(buffer));
    SkScalar surfaceScale = buffer.readScalar();
    SkScalar kd = buffer.readScalar();

    return Make(std::move(light), surfaceScale, kd, common.getInput(0), common.cropRect());
}

void SkDiffuseLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKD);
}

sk_sp<SkSpecialImage> SkDiffuseLightingImageFilter::onFilterImage(const Context& ctx,
                                                                  SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
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

#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-offset->fX), SkIntToScalar(-offset->fY));

        return this->filterImageGPU(ctx, input.get(), bounds, matrix);
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

    if (!inputBM.getPixels()) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(bounds.width(), bounds.height());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-inputOffset.x()), SkIntToScalar(-inputOffset.y()));

    sk_sp<SkImageFilterLight> transformedLight(light()->transform(matrix));

    DiffuseLightingType lightingType(fKD);
    lightBitmap(lightingType,
                                                             transformedLight.get(),
                                                             inputBM,
                                                             &dst,
                                                             surfaceScale(),
                                                             bounds);

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst, ctx.surfaceProps());
}

#if defined(SK_GANESH)
std::unique_ptr<GrFragmentProcessor> SkDiffuseLightingImageFilter::makeFragmentProcessor(
        GrSurfaceProxyView view,
        const SkIPoint& viewOffset,
        const SkMatrix& matrix,
        const SkIRect* srcBounds,
        BoundaryMode boundaryMode,
        const GrCaps& caps) const {
    SkScalar scale = this->surfaceScale() * 255;
    return DiffuseLightingEffect::Make(std::move(view),
                                       viewOffset,
                                       this->refLight(),
                                       scale,
                                       matrix,
                                       this->kd(),
                                       boundaryMode,
                                       srcBounds,
                                       caps);
}
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkSpecularLightingImageFilter::Make(sk_sp<SkImageFilterLight> light,
                                                         SkScalar surfaceScale,
                                                         SkScalar ks,
                                                         SkScalar shininess,
                                                         sk_sp<SkImageFilter> input,
                                                         const SkRect* cropRect) {
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
                                                             const SkRect* cropRect)
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
                common.cropRect());
}

void SkSpecularLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKS);
    buffer.writeScalar(fShininess);
}

sk_sp<SkSpecialImage> SkSpecularLightingImageFilter::onFilterImage(const Context& ctx,
                                                                   SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
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

#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-offset->fX), SkIntToScalar(-offset->fY));

        return this->filterImageGPU(ctx, input.get(), bounds, matrix);
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

    if (!inputBM.getPixels()) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(bounds.width(), bounds.height());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    SpecularLightingType lightingType(fKS, fShininess);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-inputOffset.x()), SkIntToScalar(-inputOffset.y()));

    sk_sp<SkImageFilterLight> transformedLight(light()->transform(matrix));

    lightBitmap(lightingType,
                                                              transformedLight.get(),
                                                              inputBM,
                                                              &dst,
                                                              surfaceScale(),
                                                              bounds);

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()), dst,
                                          ctx.surfaceProps());
}

#if defined(SK_GANESH)
std::unique_ptr<GrFragmentProcessor> SkSpecularLightingImageFilter::makeFragmentProcessor(
        GrSurfaceProxyView view,
        const SkIPoint& viewOffset,
        const SkMatrix& matrix,
        const SkIRect* srcBounds,
        BoundaryMode boundaryMode,
        const GrCaps& caps) const {
    SkScalar scale = this->surfaceScale() * 255;
    return SpecularLightingEffect::Make(std::move(view),
                                        viewOffset,
                                        this->refLight(),
                                        scale,
                                        matrix,
                                        this->ks(),
                                        this->shininess(),
                                        boundaryMode,
                                        srcBounds,
                                        caps);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)

static SkString emitNormalFunc(BoundaryMode mode,
                               const char* pointToNormalName,
                               const char* sobelFuncName) {
    SkString result;
    switch (mode) {
    case kTopLeft_BoundaryMode:
        result.printf("return %s(%s(0.0, 0.0, m[4], m[5], m[7], m[8], %g),"
                      "          %s(0.0, 0.0, m[4], m[7], m[5], m[8], %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kTop_BoundaryMode:
        result.printf("return %s(%s(0.0, 0.0, m[3], m[5], m[6], m[8], %g),"
                      "          %s(0.0, 0.0, m[4], m[7], m[5], m[8], %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gOneThird,
                                         sobelFuncName, gOneHalf);
        break;
    case kTopRight_BoundaryMode:
        result.printf("return %s(%s( 0.0,  0.0, m[3], m[4], m[6], m[7], %g),"
                      "          %s(m[3], m[6], m[4], m[7],  0.0,  0.0, %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kLeft_BoundaryMode:
        result.printf("return %s(%s(m[1], m[2], m[4], m[5], m[7], m[8], %g),"
                      "          %s( 0.0,  0.0, m[1], m[7], m[2], m[8], %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gOneHalf,
                                         sobelFuncName, gOneThird);
        break;
    case kInterior_BoundaryMode:
        result.printf("return %s(%s(m[0], m[2], m[3], m[5], m[6], m[8], %g),"
                      "          %s(m[0], m[6], m[1], m[7], m[2], m[8], %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gOneQuarter,
                                         sobelFuncName, gOneQuarter);
        break;
    case kRight_BoundaryMode:
        result.printf("return %s(%s(m[0], m[1], m[3], m[4], m[6], m[7], %g),"
                      "          %s(m[0], m[6], m[1], m[7],  0.0,  0.0, %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gOneHalf,
                                         sobelFuncName, gOneThird);
        break;
    case kBottomLeft_BoundaryMode:
        result.printf("return %s(%s(m[1], m[2], m[4], m[5],  0.0,  0.0, %g),"
                      "          %s( 0.0,  0.0, m[1], m[4], m[2], m[5], %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kBottom_BoundaryMode:
        result.printf("return %s(%s(m[0], m[2], m[3], m[5],  0.0,  0.0, %g),"
                      "          %s(m[0], m[3], m[1], m[4], m[2], m[5], %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gOneThird,
                                         sobelFuncName, gOneHalf);
        break;
    case kBottomRight_BoundaryMode:
        result.printf("return %s(%s(m[0], m[1], m[3], m[4],  0.0,  0.0, %g),"
                      "          %s(m[0], m[3], m[1], m[4],  0.0,  0.0, %g),"
                      "          surfaceScale);",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    default:
        SkASSERT(false);
        break;
    }
    return result;
}

namespace {
class LightingEffect::ImplBase : public ProgramImpl {
public:
    void emitCode(EmitArgs&) override;

protected:
    /**
     * Subclasses of LightingImpl must call INHERITED::onSetData();
     */
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    virtual void emitLightFunc(const GrFragmentProcessor*,
                               GrGLSLUniformHandler*,
                               GrGLSLFPFragmentBuilder*,
                               SkString* funcName) = 0;

private:
    UniformHandle             fSurfaceScaleUni;
    std::unique_ptr<GpuLight> fLight;
};

///////////////////////////////////////////////////////////////////////////////

class DiffuseLightingEffect::Impl : public ImplBase {
public:
    void emitLightFunc(const GrFragmentProcessor*,
                       GrGLSLUniformHandler*,
                       GrGLSLFPFragmentBuilder*,
                       SkString* funcName) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    using INHERITED = ImplBase;

    UniformHandle   fKDUni;
};

///////////////////////////////////////////////////////////////////////////////

class SpecularLightingEffect::Impl : public ImplBase {
public:
    void emitLightFunc(const GrFragmentProcessor*,
                       GrGLSLUniformHandler*,
                       GrGLSLFPFragmentBuilder*,
                       SkString* funcName) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    using INHERITED = ImplBase;

    UniformHandle   fKSUni;
    UniformHandle   fShininessUni;
};
}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

LightingEffect::LightingEffect(ClassID classID,
                               GrSurfaceProxyView view,
                               const SkIPoint& viewOffset,
                               sk_sp<const SkImageFilterLight> light,
                               SkScalar surfaceScale,
                               const SkMatrix& matrix,
                               BoundaryMode boundaryMode,
                               const SkIRect* srcBounds,
                               const GrCaps& caps)
        // Perhaps this could advertise the opaque or coverage-as-alpha optimizations?
        : INHERITED(classID, kNone_OptimizationFlags)
        , fLight(std::move(light))
        , fSurfaceScale(surfaceScale)
        , fFilterMatrix(matrix)
        , fBoundaryMode(boundaryMode) {
    static constexpr GrSamplerState kSampler(GrSamplerState::WrapMode::kClampToBorder,
                                             GrSamplerState::Filter::kNearest);
    std::unique_ptr<GrFragmentProcessor> child;
    if (srcBounds) {
        SkRect offsetSrcBounds = SkRect::Make(*srcBounds);
        offsetSrcBounds.offset(viewOffset.fX, viewOffset.fY);
        child = GrTextureEffect::MakeSubset(std::move(view), kPremul_SkAlphaType,
                                            SkMatrix::Translate(viewOffset.fX, viewOffset.fY),
                                            kSampler, offsetSrcBounds, caps);
    } else {
        child = GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType,
                                      SkMatrix::Translate(viewOffset.fX, viewOffset.fY),
                                      kSampler, caps);
    }
    this->registerChild(std::move(child), SkSL::SampleUsage::Explicit());
    this->setUsesSampleCoordsDirectly();
}

LightingEffect::LightingEffect(const LightingEffect& that)
        : INHERITED(that)
        , fLight(that.fLight)
        , fSurfaceScale(that.fSurfaceScale)
        , fFilterMatrix(that.fFilterMatrix)
        , fBoundaryMode(that.fBoundaryMode) {}

bool LightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const LightingEffect& s = sBase.cast<LightingEffect>();
    return fLight->isEqual(*s.fLight) &&
           fSurfaceScale == s.fSurfaceScale &&
           fBoundaryMode == s.fBoundaryMode;
}

///////////////////////////////////////////////////////////////////////////////

DiffuseLightingEffect::DiffuseLightingEffect(GrSurfaceProxyView view,
                                             const SkIPoint& viewOffset,
                                             sk_sp<const SkImageFilterLight> light,
                                             SkScalar surfaceScale,
                                             const SkMatrix& matrix,
                                             SkScalar kd,
                                             BoundaryMode boundaryMode,
                                             const SkIRect* srcBounds,
                                             const GrCaps& caps)
        : INHERITED(kGrDiffuseLightingEffect_ClassID,
                    std::move(view),
                    viewOffset,
                    std::move(light),
                    surfaceScale,
                    matrix,
                    boundaryMode,
                    srcBounds,
                    caps)
        , fKD(kd) {}

DiffuseLightingEffect::DiffuseLightingEffect(const DiffuseLightingEffect& that)
        : INHERITED(that), fKD(that.fKD) {}

bool DiffuseLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const DiffuseLightingEffect& s = sBase.cast<DiffuseLightingEffect>();
    return INHERITED::onIsEqual(sBase) && fKD == s.fKD;
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> DiffuseLightingEffect::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(DiffuseLightingEffect)

#if GR_TEST_UTILS

static SkPoint3 random_point3(SkRandom* random) {
    return SkPoint3::Make(SkScalarToFloat(random->nextSScalar1()),
                          SkScalarToFloat(random->nextSScalar1()),
                          SkScalarToFloat(random->nextSScalar1()));
}

static SkImageFilterLight* create_random_light(SkRandom* random) {
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
            SK_ABORT("Unexpected value.");
    }
}

std::unique_ptr<GrFragmentProcessor> DiffuseLightingEffect::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();
    SkScalar surfaceScale = d->fRandom->nextSScalar1();
    SkScalar kd = d->fRandom->nextUScalar1();
    sk_sp<SkImageFilterLight> light(create_random_light(d->fRandom));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = d->fRandom->nextUScalar1();
    }

    uint32_t boundsX = d->fRandom->nextRangeU(0, view.width());
    uint32_t boundsY = d->fRandom->nextRangeU(0, view.height());
    uint32_t boundsW = d->fRandom->nextRangeU(0, view.width());
    uint32_t boundsH = d->fRandom->nextRangeU(0, view.height());
    SkIRect srcBounds = SkIRect::MakeXYWH(boundsX, boundsY, boundsW, boundsH);
    BoundaryMode mode = static_cast<BoundaryMode>(d->fRandom->nextU() % kBoundaryModeCount);

    return DiffuseLightingEffect::Make(std::move(view),
                                       SkIPoint(),
                                       std::move(light),
                                       surfaceScale,
                                       matrix,
                                       kd,
                                       mode,
                                       &srcBounds,
                                       *d->caps());
}
#endif


///////////////////////////////////////////////////////////////////////////////

void LightingEffect::ImplBase::emitCode(EmitArgs& args) {
    const LightingEffect& le = args.fFp.cast<LightingEffect>();
    if (!fLight) {
        fLight = le.light()->createGpuLight();
    }

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fSurfaceScaleUni = uniformHandler->addUniform(&le,
                                                  kFragment_GrShaderFlag,
                                                  SkSLType::kHalf, "SurfaceScale");
    fLight->emitLightColorUniform(&le, uniformHandler);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString lightFunc;
    this->emitLightFunc(&le, uniformHandler, fragBuilder, &lightFunc);
    const GrShaderVar gSobelArgs[] = {
        GrShaderVar("a", SkSLType::kHalf),
        GrShaderVar("b", SkSLType::kHalf),
        GrShaderVar("c", SkSLType::kHalf),
        GrShaderVar("d", SkSLType::kHalf),
        GrShaderVar("e", SkSLType::kHalf),
        GrShaderVar("f", SkSLType::kHalf),
        GrShaderVar("scale", SkSLType::kHalf),
    };

    SkString sobelFuncName = fragBuilder->getMangledFunctionName("sobel");
    fragBuilder->emitFunction(SkSLType::kHalf,
                              sobelFuncName.c_str(),
                              {gSobelArgs, std::size(gSobelArgs)},
                              "return (-a + b - 2.0 * c + 2.0 * d -e + f) * scale;");
    const GrShaderVar gPointToNormalArgs[] = {
        GrShaderVar("x", SkSLType::kHalf),
        GrShaderVar("y", SkSLType::kHalf),
        GrShaderVar("scale", SkSLType::kHalf),
    };
    SkString pointToNormalName = fragBuilder->getMangledFunctionName("pointToNormal");
    fragBuilder->emitFunction(SkSLType::kHalf3,
                              pointToNormalName.c_str(),
                              {gPointToNormalArgs, std::size(gPointToNormalArgs)},
                              "return normalize(half3(-x * scale, -y * scale, 1));");

    const GrShaderVar gInteriorNormalArgs[] = {
        GrShaderVar("m", SkSLType::kHalf, 9),
        GrShaderVar("surfaceScale", SkSLType::kHalf),
    };
    SkString normalBody = emitNormalFunc(le.boundaryMode(),
                                         pointToNormalName.c_str(),
                                         sobelFuncName.c_str());
    SkString normalName = fragBuilder->getMangledFunctionName("normal");
    fragBuilder->emitFunction(SkSLType::kHalf3,
                              normalName.c_str(),
                              {gInteriorNormalArgs, std::size(gInteriorNormalArgs)},
                              normalBody.c_str());

    fragBuilder->codeAppendf("float2 coord = %s;", args.fSampleCoord);
    fragBuilder->codeAppend("half m[9];");

    const char* surfScale = uniformHandler->getUniformCStr(fSurfaceScaleUni);

    int index = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            SkString texCoords;
            texCoords.appendf("coord + half2(%d, %d)", dx, dy);
            auto sample = this->invokeChild(0, args, texCoords.c_str());
            fragBuilder->codeAppendf("m[%d] = %s.a;", index, sample.c_str());
            index++;
        }
    }
    fragBuilder->codeAppend("half3 surfaceToLight = ");
    SkString arg;
    arg.appendf("%s * m[4]", surfScale);
    fLight->emitSurfaceToLight(&le, uniformHandler, fragBuilder, arg.c_str());
    fragBuilder->codeAppend(";");
    fragBuilder->codeAppendf("return %s(%s(m, %s), surfaceToLight, ",
                             lightFunc.c_str(), normalName.c_str(), surfScale);
    fLight->emitLightColor(&le, uniformHandler, fragBuilder, "surfaceToLight");
    fragBuilder->codeAppend(");");
}

void LightingEffect::ImplBase::onSetData(const GrGLSLProgramDataManager& pdman,
                                         const GrFragmentProcessor& proc) {
    const LightingEffect& lighting = proc.cast<LightingEffect>();
    if (!fLight) {
        fLight = lighting.light()->createGpuLight();
    }

    pdman.set1f(fSurfaceScaleUni, lighting.surfaceScale());
    sk_sp<SkImageFilterLight> transformedLight(
            lighting.light()->transform(lighting.filterMatrix()));
    fLight->setData(pdman, transformedLight.get());
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

void DiffuseLightingEffect::Impl::emitLightFunc(const GrFragmentProcessor* owner,
                                                GrGLSLUniformHandler* uniformHandler,
                                                GrGLSLFPFragmentBuilder* fragBuilder,
                                                SkString* funcName) {
    const char* kd;
    fKDUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf, "KD", &kd);

    const GrShaderVar gLightArgs[] = {
        GrShaderVar("normal", SkSLType::kHalf3),
        GrShaderVar("surfaceToLight", SkSLType::kHalf3),
        GrShaderVar("lightColor", SkSLType::kHalf3)
    };
    SkString lightBody;
    lightBody.appendf("half colorScale = %s * dot(normal, surfaceToLight);", kd);
    lightBody.appendf("return half4(saturate(lightColor * colorScale), 1.0);");
    *funcName = fragBuilder->getMangledFunctionName("light");
    fragBuilder->emitFunction(SkSLType::kHalf4,
                              funcName->c_str(),
                              {gLightArgs, std::size(gLightArgs)},
                              lightBody.c_str());
}

void DiffuseLightingEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                            const GrFragmentProcessor& proc) {
    INHERITED::onSetData(pdman, proc);
    const DiffuseLightingEffect& diffuse = proc.cast<DiffuseLightingEffect>();
    pdman.set1f(fKDUni, diffuse.fKD);
}

///////////////////////////////////////////////////////////////////////////////

SpecularLightingEffect::SpecularLightingEffect(GrSurfaceProxyView view,
                                               const SkIPoint& viewOffset,
                                               sk_sp<const SkImageFilterLight> light,
                                               SkScalar surfaceScale,
                                               const SkMatrix& matrix,
                                               SkScalar ks,
                                               SkScalar shininess,
                                               BoundaryMode boundaryMode,
                                               const SkIRect* srcBounds,
                                               const GrCaps& caps)
        : INHERITED(kGrSpecularLightingEffect_ClassID,
                    std::move(view),
                    viewOffset,
                    std::move(light),
                    surfaceScale,
                    matrix,
                    boundaryMode,
                    srcBounds,
                    caps)
        , fKS(ks)
        , fShininess(shininess) {}

SpecularLightingEffect::SpecularLightingEffect(const SpecularLightingEffect& that)
        : INHERITED(that), fKS(that.fKS), fShininess(that.fShininess) {}

bool SpecularLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const SpecularLightingEffect& s = sBase.cast<SpecularLightingEffect>();
    return INHERITED::onIsEqual(sBase) && this->fKS == s.fKS && this->fShininess == s.fShininess;
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl>
SpecularLightingEffect::onMakeProgramImpl() const { return std::make_unique<Impl>(); }

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(SpecularLightingEffect)

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> SpecularLightingEffect::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();
    SkScalar surfaceScale = d->fRandom->nextSScalar1();
    SkScalar ks = d->fRandom->nextUScalar1();
    SkScalar shininess = d->fRandom->nextUScalar1();
    sk_sp<SkImageFilterLight> light(create_random_light(d->fRandom));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = d->fRandom->nextUScalar1();
    }
    BoundaryMode mode = static_cast<BoundaryMode>(d->fRandom->nextU() % kBoundaryModeCount);

    uint32_t boundsX = d->fRandom->nextRangeU(0, view.width());
    uint32_t boundsY = d->fRandom->nextRangeU(0, view.height());
    uint32_t boundsW = d->fRandom->nextRangeU(0, view.width());
    uint32_t boundsH = d->fRandom->nextRangeU(0, view.height());
    SkIRect srcBounds = SkIRect::MakeXYWH(boundsX, boundsY, boundsW, boundsH);

    return SpecularLightingEffect::Make(std::move(view),
                                        SkIPoint(),
                                        std::move(light),
                                        surfaceScale,
                                        matrix,
                                        ks,
                                        shininess,
                                        mode,
                                        &srcBounds,
                                        *d->caps());
}
#endif

///////////////////////////////////////////////////////////////////////////////

void SpecularLightingEffect::Impl::emitLightFunc(const GrFragmentProcessor* owner,
                                                 GrGLSLUniformHandler* uniformHandler,
                                                 GrGLSLFPFragmentBuilder* fragBuilder,
                                                 SkString* funcName) {
    const char* ks;
    const char* shininess;

    fKSUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf, "KS", &ks);
    fShininessUni = uniformHandler->addUniform(owner,
                                               kFragment_GrShaderFlag,
                                               SkSLType::kHalf,
                                               "Shininess",
                                               &shininess);

    const GrShaderVar gLightArgs[] = {
        GrShaderVar("normal", SkSLType::kHalf3),
        GrShaderVar("surfaceToLight", SkSLType::kHalf3),
        GrShaderVar("lightColor", SkSLType::kHalf3)
    };
    SkString lightBody;
    lightBody.appendf("half3 halfDir = half3(normalize(surfaceToLight + half3(0, 0, 1)));");
    lightBody.appendf("half colorScale = half(%s * pow(dot(normal, halfDir), %s));",
                      ks, shininess);
    lightBody.appendf("half3 color = saturate(lightColor * colorScale);");
    lightBody.appendf("return half4(color, max(max(color.r, color.g), color.b));");
    *funcName = fragBuilder->getMangledFunctionName("light");
    fragBuilder->emitFunction(SkSLType::kHalf4,
                              funcName->c_str(),
                              {gLightArgs, std::size(gLightArgs)},
                              lightBody.c_str());
}

void SpecularLightingEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                             const GrFragmentProcessor& effect) {
    INHERITED::onSetData(pdman, effect);
    const SpecularLightingEffect& spec = effect.cast<SpecularLightingEffect>();
    pdman.set1f(fKSUni, spec.fKS);
    pdman.set1f(fShininessUni, spec.fShininess);
}

///////////////////////////////////////////////////////////////////////////////
void GpuLight::emitLightColorUniform(const GrFragmentProcessor* owner,
                                     GrGLSLUniformHandler* uniformHandler) {
    fColorUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf3,
                                           "LightColor");
}

void GpuLight::emitLightColor(const GrFragmentProcessor* owner,
                              GrGLSLUniformHandler* uniformHandler,
                              GrGLSLFPFragmentBuilder* fragBuilder,
                              const char* surfaceToLight) {
    fragBuilder->codeAppend(uniformHandler->getUniformCStr(this->lightColorUni()));
}

void GpuLight::setData(const GrGLSLProgramDataManager& pdman,
                       const SkImageFilterLight* light) const {
    setUniformPoint3(pdman, fColorUni,
                     light->color().makeScale(SkScalarInvert(SkIntToScalar(255))));
}

///////////////////////////////////////////////////////////////////////////////

void GpuDistantLight::setData(const GrGLSLProgramDataManager& pdman,
                              const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kDistant_LightType);
    const SkDistantLight* distantLight = static_cast<const SkDistantLight*>(light);
    setUniformNormal3(pdman, fDirectionUni, distantLight->direction());
}

void GpuDistantLight::emitSurfaceToLight(const GrFragmentProcessor* owner,
                                         GrGLSLUniformHandler* uniformHandler,
                                         GrGLSLFPFragmentBuilder* fragBuilder,
                                         const char* z) {
    const char* dir;
    fDirectionUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf3,
                                               "LightDirection", &dir);
    fragBuilder->codeAppend(dir);
}

///////////////////////////////////////////////////////////////////////////////

void GpuPointLight::setData(const GrGLSLProgramDataManager& pdman,
                            const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kPoint_LightType);
    const SkPointLight* pointLight = static_cast<const SkPointLight*>(light);
    setUniformPoint3(pdman, fLocationUni, pointLight->location());
}

void GpuPointLight::emitSurfaceToLight(const GrFragmentProcessor* owner,
                                       GrGLSLUniformHandler* uniformHandler,
                                       GrGLSLFPFragmentBuilder* fragBuilder,
                                       const char* z) {
    const char* loc;
    fLocationUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf3,
                                              "LightLocation", &loc);
    fragBuilder->codeAppendf("normalize(%s - half3(sk_FragCoord.xy, %s))",
                             loc, z);
}

///////////////////////////////////////////////////////////////////////////////

void GpuSpotLight::setData(const GrGLSLProgramDataManager& pdman,
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

void GpuSpotLight::emitSurfaceToLight(const GrFragmentProcessor* owner,
                                      GrGLSLUniformHandler* uniformHandler,
                                      GrGLSLFPFragmentBuilder* fragBuilder,
                                      const char* z) {
    const char* location;
    fLocationUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf3,
                                              "LightLocation", &location);

    fragBuilder->codeAppendf("normalize(%s - half3(sk_FragCoord.xy, %s))",
                             location, z);
}

void GpuSpotLight::emitLightColor(const GrFragmentProcessor* owner,
                                  GrGLSLUniformHandler* uniformHandler,
                                  GrGLSLFPFragmentBuilder* fragBuilder,
                                  const char* surfaceToLight) {
    const char* color = uniformHandler->getUniformCStr(this->lightColorUni()); // created by parent class.

    const char* exponent;
    const char* cosInner;
    const char* cosOuter;
    const char* coneScale;
    const char* s;
    fExponentUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf,
                                              "Exponent", &exponent);
    fCosInnerConeAngleUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag,
                                                       SkSLType::kHalf, "CosInnerConeAngle",
                                                       &cosInner);
    fCosOuterConeAngleUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag,
                                                       SkSLType::kHalf, "CosOuterConeAngle",
                                                       &cosOuter);
    fConeScaleUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf,
                                               "ConeScale", &coneScale);
    fSUni = uniformHandler->addUniform(owner, kFragment_GrShaderFlag, SkSLType::kHalf3, "S", &s);

    const GrShaderVar gLightColorArgs[] = {
        GrShaderVar("surfaceToLight", SkSLType::kHalf3)
    };
    SkString lightColorBody;
    lightColorBody.appendf("half cosAngle = -dot(surfaceToLight, %s);", s);
    lightColorBody.appendf("if (cosAngle < %s) {", cosOuter);
    lightColorBody.appendf("return half3(0);");
    lightColorBody.appendf("}");
    lightColorBody.appendf("half scale = pow(cosAngle, %s);", exponent);
    lightColorBody.appendf("if (cosAngle < %s) {", cosInner);
    lightColorBody.appendf("return %s * scale * (cosAngle - %s) * %s;",
                           color, cosOuter, coneScale);
    lightColorBody.appendf("}");
    lightColorBody.appendf("return %s * scale;", color);
    fLightColorFunc = fragBuilder->getMangledFunctionName("lightColor");
    fragBuilder->emitFunction(SkSLType::kHalf3,
                              fLightColorFunc.c_str(),
                              {gLightColorArgs, std::size(gLightColorArgs)},
                              lightColorBody.c_str());

    fragBuilder->codeAppendf("%s(%s)", fLightColorFunc.c_str(), surfaceToLight);
}

#endif

#else

#include "src/effects/imagefilters/SkCropImageFilter.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <utility>

struct SkISize;

namespace {
// The 3D points/vectors used for lighting don't have a great analog for the rest of the image
// filtering system, and don't have any representation for ParameterSpace and LayerSpace. The
// SVG spec is also vague on how to handle managing them. Using the principle of least-surprise,
// the X and Y coordinates are treated as ParameterSpace<SkPoint|Vector> and the Z will be
// scaled by the average of the X and Y scale factors when tranforming to layer space. For uniform
// scaling transforms, this has the desirable behavior of uniformly scaling the Z axis as well.
struct ZValue {
    ZValue() : fZ(0.f) {}
    ZValue(float z) : fZ(z) {}
    operator float() const { return fZ; }

    float fZ;
};
} // anonymous namespace

namespace skif {
template<>
class LayerSpace<ZValue> {
public:
    LayerSpace() = default;
    explicit LayerSpace(ZValue z) : fData(z) {}

    float val() const { return fData.fZ; }

    static LayerSpace<ZValue> Map(const Mapping& mapping, ParameterSpace<ZValue> z) {
        // See comment on ZValue for rationale.
        skif::LayerSpace<skif::Vector> z2d = mapping.paramToLayer(
                skif::ParameterSpace<skif::Vector>({ZValue(z), ZValue(z)}));
        return LayerSpace<ZValue>(SkScalarAve(z2d.x(), z2d.y()));
    }

private:
    ZValue fData;
};
} // namespace skif

namespace {

struct Light {
    enum class Type {
        kDistant,
        kPoint,
        kSpot,
        kLast = kSpot
    };

    Type fType;
    SkColor fLightColor; // All lights

    // Location and direction are decomposed into typed XY and Z for how they are transformed from
    // parameter space to layer space.
    skif::ParameterSpace<SkPoint> fLocationXY; // Spotlight and point lights only
    skif::ParameterSpace<ZValue>  fLocationZ;  //  ""

    skif::ParameterSpace<skif::Vector> fDirectionXY; // Spotlight and distant lights only
    skif::ParameterSpace<ZValue>       fDirectionZ;  //  ""

    // Spotlight only (and unchanged by layer matrix)
    float fFalloffExponent;
    float fCosCutoffAngle;

    static Light Point(SkColor color, const SkPoint3& location) {
        return {Type::kPoint,
                color,
                skif::ParameterSpace<SkPoint>({location.fX, location.fY}),
                skif::ParameterSpace<ZValue>(location.fZ),
                /*directionXY=*/{},
                /*directionZ=*/{},
                /*falloffExponent=*/0.f,
                /*cutoffAngle=*/0.f};
    }

    static Light Distant(SkColor color, const SkPoint3& direction) {
        return {Type::kDistant,
                color,
                /*locationXY=*/{},
                /*locationZ=*/{},
                skif::ParameterSpace<skif::Vector>({direction.fX, direction.fY}),
                skif::ParameterSpace<ZValue>(direction.fZ),
                /*falloffExponent=*/0.f,
                /*cutoffAngle=*/0.f};
    }

    static Light Spot(SkColor color, const SkPoint3& location, const SkPoint3& direction,
                      float falloffExponent, float cosCutoffAngle) {
        return {Type::kSpot,
                color,
                skif::ParameterSpace<SkPoint>({location.fX, location.fY}),
                skif::ParameterSpace<ZValue>(location.fZ),
                skif::ParameterSpace<skif::Vector>({direction.fX, direction.fY}),
                skif::ParameterSpace<ZValue>(direction.fZ),
                falloffExponent,
                cosCutoffAngle};
    }
};

struct Material {
    enum class Type {
        kDiffuse,
        kSpecular,
        kLast = kSpecular
    };

    Type fType;
    // The base scale factor applied to alpha image to go from [0-1] to [0-depth] before computing
    // surface normals.
    skif::ParameterSpace<ZValue> fSurfaceDepth;

    // Non-geometric
    float fK; // Reflectance coefficient
    float fShininess; // Specular only

    static Material Diffuse(float k, float surfaceDepth) {
        return {Type::kDiffuse, skif::ParameterSpace<ZValue>(surfaceDepth), k, 0.f};
    }

    static Material Specular(float k, float shininess, float surfaceDepth) {
        return {Type::kSpecular, skif::ParameterSpace<ZValue>(surfaceDepth), k, shininess};
    }
};

class SkLightingImageFilter final : public SkImageFilter_Base {
public:
    SkLightingImageFilter(const Light& light, const Material& material, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1, nullptr)
            , fLight(light)
            , fMaterial(material) {}

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterLightingImageFilterFlattenables();
    SK_FLATTENABLE_HOOKS(SkLightingImageFilter)
    static Light LegacyDeserializeLight(SkReadBuffer& buffer);
    static sk_sp<SkFlattenable> LegacyDiffuseCreateProc(SkReadBuffer& buffer);
    static sk_sp<SkFlattenable> LegacySpecularCreateProc(SkReadBuffer& buffer);

    bool onAffectsTransparentBlack() const override { return true; }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> requiredInput(const skif::LayerSpace<SkIRect>& desiredOutput) const {
        // We request 1px of padding so that the visible normal map can do a regular Sobel kernel
        // eval. The Sobel kernel is always applied in layer pixels
        skif::LayerSpace<SkIRect> requiredInput = desiredOutput;
        requiredInput.outset(skif::LayerSpace<SkISize>({1, 1}));
        return requiredInput;
    }

    Light fLight;
    Material fMaterial;
};

// Creates a shader that performs a Sobel filter on the alpha channel of the input image, using
// 'edgeBounds' to decide how to modify the kernel weights.
sk_sp<SkShader> make_normal_shader(sk_sp<SkShader> alphaMap,
                                   const skif::LayerSpace<SkIRect>& edgeBounds,
                                   skif::LayerSpace<ZValue> surfaceDepth) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader alphaMap;"
        "uniform float4 edgeBounds;"
        "uniform half surfaceDepth;"

        "half3 normal(half3 alphaC0, half3 alphaC1, half3 alphaC2) {"
            // The right column (or bottom row) terms of the Sobel filter. The left/top is just
            // the negative, and the middle row/column is all 0s so those instructions are skipped.
            "const half3 kSobel = 0.25 * half3(1,2,1);"
            "half3 alphaR0 = half3(alphaC0.x, alphaC1.x, alphaC2.x);"
            "half3 alphaR2 = half3(alphaC0.z, alphaC1.z, alphaC2.z);"
            "half nx = dot(kSobel, alphaC2) - dot(kSobel, alphaC0);"
            "half ny = dot(kSobel, alphaR2) - dot(kSobel, alphaR0);"
            "return normalize(half3(-surfaceDepth*half2(nx, ny), 1));"
        "}"

        "half4 main(float2 coord) {"
            "half3 alphaC0 = half3("
                "alphaMap.eval(clamp(coord + float2(-1,-1), edgeBounds.LT, edgeBounds.RB)).a,"
                "alphaMap.eval(clamp(coord + float2(-1, 0), edgeBounds.LT, edgeBounds.RB)).a,"
                "alphaMap.eval(clamp(coord + float2(-1, 1), edgeBounds.LT, edgeBounds.RB)).a);"
            "half3 alphaC1 = half3("
                "alphaMap.eval(clamp(coord + float2( 0,-1), edgeBounds.LT, edgeBounds.RB)).a,"
                "alphaMap.eval(clamp(coord + float2( 0, 0), edgeBounds.LT, edgeBounds.RB)).a,"
                "alphaMap.eval(clamp(coord + float2( 0, 1), edgeBounds.LT, edgeBounds.RB)).a);"
            "half3 alphaC2 = half3("
                "alphaMap.eval(clamp(coord + float2( 1,-1), edgeBounds.LT, edgeBounds.RB)).a,"
                "alphaMap.eval(clamp(coord + float2( 1, 0), edgeBounds.LT, edgeBounds.RB)).a,"
                "alphaMap.eval(clamp(coord + float2( 1, 1), edgeBounds.LT, edgeBounds.RB)).a);"

            "half mainAlpha = alphaC1.y;" // offset = (0,0)
            "return half4(normal(alphaC0, alphaC1, alphaC2), mainAlpha);"
        "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("alphaMap") = std::move(alphaMap);
    builder.uniform("edgeBounds") = SkRect::Make(SkIRect(edgeBounds)).makeInset(0.5f, 0.5f);
    builder.uniform("surfaceDepth") = surfaceDepth.val();

    return builder.makeShader();
}

sk_sp<SkShader> make_lighting_shader(sk_sp<SkShader> normalMap,
                                     Light::Type lightType,
                                     SkColor lightColor,
                                     skif::LayerSpace<SkPoint> locationXY,
                                     skif::LayerSpace<ZValue> locationZ,
                                     skif::LayerSpace<skif::Vector> directionXY,
                                     skif::LayerSpace<ZValue> directionZ,
                                     float falloffExponent,
                                     float cosCutoffAngle,
                                     Material::Type matType,
                                     skif::LayerSpace<ZValue> surfaceDepth,
                                     float k,
                                     float shininess) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "const half kConeAAThreshold = 0.016;"
        "const half kConeScale = 1.0 / kConeAAThreshold;"

        "uniform shader normalMap;"

        // Pack's surface depth, shininess, material type (0 == diffuse) and light type
        // (< 0 = distant, 0 = point, > 0 = spot)
        "uniform half4 materialAndLightType;"

        "uniform half4 lightPosAndSpotFalloff;" // (x,y,z) are lightPos, w is spot falloff exponent
        "uniform half4 lightDirAndSpotCutoff;" // (x,y,z) are lightDir, w is spot cos(cutoffAngle)
        "uniform half3 lightColor;" // Material's k has already been multipled in

        "half3 surface_to_light(half3 coord) {"
            "if (materialAndLightType.w < 0) {"
                "return lightDirAndSpotCutoff.xyz;"
            "} else {"
                // Spot and point have the same equation
                "return normalize(lightPosAndSpotFalloff.xyz - coord);"
            "}"
        "}"

        "half spotlight_scale(half3 surfaceToLight) {"
            "half cosCutoffAngle = lightDirAndSpotCutoff.w;"
            "half cosAngle = -dot(surfaceToLight, lightDirAndSpotCutoff.xyz);"
            "if (cosAngle < cosCutoffAngle) {"
                "return 0.0;"
            "}"
            "half scale = pow(cosAngle, lightPosAndSpotFalloff.w);"
            "if (cosAngle < cosCutoffAngle + kConeAAThreshold) {"
                "return scale * (cosAngle - cosCutoffAngle) * kConeScale;"
            "} else {"
                "return scale;"
            "}"
        "}"

        "half4 compute_lighting(half3 normal, half3 surfaceToLight) {"
            // Point and distant light color contributions are constant
            "half3 color = lightColor;"
            // Spot lights fade based on the angle away from its direction
            "if (materialAndLightType.w > 0) {"
                "color *= spotlight_scale(surfaceToLight);"
            "}"

            // Diffuse and specular reflections scale the light's "color" differently
            "if (materialAndLightType.z == 0) {"
                "half coeff = dot(normal, surfaceToLight);"
                "color = saturate(coeff * color);"
                "return half4(color, 1.0);"
            "} else {"
                "half3 halfDir = normalize(surfaceToLight + half3(0, 0, 1));"
                "half shininess = materialAndLightType.y;"
                "half coeff = pow(dot(normal, halfDir), shininess);"
                "color = saturate(coeff * color);"
                "return half4(color, max(max(color.r, color.g), color.b));"
            "}"
        "}"

        "half4 main(float2 coord) {"
            "half4 normalAndA = normalMap.eval(coord);"
            "half depth = materialAndLightType.x;"
            "half3 surfaceToLight = surface_to_light(half3(half2(coord), depth*normalAndA.a));"
            "return compute_lighting(normalAndA.xyz, surfaceToLight);"
        "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("normalMap") = std::move(normalMap);

    builder.uniform("materialAndLightType") =
            SkV4{surfaceDepth.val(),
                 shininess,
                 matType == Material::Type::kDiffuse ? 0.f : 1.f,
                 lightType == Light::Type::kPoint ?
                         0.f : (lightType == Light::Type::kDistant ? -1.f : 1.f)};
    builder.uniform("lightPosAndSpotFalloff") =
            SkV4{locationXY.x(), locationXY.y(), locationZ.val(), falloffExponent};

    // Pre-normalize the light direction, but this can be (0,0,0) for point lights, which won't use
    // the uniform anyways. Avoid a division by 0 to keep ASAN happy or in the event that a spot/dir
    // light have bad user input.
    SkV3 dir{directionXY.x(), directionXY.y(), directionZ.val()};
    float invDirLen = dir.length();
    invDirLen = invDirLen ? 1.0f / invDirLen : 0.f;
    builder.uniform("lightDirAndSpotCutoff") =
            SkV4{invDirLen*dir.x, invDirLen*dir.y, invDirLen*dir.z, cosCutoffAngle};

    // Historically, the Skia lighting image filter did not apply any color space transformation to
    // the light's color. The SVG spec for the lighting effects does not stipulate how to interpret
    // the color for a light. Overall, it does not have a principled physically based approach, but
    // the closest way to interpret it, is:
    //  - the material's K is a uniformly distributed reflectance coefficient
    //  - lighting *should* be calculated in a linear color space, which is the default for SVG
    //    filters. Chromium manages these color transformations using SkImageFilters::ColorFilter
    //    so it's not necessarily reflected in the Context's color space.
    //  - it's unspecified in the SVG spec if the light color should be transformed to linear or
    //    interpreted as linear already. Regardless, if there was any transformation that needed to
    //    occur, Blink took care of it in the past so adding color space management to the light
    //    color would be a breaking change.
    //  - so for now, leave the color un-modified and apply K up front since no color space
    //    transforms need to be performed on the original light color.
    const float colorScale = k / 255.f;
    builder.uniform("lightColor") = SkV3{SkColorGetR(lightColor) * colorScale,
                                         SkColorGetG(lightColor) * colorScale,
                                         SkColorGetB(lightColor) * colorScale};

    return builder.makeShader();
}

sk_sp<SkImageFilter> make_lighting(const Light& light,
                                   const Material& material,
                                   sk_sp<SkImageFilter> input,
                                   const SkImageFilters::CropRect& cropRect) {
    // According to the spec, ks and kd can be any non-negative number:
    // http://www.w3.org/TR/SVG/filters.html#feSpecularLightingElement
    if (!SkScalarIsFinite(material.fK) || material.fK < 0.f ||
        !SkScalarIsFinite(material.fShininess) ||
        !SkScalarIsFinite(ZValue(material.fSurfaceDepth))) {
        return nullptr;
    }

    // Ensure light values are finite, and the cosine should be between -1 and 1
    if (!SkPoint(light.fLocationXY).isFinite() ||
        !SkScalarIsFinite(ZValue(light.fLocationZ)) ||
        !skif::Vector(light.fDirectionXY).isFinite() ||
        !SkScalarIsFinite(ZValue(light.fDirectionZ)) ||
        !SkScalarIsFinite(light.fFalloffExponent) ||
        !SkScalarIsFinite(light.fCosCutoffAngle) ||
        light.fCosCutoffAngle < -1.f || light.fCosCutoffAngle > 1.f) {
        return nullptr;
    }

    // If a crop rect is provided, it clamps both the input (to better match the SVG's normal
    // boundary condition spec) and the output (because otherwise it has infinite bounds).
    sk_sp<SkImageFilter> filter = std::move(input);
    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    filter = sk_sp<SkImageFilter>(
            new SkLightingImageFilter(light, material, std::move(filter)));
    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    return filter;
}

} // anonymous namespace

sk_sp<SkImageFilter> SkImageFilters::DistantLitDiffuse(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Distant(lightColor, direction),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitDiffuse(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Point(lightColor, location),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitDiffuse(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    SkPoint3 dir = target - location;
    float cosCutoffAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
    return make_lighting(Light::Spot(lightColor, location, dir, falloffExponent, cosCutoffAngle),
                         Material::Diffuse(kd, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DistantLitSpecular(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Distant(lightColor, direction),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitSpecular(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_lighting(Light::Point(lightColor, location),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitSpecular(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    SkPoint3 dir = target - location;
    float cosCutoffAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
    return make_lighting(Light::Spot(lightColor, location, dir, falloffExponent, cosCutoffAngle),
                         Material::Specular(ks, shininess, surfaceScale),
                         std::move(input), cropRect);
}

void SkRegisterLightingImageFilterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkLightingImageFilter);
    // TODO (michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDiffuseLightingImageFilter",
                            SkLightingImageFilter::LegacyDiffuseCreateProc);
    SkFlattenable::Register("SkSpecularLightingImageFilter",
                            SkLightingImageFilter::LegacySpecularCreateProc);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light;
    light.fType = buffer.read32LE(Light::Type::kLast);
    light.fLightColor = buffer.readColor();

    SkPoint3 lightPos, lightDir;
    buffer.readPoint3(&lightPos);
    light.fLocationXY = skif::ParameterSpace<SkPoint>({lightPos.fX, lightPos.fY});
    light.fLocationZ = skif::ParameterSpace<ZValue>(lightPos.fZ);

    buffer.readPoint3(&lightDir);
    light.fDirectionXY = skif::ParameterSpace<skif::Vector>({lightDir.fX, lightDir.fY});
    light.fDirectionZ = skif::ParameterSpace<ZValue>(lightDir.fZ);

    light.fFalloffExponent = buffer.readScalar();
    light.fCosCutoffAngle = buffer.readScalar();

    Material material;
    material.fType = buffer.read32LE(Material::Type::kLast);
    material.fSurfaceDepth = skif::ParameterSpace<ZValue>(buffer.readScalar());
    material.fK = buffer.readScalar();
    material.fShininess = buffer.readScalar();

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

Light SkLightingImageFilter::LegacyDeserializeLight(SkReadBuffer& buffer) {
    // Light::Type has the same order as the legacy SkImageFilterLight::LightType enum
    Light::Type lightType = buffer.read32LE(Light::Type::kLast);
    if (!buffer.isValid()) {
        return {};
    }

    // Legacy lights stored just the RGB, but as floats (notably *not* normalized to [0-1])
    SkColor lightColor = SkColorSetARGB(/*a (ignored)=*/255,
                                        /*r=*/ (U8CPU) buffer.readScalar(),
                                        /*g=*/ (U8CPU) buffer.readScalar(),
                                        /*b=*/ (U8CPU) buffer.readScalar());
    // Legacy lights only serialized fields specific to that type
    switch (lightType) {
        case Light::Type::kDistant: {
            SkPoint3 dir = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            return Light::Distant(lightColor, dir);
        }
        case Light::Type::kPoint: {
            SkPoint3 loc = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            return Light::Point(lightColor, loc);
        }
        case Light::Type::kSpot: {
            SkPoint3 loc = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            SkPoint3 target = {buffer.readScalar(), buffer.readScalar(), buffer.readScalar()};
            float falloffExponent = buffer.readScalar();
            float cosOuterConeAngle = buffer.readScalar();
            buffer.readScalar(); // skip cosInnerConeAngle, derived from outer cone angle
            buffer.readScalar(); // skip coneScale, which is a constant
            buffer.readScalar(); // skip S, which is normalize(target - loc)
            buffer.readScalar(); //  ""
            buffer.readScalar(); //  ""
            return Light::Spot(lightColor, loc, target - loc, falloffExponent, cosOuterConeAngle);
        }
    }

    SkUNREACHABLE; // Validation by read32LE() should avoid this
}

sk_sp<SkFlattenable> SkLightingImageFilter::LegacyDiffuseCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light = LegacyDeserializeLight(buffer);

    // Legacy implementations used (scale/255) when filtering, but serialized (fScale*255) so the
    // buffer held the original unmodified surface scale.
    float surfaceScale = buffer.readScalar();
    float kd = buffer.readScalar();
    Material material = Material::Diffuse(kd, surfaceScale);

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

sk_sp<SkFlattenable> SkLightingImageFilter::LegacySpecularCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light = LegacyDeserializeLight(buffer);

    // Legacy implementations used (scale/255) when filtering, but serialized (fScale*255) so the
    // buffer held the original unmodified surface scale.
    float surfaceScale = buffer.readScalar();
    float ks = buffer.readScalar();
    float shininess = buffer.readScalar();
    Material material = Material::Specular(ks, shininess, surfaceScale);

    return make_lighting(light, material, common.getInput(0), common.cropRect());
}

void SkLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);

    // Light
    buffer.writeInt((int) fLight.fType);
    buffer.writeColor(fLight.fLightColor);

    buffer.writePoint(SkPoint(fLight.fLocationXY));
    buffer.writeScalar(ZValue(fLight.fLocationZ));

    skif::Vector dirXY{fLight.fDirectionXY};
    buffer.writePoint(SkPoint{dirXY.fX, dirXY.fY});
    buffer.writeScalar(ZValue(fLight.fDirectionZ));

    buffer.writeScalar(fLight.fFalloffExponent);
    buffer.writeScalar(fLight.fCosCutoffAngle);

    // Material
    buffer.writeInt((int) fMaterial.fType);
    buffer.writeScalar(ZValue(fMaterial.fSurfaceDepth));
    buffer.writeScalar(fMaterial.fK);
    buffer.writeScalar(fMaterial.fShininess);
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkLightingImageFilter::onFilterImage(const skif::Context& ctx) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    auto mapZToLayer = [&ctx](skif::ParameterSpace<ZValue> z) {
        return skif::LayerSpace<ZValue>::Map(ctx.mapping(), z);
    };

    // Map lighting and material parameters into layer space
    skif::LayerSpace<ZValue> surfaceDepth = mapZToLayer(fMaterial.fSurfaceDepth);
    skif::LayerSpace<SkPoint> lightLocationXY = ctx.mapping().paramToLayer(fLight.fLocationXY);
    skif::LayerSpace<ZValue> lightLocationZ = mapZToLayer(fLight.fLocationZ);
    skif::LayerSpace<skif::Vector> lightDirXY = ctx.mapping().paramToLayer(fLight.fDirectionXY);
    skif::LayerSpace<ZValue> lightDirZ = mapZToLayer(fLight.fDirectionZ);

    // The normal map is determined by a 3x3 kernel, so we request a 1px outset of what should be
    // filled by the lighting equation. Ideally this means there are no boundary conditions visible.
    // If the required input is incomplete, the lighting filter handles the boundaries in two ways:
    // - When the actual child output's edge matches the desired output's edge, it uses clamped
    //   tiling at the desired output. This approximates the modified Sobel kernel's specified in
    //   https://drafts.fxtf.org/filter-effects/#feDiffuseLightingElement. NOTE: It's identical to
    //   the interior kernel and near equal on the 4 edges (only weights are biased differently).
    //   The four corners' convolution sums with clamped tiling are not equal, but should not be
    //   objectionable since the normals produced are reasonable and still further processed by the
    //   lighting equation. The increased complexity is not worth it for just 4 pixels of output.
    // - However, when the desired output is far larger than the produced image, we process the
    //   child output with the default decal tiling that the Skia image filter pipeline relies on.
    //   This creates a visual bevel at the image boundary but avoids producing streaked normals if
    //   the clamped tiling was used in all scenarios.
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(ctx.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, ctx.withNewDesiredOutput(requiredInput));

    skif::LayerSpace<SkIRect> clampRect = requiredInput; // effectively no clamping of normals
    if (!childOutput.layerBounds().contains(requiredInput)) {
        // Adjust clampRect edges to desiredOutput if the actual child output matched the lighting
        // output size (typical SVG case). Otherwise leave coordinates alone to use decal tiling
        // automatically for the pixels outside the child image but inside the desired output.
        auto edgeClamp = [](int actualEdgeValue, int requestedEdgeValue, int outputEdge) {
            return actualEdgeValue == outputEdge ? outputEdge : requestedEdgeValue;
        };
        auto inputRect = childOutput.layerBounds();
        auto clampTo = ctx.desiredOutput();
        clampRect = skif::LayerSpace<SkIRect>({
                edgeClamp(inputRect.left(),   requiredInput.left(),   clampTo.left()),
                edgeClamp(inputRect.top(),    requiredInput.top(),    clampTo.top()),
                edgeClamp(inputRect.right(),  requiredInput.right(),  clampTo.right()),
                edgeClamp(inputRect.bottom(), requiredInput.bottom(), clampTo.bottom())});
    }

    skif::FilterResult::Builder builder{ctx};
    builder.add(childOutput, /*sampleBounds=*/clampRect);
    return builder.eval([&](SkSpan<sk_sp<SkShader>> input) {
        // TODO: Once shaders are deferred in FilterResult, it will likely make sense to have an
        // internal normal map filter that uses this shader, and then have the lighting effects as
        // a separate filter. It's common for multiple lights to use the same input (producing the
        // same normal map) before being merged together. With a separate normal image filter, its
        // output would be automatically cached, and the lighting equation shader would be deferred
        // to the merge's draw operation, making for a maximum of 2 renderpasses instead of N+1.
        sk_sp<SkShader> normals = make_normal_shader(std::move(input[0]), clampRect, surfaceDepth);
        return make_lighting_shader(std::move(normals),
                                    // Light in layer space
                                    fLight.fType,
                                    fLight.fLightColor,
                                    lightLocationXY,
                                    lightLocationZ,
                                    lightDirXY,
                                    lightDirZ,
                                    fLight.fFalloffExponent,
                                    fLight.fCosCutoffAngle,
                                    // Material in layer space
                                    fMaterial.fType,
                                    surfaceDepth,
                                    fMaterial.fK,
                                    fMaterial.fShininess);
    }, ShaderFlags::kNone);
}

skif::LayerSpace<SkIRect> SkLightingImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

skif::LayerSpace<SkIRect> SkLightingImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The lighting equation is defined on the entire plane, even if the input image that defines
    // the normal map is bounded. It just is evaluated at a constant normal vector, which can still
    // produce non-constant color since the direction to the eye and light change per pixel.
    return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
}

SkRect SkLightingImageFilter::computeFastBounds(const SkRect& src) const {
    return SkRectPriv::MakeLargeS32();
}

#else // SK_ENABLE_SKSL

// Without SkSL, just return the input image filter (possibly cropped)

sk_sp<SkImageFilter> SkImageFilters::DistantLitDiffuse(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

sk_sp<SkImageFilter> SkImageFilters::PointLitDiffuse(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitDiffuse(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

sk_sp<SkImageFilter> SkImageFilters::DistantLitSpecular(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

sk_sp<SkImageFilter> SkImageFilters::PointLitSpecular(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitSpecular(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

void SkRegisterLightingImageFilterFlattenables() {}

#endif // SK_ENABLE_SKSL

#endif // SK_USE_LEGACY_LIGHTING_IMAGEFILTER

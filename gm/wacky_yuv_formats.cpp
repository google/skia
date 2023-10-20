/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/utils/SkTextUtils.h"
#include "src/base/SkHalf.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkYUVMath.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "tools/DecodeUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/YUVUtils.h"

#include <math.h>
#include <string.h>
#include <initializer_list>
#include <memory>
#include <utility>

static const int kTileWidthHeight = 128;
static const int kLabelWidth = 64;
static const int kLabelHeight = 32;
static const int kSubsetPadding = 8;
static const int kPad = 1;

using Recorder = skgpu::graphite::Recorder;

enum YUVFormat {
    // 4:2:0 formats, 24 bpp
    kP016_YUVFormat, // 16-bit Y plane + 2x2 down sampled interleaved U/V plane (2 textures)
    // 4:2:0 formats, "15 bpp" (but really 24 bpp)
    kP010_YUVFormat, // same as kP016 except "10 bpp". Note that it is the same memory layout
                     // except that the bottom 6 bits are zeroed out (2 textures)
    // TODO: we're cheating a bit w/ P010 and just treating it as unorm 16. This means its
    // fully saturated values are 65504 rather than 65535 (that is just .9995 out of 1.0 though).

    // This is laid out the same as kP016 and kP010 but uses F16 unstead of U16. In this case
    // the 10 bits/channel vs 16 bits/channel distinction isn't relevant.
    kP016F_YUVFormat,

    // 4:4:4 formats, 64 bpp
    kY416_YUVFormat,  // 16-bit AVYU values all interleaved (1 texture)

    // 4:4:4 formats, 32 bpp
    kAYUV_YUVFormat,  // 8-bit YUVA values all interleaved (1 texture)
    kY410_YUVFormat,  // AVYU w/ 10bpp for YUV and 2 for A all interleaved (1 texture)

    // 4:2:0 formats, 12 bpp
    kNV12_YUVFormat, // 8-bit Y plane + 2x2 down sampled interleaved U/V planes (2 textures)
    kNV21_YUVFormat, // same as kNV12 but w/ U/V reversed in the interleaved texture (2 textures)

    kI420_YUVFormat, // 8-bit Y plane + separate 2x2 down sampled U and V planes (3 textures)
    kYV12_YUVFormat, // 8-bit Y plane + separate 2x2 down sampled V and U planes (3 textures)

    kLast_YUVFormat = kYV12_YUVFormat
};

// Does the YUVFormat contain a slot for alpha? If not an external alpha plane is required for
// transparency.
static bool has_alpha_channel(YUVFormat format) {
    switch (format) {
        case kP016_YUVFormat:  return false;
        case kP010_YUVFormat:  return false;
        case kP016F_YUVFormat: return false;
        case kY416_YUVFormat:  return true;
        case kAYUV_YUVFormat:  return true;
        case kY410_YUVFormat:  return true;
        case kNV12_YUVFormat:  return false;
        case kNV21_YUVFormat:  return false;
        case kI420_YUVFormat:  return false;
        case kYV12_YUVFormat:  return false;
    }
    SkUNREACHABLE;
}

class YUVAPlanarConfig {
public:
    YUVAPlanarConfig(YUVFormat format, bool opaque, SkEncodedOrigin origin) : fOrigin(origin) {
        switch (format) {
            case kP016_YUVFormat:
            case kP010_YUVFormat:
            case kP016F_YUVFormat:
            case kNV12_YUVFormat:
                if (opaque) {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_UV;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                } else {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_UV_A;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                }
                break;
            case kY416_YUVFormat:
            case kY410_YUVFormat:
                if (opaque) {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kUYV;
                    fSubsampling = SkYUVAInfo::Subsampling::k444;
                } else {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kUYVA;
                    fSubsampling = SkYUVAInfo::Subsampling::k444;
                }
                break;
            case kAYUV_YUVFormat:
                if (opaque) {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kYUV;
                    fSubsampling = SkYUVAInfo::Subsampling::k444;
                } else {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kYUVA;
                    fSubsampling = SkYUVAInfo::Subsampling::k444;
                }
                break;
            case kNV21_YUVFormat:
                if (opaque) {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_VU;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                } else {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_VU_A;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                }
                break;
            case kI420_YUVFormat:
                if (opaque) {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_U_V;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                } else {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_U_V_A;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                }
                break;
            case kYV12_YUVFormat:
                if (opaque) {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_V_U;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                } else {
                    fPlaneConfig = SkYUVAInfo::PlaneConfig::kY_V_U_A;
                    fSubsampling = SkYUVAInfo::Subsampling::k420;
                }
                break;
        }
    }

    int numPlanes() const { return SkYUVAInfo::NumPlanes(fPlaneConfig); }

    SkYUVAPixmaps makeYUVAPixmaps(SkISize dimensions,
                                  SkYUVColorSpace yuvColorSpace,
                                  const SkBitmap bitmaps[],
                                  int numBitmaps) const;

private:
    SkYUVAInfo::PlaneConfig fPlaneConfig;
    SkYUVAInfo::Subsampling fSubsampling;
    SkEncodedOrigin         fOrigin;
};

SkYUVAPixmaps YUVAPlanarConfig::makeYUVAPixmaps(SkISize dimensions,
                                                SkYUVColorSpace yuvColorSpace,
                                                const SkBitmap bitmaps[],
                                                int numBitmaps) const {
    SkYUVAInfo info(dimensions, fPlaneConfig, fSubsampling, yuvColorSpace, fOrigin);
    SkPixmap pmaps[SkYUVAInfo::kMaxPlanes];
    int n = info.numPlanes();
    if (numBitmaps < n) {
        return {};
    }
    for (int i = 0; i < n; ++i) {
        pmaps[i] = bitmaps[i].pixmap();
    }
    return SkYUVAPixmaps::FromExternalPixmaps(info, pmaps);
}

// All the planes we need to construct the various YUV formats
struct PlaneData {
   SkBitmap fYFull;
   SkBitmap fUFull;
   SkBitmap fVFull;
   SkBitmap fAFull;
   SkBitmap fUQuarter; // 2x2 downsampled U channel
   SkBitmap fVQuarter; // 2x2 downsampled V channel

   SkBitmap fFull;
   SkBitmap fQuarter; // 2x2 downsampled YUVA
};

// Add a portion of a circle to 'path'. The points 'o1' and 'o2' are on the border of the circle
// and have tangents 'v1' and 'v2'.
static void add_arc(SkPath* path,
                    const SkPoint& o1, const SkVector& v1,
                    const SkPoint& o2, const SkVector& v2,
                    SkTDArray<SkRect>* circles, bool takeLongWayRound) {

    SkVector v3 = { -v1.fY, v1.fX };
    SkVector v4 = { v2.fY, -v2.fX };

    SkScalar t = ((o2.fX - o1.fX) * v4.fY - (o2.fY - o1.fY) * v4.fX) / v3.cross(v4);
    SkPoint center = { o1.fX + t * v3.fX, o1.fY + t * v3.fY };

    SkRect r = { center.fX - t, center.fY - t, center.fX + t, center.fY + t };

    if (circles) {
        circles->push_back(r);
    }

    SkVector startV = o1 - center, endV = o2 - center;
    startV.normalize();
    endV.normalize();

    SkScalar startDeg = SkRadiansToDegrees(SkScalarATan2(startV.fY, startV.fX));
    SkScalar endDeg = SkRadiansToDegrees(SkScalarATan2(endV.fY, endV.fX));

    startDeg += 360.0f;
    startDeg = fmodf(startDeg, 360.0f);

    endDeg += 360.0f;
    endDeg = fmodf(endDeg, 360.0f);

    if (endDeg < startDeg) {
        endDeg += 360.0f;
    }

    SkScalar sweepDeg = SkTAbs(endDeg - startDeg);
    if (!takeLongWayRound) {
        sweepDeg = sweepDeg - 360;
    }

    path->arcTo(r, startDeg, sweepDeg, false);
}

static SkPath create_splat(const SkPoint& o, SkScalar innerRadius, SkScalar outerRadius,
                           SkScalar ratio, int numLobes, SkTDArray<SkRect>* circles) {
    if (numLobes <= 1) {
        return SkPath();
    }

    SkPath p;

    int numDivisions = 2 * numLobes;
    SkScalar fullLobeDegrees = 360.0f / numLobes;
    SkScalar outDegrees = ratio * fullLobeDegrees / (ratio + 1.0f);
    SkScalar innerDegrees = fullLobeDegrees / (ratio + 1.0f);
    SkMatrix outerStep, innerStep;
    outerStep.setRotate(outDegrees);
    innerStep.setRotate(innerDegrees);
    SkVector curV = SkVector::Make(0.0f, 1.0f);

    if (circles) {
        circles->push_back(SkRect::MakeLTRB(o.fX - innerRadius, o.fY - innerRadius,
                                            o.fX + innerRadius, o.fY + innerRadius));
    }

    p.moveTo(o.fX + innerRadius * curV.fX, o.fY + innerRadius * curV.fY);

    for (int i = 0; i < numDivisions; ++i) {

        SkVector nextV;
        if (0 == (i % 2)) {
            nextV = outerStep.mapVector(curV.fX, curV.fY);

            SkPoint top = SkPoint::Make(o.fX + outerRadius * curV.fX,
                                        o.fY + outerRadius * curV.fY);
            SkPoint nextTop = SkPoint::Make(o.fX + outerRadius * nextV.fX,
                                            o.fY + outerRadius * nextV.fY);

            p.lineTo(top);
            add_arc(&p, top, curV, nextTop, nextV, circles, true);
        } else {
            nextV = innerStep.mapVector(curV.fX, curV.fY);

            SkPoint bot = SkPoint::Make(o.fX + innerRadius * curV.fX,
                                        o.fY + innerRadius * curV.fY);
            SkPoint nextBot = SkPoint::Make(o.fX + innerRadius * nextV.fX,
                                            o.fY + innerRadius * nextV.fY);

            p.lineTo(bot);
            add_arc(&p, bot, curV, nextBot, nextV, nullptr, false);
        }

        curV = nextV;
    }

    p.close();

    return p;
}

static SkBitmap make_bitmap(SkColorType colorType, const SkPath& path,
                            const SkTDArray<SkRect>& circles, bool opaque, bool padWithRed) {
    const SkColor kGreen   = ToolUtils::color_to_565(SkColorSetARGB(0xFF, 178, 240, 104));
    const SkColor kBlue    = ToolUtils::color_to_565(SkColorSetARGB(0xFF, 173, 167, 252));
    const SkColor kYellow  = ToolUtils::color_to_565(SkColorSetARGB(0xFF, 255, 221, 117));
    const SkColor kMagenta = ToolUtils::color_to_565(SkColorSetARGB(0xFF, 255,  60, 217));
    const SkColor kCyan    = ToolUtils::color_to_565(SkColorSetARGB(0xFF,  45, 237, 205));

    int widthHeight = kTileWidthHeight + (padWithRed ? 2 * kSubsetPadding : 0);

    SkImageInfo ii = SkImageInfo::Make(widthHeight, widthHeight,
                                       colorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(ii,
                                                                  bm.getPixels(),
                                                                  bm.rowBytes());
    if (padWithRed) {
        canvas->clear(SK_ColorRED);
        canvas->translate(kSubsetPadding, kSubsetPadding);
        canvas->clipRect(SkRect::MakeWH(kTileWidthHeight, kTileWidthHeight));
    }
    canvas->clear(opaque ? kGreen : SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setAntiAlias(false); // serialize-8888 doesn't seem to work well w/ partial transparency
    paint.setColor(kBlue);

    canvas->drawPath(path, paint);

    paint.setBlendMode(SkBlendMode::kSrc);
    for (int i = 0; i < circles.size(); ++i) {
        SkColor color;
        switch (i % 3) {
            case 0:  color = kYellow;  break;
            case 1:  color = kMagenta; break;
            default: color = kCyan;    break;
        }
        paint.setColor(color);
        paint.setAlpha(opaque ? 0xFF : 0x40);
        SkRect r = circles[i];
        r.inset(r.width()/4, r.height()/4);
        canvas->drawOval(r, paint);
    }

    return bm;
}

static void convert_rgba_to_yuva(const float mtx[20], SkColor col, uint8_t yuv[4]) {
    const uint8_t r = SkColorGetR(col);
    const uint8_t g = SkColorGetG(col);
    const uint8_t b = SkColorGetB(col);

    yuv[0] = SkTPin(SkScalarRoundToInt(mtx[ 0]*r + mtx[ 1]*g + mtx[ 2]*b + mtx[ 4]*255), 0, 255);
    yuv[1] = SkTPin(SkScalarRoundToInt(mtx[ 5]*r + mtx[ 6]*g + mtx[ 7]*b + mtx[ 9]*255), 0, 255);
    yuv[2] = SkTPin(SkScalarRoundToInt(mtx[10]*r + mtx[11]*g + mtx[12]*b + mtx[14]*255), 0, 255);
    yuv[3] = SkColorGetA(col);
}

static void extract_planes(const SkBitmap& origBM,
                           SkYUVColorSpace yuvColorSpace,
                           SkEncodedOrigin origin,
                           PlaneData* planes) {
    SkImageInfo ii = origBM.info();
    if (SkEncodedOriginSwapsWidthHeight(origin)) {
        ii = ii.makeWH(ii.height(), ii.width());
    }
    SkBitmap orientedBM;
    orientedBM.allocPixels(ii);
    SkCanvas canvas(orientedBM);
    SkMatrix matrix = SkEncodedOriginToMatrix(origin, origBM.width(), origBM.height());
    SkAssertResult(matrix.invert(&matrix));
    canvas.concat(matrix);
    canvas.drawImage(origBM.asImage(), 0, 0);

    if (yuvColorSpace == kIdentity_SkYUVColorSpace) {
        // To test the identity color space we use JPEG YUV planes
        yuvColorSpace = kJPEG_SkYUVColorSpace;
    }

    SkASSERT(!(ii.width() % 2));
    SkASSERT(!(ii.height() % 2));
    planes->fYFull.allocPixels(
            SkImageInfo::Make(ii.dimensions(), kGray_8_SkColorType, kUnpremul_SkAlphaType));
    planes->fUFull.allocPixels(
            SkImageInfo::Make(ii.dimensions(), kGray_8_SkColorType, kUnpremul_SkAlphaType));
    planes->fVFull.allocPixels(
            SkImageInfo::Make(ii.dimensions(), kGray_8_SkColorType, kUnpremul_SkAlphaType));
    planes->fAFull.allocPixels(SkImageInfo::MakeA8(ii.dimensions()));
    planes->fUQuarter.allocPixels(SkImageInfo::Make(ii.width()/2, ii.height()/2,
                                  kGray_8_SkColorType, kUnpremul_SkAlphaType));
    planes->fVQuarter.allocPixels(SkImageInfo::Make(ii.width()/2, ii.height()/2,
                                  kGray_8_SkColorType, kUnpremul_SkAlphaType));

    planes->fFull.allocPixels(
            SkImageInfo::Make(ii.dimensions(), kRGBA_F32_SkColorType, kUnpremul_SkAlphaType));
    planes->fQuarter.allocPixels(SkImageInfo::Make(ii.width()/2, ii.height()/2,
                                 kRGBA_F32_SkColorType, kUnpremul_SkAlphaType));

    float mtx[20];
    SkColorMatrix_RGB2YUV(yuvColorSpace, mtx);

    SkColor4f* dst = (SkColor4f *) planes->fFull.getAddr(0, 0);
    for (int y = 0; y < orientedBM.height(); ++y) {
        for (int x = 0; x < orientedBM.width(); ++x) {
            SkColor col = orientedBM.getColor(x, y);

            uint8_t yuva[4];

            convert_rgba_to_yuva(mtx, col, yuva);

            *planes->fYFull.getAddr8(x, y) = yuva[0];
            *planes->fUFull.getAddr8(x, y) = yuva[1];
            *planes->fVFull.getAddr8(x, y) = yuva[2];
            *planes->fAFull.getAddr8(x, y) = yuva[3];

            // TODO: render in F32 rather than converting here
            dst->fR = yuva[0] / 255.0f;
            dst->fG = yuva[1] / 255.0f;
            dst->fB = yuva[2] / 255.0f;
            dst->fA = yuva[3] / 255.0f;
            ++dst;
        }
    }

    dst = (SkColor4f *) planes->fQuarter.getAddr(0, 0);
    for (int y = 0; y < orientedBM.height()/2; ++y) {
        for (int x = 0; x < orientedBM.width()/2; ++x) {
            uint32_t yAccum = 0, uAccum = 0, vAccum = 0, aAccum = 0;

            yAccum += *planes->fYFull.getAddr8(2*x, 2*y);
            yAccum += *planes->fYFull.getAddr8(2*x+1, 2*y);
            yAccum += *planes->fYFull.getAddr8(2*x, 2*y+1);
            yAccum += *planes->fYFull.getAddr8(2*x+1, 2*y+1);

            uAccum += *planes->fUFull.getAddr8(2*x, 2*y);
            uAccum += *planes->fUFull.getAddr8(2*x+1, 2*y);
            uAccum += *planes->fUFull.getAddr8(2*x, 2*y+1);
            uAccum += *planes->fUFull.getAddr8(2*x+1, 2*y+1);

            *planes->fUQuarter.getAddr8(x, y) = uAccum / 4.0f;

            vAccum += *planes->fVFull.getAddr8(2*x, 2*y);
            vAccum += *planes->fVFull.getAddr8(2*x+1, 2*y);
            vAccum += *planes->fVFull.getAddr8(2*x, 2*y+1);
            vAccum += *planes->fVFull.getAddr8(2*x+1, 2*y+1);

            *planes->fVQuarter.getAddr8(x, y) = vAccum / 4.0f;

            aAccum += *planes->fAFull.getAddr8(2*x, 2*y);
            aAccum += *planes->fAFull.getAddr8(2*x+1, 2*y);
            aAccum += *planes->fAFull.getAddr8(2*x, 2*y+1);
            aAccum += *planes->fAFull.getAddr8(2*x+1, 2*y+1);

            // TODO: render in F32 rather than converting here
            dst->fR = yAccum / (4.0f * 255.0f);
            dst->fG = uAccum / (4.0f * 255.0f);
            dst->fB = vAccum / (4.0f * 255.0f);
            dst->fA = aAccum / (4.0f * 255.0f);
            ++dst;
        }
    }
}

// Create a 2x2 downsampled SkBitmap. It is stored in an RG texture. It can optionally be
// uv (i.e., NV12) or vu (i.e., NV21).
static SkBitmap make_quarter_2_channel(const SkBitmap& fullY,
                                       const SkBitmap& quarterU,
                                       const SkBitmap& quarterV,
                                       bool uv) {
    SkBitmap result;

    result.allocPixels(SkImageInfo::Make(fullY.width()/2,
                                         fullY.height()/2,
                                         kR8G8_unorm_SkColorType,
                                         kUnpremul_SkAlphaType));

    for (int y = 0; y < fullY.height()/2; ++y) {
        for (int x = 0; x < fullY.width()/2; ++x) {
            uint8_t u8 = *quarterU.getAddr8(x, y);
            uint8_t v8 = *quarterV.getAddr8(x, y);

            if (uv) {
                *result.getAddr16(x, y) = (v8 << 8) | u8;
            } else {
                *result.getAddr16(x, y) = (u8 << 8) | v8;
            }
        }
    }

    return result;
}

// Create some flavor of a 16bits/channel bitmap from a RGBA_F32 source
static SkBitmap make_16(const SkBitmap& src, SkColorType dstCT,
                        std::function<void(uint16_t* dstPixel, const float* srcPixel)> convert) {
    SkASSERT(src.colorType() == kRGBA_F32_SkColorType);

    SkBitmap result;

    result.allocPixels(SkImageInfo::Make(src.dimensions(), dstCT, kUnpremul_SkAlphaType));

    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.width(); ++x) {
            const float* srcPixel = (const float*) src.getAddr(x, y);
            uint16_t* dstPixel = (uint16_t*) result.getAddr(x, y);

            convert(dstPixel, srcPixel);
        }
    }

    return result;
}

static uint16_t flt_2_uint16(float flt) { return SkScalarRoundToInt(flt * 65535.0f); }

// Recombine the separate planes into some YUV format. Returns the number of planes.
static int create_YUV(const PlaneData& planes,
                      YUVFormat yuvFormat,
                      SkBitmap resultBMs[],
                      bool opaque) {
    int nextLayer = 0;

    switch (yuvFormat) {
        case kY416_YUVFormat: {
            resultBMs[nextLayer++] = make_16(planes.fFull, kR16G16B16A16_unorm_SkColorType,
                                             [] (uint16_t* dstPixel, const float* srcPixel) {
                                                 dstPixel[0] = flt_2_uint16(srcPixel[1]); // U
                                                 dstPixel[1] = flt_2_uint16(srcPixel[0]); // Y
                                                 dstPixel[2] = flt_2_uint16(srcPixel[2]); // V
                                                 dstPixel[3] = flt_2_uint16(srcPixel[3]); // A
                                             });
            break;
        }
        case kAYUV_YUVFormat: {
            SkBitmap yuvaFull;

            yuvaFull.allocPixels(SkImageInfo::Make(planes.fYFull.width(), planes.fYFull.height(),
                                                   kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));

            for (int y = 0; y < planes.fYFull.height(); ++y) {
                for (int x = 0; x < planes.fYFull.width(); ++x) {

                    uint8_t Y = *planes.fYFull.getAddr8(x, y);
                    uint8_t U = *planes.fUFull.getAddr8(x, y);
                    uint8_t V = *planes.fVFull.getAddr8(x, y);
                    uint8_t A = *planes.fAFull.getAddr8(x, y);

                    // NOT premul!
                    // V and Y swapped to match RGBA layout
                    SkColor c = SkColorSetARGB(A, V, U, Y);
                    *yuvaFull.getAddr32(x, y) = c;
                }
            }

            resultBMs[nextLayer++] = yuvaFull;
            break;
        }
        case kY410_YUVFormat: {
            SkBitmap yuvaFull;
            uint32_t Y, U, V;
            uint8_t A;

            yuvaFull.allocPixels(SkImageInfo::Make(planes.fYFull.width(), planes.fYFull.height(),
                                                   kRGBA_1010102_SkColorType,
                                                   kUnpremul_SkAlphaType));

            for (int y = 0; y < planes.fYFull.height(); ++y) {
                for (int x = 0; x < planes.fYFull.width(); ++x) {

                    Y = SkScalarRoundToInt((*planes.fYFull.getAddr8(x, y) / 255.0f) * 1023.0f);
                    U = SkScalarRoundToInt((*planes.fUFull.getAddr8(x, y) / 255.0f) * 1023.0f);
                    V = SkScalarRoundToInt((*planes.fVFull.getAddr8(x, y) / 255.0f) * 1023.0f);
                    A = SkScalarRoundToInt((*planes.fAFull.getAddr8(x, y) / 255.0f) * 3.0f);

                    // NOT premul!
                    *yuvaFull.getAddr32(x, y) = (A << 30) | (V << 20) | (Y << 10) | (U << 0);
                }
            }

            resultBMs[nextLayer++] = yuvaFull;
            break;
        }
        case kP016_YUVFormat:     // fall through
        case kP010_YUVFormat: {
            resultBMs[nextLayer++] = make_16(planes.fFull, kA16_unorm_SkColorType,
                                             [tenBitsPP = (yuvFormat == kP010_YUVFormat)]
                                             (uint16_t* dstPixel, const float* srcPixel) {
                                                 uint16_t val16 = flt_2_uint16(srcPixel[0]);
                                                 dstPixel[0] = tenBitsPP ? (val16 & 0xFFC0)
                                                                         : val16;
                                              });
            resultBMs[nextLayer++] = make_16(planes.fQuarter, kR16G16_unorm_SkColorType,
                                             [tenBitsPP = (yuvFormat == kP010_YUVFormat)]
                                             (uint16_t* dstPixel, const float* srcPixel) {
                                                 uint16_t u16 = flt_2_uint16(srcPixel[1]);
                                                 uint16_t v16 = flt_2_uint16(srcPixel[2]);
                                                 dstPixel[0] = tenBitsPP ? (u16 & 0xFFC0) : u16;
                                                 dstPixel[1] = tenBitsPP ? (v16 & 0xFFC0) : v16;
                                             });
            if (!opaque) {
                resultBMs[nextLayer++] = make_16(planes.fFull, kA16_unorm_SkColorType,
                                                 [tenBitsPP = (yuvFormat == kP010_YUVFormat)]
                                                 (uint16_t* dstPixel, const float* srcPixel) {
                                                     uint16_t val16 = flt_2_uint16(srcPixel[3]);
                                                     dstPixel[0] = tenBitsPP ? (val16 & 0xFFC0)
                                                                             : val16;
                                                 });
            }
            return nextLayer;
        }
        case kP016F_YUVFormat: {
            resultBMs[nextLayer++] = make_16(planes.fFull, kA16_float_SkColorType,
                                             [] (uint16_t* dstPixel, const float* srcPixel) {
                                                 dstPixel[0] = SkFloatToHalf(srcPixel[0]);
                                             });
            resultBMs[nextLayer++] = make_16(planes.fQuarter, kR16G16_float_SkColorType,
                                             [] (uint16_t* dstPixel, const float* srcPixel) {
                                                 dstPixel[0] = SkFloatToHalf(srcPixel[1]);
                                                 dstPixel[1] = SkFloatToHalf(srcPixel[2]);
                                             });
            if (!opaque) {
                resultBMs[nextLayer++] = make_16(planes.fFull, kA16_float_SkColorType,
                                                 [] (uint16_t* dstPixel, const float* srcPixel) {
                                                     dstPixel[0] = SkFloatToHalf(srcPixel[3]);
                                                 });
            }
            return nextLayer;
        }
        case kNV12_YUVFormat: {
            SkBitmap uvQuarter = make_quarter_2_channel(planes.fYFull,
                                                        planes.fUQuarter,
                                                        planes.fVQuarter, true);
            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = uvQuarter;
            break;
        }
        case kNV21_YUVFormat: {
            SkBitmap vuQuarter = make_quarter_2_channel(planes.fYFull,
                                                        planes.fUQuarter,
                                                        planes.fVQuarter, false);
            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = vuQuarter;
            break;
        }
        case kI420_YUVFormat:
            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = planes.fUQuarter;
            resultBMs[nextLayer++] = planes.fVQuarter;
            break;
        case kYV12_YUVFormat:
            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = planes.fVQuarter;
            resultBMs[nextLayer++] = planes.fUQuarter;
            break;
    }

    if (!opaque && !has_alpha_channel(yuvFormat)) {
        resultBMs[nextLayer++] = planes.fAFull;
    }
    return nextLayer;
}

static void draw_col_label(SkCanvas* canvas, int x, int yuvColorSpace, bool opaque) {
    static const char* kYUVColorSpaceNames[] = {"JPEG",     "601",      "709F",     "709L",
                                                "2020_8F",  "2020_8L",  "2020_10F", "2020_10L",
                                                "2020_12F", "2020_12L", "Identity"};
    static_assert(std::size(kYUVColorSpaceNames) == kLastEnum_SkYUVColorSpace + 1);

    SkPaint paint;
    SkFont  font(ToolUtils::CreatePortableTypeface("Sans", SkFontStyle::Bold()), 16);
    font.setEdging(SkFont::Edging::kAlias);

    SkRect textRect;
    SkString colLabel;

    colLabel.printf("%s", kYUVColorSpaceNames[yuvColorSpace]);
    font.measureText(colLabel.c_str(), colLabel.size(), SkTextEncoding::kUTF8, &textRect);
    int y = textRect.height();

    SkTextUtils::DrawString(canvas, colLabel.c_str(), x, y, font, paint, SkTextUtils::kCenter_Align);

    colLabel.printf("%s", opaque ? "Opaque" : "Transparent");

    font.measureText(colLabel.c_str(), colLabel.size(), SkTextEncoding::kUTF8, &textRect);
    y += textRect.height();

    SkTextUtils::DrawString(canvas, colLabel.c_str(), x, y, font, paint, SkTextUtils::kCenter_Align);
}

static void draw_row_label(SkCanvas* canvas, int y, int yuvFormat) {
    static const char* kYUVFormatNames[] = {
        "P016", "P010", "P016F", "Y416", "AYUV", "Y410", "NV12", "NV21", "I420", "YV12"
    };
    static_assert(std::size(kYUVFormatNames) == kLast_YUVFormat + 1);

    SkPaint paint;
    SkFont  font(ToolUtils::CreatePortableTypeface("Sans", SkFontStyle::Bold()), 16);
    font.setEdging(SkFont::Edging::kAlias);

    SkRect textRect;
    SkString rowLabel;

    rowLabel.printf("%s", kYUVFormatNames[yuvFormat]);
    font.measureText(rowLabel.c_str(), rowLabel.size(), SkTextEncoding::kUTF8, &textRect);
    y += kTileWidthHeight/2 + textRect.height()/2;

    canvas->drawString(rowLabel, 0, y, font, paint);
}

static sk_sp<SkColorFilter> yuv_to_rgb_colorfilter() {
    static const float kJPEGConversionMatrix[20] = {
        1.0f,  0.0f,       1.402f,    0.0f, -180.0f/255,
        1.0f, -0.344136f, -0.714136f, 0.0f,  136.0f/255,
        1.0f,  1.772f,     0.0f,      0.0f, -227.6f/255,
        0.0f,  0.0f,       0.0f,      1.0f,    0.0f
    };

    return SkColorFilters::Matrix(kJPEGConversionMatrix);
}

namespace skiagm {

// This GM creates an opaque and transparent bitmap, extracts the planes and then recombines
// them into various YUV formats. It then renders the results in the grid:
//
//                 JPEG                  601                   709                Identity
//        Transparent  Opaque   Transparent  Opaque   Transparent  Opaque   Transparent Opaque
// originals
// P016
// P010
// P016F
// Y416
// AYUV
// Y410
// NV12
// NV21
// I420
// YV12
class WackyYUVFormatsGM : public GM {
public:
    using Type = sk_gpu_test::LazyYUVImage::Type;

    WackyYUVFormatsGM(bool useTargetColorSpace, bool useSubset, Type type)
            : fUseTargetColorSpace(useTargetColorSpace), fUseSubset(useSubset), fImageType(type) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override {
        SkString name("wacky_yuv_formats");
        if (fUseTargetColorSpace) {
            name += "_cs";
        }
        if (fUseSubset) {
            name += "_domain";
        }
        switch (fImageType) {
            case Type::kFromPixmaps:
                name += "_frompixmaps";
                break;
            case Type::kFromTextures:
                break;
            case Type::kFromGenerator:
                name += "_imggen";
                break;
            case Type::kFromImages:
                name += "_fromimages";
                break;
        }

        return name;
    }

    SkISize getISize() override {
        int numCols = 2 * (kLastEnum_SkYUVColorSpace + 1); // opacity x #-color-spaces
        int numRows = 1 + (kLast_YUVFormat + 1);  // original + #-yuv-formats
        int wh = SkScalarCeilToInt(kTileWidthHeight * (fUseSubset ? 1.5f : 1.f));
        return SkISize::Make(kLabelWidth  + numCols * (wh + kPad),
                             kLabelHeight + numRows * (wh + kPad));
    }

    void createBitmaps() {
        SkPoint origin = { kTileWidthHeight/2.0f, kTileWidthHeight/2.0f };
        float outerRadius = kTileWidthHeight/2.0f - 20.0f;
        float innerRadius = 20.0f;

        {
            // transparent
            SkTDArray<SkRect> circles;
            SkPath path = create_splat(origin, innerRadius, outerRadius, 1.0f, 5, &circles);
            fOriginalBMs[0] = make_bitmap(kRGBA_8888_SkColorType, path, circles, false, fUseSubset);
        }

        {
            // opaque
            SkTDArray<SkRect> circles;
            SkPath path = create_splat(origin, innerRadius, outerRadius, 1.0f, 7, &circles);
            fOriginalBMs[1] = make_bitmap(kRGBA_8888_SkColorType, path, circles, true, fUseSubset);
        }

        if (fUseTargetColorSpace) {
            fTargetColorSpace = SkColorSpace::MakeSRGB()->makeColorSpin();
        }
    }

    bool createImages(GrDirectContext* dContext, Recorder* recorder) {
        int origin = 0;
        for (bool opaque : { false, true }) {
            for (int cs = kJPEG_SkYUVColorSpace; cs <= kLastEnum_SkYUVColorSpace; ++cs) {
                PlaneData planes;
                extract_planes(fOriginalBMs[opaque],
                               static_cast<SkYUVColorSpace>(cs),
                               static_cast<SkEncodedOrigin>(origin + 1),  // valid origins are 1...8
                               &planes);

                for (int f = kP016_YUVFormat; f <= kLast_YUVFormat; ++f) {
                    auto format = static_cast<YUVFormat>(f);
                    SkBitmap resultBMs[4];

                    int numPlanes = create_YUV(planes, format, resultBMs, opaque);
                    const YUVAPlanarConfig planarConfig(format,
                                                        opaque,
                                                        static_cast<SkEncodedOrigin>(origin + 1));
                    SkYUVAPixmaps pixmaps =
                            planarConfig.makeYUVAPixmaps(fOriginalBMs[opaque].dimensions(),
                                                         static_cast<SkYUVColorSpace>(cs),
                                                         resultBMs,
                                                         numPlanes);
                    auto lazyYUV = sk_gpu_test::LazyYUVImage::Make(std::move(pixmaps));
#if defined(SK_GRAPHITE)
                    if (recorder) {
                        fImages[opaque][cs][format] = lazyYUV->refImage(recorder, fImageType);
                    } else
#endif
                    {
                        fImages[opaque][cs][format] = lazyYUV->refImage(dContext, fImageType);
                    }
                }
                origin = (origin + 1) % 8;
            }
        }

        if (dContext) {
            // Some backends (e.g., Vulkan) require all work be completed for backend textures
            // before they are deleted. Since we don't know when we'll next have access to a
            // direct context, flush all the work now.
            dContext->flush();
            dContext->submit(GrSyncCpu::kYes);
        }

        return true;
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        auto recorder = canvas->recorder();
        this->createBitmaps();

        if (dContext && dContext->abandoned()) {
            // This isn't a GpuGM so a null 'context' is okay but an abandoned context
            // if forbidden.
            return DrawResult::kSkip;
        }

        // Only the generator is expected to work with the CPU backend.
        if (fImageType != Type::kFromGenerator && !dContext && !recorder) {
            return DrawResult::kSkip;
        }

        if (!this->createImages(dContext, recorder)) {
            *errorMsg = "Failed to create YUV images";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j <= kLastEnum_SkYUVColorSpace; ++j) {
                for (int k = 0; k <= kLast_YUVFormat; ++k) {
                    fImages[i][j][k] = nullptr;
                }
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        auto direct = GrAsDirectContext(canvas->recordingContext());
#if defined(SK_GRAPHITE)
        auto recorder = canvas->recorder();
#endif

        float cellWidth = kTileWidthHeight, cellHeight = kTileWidthHeight;
        if (fUseSubset) {
            cellWidth *= 1.5f;
            cellHeight *= 1.5f;
        }

        SkRect srcRect = SkRect::Make(fOriginalBMs[0].dimensions());
        SkRect dstRect = SkRect::MakeXYWH(kLabelWidth, 0.f, srcRect.width(), srcRect.height());

        SkCanvas::SrcRectConstraint constraint = SkCanvas::kFast_SrcRectConstraint;
        if (fUseSubset) {
            srcRect.inset(kSubsetPadding, kSubsetPadding);
            // Draw a larger rectangle to ensure bilerp filtering would normally read outside the
            // srcRect and hit the red pixels, if strict constraint weren't used.
            dstRect.fRight = kLabelWidth + 1.5f * srcRect.width();
            dstRect.fBottom = 1.5f * srcRect.height();
            constraint = SkCanvas::kStrict_SrcRectConstraint;
        }

        SkSamplingOptions sampling(SkFilterMode::kLinear);
        for (int cs = kJPEG_SkYUVColorSpace; cs <= kLastEnum_SkYUVColorSpace; ++cs) {
            SkPaint paint;
            if (kIdentity_SkYUVColorSpace == cs) {
                // The identity color space needs post processing to appear correctly
                paint.setColorFilter(yuv_to_rgb_colorfilter());
            }

            for (int opaque : { 0, 1 }) {
                dstRect.offsetTo(dstRect.fLeft, kLabelHeight);

                draw_col_label(canvas, dstRect.fLeft + cellWidth / 2, cs, opaque);

                canvas->drawImageRect(fOriginalBMs[opaque].asImage(), srcRect, dstRect,
                                      SkSamplingOptions(), nullptr, constraint);
                dstRect.offset(0.f, cellHeight + kPad);

                for (int format = kP016_YUVFormat; format <= kLast_YUVFormat; ++format) {
                    draw_row_label(canvas, dstRect.fTop, format);
                    if (fUseTargetColorSpace && fImages[opaque][cs][format]) {
                        // Making a CS-specific version of a kIdentity_SkYUVColorSpace YUV image
                        // doesn't make a whole lot of sense. The colorSpace conversion will
                        // operate on the YUV components rather than the RGB components.
                        sk_sp<SkImage> csImage;
#if defined(SK_GRAPHITE)
                        if (recorder) {
                            csImage = fImages[opaque][cs][format]->makeColorSpace(
                                    recorder, fTargetColorSpace, {});
                        } else
#endif
                        {
                            csImage = fImages[opaque][cs][format]->makeColorSpace(
                                    direct, fTargetColorSpace);
                        }
                        canvas->drawImageRect(csImage, srcRect, dstRect, sampling,
                                              &paint, constraint);
                    } else {
                        canvas->drawImageRect(fImages[opaque][cs][format], srcRect, dstRect,
                                              sampling, &paint, constraint);
                    }
                    dstRect.offset(0.f, cellHeight + kPad);
                }

                dstRect.offset(cellWidth + kPad, 0.f);
            }
        }
    }

private:
    SkBitmap                   fOriginalBMs[2];
    sk_sp<SkImage>             fImages[2][kLastEnum_SkYUVColorSpace + 1][kLast_YUVFormat + 1];
    bool                       fUseTargetColorSpace;
    bool                       fUseSubset;
    Type                       fImageType;
    sk_sp<SkColorSpace>        fTargetColorSpace;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new WackyYUVFormatsGM(/*useTargetColorSpace=*/false,
                                    /*useSubset=*/false,
                                    WackyYUVFormatsGM::Type::kFromTextures);)
DEF_GM(return new WackyYUVFormatsGM(/*useTargetColorSpace=*/false,
                                    /*useSubset=*/true,
                                    WackyYUVFormatsGM::Type::kFromTextures);)
DEF_GM(return new WackyYUVFormatsGM(/*useTargetColorSpace=*/true,
                                    /*useSubset=*/false,
                                    WackyYUVFormatsGM::Type::kFromTextures);)
DEF_GM(return new WackyYUVFormatsGM(/*useTargetColorSpace=*/false,
                                    /*useSubset=*/false,
                                    WackyYUVFormatsGM::Type::kFromGenerator);)
DEF_GM(return new WackyYUVFormatsGM(/*useTargetColorSpace=*/false,
                                    /*useSubset=*/false,
                                    WackyYUVFormatsGM::Type::kFromPixmaps);)
#if defined(SK_GRAPHITE)
DEF_GM(return new WackyYUVFormatsGM(/*useTargetColorSpace=*/false,
                                    /*useSubset=*/false,
                                    WackyYUVFormatsGM::Type::kFromImages);)
#endif

class YUVMakeColorSpaceGM : public GM {
public:
    YUVMakeColorSpaceGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("yuv_make_color_space"); }

    SkISize getISize() override {
        int numCols = 4; // (transparent, opaque) x (untagged, tagged)
        int numRows = 5; // original, YUV, subset, makeNonTextureImage, readPixels
        return SkISize::Make(numCols * (kTileWidthHeight + kPad) + kPad,
                             numRows * (kTileWidthHeight + kPad) + kPad);
    }

    void createBitmaps() {
        SkPoint origin = { kTileWidthHeight/2.0f, kTileWidthHeight/2.0f };
        float outerRadius = kTileWidthHeight/2.0f - 20.0f;
        float innerRadius = 20.0f;

        {
            // transparent
            SkTDArray<SkRect> circles;
            SkPath path = create_splat(origin, innerRadius, outerRadius, 1.0f, 5, &circles);
            fOriginalBMs[0] = make_bitmap(kN32_SkColorType, path, circles, false, false);
        }

        {
            // opaque
            SkTDArray<SkRect> circles;
            SkPath path = create_splat(origin, innerRadius, outerRadius, 1.0f, 7, &circles);
            fOriginalBMs[1] = make_bitmap(kN32_SkColorType, path, circles, true, false);
        }

        fTargetColorSpace = SkColorSpace::MakeSRGB()->makeColorSpin();
    }

    bool createImages(GrDirectContext* context) {
        for (bool opaque : { false, true }) {
            PlaneData planes;
            extract_planes(fOriginalBMs[opaque],
                           kJPEG_SkYUVColorSpace,
                           kTopLeft_SkEncodedOrigin,
                           &planes);

            SkBitmap resultBMs[4];

            create_YUV(planes, kAYUV_YUVFormat, resultBMs, opaque);

            YUVAPlanarConfig planarConfig(kAYUV_YUVFormat, opaque, kTopLeft_SkEncodedOrigin);

            auto yuvaPixmaps = planarConfig.makeYUVAPixmaps(fOriginalBMs[opaque].dimensions(),
                                                            kJPEG_Full_SkYUVColorSpace,
                                                            resultBMs,
                                                            std::size(resultBMs));

            int i = 0;
            for (sk_sp<SkColorSpace> cs : {sk_sp<SkColorSpace>(nullptr),
                                           SkColorSpace::MakeSRGB()}) {
                auto lazyYUV = sk_gpu_test::LazyYUVImage::Make(
                        yuvaPixmaps, skgpu::Mipmapped::kNo, std::move(cs));
                fImages[opaque][i++] =
                        lazyYUV->refImage(context, sk_gpu_test::LazyYUVImage::Type::kFromTextures);
            }
        }

        // Some backends (e.g., Vulkan) require all work be completed for backend textures before
        // they are deleted. Since we don't know when we'll next have access to a direct context,
        // flush all the work now.
        context->flush();
        context->submit(GrSyncCpu::kYes);

        return true;
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext || dContext->abandoned()) {
            *errorMsg = "DirectContext required to create YUV images";
            return DrawResult::kSkip;
        }

        this->createBitmaps();
        if (!this->createImages(dContext)) {
            *errorMsg = "Failed to create YUV images";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        fImages[0][0] = fImages[0][1] = fImages[1][0] = fImages[1][1] = nullptr;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* msg) override {
        SkASSERT(fImages[0][0] && fImages[0][1] && fImages[1][0] && fImages[1][1]);

        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            *msg = "YUV ColorSpace image creation requires a direct context.";
            return DrawResult::kSkip;
        }

        int x = kPad;
        for (int tagged : { 0, 1 }) {
            for (int opaque : { 0, 1 }) {
                int y = kPad;

                auto raster = fOriginalBMs[opaque].asImage()->makeColorSpace(
                      nullptr, fTargetColorSpace);
                canvas->drawImage(raster, x, y);
                y += kTileWidthHeight + kPad;

                if (fImages[opaque][tagged]) {
                    auto yuv = fImages[opaque][tagged]->makeColorSpace(dContext, fTargetColorSpace);
                    SkASSERT(yuv);
                    SkASSERT(SkColorSpace::Equals(yuv->colorSpace(), fTargetColorSpace.get()));
                    canvas->drawImage(yuv, x, y);
                    y += kTileWidthHeight + kPad;

                    SkIRect bounds = SkIRect::MakeWH(kTileWidthHeight / 2, kTileWidthHeight / 2);
                    auto subset = SkImages::SubsetTextureFrom(dContext, yuv.get(), bounds);
                    SkASSERT(subset);
                    canvas->drawImage(subset, x, y);
                    y += kTileWidthHeight + kPad;

                    auto nonTexture = yuv->makeNonTextureImage();
                    SkASSERT(nonTexture);
                    canvas->drawImage(nonTexture, x, y);
                    y += kTileWidthHeight + kPad;

                    SkBitmap readBack;
                    readBack.allocPixels(yuv->imageInfo());
                    SkAssertResult(yuv->readPixels(dContext, readBack.pixmap(), 0, 0));
                    canvas->drawImage(readBack.asImage(), x, y);
                }
                x += kTileWidthHeight + kPad;
            }
        }
        return DrawResult::kOk;
    }

private:
    SkBitmap fOriginalBMs[2];
    sk_sp<SkImage> fImages[2][2];
    sk_sp<SkColorSpace> fTargetColorSpace;

    using INHERITED = GM;
};

DEF_GM(return new YUVMakeColorSpaceGM();)

}  // namespace skiagm

///////////////

#include "include/effects/SkColorMatrix.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "tools/Resources.h"

static void draw_diff(SkCanvas* canvas, SkScalar x, SkScalar y,
                      const SkImage* a, const SkImage* b) {
    auto sh = SkShaders::Blend(SkBlendMode::kDifference,
                               a->makeShader(SkSamplingOptions()),
                               b->makeShader(SkSamplingOptions()));
    SkPaint paint;
    paint.setShader(sh);
    canvas->save();
    canvas->translate(x, y);
    canvas->drawRect(SkRect::MakeWH(a->width(), a->height()), paint);

    SkColorMatrix cm;
    cm.setScale(64, 64, 64);
    paint.setShader(sh->makeWithColorFilter(SkColorFilters::Matrix(cm)));
    canvas->translate(0, a->height());
    canvas->drawRect(SkRect::MakeWH(a->width(), a->height()), paint);

    canvas->restore();
}

// Exercises SkColorMatrix_RGB2YUV for yuv colorspaces, showing the planes, and the
// resulting (recombined) images (gpu only for now).
//
class YUVSplitterGM : public skiagm::GM {
    sk_sp<SkImage> fOrig;

public:
    YUVSplitterGM() {}

protected:
    SkString getName() const override { return SkString("yuv_splitter"); }

    SkISize getISize() override { return SkISize::Make(1280, 768); }

    void onOnceBeforeDraw() override {
        fOrig = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(fOrig->width(), 0);
        canvas->save();
        SkYUVAInfo info;
        std::array<sk_sp<SkImage>, SkYUVAInfo::kMaxPlanes> planes;
        for (auto cs : {kRec709_SkYUVColorSpace,
                        kRec601_SkYUVColorSpace,
                        kJPEG_SkYUVColorSpace,
                        kBT2020_SkYUVColorSpace}) {
            std::tie(planes, info) = sk_gpu_test::MakeYUVAPlanesAsA8(fOrig.get(),
                                                                     cs,
                                                                     SkYUVAInfo::Subsampling::k444,
                                                                     /*recording context*/ nullptr);
            SkPixmap pixmaps[4];
            for (int i = 0; i < info.numPlanes(); ++i) {
                planes[i]->peekPixels(&pixmaps[i]);
            }
            auto yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(info, pixmaps);
            auto img = SkImages::TextureFromYUVAPixmaps(canvas->recordingContext(),
                                                        yuvaPixmaps,
                                                        skgpu::Mipmapped::kNo,
                                                        /* limit to max tex size */ false,
                                                        /* color space */ nullptr);
            if (img) {
                canvas->drawImage(img, 0, 0);
                draw_diff(canvas, 0, fOrig->height(), fOrig.get(), img.get());
            }
            canvas->translate(fOrig->width(), 0);
        }
        canvas->restore();
        canvas->translate(-fOrig->width(), 0);
        int y = 0;
        for (int i = 0; i < info.numPlanes(); ++i) {
            canvas->drawImage(planes[i], 0, y);
            y += planes[i]->height();
        }
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new YUVSplitterGM; )

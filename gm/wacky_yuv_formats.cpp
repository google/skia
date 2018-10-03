/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkColorPriv.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkPath.h"

static const int kTileWidthHeight = 128;
static const int kLabelWidth = 64;
static const int kLabelHeight = 32;
static const int kPad = 1;
constexpr SkColor kGreen  = SkColorSetARGB(0xFF, 104, 240, 178);
constexpr SkColor kBlue   = SkColorSetARGB(0xFF, 252, 167, 173);
constexpr SkColor kYellow = SkColorSetARGB(0xFF, 117, 221, 255);

enum YUVFormat {
    // 4:4:4 formats, 32 bpp
    AYUV_YUVFormat,  // 8-bit YUVA values all interleaved

    // 4:2:0 formats, 12 bpp
    kNV12_YUVFormat, // 8-bit Y plane + 2x2 down sampled interleaved U/V planes
    kNV21_YUVFormat, // same as kNV12 but w/ U/V reversed in the interleaved plane

    kI420_YUVFormat, // 8-bit Y plane + 2x2 down sampled U and V planes
    kYV12_YUVFormat, // 8-bit Y plane + 2x2 down sampled V and U planes

    kLast_YUVFormat = kYV12_YUVFormat
};

// All the planes we need to construct the various YUV formats
struct PlaneData {
   SkBitmap fYFull;
   SkBitmap fUFull;
   SkBitmap fVFull;
   SkBitmap fAFull;
   SkBitmap fUQuarter; // 2x2 downsampled U channel
   SkBitmap fVQuarter; // 2x2 downsampled V channel
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

static SkBitmap make_bitmap(const SkPath& path, const SkTDArray<SkRect>& circles, bool opaque) {

    SkImageInfo ii = SkImageInfo::Make(kTileWidthHeight, kTileWidthHeight,
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirectN32(ii.width(), ii.height(),
                                                                     (SkPMColor*)bm.getPixels(),
                                                                     bm.rowBytes());

    canvas->clear(opaque ? kGreen : SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setAntiAlias(false); // serialize-8888 doesn't seem to work well w/ partial transparency
    paint.setColor(kBlue);

    canvas->drawPath(path, paint);

    paint.setColor(opaque ? kYellow : SK_ColorTRANSPARENT);
    paint.setBlendMode(SkBlendMode::kSrc);
    for (int i = 0; i < circles.count(); ++i) {
        SkRect r = circles[i];
        r.inset(r.width()/4, r.height()/4);
        canvas->drawOval(r, paint);
    }

    return bm;
}

static void extract_planes(const SkBitmap& bm, SkYUVColorSpace yuvColorSpace, PlaneData* planes) {
    SkASSERT(!(bm.width() % 2));
    SkASSERT(!(bm.height() % 2));

    planes->fYFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));
    planes->fUFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));
    planes->fVFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));
    planes->fAFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));
    planes->fUQuarter.allocPixels(SkImageInfo::MakeA8(bm.width()/2, bm.height()/2));
    planes->fVQuarter.allocPixels(SkImageInfo::MakeA8(bm.width()/2, bm.height()/2));

    float Kr, Kb, Z, S;

    switch (yuvColorSpace) {
    case kJPEG_SkYUVColorSpace:   // computer
        Kr = 0.299f;
        Kb = 0.114f;
        Z = 0.0f;
        S = 255.0f;
        break;
    case kRec601_SkYUVColorSpace: // SD
        Kr = 0.299f;
        Kb = 0.114f;
        Z = 16.0f;
        S = 219.f;
        break;
    case kRec709_SkYUVColorSpace: // HD
        Kr = 0.2126f;
        Kb = 0.0722f;
        Z = 16.0f;
        S = 219.0f;
        break;
    }

    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            SkColor col = bm.getColor(x, y);
            uint8_t r = SkColorGetR(col);
            uint8_t b = SkColorGetB(col);
            uint8_t g = SkColorGetG(col);

            float L = Kr * r + Kb * g + (1.0f - Kr - Kb) * b;

            uint8_t Y, U, V;
            Y =             SkScalarRoundToInt((219*(L-Z)/S + 16));
            U = SkScalarPin(SkScalarRoundToInt((112*(b-L) / ((1-Kb)*S) + 128)), 0, 255);
            V = SkScalarPin(SkScalarRoundToInt((112*(r-L) / ((1-Kr)*S) + 128)), 0, 255);
            *planes->fYFull.getAddr8(x, y) = Y;
            *planes->fUFull.getAddr8(x, y) = U;
            *planes->fVFull.getAddr8(x, y) = V;
            *planes->fAFull.getAddr8(x, y) = SkColorGetA(col);
        }
    }

    for (int y = 0; y < bm.height()/2; ++y) {
        for (int x = 0; x < bm.width()/2; ++x) {
            uint32_t uAccum = 0, vAccum = 0;

            uAccum += *planes->fUFull.getAddr8(2*x, 2*y);
            uAccum += *planes->fUFull.getAddr8(2*x+1, 2*y);
            uAccum += *planes->fUFull.getAddr8(2*x, 2*y+1);
            uAccum += *planes->fUFull.getAddr8(2*x+1, 2*y+1);

            *planes->fUQuarter.getAddr8(x, y) = uAccum / 4.0f;

            vAccum += *planes->fVFull.getAddr8(2*x, 2*y);
            vAccum += *planes->fVFull.getAddr8(2*x+1, 2*y);
            vAccum += *planes->fVFull.getAddr8(2*x, 2*y+1);
            vAccum += *planes->fVFull.getAddr8(2*x+1, 2*y+1);

            *planes->fVQuarter.getAddr8(x, y) = uAccum / 4.0f;
        }
    }
}

// Recombine the separate planes into some YUV format
static void create_YUV(const PlaneData& planes, YUVFormat yuvFormat,
                       SkBitmap resultBMs[], SkYUVAIndex yuvaIndices[4], bool opaque) {
    int nextLayer = 0;

    switch (yuvFormat) {
        case AYUV_YUVFormat: {
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
                    *yuvaFull.getAddr32(x, y) = SkPackARGB32NoCheck(A, Y, U, V);
                }
            }

            resultBMs[nextLayer++] = yuvaFull;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kB_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 0;
            yuvaIndices[1].fChannel = kG_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 0;
            yuvaIndices[2].fChannel = kR_SkImageSourceChannel;
            yuvaIndices[3].fIndex = 0;
            yuvaIndices[3].fChannel = kA_SkImageSourceChannel;
            break;
        }
        case kNV12_YUVFormat: {
            SkBitmap uvQuarter;

            // There isn't a RG color type. Approx w/ 2x wider A8.
            uvQuarter.allocPixels(SkImageInfo::MakeA8(planes.fYFull.width(),
                                                      planes.fYFull.height()/2));

            for (int y = 0; y < planes.fYFull.height()/2; ++y) {
                for (int x = 0; x < planes.fYFull.width()/2; ++x) {
                    *uvQuarter.getAddr8(2*x, y) = *planes.fUQuarter.getAddr8(x, y);
                    *uvQuarter.getAddr8(2*x+1, y) = *planes.fVQuarter.getAddr8(x, y);
                }
            }

            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = uvQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 1;
            yuvaIndices[1].fChannel = kR_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 1;
            yuvaIndices[2].fChannel = kG_SkImageSourceChannel;
            break;
        }
        case kNV21_YUVFormat: {
            SkBitmap vuQuarter;

            // There isn't a RG color type. Approx w/ 2x wider A8.
            vuQuarter.allocPixels(SkImageInfo::MakeA8(planes.fYFull.width(),
                                                      planes.fYFull.height()/2));

            for (int y = 0; y < planes.fYFull.height()/2; ++y) {
                for (int x = 0; x < planes.fYFull.width()/2; ++x) {
                    *vuQuarter.getAddr8(2*x, y) = *planes.fVQuarter.getAddr8(x, y);
                    *vuQuarter.getAddr8(2*x+1, y) = *planes.fUQuarter.getAddr8(x, y);
                }
            }

            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = vuQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 1;
            yuvaIndices[1].fChannel = kG_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 1;
            yuvaIndices[2].fChannel = kR_SkImageSourceChannel;
            break;
        }
        case kI420_YUVFormat:
            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = planes.fUQuarter;
            resultBMs[nextLayer++] = planes.fVQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 1;
            yuvaIndices[1].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 2;
            yuvaIndices[2].fChannel = kA_SkImageSourceChannel;
            break;
        case kYV12_YUVFormat:
            resultBMs[nextLayer++] = planes.fYFull;
            resultBMs[nextLayer++] = planes.fVQuarter;
            resultBMs[nextLayer++] = planes.fUQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 2;
            yuvaIndices[1].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 1;
            yuvaIndices[2].fChannel = kA_SkImageSourceChannel;
            break;
    }

    if (AYUV_YUVFormat != yuvFormat) {
        if (opaque) {
            yuvaIndices[3].fIndex = -1;
        } else {
            resultBMs[nextLayer] = planes.fAFull;

            yuvaIndices[3].fIndex = nextLayer;
            yuvaIndices[3].fChannel = kA_SkImageSourceChannel;
        }
    }

}

class YUVGenerator : public SkImageGenerator {
public:
    YUVGenerator(const SkImageInfo& ii,
                 YUVFormat yuvFormat,
                 SkYUVColorSpace yuvColorSpace,
                 SkYUVAIndex yuvaIndices[4],
                 SkBitmap bitmaps[4])
            : SkImageGenerator(ii)
            , fYUVFormat(yuvFormat)
            , fYUVColorSpace(yuvColorSpace) {
        memcpy(fYUVAIndices, yuvaIndices, sizeof(fYUVAIndices));

        bool used[4] = { false, false, false, false };
        for (int i = 0; i < 4; ++i) {
            if (yuvaIndices[i].fIndex >= 0) {
                SkASSERT(yuvaIndices[i].fIndex < 4);
                used[yuvaIndices[i].fIndex] = true;
            } else {
                SkASSERT(3 == i); // only the 'A' channel can be unspecified
            }
        }

        for (int i = 0; i < 4; ++i) {
            if (used[i]) {
                fYUVBitmaps[i] = bitmaps[i];
            }
        }
    }

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options&) override {

        if (kUnknown_SkColorType == fFlattened.colorType()) {
            fFlattened.allocPixels(info);

            for (int y = 0; y < info.height(); ++y) {
                for (int x = 0; x < info.width(); ++x) {

                    uint8_t alpha = 255;
                    if (fYUVAIndices[3].fIndex >= 0) {
                        int alphaIndex = fYUVAIndices[3].fIndex;
                        if (kAlpha_8_SkColorType == fYUVBitmaps[alphaIndex].colorType()) {
                            alpha = *fYUVBitmaps[alphaIndex].getAddr8(x, y);
                        } else {
                            alpha = SkColorGetA(fYUVBitmaps[alphaIndex].getColor(x, y));
                        }
                    }

                    uint8_t g;
                    if (kAlpha_8_SkColorType == fYUVBitmaps[fYUVAIndices[0].fIndex].colorType()) {
                        g = *fYUVBitmaps[fYUVAIndices[0].fIndex].getAddr8(x, y);
                    } else {
                        g = SkColorGetR(fYUVBitmaps[fYUVAIndices[0].fIndex].getColor(x, y));
                    }

                    // Making premul here.
                    *fFlattened.getAddr32(x, y) = SkPreMultiplyARGB(alpha, g, g, g);
                }
            }
        }

        return fFlattened.readPixels(info, pixels, rowBytes, 0, 0);
    }

    bool onQueryYUV8(SkYUVSizeInfo* size, SkYUVColorSpace* yuvColorSpace) const override {
        if (kI420_YUVFormat != fYUVFormat && kYV12_YUVFormat != fYUVFormat) {
            return false; // currently this API only supports planar formats
        }

        *yuvColorSpace = fYUVColorSpace;
        size->fSizes[0].fWidth = fYUVBitmaps[fYUVAIndices[0].fIndex].width();
        size->fSizes[0].fHeight = fYUVBitmaps[fYUVAIndices[0].fIndex].height();
        size->fWidthBytes[0] = fYUVBitmaps[fYUVAIndices[0].fIndex].rowBytes();

        size->fSizes[1].fWidth = fYUVBitmaps[fYUVAIndices[1].fIndex].width();
        size->fSizes[1].fHeight = fYUVBitmaps[fYUVAIndices[1].fIndex].height();
        size->fWidthBytes[1] = fYUVBitmaps[fYUVAIndices[1].fIndex].rowBytes();

        size->fSizes[2].fWidth = fYUVBitmaps[fYUVAIndices[2].fIndex].width();
        size->fSizes[2].fHeight = fYUVBitmaps[fYUVAIndices[2].fIndex].height();
        size->fWidthBytes[2] = fYUVBitmaps[fYUVAIndices[2].fIndex].rowBytes();
        return true;
    }

    bool onGetYUV8Planes(const SkYUVSizeInfo&, void* planes[3]) override {
        planes[0] = fYUVBitmaps[fYUVAIndices[0].fIndex].getAddr(0, 0);
        planes[1] = fYUVBitmaps[fYUVAIndices[1].fIndex].getAddr(0, 0);
        planes[2] = fYUVBitmaps[fYUVAIndices[2].fIndex].getAddr(0, 0);
        return true;
    }

private:
    YUVFormat       fYUVFormat;
    SkYUVColorSpace fYUVColorSpace;
    SkYUVAIndex     fYUVAIndices[4];
    SkBitmap        fYUVBitmaps[4];
    SkBitmap        fFlattened;

};

static sk_sp<SkImage> make_yuv_gen_image(const SkImageInfo& ii,
                                         YUVFormat yuvFormat,
                                         SkYUVColorSpace yuvColorSpace,
                                         SkYUVAIndex yuvaIndices[4],
                                         SkBitmap bitmaps[]) {
     std::unique_ptr<SkImageGenerator> gen(new YUVGenerator(ii, yuvFormat, yuvColorSpace,
                                                            yuvaIndices, bitmaps));

    return SkImage::MakeFromGenerator(std::move(gen));
}

static void draw_col_label(SkCanvas* canvas, int x, int yuvColorSpace, bool opaque) {
    static const char* kYUVColorSpaceNames[] = { "JPEG", "601", "709" };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kYUVColorSpaceNames) == kLastEnum_SkYUVColorSpace+1);

    SkPaint textPaint;
    textPaint.setTextAlign(SkPaint::kCenter_Align);
    sk_tool_utils::set_portable_typeface(&textPaint, nullptr, SkFontStyle::Bold());
    textPaint.setTextSize(16);

    SkRect textRect;
    SkString colLabel;

    colLabel.printf("%s", kYUVColorSpaceNames[yuvColorSpace]);
    textPaint.measureText(colLabel.c_str(), colLabel.size(), &textRect);
    int y = textRect.height();

    canvas->drawText(colLabel.c_str(), colLabel.size(), x, y, textPaint);

    colLabel.printf("%s", opaque ? "Opaque" : "Transparent");
    textPaint.measureText(colLabel.c_str(), colLabel.size(), &textRect);
    y += textRect.height();

    canvas->drawText(colLabel.c_str(), colLabel.size(), x, y, textPaint);
}

static void draw_row_label(SkCanvas* canvas, int y, int yuvFormat) {
    static const char* kYUVFormatNames[] = { "AYUV", "NV12", "NV21", "I420", "YV12" };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kYUVFormatNames) == kLast_YUVFormat+1);

    SkPaint textPaint;
    textPaint.setTextAlign(SkPaint::kLeft_Align);
    sk_tool_utils::set_portable_typeface(&textPaint, nullptr, SkFontStyle::Bold());
    textPaint.setTextSize(16);

    SkRect textRect;
    SkString rowLabel;

    rowLabel.printf("%s", kYUVFormatNames[yuvFormat]);
    textPaint.measureText(rowLabel.c_str(), rowLabel.size(), &textRect);
    y += kTileWidthHeight/2 + textRect.height()/2;

    canvas->drawText(rowLabel.c_str(), rowLabel.size(), 0, y, textPaint);
}

namespace skiagm {

// This GM creates an opaque and transparent bitmap, extracts the planes and then recombines
// them into various YUV formats. It then renders the results in the grid:
//
//                   JPEG                   601                     709
//        Transparent  Opaque       Transparent  Opaque        Transparent  Opaque
// AYUV
// NV12
// NV21
// I420
// YV12
class WackyYUVFormatsGM : public GM {
public:
    WackyYUVFormatsGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("wacky_yuv_formats");
    }

    SkISize onISize() override {
        int numCols = 2 * (kLastEnum_SkYUVColorSpace + 1); // opacity x color-space
        int numRows = 1 + (kLast_YUVFormat + 1);  // origin + # yuv formats
        return SkISize::Make(kLabelWidth  + numCols * (kTileWidthHeight + kPad),
                             kLabelHeight + numRows * (kTileWidthHeight + kPad));
    }

    void onOnceBeforeDraw() override {
        SkPoint origin = { kTileWidthHeight/2.0f, kTileWidthHeight/2.0f };
        float outerRadius = kTileWidthHeight/2.0f - 20.0f;
        float innerRadius = 20.0f;

        {
            // transparent
            SkTDArray<SkRect> circles;
            SkPath path = create_splat(origin, innerRadius, outerRadius, 1.0f, 5, &circles);
            fOriginalBMs[0] = make_bitmap(path, circles, false);
        }

        {
            // opaque
            SkTDArray<SkRect> circles;
            SkPath path = create_splat(origin, innerRadius, outerRadius, 1.0f, 7, &circles);
            fOriginalBMs[1] = make_bitmap(path, circles, true);
        }

        for (bool opaque : { false, true }) {
            for (int cs = kJPEG_SkYUVColorSpace; cs <= kLastEnum_SkYUVColorSpace; ++cs) {
                PlaneData planes;
                extract_planes(fOriginalBMs[opaque], (SkYUVColorSpace) cs, &planes);

                for (int format = AYUV_YUVFormat; format <= kLast_YUVFormat; ++format) {
                    SkBitmap resultBMs[4];
                    SkYUVAIndex yuvaIndices[4];
                    create_YUV(planes, (YUVFormat) format, resultBMs, yuvaIndices, opaque);

                    fImages[opaque][cs][format] = make_yuv_gen_image(fOriginalBMs[opaque].info(),
                                                                     (YUVFormat) format,
                                                                     (SkYUVColorSpace) cs,
                                                                     yuvaIndices,
                                                                     resultBMs);
                }
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        int x = kLabelWidth;
        for (int cs = kJPEG_SkYUVColorSpace; cs <= kLastEnum_SkYUVColorSpace; ++cs) {
            for (int opaque : { 0, 1 }) {
                int y = kLabelHeight;

                draw_col_label(canvas, x+kTileWidthHeight/2, cs, opaque);

                canvas->drawBitmap(fOriginalBMs[opaque], x, y);
                y += kTileWidthHeight + kPad;

                for (int format = AYUV_YUVFormat; format <= kLast_YUVFormat; ++format) {
                    draw_row_label(canvas, y, format);
                    canvas->drawImage(fImages[opaque][cs][format], x, y);

                    y += kTileWidthHeight + kPad;
                }

                x += kTileWidthHeight + kPad;
            }
        }
    }

private:
    SkBitmap       fOriginalBMs[2];
    sk_sp<SkImage> fImages[2][kLastEnum_SkYUVColorSpace+1][kLast_YUVFormat+1];

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new WackyYUVFormatsGM;)
}

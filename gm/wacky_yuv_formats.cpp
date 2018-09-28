/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkImagePriv.h"
#include "SkPath.h"

static const int kTileWidthHeight = 256;
static const int kPad = 1;
//constexpr SkColor kPurple = SkColorSetARGB(0xFF, 239, 135, 209);
constexpr SkColor kGreen  = SkColorSetARGB(0xFF, 104, 240, 178);
constexpr SkColor kBlue   = SkColorSetARGB(0xFF, 252, 167, 173);
//constexpr SkColor kPink   = SkColorSetARGB(0xFF, 248, 166, 253);
constexpr SkColor kYellow = SkColorSetARGB(0xFF, 117, 221, 255);

enum YUVFormats {
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
                    SkTDArray<SkRect>* circles) {

    SkScalar t = ((o2.fX - o1.fX) * v2.fY - (o2.fY - o1.fY) * v2.fX) / (v1.fX * v2.fY - v1.fY * v2.fX);
    SkPoint center = { o1.fX - t * v1.fY, o1.fY + t * v1.fX };

    SkRect r = { center.fX - t, center.fY - t, center.fX + t, center.fY + t };

    if (circles) {
        circles->push_back(r);
    }

    SkVector startV = o1 - center, endV = o2 - center;
    startV.normalize();
    endV.normalize();

    SkScalar startDeg = SkRadiansToDegrees(SkScalarATan2(-startV.fY, startV.fX));
    if (startDeg <= 0) {
        startDeg += 360.0f;
    }
    SkScalar endDeg = SkRadiansToDegrees(SkScalarATan2(-endV.fY, endV.fX));
    if (endDeg <= 0) {
        endDeg += 360.0f;
    }
    if (endDeg < startDeg) {
        endDeg += 360.0f;
    }

    SkScalar sweepDeg = SkTAbs(endDeg - startDeg);

#if 0
    path->arcTo(r, startDeg, sweepDeg, false);
#else
    path->lineTo(center);
    path->lineTo(o2);
#endif
}

static SkPath blob_star(const SkPoint& o, SkScalar innerRadius, SkScalar outerRadius,
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
            add_arc(&p, top, curV, nextTop, nextV, circles);
        } else {
            nextV = innerStep.mapVector(curV.fX, curV.fY);

            SkPoint bot = SkPoint::Make(o.fX + innerRadius * curV.fX,
                                        o.fY + innerRadius * curV.fY);
            SkPoint nextBot = SkPoint::Make(o.fX + innerRadius * nextV.fX,
                                            o.fY + innerRadius * nextV.fY);

            p.lineTo(bot);
            add_arc(&p, bot, curV, nextBot, nextV, nullptr);
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
    paint.setAntiAlias(true);
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
static void create_YUV(const PlaneData& planes, YUVFormats yuvFormat,
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

                    *yuvaFull.getAddr32(x, y) = 0;
                }
            }

            resultBMs[nextLayer++] = yuvaFull;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kB_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 0;
            yuvaIndices[1].fChannel = kG_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 0;
            yuvaIndices[2].fChannel = kR_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 0;
            yuvaIndices[2].fChannel = kA_SkImageSourceChannel;
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


namespace skiagm {

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
        return SkISize::Make(2 * kTileWidthHeight + 3 * kPad,
                             4 * kTileWidthHeight + 5 * kPad);
    }

    void onOnceBeforeDraw() override {

        {
            SkTDArray<SkRect> circles;
            SkPath path = blob_star({ 128.0f, 128.0f }, 20.0f, 54.0f, 4.0f, 5, &circles);
            fBMs[0] = make_bitmap(path, circles, false);
        }

        {
            SkTDArray<SkRect> circles;
            SkPath path = blob_star({ 128.0f, 128.0f }, 20.0f, 54.0f, 4.0f, 3, &circles);
            fBMs[1] = make_bitmap(path, circles, true);
        }
        for (bool opaque : { true, false }) {
            for (int cs = kJPEG_SkYUVColorSpace; cs <= kLastEnum_SkYUVColorSpace; ++cs) {
                PlaneData planes;
                extract_planes(fBMs[opaque], (SkYUVColorSpace) cs, &planes);

                for (int format = AYUV_YUVFormat; format <= kLast_YUVFormat; ++format) {
                    SkBitmap resultBMs[4];
                    SkYUVAIndex yuvaIndices[4];
                    create_YUV(planes, (YUVFormats) format, resultBMs, yuvaIndices, opaque);
                }
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(true);

        canvas->drawBitmap(fBMs[0], kPad, kPad);
        canvas->drawBitmap(fBMs[1], 2*kPad+kTileWidthHeight, kPad);


    }

private:
    SkBitmap fBMs[2];

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new WackyYUVFormatsGM;)
}

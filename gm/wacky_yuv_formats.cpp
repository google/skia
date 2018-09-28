/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkImagePriv.h"
#include "SkPath.h"


static SkScalar intersect(const SkPoint& o1, const SkVector& v1,
                          const SkPoint& o2, const SkVector& v2) {
    SkScalar t = ((o2.fX - o1.fX) * v2.fY - (o2.fY - o1.fY) * v2.fX) / (v1.fX * v2.fY - v1.fY * v2.fX);
    return t;
}

// Add a portion of a circle to 'path'. The points 'o1' and 'o2' on on the border of the circle
// and have tangents 'v1' and 'v2'.
static void add_arc(SkPath* path,
                    const SkPoint& o1, const SkVector& v1,
                    const SkPoint& o2, const SkVector& v2,
                    SkTDArray<SkRect>* circles) {

    SkScalar t = intersect(o1, { -v1.fY, v1.fX}, o2, { v2.fY, -v2.fX });
    SkPoint center = { o1.fX - t * v1.fY, o1.fY + t * v1.fX };

    SkRect r = { center.fX - t, center.fY - t, center.fX + t, center.fY + t };

    circles->push_back(r);

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

    path->arcTo(r, startDeg, sweepDeg, false);
}

static SkPath foo(const SkPoint& o, SkScalar innerRadius, SkScalar outerRadius,
                  SkScalar ratio, int numLobes,
                  SkTDArray<SkRect>* circles) {
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

    p.moveTo(o.fX + innerRadius * curV.fX, o.fY + innerRadius * curV.fY);

    for (int i = 0; i < numDivisions; ++i) {

        SkVector nextV;
        if (0 == (i % 2)) {
            nextV = outerStep.mapVector(curV.fX, curV.fY);

            SkPoint top = SkPoint::Make(o.fX + outerRadius * curV.fX,
                                        o.fY + outerRadius * curV.fY);
            SkPoint nextTop = SkPoint::Make(o.fX + outerRadius * nextV.fX,
                                            o.fY + outerRadius * nextV.fY);

#if 1
            SkScalar t = intersect(top, { -curV.fY, curV.fX},
                                   nextTop, { nextV.fY, -nextV.fX });
            SkPoint topMid = { top.fX - t * curV.fY, top.fY + t * curV.fX };
#endif

            p.lineTo(top);
            //add_arc(&p, top, curV, nextTop, nextV, circles);

            p.lineTo(topMid);
            p.lineTo(nextTop);
        } else {
            nextV = innerStep.mapVector(curV.fX, curV.fY);

            SkPoint bot = SkPoint::Make(o.fX + innerRadius * curV.fX,
                                        o.fY + innerRadius * curV.fY);
            SkPoint nextBot = SkPoint::Make(o.fX + innerRadius * nextV.fX,
                                            o.fY + innerRadius * nextV.fY);

#if 1
            SkScalar t = intersect(bot, { -curV.fY, curV.fX},
                                   nextBot, { nextV.fY, -nextV.fX });
            SkPoint botMid = { bot.fX - t * curV.fY, bot.fY + t * curV.fX };
#endif

            p.lineTo(bot);
            //add_arc(&p, bot, curV, nextBot, nextV, circles);

            p.lineTo(botMid);
            p.lineTo(nextBot);
        }

        curV = nextV;
    }

    p.close();

    return p;
}

static const int kTileWidthHeight = 256;
static const int kPad = 1;
static const SkColor kPurple = SkColorSetARGB(0xFF, 209, 135, 239);
static const SkColor kGreen  = SkColorSetARGB(0xFF, 178, 240, 104);

static SkBitmap bar(const SkPath& path) {

    SkImageInfo ii = SkImageInfo::Make(kTileWidthHeight, kTileWidthHeight,
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(kPurple);

    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirectN32(ii.width(), ii.height(),
                                                                     (SkPMColor*)bm.getPixels(),
                                                                     bm.rowBytes());

    canvas->clear(kGreen);
    canvas->drawPath(path, paint);

#if 0
        p.setColor(SK_ColorRED);
        for (int i = 0; i < fCircles.count(); ++i) {
            canvas->drawOval(fCircles[i], p);
        }
#endif

    return bm;
}

enum class YUVFormat {
    // 4:4:4 formats, 32 bpp
    AYUV,  // 8-bit YUVA values all interleaved

    // 4:2:0 formats, 12 bpp
    kNV12, // 8-bit Y plane + 2x2 down sampled interleaved U/V planes
    kNV21, // same as kNV12 but w/ U/V reversed in the interleaved plane

    kI420, // 8-bit Y plane + 2x2 down sampled U and V planes
    kYV12, // 8-bit Y plane + 2x2 down sampled V and U planes
};

static void extractify(const SkBitmap& bm, YUVFormat yuvFormat, SkYUVColorSpace yuvColorSpace,
                       SkBitmap resultBMs[], SkYUVAIndex yuvaIndices[4]) {
    SkASSERT(!(bm.width() % 2));
    SkASSERT(!(bm.height() % 2));

    SkBitmap yFull, uFull, vFull, aFull;

    yFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));
    uFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));
    vFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));
    aFull.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()));

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
            *yFull.getAddr8(x, y) = Y;
            *uFull.getAddr8(x, y) = U;
            *vFull.getAddr8(x, y) = V;
            *aFull.getAddr8(x, y) = SkColorGetA(col);
        }
    }

    SkBitmap uQuarter, vQuarter;

    uQuarter.allocPixels(SkImageInfo::MakeA8(bm.width()/2, bm.height()/2));
    vQuarter.allocPixels(SkImageInfo::MakeA8(bm.width()/2, bm.height()/2));

    for (int y = 0; y < bm.height()/2; ++y) {
        for (int x = 0; x < bm.width()/2; ++x) {
            uint32_t uAccum = 0, vAccum = 0;

            uAccum += *uFull.getAddr8(2*x, 2*y);
            uAccum += *uFull.getAddr8(2*x+1, 2*y);
            uAccum += *uFull.getAddr8(2*x, 2*y+1);
            uAccum += *uFull.getAddr8(2*x+1, 2*y+1);

            *uQuarter.getAddr8(x, y) = uAccum / 4.0f;

            vAccum += *vFull.getAddr8(2*x, 2*y);
            vAccum += *vFull.getAddr8(2*x+1, 2*y);
            vAccum += *vFull.getAddr8(2*x, 2*y+1);
            vAccum += *vFull.getAddr8(2*x+1, 2*y+1);

            *vQuarter.getAddr8(x, y) = uAccum / 4.0f;
        }
    }

    int nextLayer = 0;

    switch (yuvFormat) {
        case YUVFormat::AYUV: {
            SkBitmap yuvaFull;

            yuvaFull.allocPixels(SkImageInfo::Make(bm.width(), bm.height(), kRGBA_8888_SkColorType,
                                 kUnpremul_SkAlphaType));

            for (int y = 0; y < bm.height(); ++y) {
                for (int x = 0; x < bm.width(); ++x) {

                    uint8_t Y = *yFull.getAddr8(x, y);
                    uint8_t U = *uFull.getAddr8(x, y);
                    uint8_t V = *vFull.getAddr8(x, y);
                    uint8_t A = *aFull.getAddr8(x, y);

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
        case YUVFormat::kNV12: {
            SkBitmap uvQuarter;

            // There isn't a RG color type. Approx w/ 2x wider A8.
            uvQuarter.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()/2));

            for (int y = 0; y < bm.height()/2; ++y) {
                for (int x = 0; x < bm.width()/2; ++x) {
                    *uvQuarter.getAddr8(2*x, y) = *uQuarter.getAddr8(x, y);
                    *uvQuarter.getAddr8(2*x+1, y) = *vQuarter.getAddr8(x, y);
                }
            }

            resultBMs[nextLayer++] = yFull;
            resultBMs[nextLayer++] = uvQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 1;
            yuvaIndices[1].fChannel = kR_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 1;
            yuvaIndices[2].fChannel = kG_SkImageSourceChannel;
            break;
        }
        case YUVFormat::kNV21: {
            SkBitmap vuQuarter;

            // There isn't a RG color type. Approx w/ 2x wider A8.
            vuQuarter.allocPixels(SkImageInfo::MakeA8(bm.width(), bm.height()/2));

            for (int y = 0; y < bm.height()/2; ++y) {
                for (int x = 0; x < bm.width()/2; ++x) {
                    *vuQuarter.getAddr8(2*x, y) = *vQuarter.getAddr8(x, y);
                    *vuQuarter.getAddr8(2*x+1, y) = *uQuarter.getAddr8(x, y);
                }
            }

            resultBMs[nextLayer++] = yFull;
            resultBMs[nextLayer++] = vuQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 1;
            yuvaIndices[1].fChannel = kG_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 1;
            yuvaIndices[2].fChannel = kR_SkImageSourceChannel;
            break;
        }
        case YUVFormat::kI420:
            resultBMs[nextLayer++] = yFull;
            resultBMs[nextLayer++] = uQuarter;
            resultBMs[nextLayer++] = vQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 1;
            yuvaIndices[1].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 2;
            yuvaIndices[2].fChannel = kA_SkImageSourceChannel;
            break;
        case YUVFormat::kYV12:
            resultBMs[nextLayer++] = yFull;
            resultBMs[nextLayer++] = vQuarter;
            resultBMs[nextLayer++] = uQuarter;

            yuvaIndices[0].fIndex = 0;
            yuvaIndices[0].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[1].fIndex = 2;
            yuvaIndices[1].fChannel = kA_SkImageSourceChannel;
            yuvaIndices[2].fIndex = 1;
            yuvaIndices[2].fChannel = kA_SkImageSourceChannel;
            break;
    }

    if (YUVFormat::AYUV != yuvFormat) {
        if (bm.isOpaque()) {
            yuvaIndices[3].fIndex = -1;
        } else {
            resultBMs[nextLayer] = aFull;

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
        return SkISize::Make(4 * kTileWidthHeight + 5 * kPad,
                             2 * kTileWidthHeight + 3 * kPad);
    }

    void onOnceBeforeDraw() override {
        fPath = foo({ 128.0f, 128.0f }, 20.0f, 54.0f, 4.0f, 3, &fCircles);
        fBM = bar(fPath);
        SkBitmap resultBMs[4];
        SkYUVAIndex yuvaIndices[4];
        extractify(fBM, YUVFormat::kNV12, kRec601_SkYUVColorSpace, resultBMs, yuvaIndices);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(true);

        canvas->drawBitmap(fBM, kPad, kPad);
        canvas->drawBitmap(fBM, 2*kPad+kTileWidthHeight, kPad);


    }

private:

    SkPath fPath;
    SkTDArray<SkRect> fCircles;
    SkBitmap fBM;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new WackyYUVFormatsGM;)
}

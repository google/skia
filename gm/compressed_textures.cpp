/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h" // IWYU pragma: keep

#if !defined(SK_BUILD_FOR_GOOGLE3)

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrDataUtils.h"
#include "third_party/etc1/etc1.h"

class GrContext;
class GrRenderTargetContext;

static SkPoint gen_pt(float angle, const SkVector& scale) {
    SkScalar s = SkScalarSin(angle);
    SkScalar c = SkScalarCos(angle);

    return { scale.fX * c, scale.fY * s };
}

// The resulting path will be centered at (0,0) and its size will match 'dimensions'
static SkPath make_gear(SkISize dimensions, int numTeeth) {
    SkVector outerRad{ dimensions.fWidth / 2.0f, dimensions.fHeight / 2.0f };
    SkVector innerRad{ dimensions.fWidth / 2.5f, dimensions.fHeight / 2.5f };
    const float kAnglePerTooth = SK_ScalarPI / numTeeth;

    float angle = 0.0f;

    SkPath tmp;
    tmp.setFillType(SkPathFillType::kWinding);

    tmp.moveTo(gen_pt(angle, outerRad));

    for (int i = 0; i < numTeeth; ++i, angle += 2*kAnglePerTooth) {
        tmp.lineTo(gen_pt(angle+kAnglePerTooth, outerRad));
        tmp.lineTo(gen_pt(angle+kAnglePerTooth, innerRad));
        tmp.lineTo(gen_pt(angle+2*kAnglePerTooth, innerRad));
        tmp.lineTo(gen_pt(angle+2*kAnglePerTooth, outerRad));
    }

    tmp.close();

    float fInnerRad = 0.1f * SkTMin(dimensions.fWidth, dimensions.fHeight);
    if (fInnerRad > 0.5f) {
        tmp.addCircle(0.0f, 0.0f, fInnerRad, SkPathDirection::kCCW);
    }

    return tmp;
}

// Render one level of a mipmap
SkBitmap render_level(SkISize dimensions, SkColor color, SkColorType colorType, bool opaque) {
    SkPath path = make_gear(dimensions, 9);

    SkImageInfo ii = SkImageInfo::Make(dimensions.width(), dimensions.height(),
                                       colorType, opaque ? kOpaque_SkAlphaType
                                                         : kPremul_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(ii);

    bm.eraseColor(opaque ? SK_ColorBLACK : SK_ColorTRANSPARENT);

    SkCanvas c(bm);

    SkPaint paint;
    paint.setColor(color | 0xFF000000);
    paint.setAntiAlias(false);

    c.translate(dimensions.width() / 2.0f, dimensions.height() / 2.0f);
    c.drawPath(path, paint);

    return bm;
}

// Create the compressed data blob needed to represent a mipmapped 2-color texture of the specified
// compression format. In this case 2-color means either opaque black or transparent black plus
// one other color.
// Note that ETC1/ETC2_RGB8_UNORM only supports 565 opaque textures.
static sk_sp<SkData> make_compressed_data(SkISize dimensions,
                                          SkColorType colorType,
                                          bool opaque,
                                          SkImage::CompressionType compression) {
    size_t totalSize = GrCompressedDataSize(compression, dimensions, nullptr, GrMipMapped::kYes);

    sk_sp<SkData> tmp = SkData::MakeUninitialized(totalSize);
    char* pixels = (char*) tmp->writable_data();

    int numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;

    size_t offset = 0;

    // Use a different color for each mipmap level so we can visually evaluate the draws
    static const SkColor kColors[] = {
        SK_ColorRED,
        SK_ColorGREEN,
        SK_ColorBLUE,
        SK_ColorCYAN,
        SK_ColorMAGENTA,
        SK_ColorYELLOW,
        SK_ColorWHITE,
    };

    for (int i = 0; i < numMipLevels; ++i) {
        size_t levelSize = GrCompressedDataSize(compression, dimensions, nullptr, GrMipMapped::kNo);

        SkBitmap bm = render_level(dimensions, kColors[i%7], colorType, opaque);
        if (compression == SkImage::CompressionType::kETC2_RGB8_UNORM) {
            SkASSERT(bm.colorType() == kRGB_565_SkColorType);
            SkASSERT(opaque);

            if (etc1_encode_image((unsigned char*)bm.getAddr16(0, 0),
                                  bm.width(), bm.height(), 2, bm.rowBytes(),
                                  (unsigned char*) &pixels[offset])) {
                return nullptr;
            }
        } else {
            GrTwoColorBC1Compress(bm.pixmap(), kColors[i%7], &pixels[offset]);
        }

        offset += levelSize;
        dimensions = {SkTMax(1, dimensions.width()/2), SkTMax(1, dimensions.height()/2)};
    }

    return tmp;
}

// Basic test of Ganesh's ETC1 and BC1 support
// The layout is:
//               ETC2                BC1
//         --------------------------------------
//  RGB8  | kETC2_RGB8_UNORM  | kBC1_RGB8_UNORM  |
//        |--------------------------------------|
//  RGBA8 |                   | kBC1_RGBA8_UNORM |
//         --------------------------------------
//
class CompressedTexturesGM : public skiagm::GpuGM {
public:
    CompressedTexturesGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("compressed_textures");
    }

    SkISize onISize() override {
        return SkISize::Make(2*kCellWidth + 3*kPad, 2*kTexHeight + 3*kPad);
    }

    void onOnceBeforeDraw() override {
        fOpaqueETC2Data = make_compressed_data({ kTexWidth, kTexHeight },
                                               kRGB_565_SkColorType, true,
                                               SkImage::CompressionType::kETC2_RGB8_UNORM);

        fOpaqueBC1Data = make_compressed_data({ kTexWidth, kTexHeight },
                                              kRGBA_8888_SkColorType, true,
                                              SkImage::CompressionType::kBC1_RGB8_UNORM);

        fTransparentBC1Data = make_compressed_data({ kTexWidth, kTexHeight },
                                                   kRGBA_8888_SkColorType, false,
                                                   SkImage::CompressionType::kBC1_RGBA8_UNORM);
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        this->drawCell(context, canvas, fOpaqueETC2Data,
                       SkImage::CompressionType::kETC2_RGB8_UNORM, { kPad, kPad });

        this->drawCell(context, canvas, fOpaqueBC1Data,
                       SkImage::CompressionType::kBC1_RGB8_UNORM, { 2*kPad + kCellWidth, kPad });

        this->drawCell(context, canvas, fTransparentBC1Data,
                       SkImage::CompressionType::kBC1_RGBA8_UNORM,
                       { 2*kPad + kCellWidth, 2*kPad + kTexHeight });
    }

private:
    void drawCell(GrContext* context, SkCanvas* canvas, sk_sp<SkData> data,
                  SkImage::CompressionType compression, SkIVector offset) {

        sk_sp<SkImage> image = SkImage::MakeFromCompressed(context, data,
                                                           kTexWidth, kTexHeight,
                                                           compression, GrMipMapped::kYes);
        SkISize dimensions{ kTexWidth, kTexHeight };

        int numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;

        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality); // to force mipmapping

        for (int i = 0; i < numMipLevels; ++i) {
            SkRect r = SkRect::MakeXYWH(offset.fX, offset.fY,
                                        dimensions.width(), dimensions.height());

            canvas->drawImageRect(image, r, &paint);

            if (i == 0) {
                offset.fX += dimensions.width();
            } else {
                offset.fY += dimensions.height();
            }

            dimensions = {SkTMax(1, dimensions.width()/2), SkTMax(1, dimensions.height()/2)};
        }
    }

    static const int kPad = 8;
    static const int kTexWidth = 64;
    static const int kCellWidth = 1.5f * kTexWidth;
    static const int kTexHeight = 64;

    sk_sp<SkData> fOpaqueETC2Data;
    sk_sp<SkData> fOpaqueBC1Data;
    sk_sp<SkData> fTransparentBC1Data;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new CompressedTexturesGM;)

#endif

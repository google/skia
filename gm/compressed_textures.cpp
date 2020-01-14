/*
 * Copyright 2017 Google Inc.
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

static SkPath make_star(SkISize dimensions) {
    constexpr float kInnerFrac = 0.25f;
    constexpr float kMidFrac   = 0.375f;
    constexpr int kNum = 6;
    constexpr float kHalfAngle = SK_ScalarPI / kNum;

    SkPath tmp;
    float angle = 0.0f;

    tmp.moveTo({ kMidFrac * dimensions.width(), 0.0f });

#if 0
    for (int i = 0; i < kNum; ++i, angle += kHalf) {
        SkScalar s = SkScalarSin(angle);
        SkScalar c = SkScalarCos(angle);

        tmp.conicTo()
    }
#endif

    tmp.lineTo({ 0.0f, -kMidFrac * dimensions.height() });
    tmp.lineTo({ -kMidFrac * dimensions.width(), 0.0f });
    tmp.lineTo({ 0.0f, kMidFrac * dimensions.height() });
    tmp.lineTo({ kMidFrac * dimensions.width(), 0.0f });
    tmp.close();
    return tmp;
}

SkPixmap render_level(SkISize dimensions, SkColor color, SkColorType colorType, bool opaque) {
    SkPath path = make_star(dimensions);

    SkImageInfo ii = SkImageInfo::Make(dimensions.width(), dimensions.height(),
                                       colorType,
                                       opaque ? kOpaque_SkAlphaType
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

    return bm.pixmap();
}

static sk_sp<SkData> make_opaque_2color_mipmapped_compressed_data(SkISize dimensions,
                                                                  SkColorType colorType,
                                                                  SkImage::CompressionType compression) {
    size_t totalSize = GrCompressedDataSize(compression, dimensions, nullptr, GrMipMapped::kYes);

    sk_sp<SkData> tmp = SkData::MakeUninitialized(totalSize);
    char* pixels = (char*) tmp->writable_data();

    int numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;

    size_t offset = 0;

    SkColor colors[] = {
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

        SkPixmap pixmap = render_level(dimensions, colors[i%7], colorType, true);
        if (compression == SkImage::CompressionType::kETC2_RGB8_UNORM) {
            SkASSERT(pixmap.colorType() == kRGB_565_SkColorType);

            if (etc1_encode_image((unsigned char*)pixmap.addr16(),
                                  pixmap.width(), pixmap.height(), 2, pixmap.rowBytes(),
                                  (unsigned char*) &pixels[offset])) {
                return nullptr;
            }
        } else {
            GrTwoColorBC1Compress(pixmap, colors[i%7], &pixels[offset]);
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
//  RGBA8 | kETC2_RGBA8_UNORM | kBC1_RGBA8_UNORM |
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
        fOpaqueETC2Data = make_opaque_2color_mipmapped_compressed_data({ kTexWidth, kTexHeight },
                                                                       kRGB_565_SkColorType,
                                                                       SkImage::CompressionType::kETC2_RGB8_UNORM);

        fOpaqueBC1Data = make_opaque_2color_mipmapped_compressed_data({ kTexWidth, kTexHeight },
                                                                      kRGBA_8888_SkColorType,
                                                                      SkImage::CompressionType::kBC1_RGB8_UNORM);
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        this->drawCell(context, canvas, fOpaqueETC2Data,
                       SkImage::CompressionType::kETC2_RGB8_UNORM, { kPad, kPad });

        this->drawCell(context, canvas, fOpaqueBC1Data,
                       SkImage::CompressionType::kBC1_RGB8_UNORM, { 2*kPad + kCellWidth, kPad });
    }

private:
    void drawCell(GrContext* context, SkCanvas* canvas, sk_sp<SkData> data,
                  SkImage::CompressionType compression, SkIVector offset) {

        sk_sp<SkImage> image = SkImage::MakeFromCompressed(context, data,
                                                           kTexWidth, kTexHeight,
                                                           compression);
        SkISize dimensions{ kTexWidth, kTexHeight };

        int numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;

        for (int i = 0; i < numMipLevels; ++i) {
            SkRect r = SkRect::MakeXYWH(offset.fX, offset.fY, dimensions.width(), dimensions.height());

            canvas->drawImageRect(image, r, nullptr);

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

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new CompressedTexturesGM;)

#endif

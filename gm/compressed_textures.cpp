/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h" // IWYU pragma: keep

#if !defined(SK_BUILD_FOR_GOOGLE3)  // Google3 doesn't have etc1.h

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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_GpuBase.h"
#include "third_party/etc1/etc1.h"
#include "tools/gpu/ProxyUtils.h"

static SkPoint gen_pt(float angle, const SkVector& scale) {
    SkScalar s = SkScalarSin(angle);
    SkScalar c = SkScalarCos(angle);

    return { scale.fX * c, scale.fY * s };
}

// The resulting path will be centered at (0,0) and its size will match 'dimensions'
static SkPath make_gear(SkISize dimensions, int numTeeth) {
    SkVector outerRad{ dimensions.fWidth / 2.0f, dimensions.fHeight / 2.0f };
    SkVector innerRad{ dimensions.fWidth / 2.5f, dimensions.fHeight / 2.5f };
    const float kAnglePerTooth = 2.0f * SK_ScalarPI / (3 * numTeeth);

    float angle = 0.0f;

    SkPath tmp;
    tmp.setFillType(SkPathFillType::kWinding);

    tmp.moveTo(gen_pt(angle, outerRad));

    for (int i = 0; i < numTeeth; ++i, angle += 3*kAnglePerTooth) {
        tmp.lineTo(gen_pt(angle+kAnglePerTooth, outerRad));
        tmp.lineTo(gen_pt(angle+(1.5f*kAnglePerTooth), innerRad));
        tmp.lineTo(gen_pt(angle+(2.5f*kAnglePerTooth), innerRad));
        tmp.lineTo(gen_pt(angle+(3.0f*kAnglePerTooth), outerRad));
    }

    tmp.close();

    float fInnerRad = 0.1f * std::min(dimensions.fWidth, dimensions.fHeight);
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
static sk_sp<SkImage> make_compressed_image(GrDirectContext* dContext,
                                            const SkISize dimensions,
                                            SkColorType colorType,
                                            bool opaque,
                                            SkImage::CompressionType compression) {
    size_t totalSize = SkCompressedDataSize(compression, dimensions, nullptr, true);

    sk_sp<SkData> tmp = SkData::MakeUninitialized(totalSize);
    char* pixels = (char*) tmp->writable_data();

    int numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;

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

    SkISize levelDims = dimensions;
    for (int i = 0; i < numMipLevels; ++i) {
        size_t levelSize = SkCompressedDataSize(compression, levelDims, nullptr, false);

        SkBitmap bm = render_level(levelDims, kColors[i%7], colorType, opaque);
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
        levelDims = {std::max(1, levelDims.width()/2), std::max(1, levelDims.height()/2)};
    }

    sk_sp<SkImage> image;
    if (dContext) {
        image = SkImage::MakeTextureFromCompressed(dContext, std::move(tmp),
                                                   dimensions.width(),
                                                   dimensions.height(),
                                                   compression, GrMipmapped::kYes);
    } else {
        image = SkImage::MakeRasterFromCompressed(std::move(tmp),
                                                  dimensions.width(),
                                                  dimensions.height(),
                                                  compression);
    }
    return image;
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
// The nonPowerOfTwo and nonMultipleOfFour cases exercise some compression edge cases.
class CompressedTexturesGM : public skiagm::GM {
public:
    enum class Type {
        kNormal,
        kNonPowerOfTwo,
        kNonMultipleOfFour
    };

    CompressedTexturesGM(Type type) : fType(type) {
        this->setBGColor(0xFFCCCCCC);

        switch (fType) {
            case Type::kNonPowerOfTwo:
                // These dimensions force the top two mip levels to be 1x3 and 1x1
                fImgDimensions.set(20, 60);
                break;
            case Type::kNonMultipleOfFour:
                // These dimensions force the top three mip levels to be 1x7, 1x3 and 1x1
                fImgDimensions.set(13, 61); // prime numbers - just bc
                break;
            default:
                fImgDimensions.set(kBaseTexWidth, kBaseTexHeight);
                break;
        }

    }

protected:
    SkString onShortName() override {
        SkString name("compressed_textures");

        if (fType == Type::kNonPowerOfTwo) {
            name.append("_npot");
        } else if (fType == Type::kNonMultipleOfFour) {
            name.append("_nmof");
        }

        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(2*kCellWidth + 3*kPad, 2*kBaseTexHeight + 3*kPad);
    }

    DrawResult onGpuSetup(GrDirectContext* dContext, SkString* errorMsg) override {
        if (dContext && dContext->abandoned()) {
            // This isn't a GpuGM so a null 'context' is okay but an abandoned context
            // if forbidden.
            return DrawResult::kSkip;
        }

        if (dContext &&
            dContext->backend() == GrBackendApi::kDirect3D && fType == Type::kNonMultipleOfFour) {
            // skbug.com/10541 - Are non-multiple-of-four BC1 textures supported in D3D?
            return DrawResult::kSkip;
        }

        fOpaqueETC2Image = make_compressed_image(dContext, fImgDimensions,
                                                 kRGB_565_SkColorType, true,
                                                 SkImage::CompressionType::kETC2_RGB8_UNORM);

        fOpaqueBC1Image = make_compressed_image(dContext, fImgDimensions,
                                                kRGBA_8888_SkColorType, true,
                                                SkImage::CompressionType::kBC1_RGB8_UNORM);

        fTransparentBC1Image = make_compressed_image(dContext, fImgDimensions,
                                                     kRGBA_8888_SkColorType, false,
                                                     SkImage::CompressionType::kBC1_RGBA8_UNORM);

        if (!fOpaqueETC2Image || !fOpaqueBC1Image || !fTransparentBC1Image) {
            *errorMsg = "Failed to create compressed images.";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        fOpaqueETC2Image = nullptr;
        fOpaqueBC1Image = nullptr;
        fTransparentBC1Image = nullptr;
    }

    void onDraw(SkCanvas* canvas) override {
        this->drawCell(canvas, fOpaqueETC2Image.get(), { kPad, kPad });

        this->drawCell(canvas, fOpaqueBC1Image.get(), { 2*kPad + kCellWidth, kPad });

        this->drawCell(canvas, fTransparentBC1Image.get(),
                       { 2*kPad + kCellWidth, 2*kPad + kBaseTexHeight });
    }

private:
    void drawCell(SkCanvas* canvas, SkImage* image, SkIVector offset) {

        SkISize levelDimensions = fImgDimensions;
        int numMipLevels = SkMipmap::ComputeLevelCount(levelDimensions.width(),
                                                       levelDimensions.height()) + 1;

        SkSamplingOptions sampling(SkCubicResampler::Mitchell());

        bool isCompressed = false;
        if (image->isTextureBacked()) {
            const GrCaps* caps = as_IB(image)->context()->priv().caps();
            GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(image,
                                                                      canvas->recordingContext());
            isCompressed = caps->isFormatCompressed(proxy->backendFormat());
        }

        SkPaint redStrokePaint;
        redStrokePaint.setColor(SK_ColorRED);
        redStrokePaint.setStyle(SkPaint::kStroke_Style);

        for (int i = 0; i < numMipLevels; ++i) {
            SkRect r = SkRect::MakeXYWH(offset.fX, offset.fY,
                                        levelDimensions.width(), levelDimensions.height());

            canvas->drawImageRect(image, r, sampling);
            if (!isCompressed) {
                // Make it obvious which drawImages used decompressed images
                canvas->drawRect(r, redStrokePaint);
            }

            if (i == 0) {
                offset.fX += levelDimensions.width()+1;
            } else {
                offset.fY += levelDimensions.height()+1;
            }

            levelDimensions = {std::max(1, levelDimensions.width()/2),
                               std::max(1, levelDimensions.height()/2)};
        }
    }

    static const int kPad = 8;
    static const int kBaseTexWidth = 64;
    static const int kCellWidth = 1.5f * kBaseTexWidth;
    static const int kBaseTexHeight = 64;

    Type           fType;
    SkISize        fImgDimensions;

    sk_sp<SkImage> fOpaqueETC2Image;
    sk_sp<SkImage> fOpaqueBC1Image;
    sk_sp<SkImage> fTransparentBC1Image;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new CompressedTexturesGM(CompressedTexturesGM::Type::kNormal);)
DEF_GM(return new CompressedTexturesGM(CompressedTexturesGM::Type::kNonPowerOfTwo);)
DEF_GM(return new CompressedTexturesGM(CompressedTexturesGM::Type::kNonMultipleOfFour);)

#endif

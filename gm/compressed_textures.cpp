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

        SkDebugf("level size %dx%d\n", dimensions.width(), dimensions.height());

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

#include "include/core/SkStream.h"
#include "src/gpu/gl/GrGLDefines.h"

/*
 * Get an int from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint32_t get_uint(uint8_t* buffer, uint32_t i) {
    uint32_t result;
    memcpy(&result, &(buffer[i]), 4);
    return result;
}

static sk_sp<SkData> load_ktx(const char* filename,
                              SkISize* dimensions,
                              GrMipMapped* mipMapped,
                              SkImage::CompressionType* compressionType) {
    SkFILEStream input(filename);
    if (!input.isValid()) {
        return nullptr;
    }

    constexpr int kKTXIdentifierSize = 12;
    constexpr int kKTXHeaderSize = kKTXIdentifierSize + 13 * sizeof(uint32_t);
    uint8_t header[kKTXHeaderSize];

    if (input.read(header, kKTXHeaderSize) != kKTXHeaderSize) {
        return nullptr;
    }

    static const uint8_t kExpectedIdentifier[kKTXIdentifierSize] = {
        0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
    };

    if (memcmp(header, kExpectedIdentifier, kKTXIdentifierSize)) {
        return nullptr;
    }

    uint32_t endianness = get_uint(header, 12);
    if (endianness != 0x04030201) {
        // TODO: need to swap rest of header and, if glTypeSize is > 1, all
        // the texture data.
        return nullptr;
    }

    uint32_t glType = get_uint(header, 16);
    uint32_t glTypeSize = get_uint(header, 20);
    uint32_t glFormat = get_uint(header, 24);
    uint32_t glInternalFormat = get_uint(header, 28);
    uint32_t glBaseInternalFormat = get_uint(header, 32);
    uint32_t pixelWidth = get_uint(header, 36);
    uint32_t pixelHeight = get_uint(header, 40);
    uint32_t pixelDepth = get_uint(header, 44);
    uint32_t numberOfArrayElements = get_uint(header, 48);
    uint32_t numberOfFaces = get_uint(header, 52);
    uint32_t numberOfMipmapLevels = get_uint(header, 56);
    uint32_t bytesOfKeyValueData = get_uint(header, 60);

    if (glType != 0 || glFormat != 0) {  // only care about compressed data for now
        return nullptr;
    }
    SkASSERT(glTypeSize == 1); // required for compressed data

                               // We only handle these four formats right now
    switch (glInternalFormat) {
        case GR_GL_COMPRESSED_ETC1_RGB8:
        case GR_GL_COMPRESSED_RGB8_ETC2:
            *compressionType = SkImage::CompressionType::kETC2_RGB8_UNORM;
            break;
        case GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            *compressionType = SkImage::CompressionType::kBC1_RGB8_UNORM;
            break;
        case GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            *compressionType = SkImage::CompressionType::kBC1_RGBA8_UNORM;
            break;
        default:
            return nullptr;
    }

    dimensions->fWidth = pixelWidth;
    dimensions->fHeight = pixelHeight;

    if (pixelDepth != 0) {
        return nullptr; // pixel depth is always zero for 2D textures
    }

    if (numberOfFaces != 1) {
        return nullptr; // we don't support cube maps right now
    }

    if (numberOfMipmapLevels == 1) {
        *mipMapped = GrMipMapped::kNo;
    } else {
        int numRequiredMipLevels = SkMipMap::ComputeLevelCount(pixelWidth, pixelHeight)+1;
        if (numberOfMipmapLevels != numRequiredMipLevels) {
            return nullptr;
        }
        *mipMapped = GrMipMapped::kYes;
    }

    if (bytesOfKeyValueData != 0) {
        return nullptr;
    }

    SkTArray<size_t> individualMipOffsets(numberOfMipmapLevels);

    size_t dataSize = GrCompressedDataSize(*compressionType,
                                           { (int) pixelWidth, (int) pixelHeight },
                                           &individualMipOffsets, *mipMapped);
    SkASSERT(individualMipOffsets.size() == numberOfMipmapLevels);

    sk_sp<SkData> data = SkData::MakeUninitialized(dataSize);

    uint8_t* dest = (uint8_t*) data->writable_data();

    size_t offset = 0;
    for (unsigned i = 0; i < numberOfMipmapLevels; ++i) {
        uint32_t imageSize;

        if (input.read(&imageSize, 4) != 4) {
            return nullptr;
        }

        SkASSERT(offset + imageSize <= dataSize);
        SkASSERT(offset == individualMipOffsets[i]);

        if (input.read(&dest[offset], imageSize) != imageSize) {
            return nullptr;
        }

        offset += imageSize;
    }

    return data;
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
class CompressedTexturesGM : public skiagm::GpuGM {
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

        fImgDimensions.set(20, 60);
    }

protected:
    SkString onShortName() override {
        SkString name("compressed_textures");

        if (fType == Type::kNormal) {
            name.append("2");
        } else if (fType == Type::kNonPowerOfTwo) {
            name.append("_npot");
        } else if (fType == Type::kNonMultipleOfFour) {
            name.append("_nmof");
        }

        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(2*kCellWidth + 3*kPad, 2*kBaseTexHeight + 3*kPad);
    }

    void onOnceBeforeDraw() override {
#if 0
        fOpaqueETC2Data = make_compressed_data(fImgDimensions, kRGB_565_SkColorType, true,
                                               SkImage::CompressionType::kETC2_RGB8_UNORM);
#else
        SkISize dim;
        GrMipMapped mipMapped1 = GrMipMapped::kNo;
        SkImage::CompressionType compressionType1 = SkImage::CompressionType::kNone;
        fOpaqueETC2Data = load_ktx("c:\\src\\bugs\\gear-20x60-etc1.ktx",
                                   &dim,
                                   &mipMapped1,
                                   &compressionType1);
        SkASSERT(fImgDimensions == dim);
        SkASSERT(mipMapped1 == GrMipMapped::kYes);
        SkASSERT(compressionType1 == SkImage::CompressionType::kETC2_RGB8_UNORM);
#endif

#if 1
        fOpaqueBC1Data = make_compressed_data(fImgDimensions, kRGBA_8888_SkColorType, true,
                                              SkImage::CompressionType::kBC1_RGB8_UNORM);
#else
        GrMipMapped mipMapped2 = GrMipMapped::kNo;
        SkImage::CompressionType compressionType2 = SkImage::CompressionType::kNone;
        fOpaqueBC1Data = load_ktx("c:\\src\\bugs\\gear-20x60-bc1.ktx",
                                  &mipMapped2,
                                  &compressionType2);
#endif
        fTransparentBC1Data = make_compressed_data(fImgDimensions, kRGBA_8888_SkColorType, false,
                                                   SkImage::CompressionType::kBC1_RGBA8_UNORM);
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        this->drawCell(context, canvas, fOpaqueETC2Data,
                       SkImage::CompressionType::kETC2_RGB8_UNORM, { kPad, kPad });

        this->drawCell(context, canvas, fOpaqueBC1Data,
                       SkImage::CompressionType::kBC1_RGB8_UNORM, { 2*kPad + kCellWidth, kPad });

        this->drawCell(context, canvas, fTransparentBC1Data,
                       SkImage::CompressionType::kBC1_RGBA8_UNORM,
                       { 2*kPad + kCellWidth, 2*kPad + kBaseTexHeight });
    }

private:
    void drawCell(GrContext* context, SkCanvas* canvas, sk_sp<SkData> data,
                  SkImage::CompressionType compression, SkIVector offset) {

        sk_sp<SkImage> image = SkImage::MakeFromCompressed(context, data,
                                                           fImgDimensions.width(),
                                                           fImgDimensions.height(),
                                                           compression, GrMipMapped::kYes);
        SkISize levelDimensions = fImgDimensions;

        int numMipLevels = SkMipMap::ComputeLevelCount(levelDimensions.width(),
                                                       levelDimensions.height()) + 1;

        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality); // to force mipmapping

        for (int i = 0; i < numMipLevels; ++i) {
            SkRect r = SkRect::MakeXYWH(offset.fX, offset.fY,
                                        levelDimensions.width(), levelDimensions.height());

            canvas->drawImageRect(image, r, &paint);

            if (i == 0) {
                offset.fX += levelDimensions.width();
            } else {
                offset.fY += levelDimensions.height();
            }

            levelDimensions = {SkTMax(1, levelDimensions.width()/2),
                               SkTMax(1, levelDimensions.height()/2)};
        }
    }

    static const int kPad = 8;
    static const int kBaseTexWidth = 64;
    static const int kCellWidth = 1.5f * kBaseTexWidth;
    static const int kBaseTexHeight = 64;

    Type          fType;
    SkISize       fImgDimensions;
    sk_sp<SkData> fOpaqueETC2Data;
    sk_sp<SkData> fOpaqueBC1Data;
    sk_sp<SkData> fTransparentBC1Data;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new CompressedTexturesGM(CompressedTexturesGM::Type::kNormal);)
DEF_GM(return new CompressedTexturesGM(CompressedTexturesGM::Type::kNonPowerOfTwo);)
DEF_GM(return new CompressedTexturesGM(CompressedTexturesGM::Type::kNonMultipleOfFour);)

#endif

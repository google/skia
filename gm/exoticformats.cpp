/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/image/SkImage_Base.h"
#include "tools/Resources.h"
#include "tools/gpu/ProxyUtils.h"

using namespace skia_private;

//-------------------------------------------------------------------------------------------------
struct ImageInfo {
    SkISize fDim;
    skgpu::Mipmapped fMipmapped;
    SkTextureCompressionType fCompressionType;
};

/*
 * Get an int from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint32_t get_uint(uint8_t* buffer, uint32_t i) {
    uint32_t result;
    memcpy(&result, &(buffer[i]), 4);
    return result;
}

// This KTX loader is barely sufficient to load the specific files this GM requires. Use
// at your own peril.
static sk_sp<SkData> load_ktx(const char* filename, ImageInfo* imageInfo) {
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

    if (0 != memcmp(header, kExpectedIdentifier, kKTXIdentifierSize)) {
        return nullptr;
    }

    uint32_t endianness = get_uint(header, 12);
    if (endianness != 0x04030201) {
        // TODO: need to swap rest of header and, if glTypeSize is > 1, all
        // the texture data.
        return nullptr;
    }

    uint32_t glType = get_uint(header, 16);
    SkDEBUGCODE(uint32_t glTypeSize = get_uint(header, 20);)
    uint32_t glFormat = get_uint(header, 24);
    uint32_t glInternalFormat = get_uint(header, 28);
    //uint32_t glBaseInternalFormat = get_uint(header, 32);
    uint32_t pixelWidth = get_uint(header, 36);
    uint32_t pixelHeight = get_uint(header, 40);
    uint32_t pixelDepth = get_uint(header, 44);
    //uint32_t numberOfArrayElements = get_uint(header, 48);
    uint32_t numberOfFaces = get_uint(header, 52);
    int numberOfMipmapLevels = get_uint(header, 56);
    uint32_t bytesOfKeyValueData = get_uint(header, 60);

    if (glType != 0 || glFormat != 0) {  // only care about compressed data for now
        return nullptr;
    }
    SkASSERT(glTypeSize == 1); // required for compressed data

    // We only handle these four formats right now
    switch (glInternalFormat) {
        case GR_GL_COMPRESSED_ETC1_RGB8:
        case GR_GL_COMPRESSED_RGB8_ETC2:
            imageInfo->fCompressionType = SkTextureCompressionType::kETC2_RGB8_UNORM;
            break;
        case GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            imageInfo->fCompressionType = SkTextureCompressionType::kBC1_RGB8_UNORM;
            break;
        case GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            imageInfo->fCompressionType = SkTextureCompressionType::kBC1_RGBA8_UNORM;
            break;
        default:
            return nullptr;
    }

    imageInfo->fDim.fWidth = pixelWidth;
    imageInfo->fDim.fHeight = pixelHeight;

    if (pixelDepth != 0) {
        return nullptr; // pixel depth is always zero for 2D textures
    }

    if (numberOfFaces != 1) {
        return nullptr; // we don't support cube maps right now
    }

    if (numberOfMipmapLevels == 1) {
        imageInfo->fMipmapped = skgpu::Mipmapped::kNo;
    } else {
        int numRequiredMipLevels = SkMipmap::ComputeLevelCount(pixelWidth, pixelHeight)+1;
        if (numberOfMipmapLevels != numRequiredMipLevels) {
            return nullptr;
        }
        imageInfo->fMipmapped = skgpu::Mipmapped::kYes;
    }

    if (bytesOfKeyValueData != 0) {
        return nullptr;
    }

    TArray<size_t> individualMipOffsets(numberOfMipmapLevels);

    size_t dataSize = SkCompressedDataSize(imageInfo->fCompressionType,
                                           {(int)pixelWidth, (int)pixelHeight},
                                           &individualMipOffsets,
                                           imageInfo->fMipmapped == skgpu::Mipmapped::kYes);
    SkASSERT(individualMipOffsets.size() == numberOfMipmapLevels);

    sk_sp<SkData> data = SkData::MakeUninitialized(dataSize);

    uint8_t* dest = (uint8_t*) data->writable_data();

    size_t offset = 0;
    for (int i = 0; i < numberOfMipmapLevels; ++i) {
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

//-------------------------------------------------------------------------------------------------
typedef uint32_t DWORD;

// Values for the DDS_PIXELFORMAT 'dwFlags' field
constexpr unsigned int kDDPF_FOURCC      = 0x4;

struct DDS_PIXELFORMAT {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwABitMask;
};

// Values for the DDS_HEADER 'dwFlags' field
constexpr unsigned int kDDSD_CAPS        = 0x1;        // required
constexpr unsigned int kDDSD_HEIGHT      = 0x2;        // required
constexpr unsigned int kDDSD_WIDTH       = 0x4;        // required
constexpr unsigned int kDDSD_PITCH       = 0x8;
constexpr unsigned int kDDSD_PIXELFORMAT = 0x001000;   // required
constexpr unsigned int kDDSD_MIPMAPCOUNT = 0x020000;
constexpr unsigned int kDDSD_LINEARSIZE  = 0x080000;
constexpr unsigned int kDDSD_DEPTH       = 0x800000;

constexpr unsigned int kDDSD_REQUIRED = kDDSD_CAPS | kDDSD_HEIGHT | kDDSD_WIDTH | kDDSD_PIXELFORMAT;

typedef struct {
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwHeight;
    DWORD           dwWidth;
    DWORD           dwPitchOrLinearSize;
    DWORD           dwDepth;
    DWORD           dwMipMapCount;
    DWORD           dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    DWORD           dwCaps;
    DWORD           dwCaps2;
    DWORD           dwCaps3;
    DWORD           dwCaps4;
    DWORD           dwReserved2;
} DDS_HEADER;

// This DDS loader is barely sufficient to load the specific files this GM requires. Use
// at your own peril.
static sk_sp<SkData> load_dds(const char* filename, ImageInfo* imageInfo) {
    SkFILEStream input(filename);
    if (!input.isValid()) {
        return nullptr;
    }

    constexpr uint32_t kMagic = 0x20534444;
    uint32_t magic;

    if (input.read(&magic, 4) != 4) {
        return nullptr;
    }

    if (magic != kMagic) {
        return nullptr;
    }

    constexpr size_t kDDSHeaderSize = sizeof(DDS_HEADER);
    static_assert(kDDSHeaderSize == 124);
    constexpr size_t kDDSPixelFormatSize = sizeof(DDS_PIXELFORMAT);
    static_assert(kDDSPixelFormatSize == 32);

    DDS_HEADER header;

    if (input.read(&header, kDDSHeaderSize) != kDDSHeaderSize) {
        return nullptr;
    }

    if (header.dwSize != kDDSHeaderSize ||
        header.ddspf.dwSize != kDDSPixelFormatSize) {
        return nullptr;
    }

    if ((header.dwFlags & kDDSD_REQUIRED) != kDDSD_REQUIRED) {
        return nullptr;
    }

    if (header.dwFlags & (kDDSD_PITCH | kDDSD_LINEARSIZE | kDDSD_DEPTH)) {
        // TODO: support these features
    }

    imageInfo->fDim.fWidth = header.dwWidth;
    imageInfo->fDim.fHeight = header.dwHeight;

    int numberOfMipmapLevels = 1;
    if (header.dwFlags & kDDSD_MIPMAPCOUNT) {
        if (header.dwMipMapCount == 1) {
            imageInfo->fMipmapped = skgpu::Mipmapped::kNo;
        } else {
            int numRequiredLevels = SkMipmap::ComputeLevelCount(header.dwWidth, header.dwHeight)+1;
            if (header.dwMipMapCount != (unsigned) numRequiredLevels) {
                return nullptr;
            }
            imageInfo->fMipmapped = skgpu::Mipmapped::kYes;
            numberOfMipmapLevels = numRequiredLevels;
        }
    } else {
        imageInfo->fMipmapped = skgpu::Mipmapped::kNo;
    }

    if (!(header.ddspf.dwFlags & kDDPF_FOURCC)) {
        return nullptr;
    }

    // We only handle these one format right now
    switch (header.ddspf.dwFourCC) {
        case 0x31545844: // DXT1
            imageInfo->fCompressionType = SkTextureCompressionType::kBC1_RGB8_UNORM;
            break;
        default:
            return nullptr;
    }

    TArray<size_t> individualMipOffsets(numberOfMipmapLevels);

    size_t dataSize = SkCompressedDataSize(imageInfo->fCompressionType,
                                           {(int)header.dwWidth, (int)header.dwHeight},
                                           &individualMipOffsets,
                                           imageInfo->fMipmapped == skgpu::Mipmapped::kYes);
    SkASSERT(individualMipOffsets.size() == numberOfMipmapLevels);

    sk_sp<SkData> data = SkData::MakeUninitialized(dataSize);

    uint8_t* dest = (uint8_t*) data->writable_data();

    size_t amountRead = input.read(dest, dataSize);
    if (amountRead != dataSize) {
        return nullptr;
    }

    return data;
}

//-------------------------------------------------------------------------------------------------
static sk_sp<SkImage> data_to_img(GrDirectContext *direct, sk_sp<SkData> data,
                                  const ImageInfo& info) {
    if (direct) {
        return SkImages::TextureFromCompressedTextureData(direct,
                                                          std::move(data),
                                                          info.fDim.fWidth,
                                                          info.fDim.fHeight,
                                                          info.fCompressionType,
                                                          info.fMipmapped);
    } else {
        return SkImages::RasterFromCompressedTextureData(
                std::move(data), info.fDim.fWidth, info.fDim.fHeight, info.fCompressionType);
    }
}

namespace skiagm {

// This GM exercises our handling of some of the more exotic formats using externally
// generated content. Right now it only tests ETC1 and BC1.
class ExoticFormatsGM : public GM {
public:
    ExoticFormatsGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    SkString getName() const override { return SkString("exoticformats"); }

    SkISize getISize() override {
        return SkISize::Make(2*kImgWidthHeight + 3 * kPad, kImgWidthHeight + 2 * kPad);
    }

    bool loadImages(GrDirectContext *direct) {
        SkASSERT(!fETC1Image && !fBC1Image);

        {
            ImageInfo info;
            sk_sp<SkData> data = load_ktx(GetResourcePath("images/flower-etc1.ktx").c_str(), &info);
            if (data) {
                SkASSERT(info.fDim.equals(kImgWidthHeight, kImgWidthHeight));
                SkASSERT(info.fMipmapped == skgpu::Mipmapped::kNo);
                SkASSERT(info.fCompressionType == SkTextureCompressionType::kETC2_RGB8_UNORM);

                fETC1Image = data_to_img(direct, std::move(data), info);
            } else {
                SkDebugf("failed to load flower-etc1.ktx\n");
                return false;
            }
        }

        {
            ImageInfo info;
            sk_sp<SkData> data = load_dds(GetResourcePath("images/flower-bc1.dds").c_str(), &info);
            if (data) {
                SkASSERT(info.fDim.equals(kImgWidthHeight, kImgWidthHeight));
                SkASSERT(info.fMipmapped == skgpu::Mipmapped::kNo);
                SkASSERT(info.fCompressionType == SkTextureCompressionType::kBC1_RGB8_UNORM);

                fBC1Image = data_to_img(direct, std::move(data), info);
            } else {
                SkDebugf("failed to load flower-bc1.dds\n");
                return false;
            }
        }

        return true;
    }

    void drawImage(SkCanvas* canvas, SkImage* image, int x, int y) {
        if (!image) {
            return;
        }

        bool isCompressed = false;
        if (image->isTextureBacked()) {
            const GrCaps* caps = as_IB(image)->context()->priv().caps();
            GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(image,
                                                                      canvas->recordingContext());
            isCompressed = caps->isFormatCompressed(proxy->backendFormat());
        }

        canvas->drawImage(image, x, y);

        if (!isCompressed) {
            // Make it obvious which drawImages used decompressed images
            SkRect r = SkRect::MakeXYWH(x, y, kImgWidthHeight, kImgWidthHeight);
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2.0f);
            canvas->drawRect(r, paint);
        }
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (dContext && dContext->abandoned()) {
            // This isn't a GpuGM so a null 'context' is okay but an abandoned context
            // if forbidden.
            return DrawResult::kSkip;
        }

        if (!this->loadImages(dContext)) {
            *errorMsg = "Failed to create images.";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        fETC1Image = nullptr;
        fBC1Image = nullptr;
    }

    void onDraw(SkCanvas* canvas) override {
        SkASSERT(fETC1Image && fBC1Image);

        this->drawImage(canvas, fETC1Image.get(), kPad, kPad);
        this->drawImage(canvas, fBC1Image.get(), kImgWidthHeight + 2 * kPad, kPad);
    }

private:
    static const int kImgWidthHeight = 128;
    static const int kPad = 4;

    sk_sp<SkImage> fETC1Image;
    sk_sp<SkImage> fBC1Image;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ExoticFormatsGM;)
}  // namespace skiagm

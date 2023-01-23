/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegRCodec.h"

#ifdef SK_CODEC_DECODES_JPEGR
#include <array>
#include <csetjmp>
#include <cstdlib>
#include <cstring>
#include <utility>
#include "include/android/SkAndroidFrameworkUtils.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAInfo.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkStreamPriv.h"

static SkEncodedOrigin get_orientation(const std::vector<uint8_t>& exifData) {
    SkEncodedOrigin orientation = kDefault_SkEncodedOrigin;
    if (exifData.size() > 6) {
        constexpr size_t kOffset = 6;
        const size_t exifSize = exifData.size();
        const uint8_t* data = exifData.data();
        SkParseEncodedOrigin(data + kOffset, exifSize - kOffset, &orientation);
    }
    return orientation;
}

bool SkJpegRCodec::IsJpegR(const void* buffer, size_t bytesRead) {
    // We expect JPEGR file will have EXIF packet frist and custom
    // 'JR' EXIF tag first in this EXIF packet. 32 bytes is not enough to
    // parse the whole EXIF (or even the first TAG), but enough to detect
    // this TAG.
    constexpr uint8_t jpegSig[] = {0xFF, 0xD8, 0xFF, 0xE1};
    constexpr uint8_t exifSig[]{'E', 'x', 'i', 'f', '\0', '\0'};

    if (bytesRead < 32 || memcmp(buffer, jpegSig, sizeof(jpegSig))) {
        return false;
    }
    const uint8_t* dataPtr = static_cast<const uint8_t*>(buffer) + 4;
    const uint16_t exifSize = dataPtr[1] | (dataPtr[0] << 8);
    if (exifSize < 26) {
        return false;
    }
    dataPtr += 2;

    if (memcmp(dataPtr, exifSig, sizeof(exifSig))) {
        return false;
    }
    dataPtr += sizeof(exifSig);

    bool isBigEndian = false;
    if (dataPtr[0] == 0x49 && dataPtr[1] == 0x49) {
        isBigEndian = false;
    } else if (dataPtr[0] == 0x4d && dataPtr[1] == 0x4d) {
        isBigEndian = true;
    } else {
        // Wrong EXIF format
        return false;
    }
    dataPtr += 8;  // points to the TAG number

    uint16_t tagNum;
    if (isBigEndian) {
        tagNum = (dataPtr[0] << 8) | dataPtr[1];
    } else {
        tagNum = (dataPtr[1] << 8) | dataPtr[0];
    }
    if (tagNum < 1) {
        return false;
    }
    dataPtr += 2;

    if (dataPtr[0] != 0x4a || dataPtr[1] != 0x52) {
        // Wrong TAG. JPEGR shall start with 0x4a 0x52 TAG
        return false;
    }

    return true;
}

SkCodec::Result SkJpegRCodec::ReadHeader(SkStream* stream,
                                         SkCodec** codecOut,
                                         RecoveryMap** recoveryMapOut) {
    // Create a RecoveryMap object to own all of the decompress information
    std::unique_ptr<RecoveryMap> recoveryMap = std::make_unique<RecoveryMap>();
    jpegr_info_struct jpegRInfo;

    if (codecOut) {
        // Get the encoded color type
        std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
        sk_sp<SkData> data = nullptr;
        if (stream->getMemoryBase()) {
            // It is safe to make without copy because we'll hold onto the stream.
            data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
        } else {
            data = SkCopyStreamToData(stream);
            // We don't need to hold the stream anymore
            stream = nullptr;
        }
        jpegr_compressed_struct compressedImage;
        if (data->data() == nullptr || data->size() == 0) {
            return kIncompleteInput;
        }

        compressedImage.data = (void*)data->data();
        compressedImage.length = data->size();

        std::vector<uint8_t> exifData;
        std::vector<uint8_t> iccData;
        jpegRInfo.exifData = &exifData;
        jpegRInfo.iccData = &iccData;

        if (recoveryMap->getJPEGRInfo(&compressedImage, &jpegRInfo) != 0) {
            return kInvalidInput;
        }

        // JPEGR always report 10-bit color depth
        const uint8_t colorDepth = 10;
        const int bitsPerComponent = 8;

        // iccSkData will outlive iccData as it is passed to profile, hence we need a copy
        sk_sp<SkData> iccSkData = SkData::MakeWithCopy(iccData.data(), iccData.size());
        profile = SkEncodedInfo::ICCProfile::Make(std::move(iccSkData));
        // TODO: Figure out if we need to expose default profile for JPEGR and what it should be

        SkEncodedInfo info = SkEncodedInfo::Make(jpegRInfo.width,
                                                 jpegRInfo.height,
                                                 SkEncodedInfo::kRGBA_Color,
                                                 SkEncodedInfo::kOpaque_Alpha,
                                                 bitsPerComponent,
                                                 std::move(profile),
                                                 colorDepth);

        SkEncodedOrigin orientation = get_orientation(exifData);

        SkJpegRCodec* codec = new SkJpegRCodec(std::move(info),
                                               std::unique_ptr<SkStream>(stream),
                                               recoveryMap.release(),
                                               orientation,
                                               std::move(data));
        *codecOut = codec;
    } else {
        SkASSERT(nullptr != recoveryMap);
        *recoveryMapOut = recoveryMap.release();
    }
    return kSuccess;
}

std::unique_ptr<SkCodec> SkJpegRCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                      Result* result) {
    SkCodec* codec = nullptr;
    *result = ReadHeader(stream.get(), &codec, nullptr);
    if (kSuccess == *result) {
        // Codec has taken ownership of the stream, we do not need to delete it
        SkASSERT(codec);
        stream.release();
        return std::unique_ptr<SkCodec>(codec);
    }
    return nullptr;
}

SkJpegRCodec::SkJpegRCodec(SkEncodedInfo&& info,
                           std::unique_ptr<SkStream> stream,
                           RecoveryMap* recoveryMap,
                           SkEncodedOrigin origin,
                           sk_sp<SkData> data)
        : SkCodec(std::move(info), skcms_PixelFormat_RGBA_1010102, std::move(stream), origin)
        , fRecoveryMap(recoveryMap)
        , fData(std::move(data)) {}
SkJpegRCodec::~SkJpegRCodec() = default;

bool SkJpegRCodec::conversionSupported(const SkImageInfo& dstInfo,
                                       bool srcIsOpaque,
                                       bool needsColorXform) {
    SkASSERT(srcIsOpaque);

    // TODO: Implement color XForm
    SkASSERT(needsColorXform == false);

    if (kUnknown_SkAlphaType == dstInfo.alphaType()) {
        return false;
    }

    if (kOpaque_SkAlphaType != dstInfo.alphaType()) {
        SkCodecPrintf(
                "Warning: an opaque image should be decoded as opaque "
                "- it is being decoded as non-opaque, which will draw slower\n");
    }

    switch (dstInfo.colorType()) {
        case kRGBA_1010102_SkColorType:
            this->setSrcXformFormat(skcms_PixelFormat_RGBA_1010102);
            return true;
        case kRGBA_8888_SkColorType:
            this->setSrcXformFormat(skcms_PixelFormat_RGBA_8888);
            return true;
        default:
            return false;
    }
}

void SkJpegRCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& options) {
    const int srcBPP = 4;

    fSwizzler = SkSwizzler::MakeSimple(srcBPP, dstInfo, options);
    SkASSERT(fSwizzler);
}

void SkJpegRCodec::allocateStorage(const SkImageInfo& dstInfo) {
    int dstWidth = dstInfo.width();

    size_t swizzleBytes = 0;
    const SkEncodedInfo& encodedInfo = this->getEncodedInfo();
    if (fSwizzler) {
        const int srcBPP = encodedInfo.bitsPerPixel() / 8;
        swizzleBytes = srcBPP * encodedInfo.width();
        dstWidth = fSwizzler->swizzleWidth();
        SkASSERT(!this->colorXform() || SkIsAlign4(swizzleBytes));
    }

    size_t totalBytes = swizzleBytes;
    fStorage.reset(totalBytes);
    if (totalBytes > 0) {
        fSwizzleSrcRow = (swizzleBytes > 0) ? fStorage.get() : nullptr;
    }
}

SkSampler* SkJpegRCodec::getSampler(bool createIfNecessary) {
    if (!createIfNecessary || fSwizzler) {
        SkASSERT(!fSwizzler || (fSwizzleSrcRow && fStorage.get() == fSwizzleSrcRow));
        return fSwizzler.get();
    }

    this->initializeSwizzler(this->dstInfo(), this->options());
    this->allocateStorage(this->dstInfo());
    return fSwizzler.get();
}

SkCodec::Result SkJpegRCodec::decodeImage(const SkImageInfo& dstInfo, void* dst) {
    const SkColorType dstColorType = dstInfo.colorType();
    if (dstColorType != kRGBA_1010102_SkColorType && dstColorType != kRGBA_8888_SkColorType) {
        // We only support RGBA1010102 and RGBA8888 colors
        return kUnimplemented;
    }

    const bool decodeSDR = (dstColorType == kRGBA_8888_SkColorType);
    jpegr_compressed_struct compressedImage;
    jpegr_uncompressed_struct decompressedImage;
    compressedImage.data = (void*)fData->data();
    compressedImage.length = fData->size();

    // Decode to intermediate buffer
    if (dst == nullptr) {
        const SkEncodedInfo& encodedInfo = this->getEncodedInfo();
        fDecodedImage.reset(encodedInfo.width() * encodedInfo.height() *
                            encodedInfo.bitsPerPixel() / 8);
        decompressedImage.data = fDecodedImage.get();
    } else {
        decompressedImage.data = dst;
    }

    if (fRecoveryMap->decodeJPEGR(&compressedImage, &decompressedImage, nullptr, decodeSDR) != 0) {
        return kInternalError;
    }
    return kSuccess;
}

SkCodec::Result SkJpegRCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
                                                    const Options& options) {
    // We need to decode the whole image.
    // Decode it and put to a temporary storage
    SkCodec::Result result = kSuccess;
    if ((result = this->decodeImage(dstInfo, nullptr)) != kSuccess) {
        return result;
    }

    if (options.fSubset) {
        this->initializeSwizzler(dstInfo, options);
    } else {
        fSwizzler.reset(nullptr);
    }

    this->allocateStorage(dstInfo);

    return kSuccess;
}

int SkJpegRCodec::onGetScanlines(void* dst, int count, size_t dstRowBytes) {
    uint8_t* decodeDst = (uint8_t*)dst;
    uint32_t* swizzleDst = (uint32_t*)dst;
    size_t decodeDstRowBytes = dstRowBytes;
    size_t swizzleDstRowBytes = dstRowBytes;
    int dstWidth =
            this->options().fSubset ? this->options().fSubset->width() : this->dstInfo().width();
    if (fDecodedImage.get() == nullptr) {
        return 0;
    }
    if (fSwizzleSrcRow) {
        decodeDst = fSwizzleSrcRow;
        decodeDstRowBytes = 0;
        dstWidth = fSwizzler->swizzleWidth();
    }
    const int currentLine = this->nextScanline();
    const int decodeWidth = this->dstInfo().width();
    const int bpp = this->getEncodedInfo().bitsPerPixel() / 8;
    const int decodeStride = decodeWidth * bpp;

    for (int y = 0; y < count; y++) {
        memcpy(decodeDst, fDecodedImage.get() + decodeStride * (y + currentLine), decodeStride);

        if (fSwizzler) {
            fSwizzler->swizzle(swizzleDst, decodeDst);
        }

        decodeDst = SkTAddOffset<uint8_t>(decodeDst, decodeDstRowBytes);
        swizzleDst = SkTAddOffset<uint32_t>(swizzleDst, swizzleDstRowBytes);
    }

    return count;
}

bool SkJpegRCodec::onSkipScanlines(int count) { return true; }

/*
 * Performs the JpegR decode
 */
SkCodec::Result SkJpegRCodec::onGetPixels(const SkImageInfo& dstInfo,
                                          void* dst,
                                          size_t dstRowBytes,
                                          const Options& options,
                                          int* rowsDecoded) {
    if (options.fSubset) {
        return kUnimplemented;
    }
    if (this->dimensions() != dstInfo.dimensions()) {
        // No Scaling
        return kUnimplemented;
    }

    if (dstRowBytes != static_cast<size_t>(this->dimensions().width()) * 4) {
        // TODO: Add stride handling
        return kUnimplemented;
    }

    SkCodec::Result result = kSuccess;
    if ((result = this->decodeImage(dstInfo, dst)) != kSuccess) {
        return result;
    }

    *rowsDecoded = dstInfo.height();

    return kSuccess;
}
#endif  // SK_CODEC_DECODES_JPEGR

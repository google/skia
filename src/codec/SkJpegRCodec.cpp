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
#include "include/private/SkTemplates.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/core/SkStreamPriv.h"

static SkEncodedOrigin get_orientation(const jpegr_info_struct& frameInfo) {
    // TODO: Get orientation from EXIF via librecoverymap
    return kTopLeft_SkEncodedOrigin;
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
        if (recoveryMap->getJPEGRInfo(&compressedImage, &jpegRInfo) != 0) {
            return kInvalidInput;
        }

        // TODO: create profile from ICCProfile

        // JPEGR always report 10-bit color depth
        const uint8_t colorDepth = 10;

        SkEncodedInfo info = SkEncodedInfo::Make(jpegRInfo.width,
                                                 jpegRInfo.height,
                                                 SkEncodedInfo::kRGBA_Color,
                                                 SkEncodedInfo::kOpaque_Alpha,
                                                 colorDepth,
                                                 std::move(profile));

        SkEncodedOrigin orientation = get_orientation(jpegRInfo);

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
        default:
            return false;
    }
}

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

    const SkColorType dstColorType = dstInfo.colorType();
    if (dstColorType != kRGBA_1010102_SkColorType) {
        // We only support RGBA1010102 color
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

    jpegr_compressed_struct compressedImage;
    jpegr_uncompressed_struct decompressedImage;
    compressedImage.data = (void*)fData->data();
    compressedImage.length = fData->size();

    decompressedImage.data = dst;
    if (fRecoveryMap->decodeJPEGR(&compressedImage, &decompressedImage) != 0) {
        return kInternalError;
    }

    *rowsDecoded = dstInfo.height();

    return kSuccess;
}
#endif  // SK_CODEC_DECODES_JPEGR

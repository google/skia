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
#include "include/private/SkMalloc.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/core/SkStreamPriv.h"

static SkEncodedOrigin get_orientation(const jpegr_info_struct& frameInfo) {
    // TODO: Get orientation from EXIF via librecoverymap
    return kTopLeft_SkEncodedOrigin;
}

bool SkJpegRCodec::IsJpegR(const void* buffer, size_t bytesRead) {
    // We do "simple" check here. Instead of XMP parsing, which is expensive,
    // we just search for "RecoveryMap" in the APP01 tag
    constexpr uint8_t jpegSig[] = {0xFF, 0xD8, 0xFF, 0xE1};
    const char nameSpaceSig[] = "http://ns.adobe.com/xap/1.0/";
    const char jpegRSig[] = "RecoveryMap";
    uint8_t idx = 6;  // start of XMP nameSpace
    size_t bytesLeft = bytesRead;
    if (bytesRead < 36 || memcmp(buffer, jpegSig, sizeof(jpegSig))) {
        return false;
    }
    if (strncmp(static_cast<const char*>(buffer) + idx, nameSpaceSig, strlen(nameSpaceSig)) != 0) {
        return false;
    }
    idx += strlen(nameSpaceSig) + 1;  // point to the start of XMP

    // This should not happen - we should have enough bytesLeft
    if (bytesLeft <= idx) {
        SK_ABORT("Unexpected data length.");
    }

    bytesLeft -= idx;
    std::string xmpString(static_cast<const char*>(buffer) + idx, bytesLeft);
    if (xmpString.find(jpegRSig) == std::string::npos) {
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

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkDumbCodec.h"
#include "SkData.h"
#include "SkStream.h"

const char kDumbSig[] = "skdumbcd";
const size_t kDumbSigSize = 8;
const uint32_t kDumbVersion = 1;
const uint32_t kSRGB_ColorSpaceSize = 0xFFFFFFFF;
const uint32_t kMaxExpectedColorSpaceSize = 256 * 1024 * 1024;

static SkEncodedInfo info_to_encoded(const SkImageInfo& info) {
    SkEncodedInfo::Alpha at;
    SkEncodedInfo::Color ct;
    int                  bits;

    switch (info.alphaType()) {
        case kOpaque_SkAlphaType:   at = SkEncodedInfo::kOpaque_Alpha;   break;
        case kPremul_SkAlphaType:   at = SkEncodedInfo::kPremul_Alpha;   break;
        case kUnpremul_SkAlphaType: at = SkEncodedInfo::kUnpremul_Alpha; break;
        default: SK_ABORT("need real alphatype");
    }
    switch (info.colorType()) {
        case kAlpha_8_SkColorType:
            ct = SkEncodedInfo::kAlpha_Color;
            bits = 8; break;
        case kRGB_565_SkColorType:
            ct = SkEncodedInfo::kRGB_Color;
            at = SkEncodedInfo::kOpaque_Alpha;
            bits = 5;
            break;
        case kARGB_4444_SkColorType:
            ct = SkEncodedInfo::kRGBA_Color;
            bits = 4;
            break;
        case kRGBA_8888_SkColorType:
            ct = info.isOpaque() ? SkEncodedInfo::kRGB_Color : SkEncodedInfo::kRGBA_Color;
            bits = 8;
            break;
        case kBGRA_8888_SkColorType:
            ct = info.isOpaque() ? SkEncodedInfo::kBGR_Color : SkEncodedInfo::kBGRA_Color;
            bits = 8;
            break;
        case kGray_8_SkColorType:
            ct = SkEncodedInfo::kGray_Color;
            at = SkEncodedInfo::kOpaque_Alpha;
            bits = 8;
            break;
        case kRGBA_F16_SkColorType:
            ct = info.isOpaque() ? SkEncodedInfo::kRGB_Color : SkEncodedInfo::kRGBA_Color;
            bits = 16;
            break;
        default: SK_ABORT("need real colortype");
    }
    return SkEncodedInfo::Make(ct, at, bits);
}

static SkColorSpaceXform::ColorFormat info_to_xformat(const SkImageInfo& info) {
    // TODO
    return SkColorSpaceXform::kRGBA_8888_ColorFormat;
}

namespace {
    // Don't reorder these enums, just append (can mark entries as deprecated, but don't remove)

    enum StableColorType {
        kFAILED_StableCT,   // never store this value

        kAlpha_8_StableCT,
        kRGB_565_StableCT,
        kARGB_4444_StableCT,
        kRGBA_8888_StableCT,
        kBGRA_8888_StableCT,
        kGray_8_StableCT,
        kRGBA_F16_StableCT,
    };
    enum StableAlphaType {
        kFAILED_StableAT,   // never store this value

        kOpaque_StableAT,
        kPremul_StableAT,
        kUnpremul_StableAT,
    };

    const struct {
        StableColorType fStable;
        SkColorType     fSkia;
    } gCTPairs[] = {
        {   kAlpha_8_StableCT,      kAlpha_8_SkColorType    },
        {   kRGB_565_StableCT,      kRGB_565_SkColorType    },
        {   kARGB_4444_StableCT,    kARGB_4444_SkColorType  },
        {   kRGBA_8888_StableCT,    kRGBA_8888_SkColorType  },
        {   kBGRA_8888_StableCT,    kBGRA_8888_SkColorType  },
        {   kGray_8_StableCT,       kGray_8_SkColorType     },
        {   kRGBA_F16_StableCT,     kRGBA_F16_SkColorType   },
    };
    bool decode_colortype(unsigned src, SkColorType* dst) {
        for (auto pair : gCTPairs) {
            if (pair.fStable == src) {
                *dst = pair.fSkia;
                return true;
            }
        }
        return false;
    }
    StableColorType encode_colortype(SkColorType src) {
        for (auto pair : gCTPairs) {
            if (pair.fSkia == src) {
                return pair.fStable;
            }
        }
        SK_ABORT("need a stable colortype");
        return kFAILED_StableCT;
    }

    const struct {
        StableAlphaType fStable;
        SkAlphaType     fSkia;
    } gATPairs[] = {
        {   kOpaque_StableAT,   kOpaque_SkAlphaType   },
        {   kPremul_StableAT,   kPremul_SkAlphaType   },
        {   kUnpremul_StableAT, kUnpremul_SkAlphaType },
    };
    bool decode_alphatype(unsigned src, SkAlphaType* dst) {
        for (auto pair : gATPairs) {
            if (pair.fStable == src) {
                *dst = pair.fSkia;
                return true;
            }
        }
        return false;
    }
    StableAlphaType encode_alphatype(SkAlphaType src) {
        for (auto pair : gATPairs) {
            if (pair.fSkia == src) {
                return pair.fStable;
            }
        }
        SK_ABORT("need a stable alphatype");
        return kFAILED_StableAT;
    }
}

namespace {
    struct Header {
        uint32_t fSig[2];
        uint32_t fVersion;
        int32_t  fWidth;
        int32_t  fHeight;
        uint32_t fMinRowBytes;
        uint32_t fColorSpaceSize;   // zero means none, -1 means sRGB. Stored right after header
        uint32_t fColorType;
        uint32_t fAlphaType;
    };
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<SkCodec> failure(SkCodec::Result* result, SkCodec::Result reason) {
    *result = reason;
    return nullptr;
}

bool SkDumbCodec::IsDumb(const void* buffer, size_t length) {
    return length >= kDumbSigSize && !memcmp(buffer, kDumbSig, kDumbSigSize);
}

std::unique_ptr<SkCodec> SkDumbCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                    Result* result) {
    Header head;
    static_assert(sizeof(head.fSig) == kDumbSigSize, "wrong size for signature");

    if (stream->read(&head, sizeof(head)) != sizeof(head)) {
        return failure(result, kIncompleteInput);
    }
    if (memcmp(&head.fSig, kDumbSig, sizeof(head.fSig)) || head.fVersion != kDumbVersion) {
        return failure(result, kInvalidInput);
    }
    if (head.fWidth < 0 || head.fHeight < 0) {
        return failure(result, kInvalidInput);
    }

    SkColorType ct;
    SkAlphaType at;
    if (!decode_colortype(head.fColorType, &ct) || !decode_alphatype(head.fAlphaType, &at)) {
        return failure(result, kInvalidInput);
    }

    sk_sp<SkColorSpace> cs;
    if (head.fColorSpaceSize) {
        if (head.fColorSpaceSize == kSRGB_ColorSpaceSize) {
            cs = SkColorSpace::MakeSRGB();
        } else {
            if (head.fColorSpaceSize > kMaxExpectedColorSpaceSize) {
                return failure(result, kInvalidInput);
            }
            SkAutoMalloc storage(head.fColorSpaceSize);
            if (stream->read(storage.get(), head.fColorSpaceSize) != head.fColorSpaceSize) {
                return failure(result, kIncompleteInput);
            }
            cs = SkColorSpace::MakeICC(storage.get(), head.fColorSpaceSize);
            // if we fail to load the cs, we just continue
        }
    }

    SkImageInfo info = SkImageInfo::Make(head.fWidth, head.fHeight, ct, at, cs);
    size_t rowBytes = info.minRowBytes();
    if (rowBytes != head.fMinRowBytes) {
        return failure(result, kInvalidInput);
    }

    size_t size = info.computeByteSize(rowBytes);
    if (SkImageInfo::ByteSizeOverflowed(size)) {
        return failure(result, kInternalError);
    }

    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    char* row = (char*)data->writable_data();
    size_t readSize = info.width() * SkColorTypeBytesPerPixel(info.colorType());
    SkASSERT(readSize <= rowBytes);
    for (int y = 0; y < head.fHeight; ++y) {
        if (stream->read(row, readSize) != readSize) {
            return failure(result, kIncompleteInput);
        }
        row += rowBytes;
    }

    SkASSERT(stream->isAtEnd());

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkDumbCodec(info, rowBytes, std::move(data)));
}

SkDumbCodec::SkDumbCodec(const SkImageInfo& info, size_t rowBytes, sk_sp<SkData> pixels)
    : SkCodec(info.width(), info.height(), info_to_encoded(info), info_to_xformat(info),
              nullptr, sk_ref_sp(info.colorSpace()))
    , fPixmap(info, pixels->data(), rowBytes)
    , fStorage(pixels)
{}

SkCodec::Result SkDumbCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes,
                                         const Options& opts, int* rowsDecoded) {
    if (opts.fSubset) {
        // Subsets are not supported YET
        return kUnimplemented;
    }
    if (dstInfo.dimensions() != fPixmap.info().dimensions()) {
        return kInvalidScale;
    }

    if (!fPixmap.readPixels(dstInfo, dst, dstRowBytes)) {
        return kInvalidConversion;
    }
    return kSuccess;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkDumbCodec::Encode(SkWStream* stream, const SkPixmap& pm) {
    if (pm.width() < 0 || pm.height() < 0 || pm.addr() == nullptr) {
        return false;
    }

    StableColorType ct = encode_colortype(pm.colorType());
    StableAlphaType at = encode_alphatype(pm.alphaType());
    if (ct == kFAILED_StableCT || at == kFAILED_StableAT) {
        return false;
    }

    size_t minRowBytes = pm.info().minRowBytes();
    if (minRowBytes > SK_MaxU32) {
        return false;
    }

    SkColorSpace* cs = pm.colorSpace();
    sk_sp<SkData> csStorage;

    Header head;
    memcpy(head.fSig, kDumbSig, kDumbSigSize);
    head.fVersion = kDumbVersion;
    head.fWidth = pm.width();
    head.fHeight = pm.height();
    head.fMinRowBytes = SkToU32(minRowBytes);
    head.fColorSpaceSize = 0;
    if (cs) {
        if (cs->isSRGB()) {
            head.fColorSpaceSize = kSRGB_ColorSpaceSize;
        } else {
            csStorage = cs->serialize();
            SkASSERT(csStorage->size() <= kMaxExpectedColorSpaceSize);
            head.fColorSpaceSize = SkToU32(csStorage->size());
        }
    }
    head.fColorType = ct;
    head.fAlphaType = at;
    if (!stream->write(&head, sizeof(head))) {
        return false;
    }

    if (csStorage) {
        SkASSERT(head.fColorSpaceSize == csStorage->size());
        if (!stream->write(csStorage->data(), csStorage->size())) {
            return false;
        }
    }
    for (int y = 0; y < pm.height(); ++y) {
        if (!stream->write(pm.addr(0, y), minRowBytes)) {
            return false;
        }
    }
    return true;
}

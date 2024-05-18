/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkGainmapInfo.h"

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/base/SkEndian.h"
#include "src/codec/SkCodecPriv.h"

#include <cmath>
#include <cstdint>
#include <memory>

namespace {
constexpr uint8_t kIsMultiChannelMask = (1u << 7);
constexpr uint8_t kUseBaseColourSpaceMask = (1u << 6);
}  // namespace

static void write_u16_be(SkWStream* s, uint16_t value) {
    value = SkEndian_SwapBE16(value);
    s->write16(value);
}

static void write_u32_be(SkWStream* s, uint32_t value) {
    value = SkEndian_SwapBE32(value);
    s->write32(value);
}

static void write_s32_be(SkWStream* s, int32_t value) {
    value = SkEndian_SwapBE32(value);
    s->write32(value);
}

static void write_rational_be(SkWStream* s, float x) {
    // TODO(b/338342146): Select denominator to get maximum precision and robustness.
    uint32_t denominator = 0x10000000;
    if (std::abs(x) > 1.f) {
        denominator = 0x1000;
    }
    int32_t numerator = static_cast<int32_t>(std::llround(static_cast<double>(x) * denominator));
    write_s32_be(s, numerator);
    write_u32_be(s, denominator);
}

static void write_positive_rational_be(SkWStream* s, float x) {
    // TODO(b/338342146): Select denominator to get maximum precision and robustness.
    uint32_t denominator = 0x10000000;
    if (x > 1.f) {
        denominator = 0x1000;
    }
    uint32_t numerator = static_cast<uint32_t>(std::llround(static_cast<double>(x) * denominator));
    write_u32_be(s, numerator);
    write_u32_be(s, denominator);
}

static bool read_u16_be(SkStream* s, uint16_t* value) {
    if (!s->readU16(value)) {
        return false;
    }
    *value = SkEndian_SwapBE16(*value);
    return true;
}

static bool read_u32_be(SkStream* s, uint32_t* value) {
    if (!s->readU32(value)) {
        return false;
    }
    *value = SkEndian_SwapBE32(*value);
    return true;
}

static bool read_s32_be(SkStream* s, int32_t* value) {
    if (!s->readS32(value)) {
        return false;
    }
    *value = SkEndian_SwapBE32(*value);
    return true;
}

static bool read_rational_be(SkStream* s, float* value) {
    int32_t numerator = 0;
    uint32_t denominator = 0;
    if (!read_s32_be(s, &numerator)) {
        return false;
    }
    if (!read_u32_be(s, &denominator)) {
        return false;
    }
    *value = static_cast<float>(static_cast<double>(numerator) / static_cast<double>(denominator));
    return true;
}

static bool read_positive_rational_be(SkStream* s, float* value) {
    uint32_t numerator = 0;
    uint32_t denominator = 0;
    if (!read_u32_be(s, &numerator)) {
        return false;
    }
    if (!read_u32_be(s, &denominator)) {
        return false;
    }
    *value = static_cast<float>(static_cast<double>(numerator) / static_cast<double>(denominator));
    return true;
}

static bool read_iso_gainmap_version(SkStream* s) {
    // Ensure minimum version is 0.
    uint16_t minimum_version = 0;
    if (!read_u16_be(s, &minimum_version)) {
        SkCodecPrintf("Failed to read ISO 21496-1 minimum version.\n");
        return false;
    }
    if (minimum_version != 0) {
        SkCodecPrintf("Unsupported ISO 21496-1 minimum version.\n");
        return false;
    }

    // Ensure writer version is present. No value is invalid.
    uint16_t writer_version = 0;
    if (!read_u16_be(s, &writer_version)) {
        SkCodecPrintf("Failed to read ISO 21496-1 version.\n");
        return false;
    }

    return true;
}

static bool read_iso_gainmap_info(SkStream* s, SkGainmapInfo& info) {
    if (!read_iso_gainmap_version(s)) {
        SkCodecPrintf("Failed to read ISO 21496-1 version.\n");
        return false;
    }

    uint8_t flags = 0;
    if (!s->readU8(&flags)) {
        SkCodecPrintf("Failed to read ISO 21496-1 flags.\n");
        return false;
    }
    bool isMultiChannel = (flags & kIsMultiChannelMask) != 0;
    bool useBaseColourSpace = (flags & kUseBaseColourSpaceMask) != 0;

    float baseHdrHeadroom = 0.f;
    if (!read_positive_rational_be(s, &baseHdrHeadroom)) {
        SkCodecPrintf("Failed to read ISO 21496-1 base HDR headroom.\n");
        return false;
    }
    float altrHdrHeadroom = 0.f;
    if (!read_positive_rational_be(s, &altrHdrHeadroom)) {
        SkCodecPrintf("Failed to read ISO 21496-1 altr HDR headroom.\n");
        return false;
    }

    float gainMapMin[3] = {0.f};
    float gainMapMax[3] = {0.f};
    float gamma[3] = {0.f};
    float baseOffset[3] = {0.f};
    float altrOffset[3] = {0.f};

    int channelCount = isMultiChannel ? 3 : 1;
    for (int i = 0; i < channelCount; ++i) {
        if (!read_rational_be(s, gainMapMin + i)) {
            SkCodecPrintf("Failed to read ISO 21496-1 gainmap minimum.\n");
            return false;
        }
        if (!read_rational_be(s, gainMapMax + i)) {
            SkCodecPrintf("Failed to read ISO 21496-1 gainmap maximum.\n");
            return false;
        }
        if (!read_positive_rational_be(s, gamma + i)) {
            SkCodecPrintf("Failed to read ISO 21496-1 gamma.\n");
            return false;
        }
        if (!read_rational_be(s, baseOffset + i)) {
            SkCodecPrintf("Failed to read ISO 21496-1 base offset.\n");
            return false;
        }
        if (!read_rational_be(s, altrOffset + i)) {
            SkCodecPrintf("Failed to read ISO 21496-1 altr offset.\n");
            return false;
        }
    }

    info = SkGainmapInfo();
    if (!useBaseColourSpace) {
        info.fGainmapMathColorSpace = SkColorSpace::MakeSRGB();
    }
    if (baseHdrHeadroom < altrHdrHeadroom) {
        info.fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
        info.fDisplayRatioSdr = std::exp2(baseHdrHeadroom);
        info.fDisplayRatioHdr = std::exp2(altrHdrHeadroom);
    } else {
        info.fBaseImageType = SkGainmapInfo::BaseImageType::kHDR;
        info.fDisplayRatioHdr = std::exp2(baseHdrHeadroom);
        info.fDisplayRatioSdr = std::exp2(altrHdrHeadroom);
    }
    for (int i = 0; i < 3; ++i) {
        int j = i >= channelCount ? 0 : i;
        info.fGainmapRatioMin[i] = std::exp2(gainMapMin[j]);
        info.fGainmapRatioMax[i] = std::exp2(gainMapMax[j]);
        info.fGainmapGamma[i] = 1.f / gamma[j];
        switch (info.fBaseImageType) {
            case SkGainmapInfo::BaseImageType::kSDR:
                info.fEpsilonSdr[i] = baseOffset[j];
                info.fEpsilonHdr[i] = altrOffset[j];
                break;
            case SkGainmapInfo::BaseImageType::kHDR:
                info.fEpsilonHdr[i] = baseOffset[j];
                info.fEpsilonSdr[i] = altrOffset[j];
                break;
        }
    }
    return true;
}

bool SkGainmapInfo::isUltraHDRv1Compatible() const {
    // UltraHDR v1 supports having the base image be HDR in theory, but it is largely
    // untested.
    if (fBaseImageType == BaseImageType::kHDR) {
        return false;
    }
    // UltraHDR v1 doesn't support a non-base gainmap math color space.
    if (fGainmapMathColorSpace) {
        return false;
    }
    return true;
}

bool SkGainmapInfo::ParseVersion(const SkData* data) {
    if (!data) {
        return false;
    }
    auto s = SkMemoryStream::MakeDirect(data->data(), data->size());
    return read_iso_gainmap_version(s.get());
}

bool SkGainmapInfo::Parse(const SkData* data, SkGainmapInfo& info) {
    if (!data) {
        return false;
    }
    auto s = SkMemoryStream::MakeDirect(data->data(), data->size());
    return read_iso_gainmap_info(s.get(), info);
}

sk_sp<SkData> SkGainmapInfo::SerializeVersion() {
    SkDynamicMemoryWStream s;
    write_u16_be(&s, 0);  // Minimum reader version
    write_u16_be(&s, 0);  // Writer version
    return s.detachAsData();
}

static bool is_single_channel(SkColor4f c) { return c.fR == c.fG && c.fG == c.fB; };

sk_sp<SkData> SkGainmapInfo::serialize() const {
    SkDynamicMemoryWStream s;
    // Version.
    write_u16_be(&s, 0);  // Minimum reader version
    write_u16_be(&s, 0);  // Writer version

    // Flags.
    bool all_single_channel = is_single_channel(fGainmapRatioMin) &&
                              is_single_channel(fGainmapRatioMax) &&
                              is_single_channel(fGainmapGamma) && is_single_channel(fEpsilonSdr) &&
                              is_single_channel(fEpsilonHdr);
    uint8_t flags = 0;
    if (!fGainmapMathColorSpace) {
        flags |= kUseBaseColourSpaceMask;
    }
    if (!all_single_channel) {
        flags |= kIsMultiChannelMask;
    }
    s.write8(flags);

    // Base and altr headroom.
    switch (fBaseImageType) {
        case SkGainmapInfo::BaseImageType::kSDR:
            write_positive_rational_be(&s, std::log2(fDisplayRatioSdr));
            write_positive_rational_be(&s, std::log2(fDisplayRatioHdr));
            break;
        case SkGainmapInfo::BaseImageType::kHDR:
            write_positive_rational_be(&s, std::log2(fDisplayRatioHdr));
            write_positive_rational_be(&s, std::log2(fDisplayRatioSdr));
            break;
    }

    // Per-channel information.
    for (int i = 0; i < (all_single_channel ? 1 : 3); ++i) {
        write_rational_be(&s, std::log2(fGainmapRatioMin[i]));
        write_rational_be(&s, std::log2(fGainmapRatioMax[i]));
        write_positive_rational_be(&s, 1.f / fGainmapGamma[i]);
        switch (fBaseImageType) {
            case SkGainmapInfo::BaseImageType::kSDR:
                write_rational_be(&s, fEpsilonSdr[i]);
                write_rational_be(&s, fEpsilonHdr[i]);
                break;
            case SkGainmapInfo::BaseImageType::kHDR:
                write_rational_be(&s, fEpsilonHdr[i]);
                write_rational_be(&s, fEpsilonSdr[i]);
                break;
        }
    }
    return s.detachAsData();
}

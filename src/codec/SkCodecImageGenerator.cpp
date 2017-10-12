/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecImageGenerator.h"
#include "SkMakeUnique.h"

static bool should_swap_width_height(SkCodec::Origin o) {
    switch (o) {
        case SkCodec::kLeftTop_Origin:
        case SkCodec::kRightTop_Origin:
        case SkCodec::kRightBottom_Origin:
        case SkCodec::kLeftBottom_Origin:
            return true;
        default:
            break;
    }
    return false;
}

static SkImageInfo swap_width_height(SkImageInfo info) {
    return info.makeWH(info.height(), info.width());
}

enum Flags {
    kInvertX    = 1 << 0,
    kInvertY    = 1 << 1,
    kSwapXY     = 1 << 2,
};

const uint8_t gOrientationFlags[] = {
    0,                              // kTopLeft_Origin
    kInvertX,                       // kTopRight_Origin
    kInvertX | kInvertY,            // kBottomRight_Origin
               kInvertY,            // kBottomLeft_Origin
                          kSwapXY,  // kLeftTop_Origin
               kInvertY | kSwapXY,  // kRightTop_Origin
    kInvertX            | kSwapXY,  // kRightBottom_Origin
    kInvertX | kInvertY | kSwapXY,  // kLeftBottom_Origin
};

static unsigned orientation_to_flags(SkCodec::Origin o) {
    unsigned io = static_cast<int>(o) - 1;

    SkASSERT(io < SK_ARRAY_COUNT(gOrientationFlags));
    return gOrientationFlags[io];
}

template <typename T, typename GetAddr>
void apply_orientation(const SkPixmap& dst, const SkPixmap& src, unsigned flags) {
    const int maxX = src.width() - 1;
    const int maxY = src.height() - 1;
    for (int dy = 0; dy < dst.height(); ++dy) {
        for (int dx = 0; dx < dst.width(); ++dx) {
            int sx = dx;
            int sy = dy;
            if (flags & kInvertX) {
                sx = maxX - sx;
            }
            if (flags & kInvertY) {
                sy = maxY - sy;
            }
            if (flags & kSwapXY) {
                SkTSwap<int>(sx, sy);
            }
            GetAddr(dst, dx, dy) = GetAddr(src, sx, sy);
        }
    }
}

static void apply_orientation(const SkPixmap& dst, const SkPixmap& src, SkCodec::Origin o) {
    SkASSERT(dst.colorType() == src.colorType());

    const int flags = orientation_to_flags(o);
    switch (dst.info().bytesPerPixel()) {
        case 1: apply_orientation<uint8_t, [](const SkPixmap& pm, int x, int y) {
            return *pm.addr8(x, y);
        }>(dst, src, flags); break;
        case 2: apply_orientation<uint16_t, [](const SkPixmap& pm, int x, int y) {
            return *pm.addr16(x, y);
        }); break;
        case 4: apply_orientation<uint32_t, [](const SkPixmap& pm, int x, int y) {
            return *pm.addr32(x, y);
        }); break;
        case 8: apply_orientation<uint64_t, [](const SkPixmap& pm, int x, int y) {
            return *pm.addr64(x, y);
        }); break;
        default:
            sk_throw();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator> SkCodecImageGenerator::MakeFromEncodedCodec(sk_sp<SkData> data) {
    auto codec = SkCodec::MakeFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(new SkCodecImageGenerator(std::move(codec), data));
}

static SkImageInfo adjust_info(SkCodec* codec) {
    SkImageInfo info = codec->getInfo();
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        info = info.makeAlphaType(kPremul_SkAlphaType);
    }
    if (should_swap_width_height(codec->getOrigin())) {
        info = swap_width_height(info);
    }
    return info;
}

SkCodecImageGenerator::SkCodecImageGenerator(std::unique_ptr<SkCodec> codec, sk_sp<SkData> data)
    : INHERITED(adjust_info(codec.get()))
    , fCodec(std::move(codec))
    , fData(std::move(data))
{}

SkData* SkCodecImageGenerator::onRefEncodedData() {
    return SkRef(fData.get());
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& requestInfo, void* requestPixels,
                                        size_t requestRowBytes, const Options& opts) {
    const SkCodec::Origin origin = fCodec->getOrigin();
    const SkPixmap request(requestInfo, requestPixels, requestRowBytes);
    const SkPixmap* codecMap = &request;
    SkAutoPixmapStorage storage;    // used if we have to post-orient the output from the codec

    if (origin != SkCodec::kTopLeft_Origin) {
        SkImageInfo info = requestInfo;
        if (should_swap_width_height(origin)) {
            info = swap_width_height(info);
        }
        // need a tmp buffer to receive the pixels, so we can post-orient them
        if (!storage.tryAlloc(info)) {
            return false;
        }
        codecMap = &storage;
    }

    SkCodec::Options codecOpts;
    codecOpts.fPremulBehavior = opts.fBehavior;
    SkCodec::Result result = fCodec->getPixels(*codecMap, &codecOpts);
    switch (result) {
        case SkCodec::kSuccess:
            if (codecMap != &request) {
                apply_orientation(request, *codecMap, origin);
            }
            // fall through
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            return true;
        default:
            return false;
    }
}

bool SkCodecImageGenerator::onQueryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const
{
    return fCodec->queryYUV8(sizeInfo, colorSpace);
}

bool SkCodecImageGenerator::onGetYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) {
    SkCodec::Result result = fCodec->getYUV8Planes(sizeInfo, planes);

    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            return true;
        default:
            return false;
    }
}

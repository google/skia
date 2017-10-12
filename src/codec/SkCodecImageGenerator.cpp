/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoPixmapStorage.h"
#include "SkCodecImageGenerator.h"
#include "SkMakeUnique.h"

enum Flags {
    kMirrorX    = 1 << 0,
    kMirrorY    = 1 << 1,
    kSwapXY     = 1 << 2,
};

const uint8_t gOrientationFlags[] = {
    0,                              // kTopLeft_Origin
    kMirrorX,                       // kTopRight_Origin
    kMirrorX | kMirrorY,            // kBottomRight_Origin
               kMirrorY,            // kBottomLeft_Origin
                          kSwapXY,  // kLeftTop_Origin
    kMirrorX            | kSwapXY,  // kRightTop_Origin
    kMirrorX | kMirrorY | kSwapXY,  // kRightBottom_Origin
               kMirrorY | kSwapXY,  // kLeftBottom_Origin
};

static unsigned orientation_to_flags(SkCodec::Origin o) {
    unsigned io = static_cast<int>(o) - 1;
    SkASSERT(io < SK_ARRAY_COUNT(gOrientationFlags));
    return gOrientationFlags[io];
}

static bool should_swap_width_height(SkCodec::Origin o) {
    return SkToBool(orientation_to_flags(o) & kSwapXY);
}

static SkImageInfo swap_width_height(SkImageInfo info) {
    return info.makeWH(info.height(), info.width());
}

template <typename T, typename F>
void apply(const SkPixmap& dst, const SkPixmap& src, unsigned flags, F getAddr) {
    const int maxX = dst.width() - 1;
    const int maxY = dst.height() - 1;
    T* dstRow = (T*)dst.writable_addr();
    for (int dy = 0; dy < dst.height(); ++dy) {
        for (int dx = 0; dx < dst.width(); ++dx) {
            int sx = dx;
            int sy = dy;
            if (flags & kMirrorX) {
                sx = maxX - sx;
            }
            if (flags & kMirrorY) {
                sy = maxY - sy;
            }
            if (flags & kSwapXY) {
                SkTSwap<int>(sx, sy);
            }
            dstRow[dx] = getAddr(src, sx, sy);
        }
        dstRow = (T*)((char*)dstRow + dst.rowBytes());
    }
}

static void apply_orientation(const SkPixmap& dst, const SkPixmap& src, SkCodec::Origin o) {
    SkASSERT(dst.colorType() == src.colorType());

    const int flags = orientation_to_flags(o);
    switch (dst.info().bytesPerPixel()) {
        case 1:
            apply<uint8_t>(dst, src, flags, [](const SkPixmap& pm, int x, int y) {
                return *pm.addr8(x, y);
            }); break;
        case 2:
            apply<uint16_t>(dst, src, flags, [](const SkPixmap& pm, int x, int y) {
                return *pm.addr16(x, y);
            }); break;
        case 4:
            apply<uint32_t>(dst, src, flags, [](const SkPixmap& pm, int x, int y) {
                return *pm.addr32(x, y);
            }); break;
        case 8:
            apply<uint64_t>(dst, src, flags, [](const SkPixmap& pm, int x, int y) {
                return *pm.addr64(x, y);
            }); break;
        default:
            SK_ABORT("unexpected bytesPerPixel");
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

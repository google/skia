/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlitter_A8.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h" // IWYU pragma: keep
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkDrawTypes.h"
#include "src/core/SkMask.h"

#include <cstring>
#include <optional>

SkA8_Coverage_Blitter::SkA8_Coverage_Blitter(const SkPixmap& device, const SkPaint& paint)
    : fDevice(device)
{
    SkASSERT(nullptr == paint.getShader());
    SkASSERT(nullptr == paint.getColorFilter());
}

void SkA8_Coverage_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                      const int16_t runs[]) {
    uint8_t* device = fDevice.writable_addr8(x, y);
    SkDEBUGCODE(int totalCount = 0;)

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count == 0) {
            return;
        }
        if (antialias[0]) {
            memset(device, antialias[0], count);
        }
        runs += count;
        antialias += count;
        device += count;

        SkDEBUGCODE(totalCount += count;)
    }
    SkASSERT(fDevice.width() == totalCount);
}

void SkA8_Coverage_Blitter::blitH(int x, int y, int width) {
    memset(fDevice.writable_addr8(x, y), 0xFF, width);
}

void SkA8_Coverage_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (0 == alpha) {
        return;
    }

    uint8_t* dst = fDevice.writable_addr8(x, y);
    const size_t dstRB = fDevice.rowBytes();
    while (--height >= 0) {
        *dst = alpha;
        dst += dstRB;
    }
}

void SkA8_Coverage_Blitter::blitRect(int x, int y, int width, int height) {
    uint8_t* dst = fDevice.writable_addr8(x, y);
    const size_t dstRB = fDevice.rowBytes();
    while (--height >= 0) {
        memset(dst, 0xFF, width);
        dst += dstRB;
    }
}

void SkA8_Coverage_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (SkMask::kA8_Format != mask.fFormat) {
        this->SkBlitter::blitMask(mask, clip);
        return;
    }

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    uint8_t* dst = fDevice.writable_addr8(x, y);
    const uint8_t* src = mask.getAddr8(x, y);
    const size_t srcRB = mask.fRowBytes;
    const size_t dstRB = fDevice.rowBytes();

    while (--height >= 0) {
        memcpy(dst, src, width);
        dst += dstRB;
        src += srcRB;
    }
}

//////////////

static inline uint8_t div255(unsigned prod) {
    SkASSERT(prod <= 255*255);
    return (prod + 128) * 257 >> 16;
}

static inline unsigned u8_lerp(uint8_t a, uint8_t b, uint8_t t) {
    return div255((255 - t) * a + t * b);
}

using AlphaProc = uint8_t(*)(uint8_t src, uint8_t dst);

static uint8_t srcover_p (uint8_t src, uint8_t dst) { return src + div255((255 - src) * dst); }
static uint8_t src_p     (uint8_t src, uint8_t dst) { return src; }

template <typename Mode> void A8_row_bw(uint8_t dst[], uint8_t src, int N, Mode proc) {
    for (int i = 0; i < N; ++i) {
        dst[i] = proc(src, dst[i]);
    }
}
using A8_RowBlitBW = void(*)(uint8_t[], uint8_t, int);

template <typename Mode>
void A8_row_aa(uint8_t dst[], uint8_t src, int N, uint8_t aa, Mode proc, const bool canFoldAA) {
    if (canFoldAA) {
        src = div255(src * aa);
        for (int i = 0; i < N; ++i) {
            dst[i] = proc(src, dst[i]);
        }
    } else {
        for (int i = 0; i < N; ++i) {
            dst[i] = u8_lerp(dst[i], proc(src, dst[i]), aa);
        }
    }
}
using A8_RowBlitAA = void(*)(uint8_t[], uint8_t, int, uint8_t aa);

#define WRAP_BLIT(proc, canFoldAA)                      \
    proc,                                               \
    [](uint8_t dst[], uint8_t src, int N)               \
      { A8_row_bw(dst, src, N, proc); },                \
    [](uint8_t dst[], uint8_t src, int N, uint8_t aa)   \
      { A8_row_aa(dst, src, N, aa, proc, canFoldAA); }

struct A8_RowBlitBWPair {
    SkBlendMode     mode;
    AlphaProc       oneProc;
    A8_RowBlitBW    bwProc;
    A8_RowBlitAA    aaProc;
};
constexpr A8_RowBlitBWPair gA8_RowBlitPairs[] = {
    {SkBlendMode::kSrcOver,  WRAP_BLIT(srcover_p,  true)},
    {SkBlendMode::kSrc,      WRAP_BLIT(src_p,      false)},
};
#undef WRAP_BLIT

static const A8_RowBlitBWPair* find_a8_rowproc_pair(SkBlendMode bm) {
    for (auto& pair : gA8_RowBlitPairs) {
        if (pair.mode == bm) {
            return &pair;
        }
    }
    return nullptr;
}

class SkA8_Blitter : public SkBlitter {
public:
    SkA8_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect&) override;

private:
    const SkPixmap  fDevice;
    AlphaProc       fOneProc;
    A8_RowBlitBW    fBWProc;
    A8_RowBlitAA    fAAProc;
    SkAlpha         fSrc;

    using INHERITED = SkBlitter;
};

SkA8_Blitter::SkA8_Blitter(const SkPixmap& device,
                           const SkPaint& paint) : fDevice(device) {
    SkASSERT(nullptr == paint.getShader());
    SkASSERT(nullptr == paint.getColorFilter());
    auto mode = paint.asBlendMode();
    SkASSERT(mode);
    auto pair = find_a8_rowproc_pair(*mode);
    SkASSERT(pair);

    fOneProc = pair->oneProc;
    fBWProc  = pair->bwProc;
    fAAProc  = pair->aaProc;
    fSrc = paint.getAlpha();
}

void SkA8_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) {
    uint8_t* device = fDevice.writable_addr8(x, y);
    SkDEBUGCODE(int totalCount = 0;)

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count == 0) {
            return;
        }

        if (antialias[0] == 0xFF) {
            fBWProc(device, fSrc, count);
        } else if (antialias[0] != 0) {
            fAAProc(device, fSrc, count, antialias[0]);
        }

        runs += count;
        antialias += count;
        device += count;

        SkDEBUGCODE(totalCount += count;)
    }
    SkASSERT(fDevice.width() == totalCount);
}

void SkA8_Blitter::blitH(int x, int y, int width) {
    fBWProc(fDevice.writable_addr8(x, y), fSrc, width);
}

void SkA8_Blitter::blitV(int x, int y, int height, SkAlpha aa) {
    uint8_t* device = fDevice.writable_addr8(x, y);
    const size_t dstRB = fDevice.rowBytes();

    if (aa == 0xFF) {
        while (--height >= 0) {
            *device = fOneProc(fSrc, *device);
            device += dstRB;
        }
    } else if (aa != 0) {
        while (--height >= 0) {
            fAAProc(device, fSrc, 1, aa);
            device += dstRB;
        }
    }
}

void SkA8_Blitter::blitRect(int x, int y, int width, int height) {
    uint8_t* device = fDevice.writable_addr8(x, y);
    const size_t dstRB = fDevice.rowBytes();

    while (--height >= 0) {
        fBWProc(device, fSrc, width);
        device += dstRB;
    }
}

void SkA8_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (SkMask::kA8_Format != mask.fFormat) {
        this->INHERITED::blitMask(mask, clip);
        return;
    }

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    uint8_t* dst = fDevice.writable_addr8(x, y);
    const uint8_t* src = mask.getAddr8(x, y);
    const size_t srcRB = mask.fRowBytes;
    const size_t dstRB = fDevice.rowBytes();

    while (--height >= 0) {
        for (int i = 0; i < width; ++i) {
            dst[i] = u8_lerp(dst[i], fOneProc(fSrc, dst[i]), src[i]);
        }
        dst += dstRB;
        src += srcRB;
    }
}

//////////////////

SkBlitter* SkA8Blitter_Choose(const SkPixmap& dst,
                              const SkMatrix& ctm,
                              const SkPaint& paint,
                              SkArenaAlloc* alloc,
                              SkDrawCoverage drawCoverage,
                              sk_sp<SkShader> clipShader,
                              const SkSurfaceProps&) {
    if (dst.colorType() != SkColorType::kAlpha_8_SkColorType) {
        return nullptr;
    }
    if (paint.getShader() || paint.getColorFilter()) {
        return nullptr;
    }
    if (clipShader) {
        return nullptr; // would not be hard to support ...?
    }

    if (drawCoverage == SkDrawCoverage::kYes) {
        return alloc->make<SkA8_Coverage_Blitter>(dst, paint);
    } else {
        // we only support certain blendmodes...
        auto mode = paint.asBlendMode();
        if (mode && find_a8_rowproc_pair(*mode)) {
            return alloc->make<SkA8_Blitter>(dst, paint);
        }
    }
    return nullptr;
}

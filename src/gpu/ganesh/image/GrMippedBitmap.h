/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMippedBitmap_DEFINED
#define GrMippedBitmap_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkMipmap.h"

#include <optional>

class SkMipmap;

// A wrapper for an immutable SkBitmap that includes ganesh-specific things, such as mipmaps.
class GrMippedBitmap {
public:
    explicit GrMippedBitmap(SkBitmap b) : fBitmap(b) {}
    explicit GrMippedBitmap(SkBitmap b, sk_sp<const SkMipmap> mipmaps)
            : fBitmap(b), fMips(mipmaps) {}

    ~GrMippedBitmap() = default;
    GrMippedBitmap(const GrMippedBitmap&) = default;
    GrMippedBitmap(GrMippedBitmap&&) = default;
    GrMippedBitmap& operator=(const GrMippedBitmap&) = default;
    GrMippedBitmap& operator=(GrMippedBitmap&&) = default;

    SkAlphaType alphaType() const { return fBitmap.alphaType(); }
    SkBitmap bitmap() const { return fBitmap; }
    SkColorType colorType() const { return fBitmap.colorType(); }
    sk_sp<const SkMipmap> mips() const { return fMips; }

    using ReleaseProc = void(void* pixels, void* context);

    static std::optional<GrMippedBitmap> Make(
            SkImageInfo, const void* pixels, size_t rowBytes, ReleaseProc, void* context);
    static std::optional<GrMippedBitmap> Make(SkImageInfo ii, const void* pixels, size_t rowBytes) {
        return Make(ii, pixels, rowBytes, nullptr, nullptr);
    }
    static std::optional<GrMippedBitmap> Make(const SkPixmap& p) {
        return Make(p.info(), p.addr(), p.rowBytes());
    }

private:
    SkBitmap fBitmap;
    sk_sp<const SkMipmap> fMips;
};

#endif

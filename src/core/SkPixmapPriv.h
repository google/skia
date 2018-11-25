/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmapPriv_DEFINED
#define SkPixmapPriv_DEFINED

#include "SkPixmap.h"
#include "SkEncodedOrigin.h"
#include "SkAutoPixmapStorage.h"

class SkPixmapPriv {
public:
    // These flag are applied in this order (swap is applied last)
    enum OrientFlags {
        kMirrorX = 1 << 0,
        kMirrorY = 1 << 1,
        kSwapXY  = 1 << 2,
    };

    static OrientFlags OriginToOrient(SkEncodedOrigin);

    /**
     *  Copy the pixels in this pixmap into dst, applying the orientation transformations specified
     *  by the flags. If the inputs are invalid, this returns false and no copy is made.
     */
    static bool Orient(const SkPixmap& dst, const SkPixmap& src, OrientFlags);

    static bool ShouldSwapWidthHeight(SkEncodedOrigin o);
    static SkImageInfo SwapWidthHeight(const SkImageInfo& info);

    /**
     *  Decode an image and then copy into dst, applying origin.
     *
     *  @param dst SkPixmap to write the final image, after
     *      applying the origin.
     *  @param origin SkEncodedOrigin to apply to the raw pixels.
     *  @param decode Function for decoding into a pixmap without
     *      applying the origin.
     */
    static bool Orient(const SkPixmap& dst, SkEncodedOrigin origin,
            std::function<bool(const SkPixmap&)> decode) {
        SkAutoPixmapStorage storage;
        const SkPixmap* tmp = &dst;
        if (origin != kTopLeft_SkEncodedOrigin) {
            auto info = dst.info();
            if (ShouldSwapWidthHeight(origin)) {
                info = SwapWidthHeight(info);
            }
            if (!storage.tryAlloc(info)) {
                return false;
            }
            tmp = &storage;
        }
        if (!decode(*tmp)) {
            return false;
        }
        if (tmp != &dst) {
            return Orient(dst, *tmp, OriginToOrient(origin));
        }
        return true;
    }

    static void ResetPixmapKeepInfo(SkPixmap* pm, const void* address, size_t rowBytes) {
        pm->fRowBytes = rowBytes;
        pm->fPixels = address;
    }
};

#endif


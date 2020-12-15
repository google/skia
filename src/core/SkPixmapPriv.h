/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmapPriv_DEFINED
#define SkPixmapPriv_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkAutoPixmapStorage.h"

class SkPixmapPriv {
public:
    /**
     *  Copy the pixels in this pixmap into dst, applying the orientation transformations specified
     *  by the flags. If the inputs are invalid, this returns false and no copy is made.
     */
    static bool Orient(const SkPixmap& dst, const SkPixmap& src, SkEncodedOrigin);

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

    template <typename Fn>
    static bool Orient(const SkPixmap& dst, SkEncodedOrigin origin, Fn&& decode) {
        SkAutoPixmapStorage storage;
        const SkPixmap* tmp = &dst;
        if (origin != kTopLeft_SkEncodedOrigin) {
            auto info = dst.info();
            if (SkEncodedOriginSwapsWidthHeight(origin)) {
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
            return Orient(dst, *tmp, origin);
        }
        return true;
    }

    static void ResetPixmapKeepInfo(SkPixmap* pm, const void* address, size_t rowBytes) {
        pm->fRowBytes = rowBytes;
        pm->fPixels = address;
    }
};

#endif

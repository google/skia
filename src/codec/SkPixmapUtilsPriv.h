/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmapUtilsPriv_DEFINED
#define SkPixmapUtilsPriv_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/codec/SkPixmapUtils.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkAutoPixmapStorage.h"

namespace SkPixmapUtils {

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
bool Orient(const SkPixmap& dst, SkEncodedOrigin origin, Fn&& decode) {
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

}  // namespace SkPixmapUtils

#endif // SkPixmapUtilsPriv_DEFINED

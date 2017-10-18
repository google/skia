/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmapPriv_DEFINED
#define SkPixmapPriv_DEFINED

#include "SkPixmap.h"
#include "SkCodec.h"

class SkPixmapPriv {
public:
    // These flag are applied in this order (swap is applied last)
    enum OrientFlags {
        kMirrorX = 1 << 0,
        kMirrorY = 1 << 1,
        kSwapXY  = 1 << 2,
    };

    static OrientFlags OriginToOrient(SkCodec::Origin);

    /**
     *  Copy the pixels in this pixmap into dst, applying the orientation transformations specified
     *  by the flags. If the inputs are invalid, this returns false and no copy is made.
     */
    static bool Orient(const SkPixmap& dst, const SkPixmap& src, OrientFlags);
};

#endif


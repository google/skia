/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureUtils_DEFINED
#define SkPictureUtils_DEFINED

#include "SkPicture.h"

class SkData;
struct SkRect;

class SK_API SkPictureUtils {
public:
    /**
     *  Given a rectangular visible "window" into the picture, return an array
     *  of SkPixelRefs that might intersect that area. To keep the call fast,
     *  the returned list is not guaranteed to be exact, so it may miss some,
     *  and it may return false positives.
     *
     *  The pixelrefs returned in the SkData are already owned by the picture,
     *  so the returned pointers are only valid while the picture is in scope
     *  and remains unchanged.
     */
    static SkData* GatherPixelRefs(SkPicture* pict, const SkRect& area);
};

#endif

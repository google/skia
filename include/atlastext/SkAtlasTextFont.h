/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtlasTextFont_DEFINED
#define SkAtlasTextFont_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"

/** Represents a font at a size. TODO: What else do we need here (skewX, scaleX, vertical, ...)? */
class SK_API SkAtlasTextFont : public SkRefCnt {
public:
    static sk_sp<SkAtlasTextFont> Make(sk_sp<SkTypeface> typeface, SkScalar size) {
        return sk_sp<SkAtlasTextFont>(new SkAtlasTextFont(std::move(typeface), size));
    }

    SkTypeface* typeface() const { return fTypeface.get(); }

    sk_sp<SkTypeface> refTypeface() const { return fTypeface; }

    SkScalar size() const { return fSize; }

    SkFont makeFont() const { return SkFont(fTypeface, fSize); }

private:
    SkAtlasTextFont(sk_sp<SkTypeface> typeface, SkScalar size)
            : fTypeface(std::move(typeface)), fSize(size) {}

    sk_sp<SkTypeface> fTypeface;
    SkScalar fSize;
};

#endif

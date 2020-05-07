/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCustomTypeface_DEFINED
#define SkCustomTypeface_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkTypeface.h"

#include <vector>

class SkStream;

class SkCustomTypefaceBuilder {
public:
    SkCustomTypefaceBuilder(int numGlyphs);

    void setGlyph(SkGlyphID, float advance, const SkPath&);
    void setGlyph(SkGlyphID, float advance, const SkPath&, const SkPaint&);
    void setGlyph(SkGlyphID, float advance, sk_sp<SkImage>, float scale);
    void setGlyph(SkGlyphID, float advance, sk_sp<SkPicture>);

    sk_sp<SkTypeface> detach();

private:
    int fGlyphCount;
    std::vector<SkPath> fPaths;
    std::vector<float>  fAdvances;

    static sk_sp<SkTypeface> Deserialize(SkStream*);

    friend class SkTypeface;
};

#endif

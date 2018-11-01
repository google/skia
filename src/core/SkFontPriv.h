/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontPriv_DEFINED
#define SkFontPriv_DEFINED

#include "SkMatrix.h"
#include "SkFont.h"
#include "SkTypeface.h"

class SkReadBuffer;
class SkWriteBuffer;

class SkFontPriv {
public:
    static void ScaleFontMetrics(SkFontMetrics*, SkScalar);

    /**
     *  Return a matrix that applies the paint's text values: size, scale, skew
     */
    static void MakeTextMatrix(SkMatrix* matrix, SkScalar size, SkScalar scaleX, SkScalar skewX) {
        matrix->setScale(size * scaleX, size);
        if (skewX) {
            matrix->postSkew(skewX, 0);
        }
    }

    static void MakeTextMatrix(SkMatrix* matrix, const SkFont& font) {
        MakeTextMatrix(matrix, font.getSize(), font.getScaleX(), font.getSkewX());
    }

    // returns -1 if buffer is invalid for specified encoding
    static int ValidCountText(const void* text, size_t length, SkTextEncoding);

    static SkTypeface* GetTypefaceOrDefault(const SkFont& font) {
        return font.getTypeface() ? font.getTypeface() : SkTypeface::GetDefaultTypeface();
    }

    static sk_sp<SkTypeface> RefTypefaceOrDefault(const SkFont& font) {
        return font.getTypeface() ? font.refTypeface() : SkTypeface::MakeDefault();
    }

    static void Flatten(const SkFont& font, SkWriteBuffer& buffer);
    static bool Unflatten(SkFont* font, SkReadBuffer& buffer);
};

#endif

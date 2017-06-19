/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintPriv_DEFINED
#define SkPaintPriv_DEFINED

#include "SkImageInfo.h"
#include "SkPaint.h"
#include "SkMatrix.h"

class SkBitmap;
class SkImage;

class SkPaintPriv {
public:
    enum ShaderOverrideOpacity {
        kNone_ShaderOverrideOpacity,        //!< there is no overriding shader (bitmap or image)
        kOpaque_ShaderOverrideOpacity,      //!< the overriding shader is opaque
        kNotOpaque_ShaderOverrideOpacity,   //!< the overriding shader may not be opaque
    };

    /**
     *  Returns true if drawing with this paint (or nullptr) will ovewrite all affected pixels.
     *
     *  Note: returns conservative true, meaning it may return false even though the paint might
     *        in fact overwrite its pixels.
     */
    static bool Overwrites(const SkPaint* paint, ShaderOverrideOpacity);

    static bool Overwrites(const SkPaint& paint) {
        return Overwrites(&paint, kNone_ShaderOverrideOpacity);
    }

    /**
     *  Returns true if drawing this bitmap with this paint (or nullptr) will ovewrite all affected
     *  pixels.
     */
    static bool Overwrites(const SkBitmap&, const SkPaint* paint);

    /**
     *  Returns true if drawing this image with this paint (or nullptr) will ovewrite all affected
     *  pixels.
     */
    static bool Overwrites(const SkImage*, const SkPaint* paint);

    static void ScaleFontMetrics(SkPaint::FontMetrics*, SkScalar);

    /**
     *  Return a matrix that applies the paint's text values: size, scale, skew
     */
    static void MakeTextMatrix(SkMatrix* matrix, SkScalar size, SkScalar scaleX, SkScalar skewX) {
        matrix->setScale(size * scaleX, size);
        if (skewX) {
            matrix->postSkew(skewX, 0);
        }
    }

    static void MakeTextMatrix(SkMatrix* matrix, const SkPaint& paint) {
        MakeTextMatrix(matrix, paint.getTextSize(), paint.getTextScaleX(), paint.getTextSkewX());
    }
    
    static bool ShouldDither(const SkPaint&, SkColorType);
};

#endif

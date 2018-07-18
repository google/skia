/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintPriv_DEFINED
#define SkPaintPriv_DEFINED

#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkTypeface.h"

class SkBitmap;
class SkImage;
class SkReadBuffer;
class SkWriteBuffer;

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

    // returns -1 if buffer is invalid for specified encoding
    static int ValidCountText(const void* text, size_t length, SkPaint::TextEncoding);

    static SkTypeface* GetTypefaceOrDefault(const SkPaint& paint) {
        return paint.getTypeface() ? paint.getTypeface() : SkTypeface::GetDefaultTypeface();
    }

    static sk_sp<SkTypeface> RefTypefaceOrDefault(const SkPaint& paint) {
        return paint.getTypeface() ? paint.refTypeface() : SkTypeface::MakeDefault();
    }

    /** Serializes SkPaint into a buffer. A companion unflatten() call
    can reconstitute the paint at a later time.

    @param buffer  SkWriteBuffer receiving the flattened SkPaint data
    */
    static void Flatten(const SkPaint& paint, SkWriteBuffer& buffer);

    /** Populates SkPaint, typically from a serialized stream, created by calling
    flatten() at an earlier time.

    SkReadBuffer class is not public, so unflatten() cannot be meaningfully called
    by the client.

    @param buffer  serialized data describing SkPaint content
    @return        false if the buffer contains invalid data
    */
    static bool Unflatten(SkPaint* paint, SkReadBuffer& buffer);

};

#endif

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintPriv_DEFINED
#define SkPaintPriv_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"

class SkColorSpace;
class SkReadBuffer;
class SkWriteBuffer;
enum SkColorType : int;

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

    static bool ShouldDither(const SkPaint&, SkColorType);

    /*
     * The luminance color is used to determine which Gamma Canonical color to map to.  This is
     * really only used by backends which want to cache glyph masks, and need some way to know if
     * they need to generate new masks based off a given color.
     */
    static SkColor ComputeLuminanceColor(const SkPaint&);

    /** Serializes SkPaint into a buffer. A companion unflatten() call
    can reconstitute the paint at a later time.

    @param buffer  SkWriteBuffer receiving the flattened SkPaint data
    */
    static void Flatten(const SkPaint& paint, SkWriteBuffer& buffer);

    /** Populates SkPaint, typically from a serialized stream, created by calling
        flatten() at an earlier time.
    */
    static SkPaint Unflatten(SkReadBuffer& buffer);

    // If this paint has any color filter, fold it into the shader and/or paint color
    // so that it draws the same but getColorFilter() returns nullptr.
    //
    // Since we may be filtering now, we need to know what color space to filter in,
    // typically the color space of the device we're drawing into.
    static void RemoveColorFilter(SkPaint*, SkColorSpace* dstCS);

};

#endif

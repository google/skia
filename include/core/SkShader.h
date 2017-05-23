/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShader_DEFINED
#define SkShader_DEFINED

#include "SkBitmap.h"
#include "SkFilterQuality.h"
#include "SkFlattenable.h"
#include "SkImageInfo.h"
#include "SkMask.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "../gpu/GrColor.h"

class SkArenaAlloc;
class SkColorFilter;
class SkColorSpace;
class SkColorSpaceXformer;
class SkImage;
class SkPath;
class SkPicture;
class SkRasterPipeline;
class GrContext;
class GrFragmentProcessor;

/** \class SkShader
 *
 *  Shaders specify the source color(s) for what is being drawn. If a paint
 *  has no shader, then the paint's color is used. If the paint has a
 *  shader, then the shader's color(s) are use instead, but they are
 *  modulated by the paint's alpha. This makes it easy to create a shader
 *  once (e.g. bitmap tiling or gradient) and then change its transparency
 *  w/o having to modify the original shader... only the paint's alpha needs
 *  to be modified.
 */
class SK_API SkShader : public SkFlattenable {
public:
    enum TileMode {
        /** replicate the edge color if the shader draws outside of its
         *  original bounds
         */
        kClamp_TileMode,

        /** repeat the shader's image horizontally and vertically */
        kRepeat_TileMode,

        /** repeat the shader's image horizontally and vertically, alternating
         *  mirror images so that adjacent images always seam
         */
        kMirror_TileMode,

#if 0
        /** only draw within the original domain, return 0 everywhere else */
        kDecal_TileMode,
#endif
    };

    enum {
        kTileModeCount = kMirror_TileMode + 1
    };

    /**
     *  Returns the local matrix.
     *
     *  FIXME: This can be incorrect for a Shader with its own local matrix
     *  that is also wrapped via CreateLocalMatrixShader.
     */
    const SkMatrix& getLocalMatrix() const;

    /**
     *  Returns true if the shader is guaranteed to produce only opaque
     *  colors, subject to the SkPaint using the shader to apply an opaque
     *  alpha value. Subclasses should override this to allow some
     *  optimizations.
     */
    virtual bool isOpaque() const { return false; }

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    /**
     *  Returns true if this shader is just a bitmap, and if not null, returns the bitmap,
     *  localMatrix, and tilemodes. If this is not a bitmap, returns false and ignores the
     *  out-parameters.
     */
    bool isABitmap(SkBitmap* outTexture, SkMatrix* outMatrix, TileMode xy[2]) const;

    bool isABitmap() const {
        return this->isABitmap(nullptr, nullptr, nullptr);
    }
#endif

    /**
     *  Iff this shader is backed by a single SkImage, return its ptr (the caller must ref this
     *  if they want to keep it longer than the lifetime of the shader). If not, return nullptr.
     */
    SkImage* isAImage(SkMatrix* localMatrix, TileMode xy[2]) const;

    bool isAImage() const {
        return this->isAImage(nullptr, nullptr) != nullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //  Methods to create combinations or variants of shaders

    /**
     *  Return a shader that will apply the specified localMatrix to this shader.
     *  The specified matrix will be applied before any matrix associated with this shader.
     */
    sk_sp<SkShader> makeWithLocalMatrix(const SkMatrix&) const;

    /**
     *  Create a new shader that produces the same colors as invoking this shader and then applying
     *  the colorfilter.
     */
    sk_sp<SkShader> makeWithColorFilter(sk_sp<SkColorFilter>) const;

    //////////////////////////////////////////////////////////////////////////
    //  Factory methods for stock shaders

    /**
     *  Call this to create a new "empty" shader, that will not draw anything.
     */
    static sk_sp<SkShader> MakeEmptyShader();

    /**
     *  Call this to create a new shader that just draws the specified color. This should always
     *  draw the same as a paint with this color (and no shader).
     */
    static sk_sp<SkShader> MakeColorShader(SkColor);

    /**
     *  Create a shader that draws the specified color (in the specified colorspace).
     *
     *  This works around the limitation that SkPaint::setColor() only takes byte values, and does
     *  not support specific colorspaces.
     */
    static sk_sp<SkShader> MakeColorShader(const SkColor4f&, sk_sp<SkColorSpace>);

    static sk_sp<SkShader> MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src, SkBlendMode);

    /** Call this to create a new shader that will draw with the specified bitmap.
     *
     *  If the bitmap cannot be used (e.g. has no pixels, or its dimensions
     *  exceed implementation limits (currently at 64K - 1)) then SkEmptyShader
     *  may be returned.
     *
     *  If the src is kA8_Config then that mask will be colorized using the color on
     *  the paint.
     *
     *  @param src  The bitmap to use inside the shader
     *  @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
     *  @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
     *  @return     Returns a new shader object. Note: this function never returns null.
    */
    static sk_sp<SkShader> MakeBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                            const SkMatrix* localMatrix = nullptr);

    // NOTE: You can create an SkImage Shader with SkImage::newShader().

    /** Call this to create a new shader that will draw with the specified picture.
     *
     *  @param src  The picture to use inside the shader (if not NULL, its ref count
     *              is incremented). The SkPicture must not be changed after
     *              successfully creating a picture shader.
     *  @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
     *  @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
     *  @param tile The tile rectangle in picture coordinates: this represents the subset
     *              (or superset) of the picture used when building a tile. It is not
     *              affected by localMatrix and does not imply scaling (only translation
     *              and cropping). If null, the tile rect is considered equal to the picture
     *              bounds.
     *  @return     Returns a new shader object. Note: this function never returns null.
    */
    static sk_sp<SkShader> MakePictureShader(sk_sp<SkPicture> src, TileMode tmx, TileMode tmy,
                                             const SkMatrix* localMatrix, const SkRect* tile);

protected:
    SkShader() = default;

private:
    typedef SkFlattenable INHERITED;
};

#endif

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_DEFINED
#define SkImage_DEFINED

#include "SkImageEncoder.h"
#include "SkRefCnt.h"
#include "SkScalar.h"

class SkData;
class SkCanvas;
class SkPaint;
class SkShader;
class GrContext;
class GrTexture;

// need for TileMode
#include "SkShader.h"

/**
 *  SkImage is an abstraction for drawing a rectagle of pixels, though the
 *  particular type of image could be actually storing its data on the GPU, or
 *  as drawing commands (picture or PDF or otherwise), ready to be played back
 *  into another canvas.
 *
 *  The content of SkImage is always immutable, though the actual storage may
 *  change, if for example that image can be re-created via encoded data or
 *  other means.
 */
class SK_API SkImage : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkImage)

    enum ColorType {
        kAlpha_8_ColorType,
        kRGB_565_ColorType,
        kRGBA_8888_ColorType,
        kBGRA_8888_ColorType,
        kPMColor_ColorType,

        kLastEnum_ColorType = kPMColor_ColorType
    };

    enum AlphaType {
        kIgnore_AlphaType,
        kOpaque_AlphaType,
        kPremul_AlphaType,
        kUnpremul_AlphaType,

        kLastEnum_AlphaType = kUnpremul_AlphaType
    };

    struct Info {
        int         fWidth;
        int         fHeight;
        ColorType   fColorType;
        AlphaType   fAlphaType;
    };

    static SkImage* NewRasterCopy(const Info&, const void* pixels, size_t rowBytes);
    static SkImage* NewRasterData(const Info&, SkData* pixels, size_t rowBytes);
    static SkImage* NewEncodedData(SkData*);

    /**
     * GrTexture is a more logical parameter for this factory, but its
     * interactions with scratch cache still has issues, so for now we take
     * SkBitmap instead. This will be changed in the future. skbug.com/1449
     */
    static SkImage* NewTexture(const SkBitmap&);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    uint32_t uniqueID() const { return fUniqueID; }

    /**
     * Return the GrTexture that stores the image pixels. Calling getTexture
     * does not affect the reference count of the GrTexture object.
     * Will return NULL if the image does not use a texture.
     */
    GrTexture* getTexture();

    SkShader*   newShaderClamp() const;
    SkShader*   newShader(SkShader::TileMode, SkShader::TileMode) const;

    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*);

    /**
     *  Draw the image, cropped to the src rect, to the dst rect of a canvas.
     *  If src is larger than the bounds of the image, the rest of the image is
     *  filled with transparent black pixels.
     *
     *  See SkCanvas::drawBitmapRectToRect for similar behavior.
     */
    void draw(SkCanvas*, const SkRect* src, const SkRect& dst, const SkPaint*);

    /**
     *  Encode the image's pixels and return the result as a new SkData, which
     *  the caller must manage (i.e. call unref() when they are done).
     *
     *  If the image type cannot be encoded, or the requested encoder type is
     *  not supported, this will return NULL.
     */
    SkData* encode(SkImageEncoder::Type t = SkImageEncoder::kPNG_Type,
                   int quality = 80) const;

protected:
    SkImage(int width, int height) :
        fWidth(width),
        fHeight(height),
        fUniqueID(NextUniqueID()) {

        SkASSERT(width >= 0);
        SkASSERT(height >= 0);
    }

private:
    const int       fWidth;
    const int       fHeight;
    const uint32_t  fUniqueID;

    static uint32_t NextUniqueID();

    typedef SkRefCnt INHERITED;
};

#endif

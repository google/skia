/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_DEFINED
#define SkImage_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"

class SkData;
class SkCanvas;
class SkPaint;
class SkShader;
class GrContext;
struct GrPlatformTextureDesc;

// need for TileMode
#include "SkShader.h"

////// EXPERIMENTAL

class SkColorSpace;

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
class SkImage : public SkRefCnt {
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

    static SkImage* NewRasterCopy(const Info&, SkColorSpace*, const void* pixels, size_t rowBytes);
    static SkImage* NewRasterData(const Info&, SkColorSpace*, SkData* pixels, size_t rowBytes);
    static SkImage* NewEncodedData(SkData*);
    static SkImage* NewTexture(GrContext*, const GrPlatformTextureDesc&);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    uint32_t uniqueID() const { return fUniqueID; }

    SkShader*   newShaderClamp() const;
    SkShader*   newShader(SkShader::TileMode, SkShader::TileMode) const;

    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*);

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

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_DEFINED
#define SkImage_DEFINED


////// EXPERIMENTAL


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
    enum ColorType {
        kA8_ColorType,
        kRGB_565_ColorType,
        kRGBA_8888_ColorType,
        kBGRA_8888_ColorType,
        kPMColor_ColorType,
    };
    
    enum AlphaType {
        kIgnore_AlphaType,
        kOpaque_AlphaType,
        kPremul_AlphaType,
        kUnpremul_AlphaType
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

    int         width() const;
    int         height() const;
    uint32_t    uniqueID() const;
    
    SkShader*   newShaderClamp() const;
    SkShader*   newShader(SkShader::TileMode, SkShader::TileMode) const;
    
    void SkCanvas::drawImage(...);
};

/**
 *  SkSurface represents the backend/results of drawing to a canvas. For raster
 *  drawing, the surface will be pixels, but (for example) when drawing into
 *  a PDF or Picture canvas, the surface stores the recorded commands.
 *
 *  To draw into a canvas, first create the appropriate type of Surface, and
 *  then request the canvas from the surface.
 */
class SkSurface : public SkRefCnt {
public:
    static SkSurface*   NewRasterDirect(const Info&, SkColorSpace*,
                                        const void* pixels, size_t rowBytes);
    static SkSurface*   NewRaster(const Info&, SkColorSpace*);
    static SkSurface*   NewGpu(GrContext*);
    static SkSurface*   NewPDF(...);
    static SkSurface*   NewXPS(...);
    static SkSurface*   NewPicture(int width, int height);

    /**
     *  Return a canvas that will draw into this surface
     */
    SkCanvas* newCanvas();

    /**
     *  Returns an image of the current state of the surface pixels up to this
     *  point. Subsequent changes to the surface (by drawing into its canvas)
     *  will not be reflected in this image.
     */
    SkImage* newImageShapshot();

    /**
     *  Thought the caller could get a snapshot image explicitly, and draw that,
     *  it seems that directly drawing a surface into another canvas might be
     *  a common pattern, and that we could possibly be more efficient, since
     *  we'd know that the "snapshot" need only live until we've handed it off
     *  to the canvas.
     */
    void SkCanvas::drawSurface(...);
};

#endif

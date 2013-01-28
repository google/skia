/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PictureRenderer_DEFINED
#define PictureRenderer_DEFINED

#include "SkCountdown.h"
#include "SkDrawFilter.h"
#include "SkMath.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkRunnable.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkThreadPool.h"
#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrContext.h"
#endif

class SkBitmap;
class SkCanvas;
class SkGLContext;
class SkThread;

namespace sk_tools {

class TiledPictureRenderer;

class PictureRenderer : public SkRefCnt {

public:
    enum SkDeviceTypes {
        kBitmap_DeviceType,
#if SK_SUPPORT_GPU
        kGPU_DeviceType
#endif
    };

    enum BBoxHierarchyType {
        kNone_BBoxHierarchyType = 0,
        kRTree_BBoxHierarchyType,
        kTileGrid_BBoxHierarchyType,
    };

    // this uses SkPaint::Flags as a base and adds additional flags
    enum DrawFilterFlags {
        kNone_DrawFilterFlag = 0,
        kBlur_DrawFilterFlag = 0x4000, // toggles between blur and no blur
        kHinting_DrawFilterFlag = 0x8000, // toggles between no hinting and normal hinting
        kSlightHinting_DrawFilterFlag = 0x10000, // toggles between slight and normal hinting
        kAAClip_DrawFilterFlag = 0x20000, // toggles between soft and hard clip
    };

    SK_COMPILE_ASSERT(!(kBlur_DrawFilterFlag & SkPaint::kAllFlags), blur_flag_must_be_greater);
    SK_COMPILE_ASSERT(!(kHinting_DrawFilterFlag & SkPaint::kAllFlags),
            hinting_flag_must_be_greater);
    SK_COMPILE_ASSERT(!(kSlightHinting_DrawFilterFlag & SkPaint::kAllFlags),
            slight_hinting_flag_must_be_greater);

    /**
     * Called with each new SkPicture to render.
     */
    virtual void init(SkPicture* pict);

    /**
     *  Set the viewport so that only the portion listed gets drawn.
     */
    void setViewport(SkISize size) { fViewport = size; }

    /**
     *  Set the scale factor at which draw the picture.
     */
    void setScaleFactor(SkScalar scale) { fScaleFactor = scale; }

    /**
     * Perform any setup that should done prior to each iteration of render() which should not be
     * timed.
     */
    virtual void setup() {}

    /**
     * Perform work that is to be timed. Typically this is rendering, but is also used for recording
     * and preparing picture for playback by the subclasses which do those.
     * If path is non-null, subclass implementations should call write().
     * @param path If non-null, also write the output to the file specified by path. path should
     *             have no extension; it will be added by write().
     * @return bool True if rendering succeeded and, if path is non-null, the output was
     *             successfully written to a file.
     */
    virtual bool render(const SkString* path, SkBitmap** out = NULL) = 0;

    /**
     * Called once finished with a particular SkPicture, before calling init again, and before
     * being done with this Renderer.
     */
    virtual void end();

    /**
     * If this PictureRenderer is actually a TiledPictureRender, return a pointer to this as a
     * TiledPictureRender so its methods can be called.
     */
    virtual TiledPictureRenderer* getTiledRenderer() { return NULL; }

    /**
     * Resets the GPU's state. Does nothing if the backing is raster. For a GPU renderer, calls
     * flush, and calls finish if callFinish is true.
     * @param callFinish Whether to call finish.
     */
    void resetState(bool callFinish);

    void setDeviceType(SkDeviceTypes deviceType) {
        fDeviceType = deviceType;
    }

    void setDrawFilters(DrawFilterFlags const * const filters, const SkString& configName) {
        memcpy(fDrawFilters, filters, sizeof(fDrawFilters));
        fDrawFiltersConfig = configName;
    }

    void setBBoxHierarchyType(BBoxHierarchyType bbhType) {
        fBBoxHierarchyType = bbhType;
    }

    void setGridSize(int width, int height) {
        fGridWidth = width;
        fGridHeight = height;
    }

    bool isUsingBitmapDevice() {
        return kBitmap_DeviceType == fDeviceType;
    }

    virtual SkString getPerIterTimeFormat() { return SkString("%.2f"); }

    virtual SkString getNormalTimeFormat() { return SkString("%6.2f"); }

    /**
     * Reports the configuration of this PictureRenderer.
     */
    SkString getConfigName() {
        SkString config = this->getConfigNameInternal();
        if (!fViewport.isEmpty()) {
            config.appendf("_viewport_%ix%i", fViewport.width(), fViewport.height());
        }
        if (kRTree_BBoxHierarchyType == fBBoxHierarchyType) {
            config.append("_rtree");
        } else if (kTileGrid_BBoxHierarchyType == fBBoxHierarchyType) {
            config.append("_grid");
        }
#if SK_SUPPORT_GPU
        if (this->isUsingGpuDevice()) {
            config.append("_gpu");
        }
#endif
        config.append(fDrawFiltersConfig.c_str());
        return config;
    }

#if SK_SUPPORT_GPU
    bool isUsingGpuDevice() {
        return kGPU_DeviceType == fDeviceType;
    }

    SkGLContext* getGLContext() {
        if (this->isUsingGpuDevice()) {
            return fGrContextFactory.getGLContext(GrContextFactory::kNative_GLContextType);
        } else {
            return NULL;
        }
    }

    GrContext* getGrContext() {
        return fGrContext;
    }
#endif

    PictureRenderer()
        : fPicture(NULL)
        , fDeviceType(kBitmap_DeviceType)
        , fBBoxHierarchyType(kNone_BBoxHierarchyType)
        , fGridWidth(0)
        , fGridHeight(0)
#if SK_SUPPORT_GPU
        , fGrContext(fGrContextFactory.get(GrContextFactory::kNative_GLContextType))
#endif
        , fScaleFactor(SK_Scalar1)
        {
            sk_bzero(fDrawFilters, sizeof(fDrawFilters));
            fViewport.set(0, 0);
        }

protected:
    SkAutoTUnref<SkCanvas> fCanvas;
    SkPicture*             fPicture;
    SkDeviceTypes          fDeviceType;
    BBoxHierarchyType      fBBoxHierarchyType;
    DrawFilterFlags        fDrawFilters[SkDrawFilter::kTypeCount];
    SkString               fDrawFiltersConfig;
    int                    fGridWidth, fGridHeight; // used when fBBoxHierarchyType is TileGrid

#if SK_SUPPORT_GPU
    GrContextFactory fGrContextFactory;
    GrContext* fGrContext;
#endif

    void buildBBoxHierarchy();

    /**
     * Return the total width that should be drawn. If the viewport width has been set greater than
     * 0, this will be the minimum of the current SkPicture's width and the viewport's width.
     */
    int getViewWidth();

    /**
     * Return the total height that should be drawn. If the viewport height has been set greater
     * than 0, this will be the minimum of the current SkPicture's height and the viewport's height.
     */
    int getViewHeight();

    /**
     * Scales the provided canvas to the scale factor set by setScaleFactor.
     */
    void scaleToScaleFactor(SkCanvas*);

    SkPicture* createPicture();
    uint32_t recordFlags();
    SkCanvas* setupCanvas();
    virtual SkCanvas* setupCanvas(int width, int height);

private:
    SkISize                fViewport;
    SkScalar               fScaleFactor;

    virtual SkString getConfigNameInternal() = 0;

    typedef SkRefCnt INHERITED;
};

/**
 * This class does not do any rendering, but its render function executes recording, which we want
 * to time.
 */
class RecordPictureRenderer : public PictureRenderer {
    virtual bool render(const SkString*, SkBitmap** out = NULL) SK_OVERRIDE;

    virtual SkString getPerIterTimeFormat() SK_OVERRIDE { return SkString("%.4f"); }

    virtual SkString getNormalTimeFormat() SK_OVERRIDE { return SkString("%6.4f"); }

protected:
    virtual SkCanvas* setupCanvas(int width, int height) SK_OVERRIDE;

private:
    virtual SkString getConfigNameInternal() SK_OVERRIDE;
};

class PipePictureRenderer : public PictureRenderer {
public:
    virtual bool render(const SkString*, SkBitmap** out = NULL) SK_OVERRIDE;

private:
    virtual SkString getConfigNameInternal() SK_OVERRIDE;

    typedef PictureRenderer INHERITED;
};

class SimplePictureRenderer : public PictureRenderer {
public:
    virtual void init(SkPicture* pict) SK_OVERRIDE;

    virtual bool render(const SkString*, SkBitmap** out = NULL) SK_OVERRIDE;

private:
    virtual SkString getConfigNameInternal() SK_OVERRIDE;

    typedef PictureRenderer INHERITED;
};

class TiledPictureRenderer : public PictureRenderer {
public:
    TiledPictureRenderer();

    virtual void init(SkPicture* pict) SK_OVERRIDE;

    /**
     * Renders to tiles, rather than a single canvas. If a path is provided, a separate file is
     * created for each tile, named "path0.png", "path1.png", etc.
     * Multithreaded mode currently does not support writing to a file.
     */
    virtual bool render(const SkString* path, SkBitmap** out = NULL) SK_OVERRIDE;

    virtual void end() SK_OVERRIDE;

    void setTileWidth(int width) {
        fTileWidth = width;
    }

    int getTileWidth() const {
        return fTileWidth;
    }

    void setTileHeight(int height) {
        fTileHeight = height;
    }

    int getTileHeight() const {
        return fTileHeight;
    }

    void setTileWidthPercentage(double percentage) {
        fTileWidthPercentage = percentage;
    }

    double getTileWidthPercentage() const {
        return fTileWidthPercentage;
    }

    void setTileHeightPercentage(double percentage) {
        fTileHeightPercentage = percentage;
    }

    double getTileHeightPercentage() const {
        return fTileHeightPercentage;
    }

    void setTileMinPowerOf2Width(int width) {
        SkASSERT(SkIsPow2(width) && width > 0);
        if (!SkIsPow2(width) || width <= 0) {
            return;
        }

        fTileMinPowerOf2Width = width;
    }

    int getTileMinPowerOf2Width() const {
        return fTileMinPowerOf2Width;
    }

    virtual TiledPictureRenderer* getTiledRenderer() SK_OVERRIDE { return this; }

    /**
     * Report the number of tiles in the x and y directions. Must not be called before init.
     * @param x Output parameter identifying the number of tiles in the x direction.
     * @param y Output parameter identifying the number of tiles in the y direction.
     * @return True if the tiles have been set up, and x and y are meaningful. If false, x and y are
     *         unmodified.
     */
    bool tileDimensions(int& x, int&y);

    /**
     * Move to the next tile and return its indices. Must be called before calling drawCurrentTile
     * for the first time.
     * @param i Output parameter identifying the column of the next tile to be drawn on the next
     *          call to drawNextTile.
     * @param j Output parameter identifying the row  of the next tile to be drawn on the next call
     *          to drawNextTile.
     * @param True if the tiles have been created and the next tile to be drawn by drawCurrentTile
     *        is within the range of tiles. If false, i and j are unmodified.
     */
    bool nextTile(int& i, int& j);

    /**
     * Render one tile. This will draw the same tile each time it is called until nextTile is
     * called. The tile rendered will depend on how many calls have been made to nextTile.
     * It is an error to call this without first calling nextTile, or if nextTile returns false.
     */
    void drawCurrentTile();

protected:
    SkTDArray<SkRect> fTileRects;

    virtual SkCanvas* setupCanvas(int width, int height) SK_OVERRIDE;
    virtual SkString getConfigNameInternal() SK_OVERRIDE;

private:
    int    fTileWidth;
    int    fTileHeight;
    double fTileWidthPercentage;
    double fTileHeightPercentage;
    int    fTileMinPowerOf2Width;

    // These variables are only used for timing individual tiles.
    // Next tile to draw in fTileRects.
    int    fCurrentTileOffset;
    // Number of tiles in the x direction.
    int    fTilesX;
    // Number of tiles in the y direction.
    int    fTilesY;

    void setupTiles();
    void setupPowerOf2Tiles();

    typedef PictureRenderer INHERITED;
};

class CloneData;

class MultiCorePictureRenderer : public TiledPictureRenderer {
public:
    explicit MultiCorePictureRenderer(int threadCount);

    ~MultiCorePictureRenderer();

    virtual void init(SkPicture* pict) SK_OVERRIDE;

    /**
     * Behaves like TiledPictureRenderer::render(), only using multiple threads.
     */
    virtual bool render(const SkString* path, SkBitmap** out = NULL) SK_OVERRIDE;

    virtual void end() SK_OVERRIDE;

private:
    virtual SkString getConfigNameInternal() SK_OVERRIDE;

    const int            fNumThreads;
    SkTDArray<SkCanvas*> fCanvasPool;
    SkThreadPool         fThreadPool;
    SkPicture*           fPictureClones;
    CloneData**          fCloneData;
    SkCountdown          fCountdown;

    typedef TiledPictureRenderer INHERITED;
};

/**
 * This class does not do any rendering, but its render function executes turning an SkPictureRecord
 * into an SkPicturePlayback, which we want to time.
 */
class PlaybackCreationRenderer : public PictureRenderer {
public:
    virtual void setup() SK_OVERRIDE;

    virtual bool render(const SkString*, SkBitmap** out = NULL) SK_OVERRIDE;

    virtual SkString getPerIterTimeFormat() SK_OVERRIDE { return SkString("%.4f"); }

    virtual SkString getNormalTimeFormat() SK_OVERRIDE { return SkString("%6.4f"); }

private:
    SkAutoTUnref<SkPicture> fReplayer;

    virtual SkString getConfigNameInternal() SK_OVERRIDE;

    typedef PictureRenderer INHERITED;
};

extern PictureRenderer* CreateGatherPixelRefsRenderer();
extern PictureRenderer* CreatePictureCloneRenderer();

}

#endif  // PictureRenderer_DEFINED

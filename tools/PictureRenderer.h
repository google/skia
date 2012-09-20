/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PictureRenderer_DEFINED
#define PictureRenderer_DEFINED
#include "SkMath.h"
#include "SkPicture.h"
#include "SkTypes.h"
#include "SkTDArray.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrContext.h"
#endif

class SkBitmap;
class SkCanvas;
class SkGLContext;
class ThreadSafePipeController;

namespace sk_tools {

class PictureRenderer : public SkRefCnt {
public:
    enum SkDeviceTypes {
        kBitmap_DeviceType,
#if SK_SUPPORT_GPU
        kGPU_DeviceType
#endif
    };

    virtual void init(SkPicture* pict);

    /**
     * Perform any setup that should done prior to each iteration of render() which should not be
     * timed.
     */
    virtual void setup() {}

    /**
     * Perform work that is to be timed. Typically this is rendering, but is also used for recording
     * and preparing picture for playback by the subclasses which do those.
     * @param doExtraWorkToDrawToBaseCanvas Perform extra work to draw to fCanvas. Some subclasses
     *                                      will automatically draw to fCanvas, but in the tiled
     *                                      case, for example, true needs to be passed so that
     *                                      the tiles will be stitched together on fCanvas.
     */
    virtual void render(bool doExtraWorkToDrawToBaseCanvas) = 0;

    virtual void end();
    void resetState();

    void setDeviceType(SkDeviceTypes deviceType) {
        fDeviceType = deviceType;
    }

    bool isUsingBitmapDevice() {
        return kBitmap_DeviceType == fDeviceType;
    }

    virtual SkString getPerIterTimeFormat() { return SkString("%.2f"); }

    virtual SkString getNormalTimeFormat() { return SkString("%6.2f"); }

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
#if SK_SUPPORT_GPU
        , fGrContext(fGrContextFactory.get(GrContextFactory::kNative_GLContextType))
#endif
        {}

    bool write(const SkString& path) const;

protected:
    SkCanvas* setupCanvas();
    virtual SkCanvas* setupCanvas(int width, int height);

    SkAutoTUnref<SkCanvas> fCanvas;
    SkPicture* fPicture;
    SkDeviceTypes fDeviceType;

#if SK_SUPPORT_GPU
    GrContextFactory fGrContextFactory;
    GrContext* fGrContext;
#endif

private:
    typedef SkRefCnt INHERITED;
};

/**
 * This class does not do any rendering, but its render function executes recording, which we want
 * to time.
 */
class RecordPictureRenderer : public PictureRenderer {
    virtual void render(bool doExtraWorkToDrawToBaseCanvas) SK_OVERRIDE;

    virtual SkString getPerIterTimeFormat() SK_OVERRIDE { return SkString("%.4f"); }

    virtual SkString getNormalTimeFormat() SK_OVERRIDE { return SkString("%6.4f"); }
};

class PipePictureRenderer : public PictureRenderer {
public:
    virtual void render(bool doExtraWorkToDrawToBaseCanvas) SK_OVERRIDE;

private:
    typedef PictureRenderer INHERITED;
};

class SimplePictureRenderer : public PictureRenderer {
public:
    virtual void render(bool doExtraWorkToDrawToBaseCanvas) SK_OVERRIDE;

private:
    typedef PictureRenderer INHERITED;
};

class TiledPictureRenderer : public PictureRenderer {
public:
    TiledPictureRenderer();

    virtual void init(SkPicture* pict) SK_OVERRIDE;
    virtual void setup() SK_OVERRIDE;
    virtual void render(bool doExtraWorkToDrawToBaseCanvas) SK_OVERRIDE;
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

    /**
     * Set the number of threads to use for drawing. Non-positive numbers will set it to 1.
     */
    void setNumberOfThreads(int num) {
        fNumThreads = SkMax32(num, 1);
    }

    void setUsePipe(bool usePipe) {
        fUsePipe = usePipe;
    }

    ~TiledPictureRenderer();

private:
    bool              fUsePipe;
    int               fTileWidth;
    int               fTileHeight;
    double            fTileWidthPercentage;
    double            fTileHeightPercentage;
    int               fTileMinPowerOf2Width;
    SkTDArray<SkRect> fTileRects;

    // These are only used for multithreaded rendering
    int32_t                   fTileCounter;
    int                       fNumThreads;
    SkTDArray<SkCanvas*>      fCanvasPool;
    SkPicture*                fPictureClones;
    ThreadSafePipeController* fPipeController;

    void setupTiles();
    void setupPowerOf2Tiles();
    virtual SkCanvas* setupCanvas(int width, int height) SK_OVERRIDE;
    bool multiThreaded() { return fNumThreads > 1; }

    typedef PictureRenderer INHERITED;
};

/**
 * This class does not do any rendering, but its render function executes turning an SkPictureRecord
 * into an SkPicturePlayback, which we want to time.
 */
class PlaybackCreationRenderer : public PictureRenderer {
public:
    virtual void setup() SK_OVERRIDE;

    virtual void render(bool doExtraWorkToDrawToBaseCanvas) SK_OVERRIDE;

    virtual SkString getPerIterTimeFormat() SK_OVERRIDE { return SkString("%.4f"); }

    virtual SkString getNormalTimeFormat() SK_OVERRIDE { return SkString("%6.4f"); }

private:
    SkPicture fReplayer;
    typedef PictureRenderer INHERITED;
};

}

#endif  // PictureRenderer_DEFINED

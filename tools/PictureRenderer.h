/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PictureRenderer_DEFINED
#define PictureRenderer_DEFINED
#include "SkTypes.h"
#include "SkTDArray.h"
#include "SkRefCnt.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrContext.h"
#endif

class SkBitmap;
class SkCanvas;
class SkGLContext;
class SkPicture;

namespace sk_tools {

class PictureRenderer : public SkRefCnt {
public:
    virtual void init(SkPicture* pict);
    virtual void render() = 0;
    virtual void end();
    virtual void resetState();

    SkCanvas* getCanvas() {
        return fCanvas.get();
    }

    void setUseBitmapDevice() {
        fDeviceType = kBitmap_DeviceType;
    }

    bool isUsingBitmapDevice() {
        return kBitmap_DeviceType == fDeviceType;
    }

#if SK_SUPPORT_GPU
    void setUseGpuDevice() {
        fDeviceType = kGPU_DeviceType;
    }

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
#endif

    PictureRenderer()
        : fPicture(NULL)
        , fDeviceType(kBitmap_DeviceType)
#if SK_SUPPORT_GPU
        , fGrContext(fGrContextFactory.get(GrContextFactory::kNative_GLContextType))
#endif
        {}

protected:
    enum SkDeviceTypes {
        kBitmap_DeviceType,
#if SK_SUPPORT_GPU
        kGPU_DeviceType
#endif
    };

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

class PipePictureRenderer : public PictureRenderer {
public:
    virtual void render() SK_OVERRIDE;

private:
    typedef PictureRenderer INHERITED;
};

class SimplePictureRenderer : public PictureRenderer {
public:
    virtual void render () SK_OVERRIDE;

private:
    typedef PictureRenderer INHERITED;
};

class TiledPictureRenderer : public PictureRenderer {
public:
    TiledPictureRenderer();

    virtual void init(SkPicture* pict) SK_OVERRIDE;
    virtual void render() SK_OVERRIDE;
    virtual void end() SK_OVERRIDE;
    void drawTiles();

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

    int numTiles() const {
        return fTiles.count();
    }

    ~TiledPictureRenderer();

private:
    struct TileInfo {
        SkBitmap* fBitmap;
        SkCanvas* fCanvas;
    };

    int fTileWidth;
    int fTileHeight;
    double fTileWidthPercentage;
    double fTileHeightPercentage;

    SkTDArray<TileInfo> fTiles;

    // Clips the tile to an area that is completely in what the SkPicture says is the
    // drawn-to area. This is mostly important for tiles on the right and bottom edges
    // as they may go over this area and the picture may have some commands that
    // draw outside of this area and so should not actually be written.
    void clipTile(const TileInfo& tile);
    void addTile(int tile_x_start, int tile_y_start);
    void setupTiles();
    // We manually delete the tiles instead of having a destructor on TileInfo as
    // the destructor on TileInfo will be during a realloc. This would result in
    // the canvases and bitmaps being prematurely deleted.
    void deleteTiles();
    void copyTilesToCanvas();

    typedef PictureRenderer INHERITED;
};

}

#endif  // PictureRenderer_DEFINED

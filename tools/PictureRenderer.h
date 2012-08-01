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

class SkBitmap;
class SkCanvas;
class SkPicture;

namespace sk_tools {

class PictureRenderer : public SkRefCnt {
public:
    virtual void init(const SkPicture& pict){}
    virtual void render(SkPicture* pict, SkCanvas* canvas) = 0;
};

class PipePictureRenderer : public PictureRenderer {
public:
    virtual void render(SkPicture* pict, SkCanvas* canvas);
};

class SimplePictureRenderer : public PictureRenderer {
public:
    virtual void render (SkPicture* pict, SkCanvas* canvas);
};

class TiledPictureRenderer : public PictureRenderer {
public:
    TiledPictureRenderer();

    virtual void init(const SkPicture& pict);
    virtual void render(SkPicture* pict, SkCanvas* canvas);
    void drawTiles(SkPicture* pict);

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
    static void clipTile(const SkPicture& picture, const TileInfo& tile);
    void addTile(const SkPicture& picture, int tile_x_start, int tile_y_start);
    void setupTiles(const SkPicture& picture);
    // We manually delete the tiles instead of having a destructor on TileInfo as
    // the destructor on TileInfo will be during a realloc. This would result in
    // the canvases and bitmaps being prematurely deleted.
    void deleteTiles();
    void copyTilesToCanvas(const SkPicture& pict, SkCanvas* destination);
};

}

#endif  // PictureRenderer_DEFINED

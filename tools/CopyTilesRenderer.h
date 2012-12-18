/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CopyTilesRenderer_DEFINED
#define CopyTilesRenderer_DEFINED

#include "PictureRenderer.h"
#include "SkTypes.h"

class SkPicture;
class SkString;

namespace sk_tools {
    /**
     *  PictureRenderer that draws the picture and then extracts it into tiles. For large pictures,
     *  it will divide the picture into large tiles and draw the picture once for each large tile.
     */
    class CopyTilesRenderer : public TiledPictureRenderer {

    public:
        CopyTilesRenderer(int x, int y);
        virtual void init(SkPicture* pict) SK_OVERRIDE;

        /**
         *  Similar to TiledPictureRenderer, this will draw a PNG for each tile. However, the
         *  numbering (and actual tiles) will be different.
         */
        virtual bool render(const SkString* path, SkBitmap** out) SK_OVERRIDE;

    private:
        int fXTilesPerLargeTile;
        int fYTilesPerLargeTile;

        int fLargeTileWidth;
        int fLargeTileHeight;

        virtual SkString getConfigNameInternal() SK_OVERRIDE;

        typedef TiledPictureRenderer INHERITED;
    };
} // sk_tools
#endif // CopyTilesRenderer_DEFINED

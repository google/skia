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

struct GrContextOptions;
class SkPicture;
class SkString;

namespace sk_tools {
    /**
     *  PictureRenderer that draws the picture and then extracts it into tiles. For large pictures,
     *  it will divide the picture into large tiles and draw the picture once for each large tile.
     */
    class CopyTilesRenderer : public TiledPictureRenderer {

    public:
#if SK_SUPPORT_GPU
        CopyTilesRenderer(const GrContextOptions &opts, int x, int y);
#else
        CopyTilesRenderer(int x, int y);
#endif
        virtual void init(const SkPicture* pict, 
                          const SkString* writePath, 
                          const SkString* mismatchPath,
                          const SkString* inputFilename,
                          bool useChecksumBasedFilenames,
                          bool useMultiPictureDraw) override;

        /**
         *  Similar to TiledPictureRenderer, this will draw a PNG for each tile. However, the
         *  numbering (and actual tiles) will be different.
         */
        bool render(SkBitmap** out) override;

        bool supportsTimingIndividualTiles() override { return false; }

    private:
        int fXTilesPerLargeTile;
        int fYTilesPerLargeTile;

        int fLargeTileWidth;
        int fLargeTileHeight;

        SkString getConfigNameInternal() override;

        typedef TiledPictureRenderer INHERITED;
    };
} // sk_tools
#endif // CopyTilesRenderer_DEFINED

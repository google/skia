/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDDLTextureStripAtlas_DEFINED
#define GrDDLTextureStripAtlas_DEFINED

#include "GrTextureStripAtlas.h"

#include "SkBitmap.h"
#include "SkTDArray.h"

/**
 * The DDL version of the texture strip atlas consolidates individual strips into a larger texture
 * until some limit is reached, at which point a new large texture is started.
 * This can lead to the same strip being duplicated in VRAM. This can happen if a strip appears once
 * early in a rendering (that has, say, a lot of gradients) and then again later in the rendering
 * when one of the large textures has been filled. The second, probably more common, case is
 * if the same strip is used in different DDL recordings. Since the texture strip atlases aren't
 * dedupped across threads, if the same strip is used in two different DDL recordings it will
 * be duplicated in both of the DDL recorders' atlases.
 * Note, one additional feature of the DDL texture strip atlases is that, if DDL recording is ended
 * before one of the large textures is full, the large texture will be "shrunk" to fit its
 * contents.
 */
class GrDDLTextureStripAtlas final : public GrTextureStripAtlas {
public:
    ~GrDDLTextureStripAtlas() final;

    // Overrides from GrTextureStripAtlas
    void lockRow(int row) final { /* The DDL version doesn't lock & unlock individual rows */}
    void unlockRow(int row) final { /* The DDL version doesn't lock & unlock individual rows */}

    // Caution: this method will only return the appropriate proxy after a successful 'addStrip'
    // call has been made. Additionally, the proxy return will be fully lazy (i.e., its final
    // height will be unknown).
    sk_sp<GrTextureProxy> asTextureProxyRef() const final {
        SkASSERT(fCurProxy);
        return fCurProxy;
    }

private:
    friend class GrTextureStripAtlasManager; // for ctor

    // Overrides from GrTextureStripAtlas
    int addStrip(GrContext*, const SkBitmap&) final;
    void finish(GrProxyProvider*) final;

    /**
     * The state of a single row in our cache. For the DDL texture strip atlas we hold onto all
     * the individual strip bitmaps and, upon finish, combine them all into a single bitmap.
     */
    struct AtlasRow : ::SkNoncopyable {
        AtlasRow() {}

        SkBitmap fBitmap;
    };

    /**
     * Only the GrTextureStripAtlasManager is allowed to create GrDDLTextureStripAtlas
     */
    GrDDLTextureStripAtlas(const Desc& desc);

    /**
     * Searches the key table for a key and returns the index if found; if not found, it returns
     * the bitwise not of the index at which we could insert the key to maintain a sorted list.
     **/
    int searchByKey(uint32_t key);

    SkDEBUGCODE(void validate();)

    sk_sp<GrTextureProxy> fCurProxy;    // the lazy proxy that will be split off in the finish call
    SkBitmap*             fAtlasBitmap; // the bitmap backing 'fCurProxy'

    const uint16_t        fMaxNumRows;
    uint16_t              fCurRow;

    AtlasRow*             fRows;        // We just store the source bitmap for each row.

    // A list of pointers to AtlasRows that currently contain cached images, sorted by key
    SkTDArray<AtlasRow*> fKeyTable;

    typedef GrTextureStripAtlas INHERITED;
};

#endif

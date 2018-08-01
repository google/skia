/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDynamicTextureStripAtlas_DEFINED
#define GrDynamicTextureStripAtlas_DEFINED

#include "GrTextureStripAtlas.h"
#include "SkTDArray.h"

class GrSurfaceContext;

/**
 * Maintains a single large texture whose rows store many textures of a small fixed height,
 * stored in rows across the x-axis such that we can safely wrap/repeat them horizontally.
 */
class GrDynamicTextureStripAtlas final : public GrTextureStripAtlas {
public:
    ~GrDynamicTextureStripAtlas() final;

    /**
     * This is intended to be used when cloning a processor that already holds a lock. It is
     * assumed that the row already has at least one lock.
     */
    void lockRow(int row) final;
    void unlockRow(int row) final;

    sk_sp<GrTextureProxy> asTextureProxyRef() const final;

private:
    friend class GrTextureStripAtlasManager; // for ctor

    /**
     * Only the GrTextureStripAtlasManager is allowed to create GrTextureStripAtlases
     */
    GrDynamicTextureStripAtlas(const Desc& desc);

    /**
     * Add a texture to the atlas
     *  @param data Bitmap data to copy into the row
     *  @return The row index we inserted into, or -1 if we failed to find an open row. The caller
     *      is responsible for calling unlockRow() with this row index when it's done with it.
     */
    int addStrip(GrContext*, const SkBitmap&) final;

    void finish(GrProxyProvider*) final { SkASSERT(0); }  // this is only called in DDL mode

    static uint32_t CreateUniqueID();

    // Key to indicate an atlas row without any meaningful data stored in it
    const static uint32_t kEmptyAtlasRowKey = 0xffffffff;

    /**
     * The state of a single row in our cache, next/prev pointers allow these to be chained
     * together to represent LRU status
     */
    struct AtlasRow : ::SkNoncopyable {
        AtlasRow() : fKey(kEmptyAtlasRowKey), fLocks(0), fNext(nullptr), fPrev(nullptr) { }
        // GenerationID of the bitmap that is represented by this row, 0xffffffff means "empty"
        uint32_t fKey;
        // How many times this has been locked (0 == unlocked)
        int32_t fLocks;
        // We maintain an LRU linked list between unlocked nodes with these pointers
        AtlasRow* fNext;
        AtlasRow* fPrev;
    };

    void lockTexture(GrContext*);
    void unlockTexture();

    /**
     * Initialize our LRU list (if one already exists, clear it and start anew)
     */
    void initLRU();

    /**
     * Grabs the least recently used free row out of the LRU list, returns nullptr if no rows
     * are free.
     */
    AtlasRow* getLRU();

    void appendLRU(AtlasRow* row);
    void removeFromLRU(AtlasRow* row);

    /**
     * Searches the key table for a key and returns the index if found; if not found, it returns
     * the bitwise not of the index at which we could insert the key to maintain a sorted list.
     **/
    int searchByKey(uint32_t key);

    /**
     * Compare two atlas rows by key, so we can sort/search by key
     */
    static bool KeyLess(const AtlasRow& lhs, const AtlasRow& rhs) {
        return lhs.fKey < rhs.fKey;
    }

#ifdef SK_DEBUG
    void validate();
#endif

    // A unique ID for this atlas, so we can be sure that if we
    // get a texture back from the texture cache, that it's the same one we last used.
    const uint32_t fCacheKey;

    // Total locks on all rows (when this reaches zero, we can unlock our texture)
    int32_t fLockedRows;

    const uint16_t fNumRows;
    sk_sp<GrSurfaceContext> fTexContext;

    SkScalar fNormalizedYHeight;

    // Array of AtlasRows which store the state of all our rows. Stored in a contiguous array, in
    // order that they appear in our texture, this means we can subtract this pointer from a row
    // pointer to get its index in the texture, and can save storing a row number in AtlasRow.
    AtlasRow* fRows;

    // Head and tail for linked list of least-recently-used rows (front = least recently used).
    // Note that when a texture is locked, it gets removed from this list until it is unlocked.
    AtlasRow* fLRUFront;
    AtlasRow* fLRUBack;

    // A list of pointers to AtlasRows that currently contain cached images, sorted by key
    SkTDArray<AtlasRow*> fKeyTable;

    typedef GrTextureStripAtlas INHERITED;
};

#endif

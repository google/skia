/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureStripAtlas_DEFINED
#define GrTextureStripAtlas_DEFINED

#include "SkBitmap.h"
#include "SkOpts.h"
#include "SkGr.h"
#include "SkTDArray.h"
#include "SkTDynamicHash.h"
#include "SkTypes.h"

class GrSurfaceContext;
class GrTextureProxy;

/**
 * Maintains a single large texture whose rows store many textures of a small fixed height,
 * stored in rows across the x-axis such that we can safely wrap/repeat them horizontally.
 */
class GrTextureStripAtlas {
public:
    /**
     * Descriptor struct which we'll use as a hash table key
     **/
    struct Desc {
        Desc() { sk_bzero(this, sizeof(*this)); }
        GrContext* fContext;
        GrPixelConfig fConfig;
        uint16_t fWidth, fHeight, fRowHeight;
        uint16_t fUnusedPadding;
        bool operator==(const Desc& other) const {
            return 0 == memcmp(this, &other, sizeof(Desc));
        }
    };

    /**
     * Try to find an atlas with the required parameters, creates a new one if necessary
     */
    static GrTextureStripAtlas* GetAtlas(const Desc& desc);

    ~GrTextureStripAtlas();

    /**
     * Add a texture to the atlas
     *  @param data Bitmap data to copy into the row
     *  @return The row index we inserted into, or -1 if we failed to find an open row. The caller
     *      is responsible for calling unlockRow() with this row index when it's done with it.
     */
    int lockRow(const SkBitmap& data);
    void unlockRow(int row);

    /**
     * These functions help turn an integer row index in [0, 1, 2, ... numRows] into a scalar y
     * texture coordinate in [0, 1] that we can use in a shader.
     *
     * If a regular texture access without using the atlas looks like:
     *
     *      texture2D(sampler, vec2(x, y))
     *
     * Then when using the atlas we'd replace it with:
     *
     *       texture2D(sampler, vec2(x, yOffset + y * scaleFactor))
     *
     * Where yOffset, returned by getYOffset(), is the offset to the start of the row within the
     * atlas and scaleFactor, returned by getNormalizedTexelHeight, is the normalized height of
     * one texel row.
     */
    SkScalar getYOffset(int row) const { return SkIntToScalar(row) / fNumRows; }
    SkScalar getNormalizedTexelHeight() const { return fNormalizedYHeight; }

    GrContext* getContext() const { return fDesc.fContext; }

    sk_sp<GrTextureProxy> asTextureProxyRef() const;

private:

    // Key to indicate an atlas row without any meaningful data stored in it
    const static uint32_t kEmptyAtlasRowKey = 0xffffffff;

    /**
     * The state of a single row in our cache, next/prev pointers allow these to be chained
     * together to represent LRU status
     */
    struct AtlasRow : SkNoncopyable {
        AtlasRow() : fKey(kEmptyAtlasRowKey), fLocks(0), fNext(nullptr), fPrev(nullptr) { }
        // GenerationID of the bitmap that is represented by this row, 0xffffffff means "empty"
        uint32_t fKey;
        // How many times this has been locked (0 == unlocked)
        int32_t fLocks;
        // We maintain an LRU linked list between unlocked nodes with these pointers
        AtlasRow* fNext;
        AtlasRow* fPrev;
    };

    /**
     * We'll only allow construction via the static GrTextureStripAtlas::GetAtlas
     */
    GrTextureStripAtlas(Desc desc);

    void lockTexture();
    void unlockTexture();

    /**
     * Initialize our LRU list (if one already exists, clear it and start anew)
     */
    void initLRU();

    /**
     * Grabs the least recently used free row out of the LRU list, returns nullptr if no rows are free.
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

    /**
     * Clean up callback registered with GrContext. Allows this class to
     * free up any allocated AtlasEntry and GrTextureStripAtlas objects
     */
    static void CleanUp(const GrContext* context, void* info);

    // Hash table entry for atlases
    class AtlasEntry : public ::SkNoncopyable {
    public:
        // for SkTDynamicHash
        static const Desc& GetKey(const AtlasEntry& entry) { return entry.fDesc; }
        static uint32_t Hash(const Desc& desc) { return SkOpts::hash(&desc, sizeof(Desc)); }

        // AtlasEntry proper
        AtlasEntry() : fAtlas(nullptr) {}
        ~AtlasEntry() { delete fAtlas; }
        Desc fDesc;
        GrTextureStripAtlas* fAtlas;
    };

    class Hash;
    static Hash* gAtlasCache;

    static Hash* GetCache();

    // We increment gCacheCount for each atlas
    static int32_t gCacheCount;

    // A unique ID for this texture (formed with: gCacheCount++), so we can be sure that if we
    // get a texture back from the texture cache, that it's the same one we last used.
    const int32_t fCacheKey;

    // Total locks on all rows (when this reaches zero, we can unlock our texture)
    int32_t fLockedRows;

    const Desc fDesc;
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
};

#endif

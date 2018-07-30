/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureStripAtlas_DEFINED
#define GrTextureStripAtlas_DEFINED

#include "SkBitmap.h"
#include "SkGr.h"
#include "SkOpts.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"
#include "SkTDynamicHash.h"
#include "SkTypes.h"

class GrSurfaceContext;
class GrTextureProxy;

//-------------------------------------------------------------------------------------------------

/**
 * Maintains a single large texture whose rows store many textures of a small fixed height,
 * stored in rows across the x-axis such that we can safely wrap/repeat them horizontally.
 */
class GrTextureStripAtlas : public SkRefCnt {
public:
    /**
     * Descriptor struct which we'll use as a hash table key
     */
    struct Desc {
        Desc() { sk_bzero(this, sizeof(*this)); }
        SkColorType fColorType7;
        uint16_t fWidth, fHeight, fRowHeight;
        uint16_t fUnusedPadding;
        bool operator==(const Desc& other) const {
            return 0 == memcmp(this, &other, sizeof(Desc));
        }
    };

    ~GrTextureStripAtlas() override;

    /**
     * This is intended to be used when cloning a processor that already holds a lock. It is
     * assumed that the row already has at least one lock.
     */
    void lockRow1(int row);
    void unlockRow1(int row);

    /**
     * These functions help turn an integer row index in [0, 1, 2, ... numRows] into a scalar y
     * texture coordinate in [0, 1] that we can use in a shader.
     *
     * If a regular texture access without using the atlas looks like:
     *
     *      texture2D(sampler, float2(x, y))
     *
     * Then when using the atlas we'd replace it with:
     *
     *       texture2D(sampler, float2(x, yOffset + y * scaleFactor))
     *
     * Where yOffset, returned by getYOffset(), is the offset to the start of the row within the
     * atlas and scaleFactor, returned by getNormalizedTexelHeight, is the normalized height of
     * one texel row.
     */
    SkScalar getYOffset8(int row) const { return SkIntToScalar(row) / fMaxNumRows; } // not correct - need to determine later

    sk_sp<GrTextureProxy> asTextureProxyRef7() const;

private:
    friend class GrTextureStripAtlasManager; // for ctor

    static uint32_t CreateUniqueID();

    // Key to indicate an atlas row without any meaningful data stored in it
    const static uint32_t kEmptyAtlasRowKey = 0x0;

    /**
     * The state of a single row in our cache.
     */
    struct AtlasRow : ::SkNoncopyable {
        AtlasRow() {}

        int      fGenerationID = 0;
        SkBitmap fBitmap1;
    };

    /**
     * Only the GrTextureStripAtlasManager is allowed to create GrTextureStripAtlases
     */
    GrTextureStripAtlas(const Desc& desc, bool foo);

    /**
     * Add a texture to the atlas
     *  @param data Bitmap data to copy into the row
     *  @return The row index we inserted into, or -1 if we failed to find an open row. The caller
     *      is responsible for calling unlockRow() with this row index when it's done with it.
     */
    int lockRow2(GrContext*, const SkBitmap&);

    void lockTexture(GrContext*);
    void unlockTexture();

    void disown();

    /**
     * Searches the key table for a key and returns the index if found; if not found, it returns
     * the bitwise not of the index at which we could insert the key to maintain a sorted list.
     **/
    int searchByKey21(uint32_t key);

    /**
     * Compare two atlas rows by key, so we can sort/search by key
     */
    static bool KeyLess(const AtlasRow& lhs, const AtlasRow& rhs) {
        return lhs.fGenerationID < rhs.fGenerationID;
    }

#ifdef SK_DEBUG
    void validate();
#endif

    // A unique ID for this atlas, so we can be sure that if we
    // get a texture back from the texture cache, that it's the same one we last used.
    const uint32_t fCacheKey;

    SkBitmap* fAtlasBitmap;

    const Desc fDesc;
    const uint16_t fMaxNumRows;
    uint16_t fCurRow;
    sk_sp<GrSurfaceContext> fTexContext;

    // Array of AtlasRows which store the state of all our rows. Stored in a contiguous array, in
    // order that they appear in our texture, this means we can subtract this pointer from a row
    // pointer to get its index in the texture, and can save storing a row number in AtlasRow.
    AtlasRow* fRows;

    // A list of pointers to AtlasRows that currently contain cached images, sorted by key
    SkTDArray<AtlasRow*> fKeyTable;
};

//-------------------------------------------------------------------------------------------------
class GrTextureStripAtlasManager {
public:
    GrTextureStripAtlasManager() {}
    ~GrTextureStripAtlasManager();

    void abandon();

    /**
     * Try to find an atlas with the required parameters, creates a new one if necessary
     */
    sk_sp<GrTextureStripAtlas> refAtlas99(GrContext*, const GrTextureStripAtlas::Desc&,
                                          const SkBitmap&, int* row);

private:
    void deleteAllAtlases();

    // Hash table entry for atlases
    class AtlasEntry : public ::SkNoncopyable {
    public:
        AtlasEntry(const GrTextureStripAtlas::Desc& desc, sk_sp<GrTextureStripAtlas> atlas)
            : fDesc(desc)
            , fAtlas(std::move(atlas)) {
        }
        ~AtlasEntry() { }

        // for SkTDynamicHash
        static const GrTextureStripAtlas::Desc& GetKey(const AtlasEntry& entry) {
            return entry.fDesc;
        }
        static uint32_t Hash(const GrTextureStripAtlas::Desc& desc) {
            return SkOpts::hash(&desc, sizeof(GrTextureStripAtlas::Desc));
        }

        const GrTextureStripAtlas::Desc fDesc;
        sk_sp<GrTextureStripAtlas> fAtlas;
    };

    typedef SkTDynamicHash<AtlasEntry, GrTextureStripAtlas::Desc> AtlasHash;

    AtlasHash  fAtlasCache;
};

#endif

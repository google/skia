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

/**
 * Base class for the texture strip atlases.
 * It is ref counted because the GradientShader and TableColorFilter are given a pointer to it
 * so that they can lock and unlock rows.
 */
class GrTextureStripAtlasBase : public SkRefCnt {
public:
    /**
     * Descriptor struct which we'll use as a hash table key in the GrTextureStripAtlasManager.
     */
    struct Desc1 {
        Desc1() { sk_bzero(this, sizeof(*this)); }
        SkColorType fColorType7;
        uint16_t fWidth;
        uint16_t fHeight; // the max height for the DDL version, the size of the atlas for normal
        uint16_t fRowHeight;
        uint16_t fUnusedPadding;

        bool operator==(const Desc1& other) const {
            return 0 == memcmp(this, &other, sizeof(Desc1));
        }
    };

    ~GrTextureStripAtlasBase() override {}

    const Desc1& desc() { return fDesc77; }

    /**
     * This is intended to be used when cloning a processor that already holds a lock. It is
     * assumed that the row already has at least one lock.
     */
    virtual void lockRow1(int row) = 0;

    /**
     * Some user of a given row is done. Release that row for reuse.
     */
    virtual void unlockRow1(int row) = 0;

    /**
     * Get the texture proxy backing this atlas. Note that the texture proxy may be fully lazy
     * (i.e., when recording DDLs) and, in particular, the final height may not be known.
     */
    virtual sk_sp<GrTextureProxy> asTextureProxyRef7() const = 0;

protected:
    GrTextureStripAtlasBase(const Desc1& desc) : fDesc77(desc) {}

private:
    friend class GrTextureStripAtlasManager; // for addStrip, finish

    /**
     * Add a texture strip to the atlas
     *  @param context Everyone's favorite class
     *  @param data    Bitmap data to copy into the row
     *  @return The row index we inserted into, or -1 if we failed to find an open row. The caller
     *      is responsible for calling unlockRow() with this row index when it's done with it.
     */
    virtual int addStrip(GrContext*, const SkBitmap&) = 0;

    /**
     * This method is called when an atlas needs to finish its work on the current texture.
     * Currently it is only called in DDL mode and when either:
     *        a given atlas has become full or,
     *        a DDL is being snapped from a DDL recorder
     */
    virtual void finish(GrContext*) = 0;

    Desc1 fDesc77;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

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
class GrDDLTextureStripAtlas : public GrTextureStripAtlasBase {
public:
    ~GrDDLTextureStripAtlas() override;

    // Overrides from GrTextureStripAtlasBase
    void lockRow1(int row) override { /* The DDL version doesn't lock & unlock individual rows */}
    void unlockRow1(int row) override { /* The DDL version doesn't lock & unlock individual rows */}
    sk_sp<GrTextureProxy> asTextureProxyRef7() const override;

private:
    friend class GrTextureStripAtlasManager; // for ctor

    // Overrides from GrTextureStripAtlasBase
    int addStrip(GrContext*, const SkBitmap&) override;
    void finish(GrContext*) override;

//    static uint32_t CreateUniqueID();

    // Key to indicate an atlas row without any meaningful data stored in it
//    const static uint32_t kEmptyAtlasRowKey = 0x0;

    /**
     * The state of a single row in our cache. For the DDL texture strip atlas we hold onto all
     * the individual strip bitmaps and, upon finish, combine them all into a single bitmap.
     */
    struct AtlasRow : ::SkNoncopyable {
        AtlasRow() {}

        SkBitmap fBitmap1;
    };

    /**
     * Only the GrTextureStripAtlasManager is allowed to create GrTextureStripAtlases
     */
    GrDDLTextureStripAtlas(const GrCaps* caps, const Desc1& desc);

//    void lockTexture(GrContext*);
//    void unlockTexture();


    /**
     * Searches the key table for a key and returns the index if found; if not found, it returns
     * the bitwise not of the index at which we could insert the key to maintain a sorted list.
     **/
    int searchByKey21(uint32_t key);

#ifdef SK_DEBUG
    void validate();
#endif

    // A unique ID for this atlas, so we can be sure that if we
    // get a texture back from the texture cache, that it's the same one we last used.
//    const uint32_t fCacheKey;

    // This is the lazy proxy that will be split off in the finish call.
    sk_sp<GrTextureProxy> fCurProxy;

    SkBitmap* fAtlasBitmap;

    const Desc1 fDesc1;
    const uint16_t fMaxNumRows;
    uint16_t fCurRow;
//    sk_sp<GrSurfaceContext> fTexContext;

    // Array of AtlasRows which store the state of all our rows. Stored in a contiguous array, in
    // order that they appear in our texture, this means we can subtract this pointer from a row
    // pointer to get its index in the texture, and can save storing a row number in AtlasRow.
    AtlasRow* fRows; // make a unique_ptr?

    // A list of pointers to AtlasRows that currently contain cached images, sorted by key
    SkTDArray<AtlasRow*> fKeyTable;

    typedef GrTextureStripAtlasBase INHERITED;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class GrTextureStripAtlasManager {
public:
    GrTextureStripAtlasManager() {}
    ~GrTextureStripAtlasManager();

    void abandon();

    /**
     * Try to find an atlas with the required parameters, creates a new one if necessary
     */
    sk_sp<GrTextureStripAtlasBase> addStrip(GrContext*,
                                            const GrTextureStripAtlasBase::Desc1&,
                                            const SkBitmap&, int* row);

private:
    void deleteAllAtlases();

    // Hash table entry for atlases
    class AtlasEntry : public ::SkNoncopyable {
    public:
        AtlasEntry(sk_sp<GrTextureStripAtlasBase> atlas) : fAtlas(std::move(atlas)) {}
        ~AtlasEntry() { }

        // for SkTDynamicHash
        static const GrTextureStripAtlasBase::Desc1& GetKey(const AtlasEntry& entry) {
            return entry.fAtlas->desc();
        }
        static uint32_t Hash(const GrTextureStripAtlasBase::Desc1& desc) {
            return SkOpts::hash(&desc, sizeof(GrTextureStripAtlasBase::Desc1));
        }

        sk_sp<GrTextureStripAtlasBase> fAtlas;
    };

    typedef SkTDynamicHash<AtlasEntry, GrTextureStripAtlasBase::Desc1> AtlasHash;

    AtlasHash  fAtlasCache;
};

#endif

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureStripAtlas_DEFINED
#define GrTextureStripAtlas_DEFINED

#include "SkNoncopyable.h"
#include "SkOpts.h"
#include "SkRefCnt.h"
#include "SkTDynamicHash.h"

class GrContext;
class GrProxyProvider;
class GrTextureProxy;
class SkBitmap;

/**
 * Base class for the texture strip atlases.
 * It is ref counted because the GradientShader and TableColorFilter are given a pointer to it
 * so that they can lock and unlock rows.
 */
class GrTextureStripAtlas : public SkRefCnt {
public:
    /**
     * Descriptor struct which we'll use both to find and initialize an atlas and as a hash
     * table key in the GrTextureStripAtlasManager.
     */
    struct Desc {
        Desc() { sk_bzero(this, sizeof(*this)); }
        SkColorType fColorType;
        uint16_t    fWidth;
        uint16_t    fHeight; // the max height for the DDL version, the size of the atlas for normal
        uint16_t    fRowHeight;
        uint16_t    fUnusedPadding;

        bool operator==(const Desc& other) const {
            return 0 == memcmp(this, &other, sizeof(Desc));
        }
    };

    ~GrTextureStripAtlas() override {}

    /**
     * This is intended to be used when cloning a processor that already holds a lock. It is
     * assumed that the row already has at least one lock.
     */
    virtual void lockRow(int row) = 0;

    /**
     * Some user of a given row is done. Release that row for reuse.
     */
    virtual void unlockRow(int row) = 0;

    /**
     * This returns the absolute Y location of the given row in the atlas. For atlases with
     * 'fRowHeight' > 1, this is Y location of the topmost row of the atlas entry. It is always
     * the middle of the row.
     */
    SkScalar rowToTextureY(int row) const {
        return row * fDesc.fRowHeight + SK_ScalarHalf;
    }

    /**
     * Get the texture proxy backing this atlas. Note that the texture proxy may be fully lazy
     * (i.e., when recording DDLs) and, in particular, the final height may not be known.
     */
    virtual sk_sp<GrTextureProxy> asTextureProxyRef() const = 0;

protected:
    GrTextureStripAtlas(const Desc& desc) : fDesc(desc) {}

    const Desc fDesc;

private:
    friend class GrTextureStripAtlasManager; // for addStrip, finish

    /**
     * Add a texture strip to the atlas
     *  @param context Everyone's favorite class
     *  @param bitmap  Bitmap data to copy into the row
     *  @return The row index we inserted into, or -1 if we failed to find an open row. The caller
     *      is responsible for calling unlockRow() with this row index when it's done with it.
     */
    virtual int addStrip(GrContext*, const SkBitmap& bitmap) = 0;

    /**
     * This method is called when an atlas needs to finish its work on the current texture.
     * Currently it is only called in DDL mode and when either:
     *        a given atlas has become full or,
     *        a DDL is being snapped from a DDL recorder
     */
    virtual void finish(GrProxyProvider*) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class GrTextureStripAtlasManager {
public:
    GrTextureStripAtlasManager() {}
    ~GrTextureStripAtlasManager();

    void abandon();
    void finish(GrProxyProvider*);

    /**
     * Add a new texture strip to the atlas matching the descriptor. Upon failure, nullptr
     * will be returned and 'row' will be set to -1.
     */
    sk_sp<GrTextureStripAtlas> addStrip(GrContext*,
                                        const GrTextureStripAtlas::Desc&,
                                        const SkBitmap&, int* row);

private:
    void deleteAllAtlases();

    // Hash table entry for atlases
    class AtlasEntry : public ::SkNoncopyable {
    public:
        AtlasEntry(sk_sp<GrTextureStripAtlas> atlas) : fAtlas(std::move(atlas)) {}
        ~AtlasEntry() { }

        // for SkTDynamicHash
        static const GrTextureStripAtlas::Desc& GetKey(const AtlasEntry& entry) {
            return entry.fAtlas->fDesc;
        }
        static uint32_t Hash(const GrTextureStripAtlas::Desc& desc) {
            return SkOpts::hash(&desc, sizeof(GrTextureStripAtlas::Desc));
        }

        sk_sp<GrTextureStripAtlas> fAtlas;
    };

    typedef SkTDynamicHash<AtlasEntry, GrTextureStripAtlas::Desc> AtlasHash;

    AtlasHash fAtlasCache;
};

#endif

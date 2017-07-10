/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextBlob_DEFINED
#define SkTextBlob_DEFINED

#include "../private/SkTemplates.h"
#include "../private/SkAtomics.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkRefCnt.h"

class SkReadBuffer;
class SkWriteBuffer;

typedef std::function<void(SkTypeface*)> SkTypefaceCataloger;
typedef std::function<sk_sp<SkTypeface>(uint32_t)> SkTypefaceResolver;

/** \class SkTextBlob

    SkTextBlob combines multiple text runs into an immutable, ref-counted structure.
*/
class SK_API SkTextBlob final : public SkNVRefCnt<SkTextBlob> {
public:
    /**
     *  Returns a conservative blob bounding box.
     */
    const SkRect& bounds() const { return fBounds; }

    /**
     *  Return a non-zero, unique value representing the text blob.
     */
    uint32_t uniqueID() const { return fUniqueID; }

    /**
     *  Serialize to a buffer.
     */
    void flatten(SkWriteBuffer&) const;

    /**
     *  Recreate an SkTextBlob that was serialized into a buffer.
     *
     *  @param  SkReadBuffer Serialized blob data.
     *  @return A new SkTextBlob representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static sk_sp<SkTextBlob> MakeFromBuffer(SkReadBuffer&);

    static const SkTextBlob* CreateFromBuffer(SkReadBuffer& buffer) {
        return MakeFromBuffer(buffer).release();
    }

    enum GlyphPositioning : uint8_t {
        kDefault_Positioning      = 0, // Default glyph advances -- zero scalars per glyph.
        kHorizontal_Positioning   = 1, // Horizontal positioning -- one scalar per glyph.
        kFull_Positioning         = 2  // Point positioning -- two scalars per glyph.
    };

    /**
     *  Serialize the typeface into a data blob, storing type uniqueID of each referenced typeface.
     *  During this process, each time a typeface is encountered, it is passed to the catalog,
     *  allowing the caller to what typeface IDs will need to be resolved in Deserialize().
     */
    sk_sp<SkData> serialize(const SkTypefaceCataloger&) const;

    /**
     *  Re-create a text blob previously serialized. Since the serialized form records the uniqueIDs
     *  of its typefaces, deserialization requires that the caller provide the corresponding
     *  SkTypefaces for those IDs.
     */
    static sk_sp<SkTextBlob> Deserialize(const void* data, size_t size, const SkTypefaceResolver&);

private:
    friend class SkNVRefCnt<SkTextBlob>;
    class RunRecord;

    explicit SkTextBlob(const SkRect& bounds);

    ~SkTextBlob();

    // Memory for objects of this class is created with sk_malloc rather than operator new and must
    // be freed with sk_free.
    void operator delete(void* p) { sk_free(p); }
    void* operator new(size_t) {
        SkFAIL("All blobs are created by placement new.");
        return sk_malloc_throw(0);
    }
    void* operator new(size_t, void* p) { return p; }

    static unsigned ScalarsPerGlyph(GlyphPositioning pos);

    // Call when this blob is part of the key to a cache entry. This allows the cache
    // to know automatically those entries can be purged when this SkTextBlob is deleted.
    void notifyAddedToCache() const {
        fAddedToCache.store(true);
    }

    friend class GrTextBlobCache;
    friend class SkTextBlobBuilder;
    friend class SkTextBlobRunIterator;

    const SkRect           fBounds;
    const uint32_t         fUniqueID;
    mutable SkAtomic<bool> fAddedToCache;

    SkDEBUGCODE(size_t fStorageSize;)

    // The actual payload resides in externally-managed storage, following the object.
    // (see the .cpp for more details)

    typedef SkRefCnt INHERITED;
};

/** \class SkTextBlobBuilder

    Helper class for constructing SkTextBlobs.
 */
class SK_API SkTextBlobBuilder {
public:
    SkTextBlobBuilder();

    ~SkTextBlobBuilder();

    /**
     *  Returns an immutable SkTextBlob for the current runs/glyphs,
     *  or nullptr if no runs were allocated.
     *
     *  The builder is reset and can be reused.
     */
    sk_sp<SkTextBlob> make();

    /**
     *  Glyph and position buffers associated with a run.
     *
     *  A run is a sequence of glyphs sharing the same font metrics
     *  and positioning mode.
     *
     *  If textByteCount is 0, utf8text and clusters will be NULL (no
     *  character information will be associated with the glyphs).
     *
     *  utf8text will point to a buffer of size textByteCount bytes.
     *
     *  clusters (if not NULL) will point to an array of size count.
     *  For each glyph, give the byte-offset into the text for the
     *  first byte in the first character in that glyph's cluster.
     *  Each value in the array should be an integer less than
     *  textByteCount.  Values in the array should either be
     *  monotonically increasing (left-to-right text) or monotonically
     *  decreasing (right-to-left text).  This definiton is conviently
     *  the same as used by Harfbuzz's hb_glyph_info_t::cluster field,
     *  except that Harfbuzz interleaves glyphs and clusters.
     */
    struct RunBuffer {
        SkGlyphID* glyphs;
        SkScalar* pos;
        char* utf8text;
        uint32_t* clusters;
    };

    /**
     *  Allocates a new default-positioned run and returns its writable glyph buffer
     *  for direct manipulation.
     *
     *  @param font    The font to be used for this run.
     *  @param count   Number of glyphs.
     *  @param x,y     Position within the blob.
     *  @param textByteCount length of the original UTF-8 text that
     *                 corresponds to this sequence of glyphs.  If 0,
     *                 text will not be included in the textblob.
     *  @param lang    Language code, currently unimplemented.
     *  @param bounds  Optional run bounding box. If known in advance (!= NULL), it will
     *                 be used when computing the blob bounds, to avoid re-measuring.
     *
     *  @return        A writable glyph buffer, valid until the next allocRun() or
     *                 build() call. The buffer is guaranteed to hold @count@ glyphs.
     */
    const RunBuffer& allocRunText(const SkPaint& font,
                                  int count,
                                  SkScalar x,
                                  SkScalar y,
                                  int textByteCount,
                                  SkString lang,
                                  const SkRect* bounds = NULL);
    const RunBuffer& allocRun(const SkPaint& font, int count, SkScalar x, SkScalar y,
                              const SkRect* bounds = NULL) {
        return this->allocRunText(font, count, x, y, 0, SkString(), bounds);
    }

    /**
     *  Allocates a new horizontally-positioned run and returns its writable glyph and position
     *  buffers for direct manipulation.
     *
     *  @param font    The font to be used for this run.
     *  @param count   Number of glyphs.
     *  @param y       Vertical offset within the blob.
     *  @param textByteCount length of the original UTF-8 text that
     *                 corresponds to this sequence of glyphs.  If 0,
     *                 text will not be included in the textblob.
     *  @param lang    Language code, currently unimplemented.
     *  @param bounds  Optional run bounding box. If known in advance (!= NULL), it will
     *                 be used when computing the blob bounds, to avoid re-measuring.
     *
     *  @return        Writable glyph and position buffers, valid until the next allocRun()
     *                 or build() call. The buffers are guaranteed to hold @count@ elements.
     */
    const RunBuffer& allocRunTextPosH(const SkPaint& font, int count, SkScalar y,
                                      int textByteCount, SkString lang,
                                      const SkRect* bounds = NULL);
    const RunBuffer& allocRunPosH(const SkPaint& font, int count, SkScalar y,
                                  const SkRect* bounds = NULL) {
        return this->allocRunTextPosH(font, count, y, 0, SkString(), bounds);
    }

    /**
     *  Allocates a new fully-positioned run and returns its writable glyph and position
     *  buffers for direct manipulation.
     *
     *  @param font   The font to be used for this run.
     *  @param count  Number of glyphs.
     *  @param textByteCount length of the original UTF-8 text that
     *                 corresponds to this sequence of glyphs.  If 0,
     *                 text will not be included in the textblob.
     *  @param lang    Language code, currently unimplemented.
     *  @param bounds Optional run bounding box. If known in advance (!= NULL), it will
     *                be used when computing the blob bounds, to avoid re-measuring.
     *
     *  @return       Writable glyph and position buffers, valid until the next allocRun()
     *                or build() call. The glyph buffer and position buffer are
     *                guaranteed to hold @count@ and 2 * @count@ elements, respectively.
     */
    const RunBuffer& allocRunTextPos(const SkPaint& font, int count,
                                     int textByteCount, SkString lang,
                                     const SkRect* bounds = NULL);
    const RunBuffer& allocRunPos(const SkPaint& font, int count,
                                 const SkRect* bounds = NULL) {
        return this->allocRunTextPos(font, count, 0, SkString(), bounds);
    }

private:
    void reserve(size_t size);
    void allocInternal(const SkPaint& font, SkTextBlob::GlyphPositioning positioning,
                       int count, int textBytes, SkPoint offset, const SkRect* bounds);
    bool mergeRun(const SkPaint& font, SkTextBlob::GlyphPositioning positioning,
                  int count, SkPoint offset);
    void updateDeferredBounds();

    static SkRect ConservativeRunBounds(const SkTextBlob::RunRecord&);
    static SkRect TightRunBounds(const SkTextBlob::RunRecord&);

    SkAutoTMalloc<uint8_t> fStorage;
    size_t                 fStorageSize;
    size_t                 fStorageUsed;

    SkRect                 fBounds;
    int                    fRunCount;
    bool                   fDeferredBounds;
    size_t                 fLastRun; // index into fStorage

    RunBuffer              fCurrentRunBuffer;
};

#endif // SkTextBlob_DEFINED

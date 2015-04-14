/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextBlob_DEFINED
#define SkTextBlob_DEFINED

#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkTArray.h"
#include "SkTDArray.h"

class SkReadBuffer;
class SkWriteBuffer;

/** \class SkTextBlob

    SkTextBlob combines multiple text runs into an immutable, ref-counted structure.
*/
class SK_API SkTextBlob : public SkRefCnt {
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
    static const SkTextBlob* CreateFromBuffer(SkReadBuffer&);

private:
    enum GlyphPositioning {
        kDefault_Positioning      = 0, // Default glyph advances -- zero scalars per glyph.
        kHorizontal_Positioning   = 1, // Horizontal positioning -- one scalar per glyph.
        kFull_Positioning         = 2  // Point positioning -- two scalars per glyph.
    };

    class RunRecord;

    class RunIterator {
    public:
        RunIterator(const SkTextBlob* blob);

        bool done() const;
        void next();

        uint32_t glyphCount() const;
        const uint16_t* glyphs() const;
        const SkScalar* pos() const;
        const SkPoint& offset() const;
        void applyFontToPaint(SkPaint*) const;
        GlyphPositioning positioning() const;
        bool isLCD() const;

    private:
        const RunRecord* fCurrentRun;
        int              fRemainingRuns;

        SkDEBUGCODE(uint8_t* fStorageTop;)
    };

    SkTextBlob(int runCount, const SkRect& bounds);

    virtual ~SkTextBlob();

    // Memory for objects of this class is created with sk_malloc rather than operator new and must
    // be freed with sk_free.
    void operator delete(void* p) { sk_free(p); }
    void* operator new(size_t) {
        SkFAIL("All blobs are created by placement new.");
        return sk_malloc_throw(0);
    }
    void* operator new(size_t, void* p) { return p; }

    static unsigned ScalarsPerGlyph(GlyphPositioning pos);

    friend class GrAtlasTextContext;
    friend class GrTextBlobCache;
    friend class GrTextContext;
    friend class SkBaseDevice;
    friend class SkTextBlobBuilder;
    friend class TextBlobTester;

    const int        fRunCount;
    const SkRect     fBounds;
    const uint32_t fUniqueID;

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
     *  Returns an immutable SkTextBlob for the current runs/glyphs. The builder is reset and
     *  can be reused.
     */
    const SkTextBlob* build();

    /**
     *  Glyph and position buffers associated with a run.
     *
     *  A run is a sequence of glyphs sharing the same font metrics and positioning mode.
     */
    struct RunBuffer {
        uint16_t* glyphs;
        SkScalar* pos;
    };

    /**
     *  Allocates a new default-positioned run and returns its writable glyph buffer
     *  for direct manipulation.
     *
     *  @param font    The font to be used for this run.
     *  @param count   Number of glyphs.
     *  @param x,y     Position within the blob.
     *  @param bounds  Optional run bounding box. If known in advance (!= NULL), it will
     *                 be used when computing the blob bounds, to avoid re-measuring.
     *
     *  @return        A writable glyph buffer, valid until the next allocRun() or
     *                 build() call. The buffer is guaranteed to hold @count@ glyphs.
     */
    const RunBuffer& allocRun(const SkPaint& font, int count, SkScalar x, SkScalar y,
                              const SkRect* bounds = NULL);

    /**
     *  Allocates a new horizontally-positioned run and returns its writable glyph and position
     *  buffers for direct manipulation.
     *
     *  @param font    The font to be used for this run.
     *  @param count   Number of glyphs.
     *  @param y       Vertical offset within the blob.
     *  @param bounds  Optional run bounding box. If known in advance (!= NULL), it will
     *                 be used when computing the blob bounds, to avoid re-measuring.
     *
     *  @return        Writable glyph and position buffers, valid until the next allocRun()
     *                 or build() call. The buffers are guaranteed to hold @count@ elements.
     */
    const RunBuffer& allocRunPosH(const SkPaint& font, int count, SkScalar y,
                                  const SkRect* bounds = NULL);

    /**
     *  Allocates a new fully-positioned run and returns its writable glyph and position
     *  buffers for direct manipulation.
     *
     *  @param font   The font to be used for this run.
     *  @param count  Number of glyphs.
     *  @param bounds Optional run bounding box. If known in advance (!= NULL), it will
     *                be used when computing the blob bounds, to avoid re-measuring.
     *
     *  @return       Writable glyph and position buffers, valid until the next allocRun()
     *                or build() call. The glyph buffer and position buffer are
     *                guaranteed to hold @count@ and 2 * @count@ elements, respectively.
     */
    const RunBuffer& allocRunPos(const SkPaint& font, int count, const SkRect* bounds = NULL);

private:
    void reserve(size_t size);
    void allocInternal(const SkPaint& font, SkTextBlob::GlyphPositioning positioning,
                       int count, SkPoint offset, const SkRect* bounds);
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

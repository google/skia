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
     *  Returns the blob bounding box.
     */
    const SkRect& bounds() const { return fBounds; }

    /**
     *  Return a non-zero, unique value representing the text blob.
     */
    uint32_t uniqueID() const;

private:
    enum GlyphPositioning {
        kDefault_Positioning      = 0, // Default glyph advances -- zero scalars per glyph.
        kHorizontal_Positioning   = 1, // Horizontal positioning -- one scalar per glyph.
        kFull_Positioning         = 2  // Point positioning -- two scalars per glyph.
    };

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

    private:
        const SkTextBlob* fBlob;
        int               fIndex;
    };

    // A run is a sequence of glyphs sharing the same font metrics and positioning mode.
    struct Run {
        uint32_t         count;
        uint32_t         glyphStart; // index into fGlyphBuffer
        uint32_t         posStart;   // index into fPosBuffer
        SkPoint          offset;     // run offset (unsued for fully positioned glyphs)
        SkPaint          font;
        GlyphPositioning positioning;
    };

    SkTextBlob(uint16_t* glyphs, SkScalar* pos, const SkTArray<Run>* runs, const SkRect& bounds);

    void flatten(SkWriteBuffer&) const;
    static const SkTextBlob* CreateFromBuffer(SkReadBuffer&);

    static unsigned ScalarsPerGlyph(GlyphPositioning pos);

    friend class SkCanvas;
    friend class SkPictureData;
    friend class SkTextBlobBuilder;
    friend class TextBlobTester;

    const SkAutoTMalloc<uint16_t>       fGlyphBuffer;
    const SkAutoTMalloc<SkScalar>       fPosBuffer;

    // SkTArray required here for run font destruction.
    SkAutoTDelete<const SkTArray<Run> > fRuns;
    const SkRect                        fBounds;

    mutable uint32_t                    fUniqueID;

    typedef SkRefCnt INHERITED;
};

/** \class SkTextBlobBuilder

    Helper class for constructing SkTextBlobs.
 */
class SK_API SkTextBlobBuilder {
public:
    /**
     *  @param runs The number of runs to be added, if known. This is a storage hint and
     *              not a limit.
     */
    SkTextBlobBuilder(unsigned runs = 0);

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
    void allocInternal(const SkPaint& font, SkTextBlob::GlyphPositioning positioning,
                       int count, SkPoint offset, const SkRect* bounds);
    void ensureRun(const SkPaint& font, SkTextBlob::GlyphPositioning positioning,
                   const SkPoint& offset);
    void updateDeferredBounds();

    SkTDArray<uint16_t>        fGlyphBuffer;
    SkTDArray<SkScalar>        fPosBuffer;
    SkTArray<SkTextBlob::Run>* fRuns;

    SkRect                     fBounds;
    bool                       fDeferredBounds;

    RunBuffer                  fCurrentRunBuffer;
};

#endif // SkTextBlob_DEFINED

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextBlobPriv_DEFINED
#define SkTextBlobPriv_DEFINED

#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkSafeMath.h"
#include "SkShader.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

class SkReadBuffer;
class SkWriteBuffer;

class SkTextBlobPriv {
public:
    /**
     *  Serialize to a buffer.
     */
    static void Flatten(const SkTextBlob& , SkWriteBuffer&);

    /**
     *  Recreate an SkTextBlob that was serialized into a buffer.
     *
     *  @param  SkReadBuffer Serialized blob data.
     *  @return A new SkTextBlob representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static sk_sp<SkTextBlob> MakeFromBuffer(SkReadBuffer&);
};

class SkTextBlobBuilderPriv {
public:
    static const SkTextBlobBuilder::RunBuffer& AllocRunText(SkTextBlobBuilder* builder,
            const SkPaint& font, int count, SkScalar x, SkScalar y, int textByteCount,
            SkString lang, const SkRect* bounds = nullptr) {
        return builder->allocRunText(font, count, x, y, textByteCount, lang, bounds);
    }
    static const SkTextBlobBuilder::RunBuffer& AllocRunTextPosH(SkTextBlobBuilder* builder,
            const SkPaint& font, int count, SkScalar y, int textByteCount, SkString lang,
            const SkRect* bounds = nullptr) {
        return builder->allocRunTextPosH(font, count, y, textByteCount, lang, bounds);
    }
    static const SkTextBlobBuilder::RunBuffer& AllocRunTextPos(SkTextBlobBuilder* builder,
            const SkPaint& font, int count, int textByteCount, SkString lang,
            const SkRect* bounds = nullptr) {
        return builder->allocRunTextPos(font, count, textByteCount, lang, bounds);
    }
};

// TODO(fmalita): replace with SkFont.
class SkRunFont : SkNoncopyable {
public:
    SkRunFont(const SkPaint& paint)
            : fSize(paint.getTextSize())
            , fScaleX(paint.getTextScaleX())
            , fTypeface(SkPaintPriv::RefTypefaceOrDefault(paint))
            , fSkewX(paint.getTextSkewX())
            , fAlign(paint.getTextAlign())
            , fHinting(paint.getHinting())
            , fFlags(paint.getFlags() & kFlagsMask) { }

    void applyToPaint(SkPaint* paint) const {
        paint->setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        paint->setTypeface(fTypeface);
        paint->setTextSize(fSize);
        paint->setTextScaleX(fScaleX);
        paint->setTextSkewX(fSkewX);
        paint->setTextAlign(static_cast<SkPaint::Align>(fAlign));
        paint->setHinting(static_cast<SkPaint::Hinting>(fHinting));

        paint->setFlags((paint->getFlags() & ~kFlagsMask) | fFlags);
    }

    bool operator==(const SkRunFont& other) const {
        return fTypeface == other.fTypeface
               && fSize == other.fSize
               && fScaleX == other.fScaleX
               && fSkewX == other.fSkewX
               && fAlign == other.fAlign
               && fHinting == other.fHinting
               && fFlags == other.fFlags;
    }

    bool operator!=(const SkRunFont& other) const {
        return !(*this == other);
    }

    uint32_t flags() const { return fFlags; }

private:
    friend SkPaint;
    const static uint32_t kFlagsMask =
            SkPaint::kAntiAlias_Flag          |
            SkPaint::kFakeBoldText_Flag       |
            SkPaint::kLinearText_Flag         |
            SkPaint::kSubpixelText_Flag       |
            SkPaint::kLCDRenderText_Flag      |
            SkPaint::kEmbeddedBitmapText_Flag |
            SkPaint::kAutoHinting_Flag        |
            SkPaint::kVerticalText_Flag       ;

    SkScalar                 fSize;
    SkScalar                 fScaleX;

    // Keep this sk_sp off the first position, to avoid interfering with SkNoncopyable
    // empty baseclass optimization (http://code.google.com/p/skia/issues/detail?id=3694).
    sk_sp<SkTypeface>        fTypeface;
    SkScalar                 fSkewX;

    static_assert(SkPaint::kAlignCount < 4, "insufficient_align_bits");
    uint32_t                 fAlign : 2;
    static_assert(SkPaint::kFull_Hinting < 4, "insufficient_hinting_bits");
    uint32_t                 fHinting : 2;
    static_assert((kFlagsMask & 0xffff) == kFlagsMask, "insufficient_flags_bits");
    uint32_t                 fFlags : 16;

    typedef SkNoncopyable INHERITED;
};

struct SkRunFontStorageEquivalent {
    SkScalar fSize, fScaleX;
    void*    fTypeface;
    SkScalar fSkewX;
    uint32_t fFlags;
};
static_assert(sizeof(SkRunFont) == sizeof(SkRunFontStorageEquivalent), "runfont_should_stay_packed");

//
// Textblob data is laid out into externally-managed storage as follows:
//
//    -----------------------------------------------------------------------------
//   | SkTextBlob | RunRecord | Glyphs[] | Pos[] | RunRecord | Glyphs[] | Pos[] | ...
//    -----------------------------------------------------------------------------
//
//  Each run record describes a text blob run, and can be used to determine the (implicit)
//  location of the following record.
//
// Extended Textblob runs have more data after the Pos[] array:
//
//    -------------------------------------------------------------------------
//    ... | RunRecord | Glyphs[] | Pos[] | TextSize | Clusters[] | Text[] | ...
//    -------------------------------------------------------------------------
//
// To determine the length of the extended run data, the TextSize must be read.
//
// Extended Textblob runs may be mixed with non-extended runs.

SkDEBUGCODE(static const unsigned kRunRecordMagic = 0xb10bcafe;)

namespace {
struct RunRecordStorageEquivalent {
    SkRunFont  fFont;
    SkPoint  fOffset;
    uint32_t fCount;
    uint32_t fFlags;
    SkDEBUGCODE(unsigned fMagic;)
};
}

class SkTextBlob::RunRecord {
public:
    RunRecord(uint32_t count, uint32_t textSize,  const SkPoint& offset, const SkPaint& font, GlyphPositioning pos)
            : fFont(font)
            , fCount(count)
            , fOffset(offset)
            , fFlags(pos) {
        SkASSERT(static_cast<unsigned>(pos) <= Flags::kPositioning_Mask);

        SkDEBUGCODE(fMagic = kRunRecordMagic);
        if (textSize > 0) {
            fFlags |= kExtended_Flag;
            *this->textSizePtr() = textSize;
        }
    }

    uint32_t glyphCount() const {
        return fCount;
    }

    const SkPoint& offset() const {
        return fOffset;
    }

    const SkRunFont& font() const {
        return fFont;
    }

    GlyphPositioning positioning() const {
        return static_cast<GlyphPositioning>(fFlags & kPositioning_Mask);
    }

    uint16_t* glyphBuffer() const {
        static_assert(SkIsAlignPtr(sizeof(RunRecord)), "");
        // Glyphs are stored immediately following the record.
        return reinterpret_cast<uint16_t*>(const_cast<RunRecord*>(this) + 1);
    }

    SkScalar* posBuffer() const {
        // Position scalars follow the (aligned) glyph buffer.
        return reinterpret_cast<SkScalar*>(reinterpret_cast<uint8_t*>(this->glyphBuffer()) +
                                           SkAlign4(fCount * sizeof(uint16_t)));
    }

    uint32_t textSize() const { return isExtended() ? *this->textSizePtr() : 0; }

    uint32_t* clusterBuffer() const {
        // clusters follow the textSize.
        return isExtended() ? 1 + this->textSizePtr() : nullptr;
    }

    char* textBuffer() const {
        return isExtended()
               ? reinterpret_cast<char*>(this->clusterBuffer() + fCount)
               : nullptr;
    }

    static size_t StorageSize(uint32_t glyphCount, uint32_t textSize,
                              SkTextBlob::GlyphPositioning positioning,
                              SkSafeMath* safe) {
        static_assert(SkIsAlign4(sizeof(SkScalar)), "SkScalar size alignment");

        auto glyphSize = safe->mul(glyphCount, sizeof(uint16_t)),
                posSize = safe->mul(PosCount(glyphCount, positioning, safe), sizeof(SkScalar));

        // RunRecord object + (aligned) glyph buffer + position buffer
        auto size = sizeof(SkTextBlob::RunRecord);
        size = safe->add(size, safe->alignUp(glyphSize, 4));
        size = safe->add(size, posSize);

        if (textSize) {  // Extended run.
            size = safe->add(size, sizeof(uint32_t));
            size = safe->add(size, safe->mul(glyphCount, sizeof(uint32_t)));
            size = safe->add(size, textSize);
        }

        return safe->alignUp(size, sizeof(void*));
    }

    static const RunRecord* First(const SkTextBlob* blob) {
        // The first record (if present) is stored following the blob object.
        // (aligned up to make the RunRecord aligned too)
        return reinterpret_cast<const RunRecord*>(SkAlignPtr((uintptr_t)(blob + 1)));
    }

    static const RunRecord* Next(const RunRecord* run) {
        return SkToBool(run->fFlags & kLast_Flag) ? nullptr : NextUnchecked(run);
    }

    void validate(const uint8_t* storageTop) const {
        SkASSERT(kRunRecordMagic == fMagic);
        SkASSERT((uint8_t*)NextUnchecked(this) <= storageTop);

        SkASSERT(glyphBuffer() + fCount <= (uint16_t*)posBuffer());
        SkASSERT(posBuffer() + fCount * ScalarsPerGlyph(positioning())
                 <= (SkScalar*)NextUnchecked(this));
        if (isExtended()) {
            SkASSERT(textSize() > 0);
            SkASSERT(textSizePtr() < (uint32_t*)NextUnchecked(this));
            SkASSERT(clusterBuffer() < (uint32_t*)NextUnchecked(this));
            SkASSERT(textBuffer() + textSize() <= (char*)NextUnchecked(this));
        }
        static_assert(sizeof(SkTextBlob::RunRecord) == sizeof(RunRecordStorageEquivalent),
                      "runrecord_should_stay_packed");
    }

private:
    friend class SkTextBlobBuilder;

    enum Flags {
        kPositioning_Mask = 0x03, // bits 0-1 reserved for positioning
        kLast_Flag        = 0x04, // set for the last blob run
        kExtended_Flag    = 0x08, // set for runs with text/cluster info
    };

    static const RunRecord* NextUnchecked(const RunRecord* run) {
        SkSafeMath safe;
        auto res = reinterpret_cast<const RunRecord*>(
                reinterpret_cast<const uint8_t*>(run)
                + StorageSize(run->glyphCount(), run->textSize(), run->positioning(), &safe));
        SkASSERT(safe);
        return res;
    }

    static size_t PosCount(uint32_t glyphCount,
                           SkTextBlob::GlyphPositioning positioning,
                           SkSafeMath* safe) {
        return safe->mul(glyphCount, ScalarsPerGlyph(positioning));
    }

    uint32_t* textSizePtr() const {
        // textSize follows the position buffer.
        SkASSERT(isExtended());
        SkSafeMath safe;
        auto res = (uint32_t*)(&this->posBuffer()[PosCount(fCount, positioning(), &safe)]);
        SkASSERT(safe);
        return res;
    }

    void grow(uint32_t count) {
        SkScalar* initialPosBuffer = posBuffer();
        uint32_t initialCount = fCount;
        fCount += count;

        // Move the initial pos scalars to their new location.
        size_t copySize = initialCount * sizeof(SkScalar) * ScalarsPerGlyph(positioning());
        SkASSERT((uint8_t*)posBuffer() + copySize <= (uint8_t*)NextUnchecked(this));

        // memmove, as the buffers may overlap
        memmove(posBuffer(), initialPosBuffer, copySize);
    }

    bool isExtended() const {
        return fFlags & kExtended_Flag;
    }

    SkRunFont        fFont;
    uint32_t         fCount;
    SkPoint          fOffset;
    uint32_t         fFlags;

    SkDEBUGCODE(unsigned fMagic;)
};

// (paint->getFlags() & ~kFlagsMask) | fFlags
inline SkPaint::SkPaint(const SkPaint& basePaint, const SkRunFont& runFont)
        : fTypeface{runFont.fTypeface}
        , fPathEffect{basePaint.fPathEffect}
        , fShader{basePaint.fShader}
        , fMaskFilter{basePaint.fMaskFilter}
        , fColorFilter{basePaint.fColorFilter}
        , fDrawLooper{basePaint.fDrawLooper}
        , fImageFilter{basePaint.fImageFilter}
        , fTextSize{runFont.fSize}
        , fTextScaleX{runFont.fScaleX}
        , fTextSkewX{runFont.fSkewX}
        , fColor4f{basePaint.fColor4f}
        , fWidth{basePaint.fWidth}
        , fMiterLimit{basePaint.fMiterLimit}
        , fBlendMode{basePaint.fBlendMode}
        , fBitfieldsUInt{(basePaint.fBitfieldsUInt & ~SkRunFont::kFlagsMask) | runFont.fFlags} {
    fBitfields.fTextEncoding = kGlyphID_TextEncoding;
    fBitfields.fHinting = runFont.fHinting;
    fBitfields.fTextAlign = runFont.fAlign;
}

/**
 *  Iterate through all of the text runs of the text blob.  For example:
 *    for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
 *         .....
 *    }
 */
class SkTextBlobRunIterator {
public:
    SkTextBlobRunIterator(const SkTextBlob* blob);

    enum GlyphPositioning : uint8_t {
        kDefault_Positioning      = 0, // Default glyph advances -- zero scalars per glyph.
        kHorizontal_Positioning   = 1, // Horizontal positioning -- one scalar per glyph.
        kFull_Positioning         = 2  // Point positioning -- two scalars per glyph.
    };

    bool done() const {
        return !fCurrentRun;
    }
    void next();

    uint32_t glyphCount() const {
        SkASSERT(!this->done());
        return fCurrentRun->glyphCount();
    }
    const uint16_t* glyphs() const {
        SkASSERT(!this->done());
        return fCurrentRun->glyphBuffer();
    }
    const SkScalar* pos() const {
        SkASSERT(!this->done());
        return fCurrentRun->posBuffer();
    }
    const SkPoint& offset() const {
        SkASSERT(!this->done());
        return fCurrentRun->offset();
    }
    const SkRunFont& runFont() const {
        SkASSERT(!this->done());
        return fCurrentRun->font();
    }
    void applyFontToPaint(SkPaint*) const;
    GlyphPositioning positioning() const;
    uint32_t* clusters() const {
        SkASSERT(!this->done());
        return fCurrentRun->clusterBuffer();
    }
    uint32_t textSize() const {
        SkASSERT(!this->done());
        return fCurrentRun->textSize();
    }
    char* text() const {
        SkASSERT(!this->done());
        return fCurrentRun->textBuffer();
    }

    bool isLCD() const;

private:
    const SkTextBlob::RunRecord* fCurrentRun;

    SkDEBUGCODE(uint8_t* fStorageTop;)
};

#endif // SkTextBlobPriv_DEFINED

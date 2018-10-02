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
    SkRunFont(const SkPaint& paint);

    void applyToPaint(SkPaint* paint) const;

    bool operator==(const SkRunFont& other) const;

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
                              SkSafeMath* safe);

    static const RunRecord* First(const SkTextBlob* blob);

    static const RunRecord* Next(const RunRecord* run);

    void validate(const uint8_t* storageTop) const;

private:
    friend class SkTextBlobBuilder;

    enum Flags {
        kPositioning_Mask = 0x03, // bits 0-1 reserved for positioning
        kLast_Flag        = 0x04, // set for the last blob run
        kExtended_Flag    = 0x08, // set for runs with text/cluster info
    };

    static const RunRecord* NextUnchecked(const RunRecord* run);

    static size_t PosCount(uint32_t glyphCount,
                           SkTextBlob::GlyphPositioning positioning,
                           SkSafeMath* safe);

    uint32_t* textSizePtr() const;

    void grow(uint32_t count);

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

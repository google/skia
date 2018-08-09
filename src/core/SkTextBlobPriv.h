/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextBlobPriv_DEFINED
#define SkTextBlobPriv_DEFINED

#include "SkTextBlob.h"

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
    static const SkTextBlobBuilder::RunBuffer& AllocRunTextPos(SkTextBlobBuilder* builder,
            const SkPaint& font, int count, int textByteCount, SkString lang,
            const SkRect* bounds = nullptr) {
        return builder->allocRunTextPos(font, count, textByteCount, lang, bounds);
    }
};

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

    bool done() const;
    void next();

    uint32_t glyphCount() const;
    const uint16_t* glyphs() const;
    const SkScalar* pos() const;
    const SkPoint& offset() const;
    void applyFontToPaint(SkPaint*) const;
    GlyphPositioning positioning() const;
    uint32_t* clusters() const;
    uint32_t textSize() const;
    char* text() const;

    bool isLCD() const;

private:
    const SkTextBlob::RunRecord* fCurrentRun;

    SkDEBUGCODE(uint8_t* fStorageTop;)
};

#endif // SkTextBlobPriv_DEFINED

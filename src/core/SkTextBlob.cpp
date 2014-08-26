/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextBlob.h"

#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkTextBlob::SkTextBlob(uint16_t *glyphs, SkScalar *pos, const SkTArray<Run> *runs,
                       const SkRect& bounds)
    : fGlyphBuffer(glyphs)
    , fPosBuffer(pos)
    , fRuns(runs)
    , fBounds(bounds) {
}

uint32_t SkTextBlob::uniqueID() const {
    static int32_t  gTextBlobGenerationID; // = 0;

    // loop in case our global wraps around, as we never want to return SK_InvalidGenID
    while (SK_InvalidGenID == fUniqueID) {
        fUniqueID = sk_atomic_inc(&gTextBlobGenerationID) + 1;
    }

    return fUniqueID;
}

unsigned SkTextBlob::ScalarsPerGlyph(GlyphPositioning pos) {
    // GlyphPositioning values are directly mapped to scalars-per-glyph.
    SkASSERT(pos <= 2);
    return pos;
}

void SkTextBlob::flatten(SkWriteBuffer& buffer) const {
    int runCount = (NULL == fRuns.get()) ? 0 : fRuns->count();

    buffer.write32(runCount);
    buffer.writeRect(fBounds);

    SkPaint runPaint;
    RunIterator it(this);
    while (!it.done()) {
        SkASSERT(it.glyphCount() > 0);

        buffer.write32(it.glyphCount());
        buffer.write32(it.positioning());
        buffer.writePoint(it.offset());
        // This should go away when switching to SkFont
        it.applyFontToPaint(&runPaint);
        buffer.writePaint(runPaint);

        buffer.writeByteArray(it.glyphs(), it.glyphCount() * sizeof(uint16_t));
        buffer.writeByteArray(it.pos(),
            it.glyphCount() * sizeof(SkScalar) * ScalarsPerGlyph(it.positioning()));

        it.next();
        SkDEBUGCODE(runCount--);
    }
    SkASSERT(0 == runCount);
}

const SkTextBlob* SkTextBlob::CreateFromBuffer(SkReadBuffer& reader) {
    int runCount = reader.read32();
    if (runCount < 0) {
        return NULL;
    }

    SkRect bounds;
    reader.readRect(&bounds);

    SkTextBlobBuilder blobBuilder;
    for (int i = 0; i < runCount; ++i) {
        int glyphCount = reader.read32();
        GlyphPositioning pos = static_cast<GlyphPositioning>(reader.read32());
        if (glyphCount <= 0 || pos > kFull_Positioning) {
            return NULL;
        }

        SkPoint offset;
        reader.readPoint(&offset);
        SkPaint font;
        reader.readPaint(&font);

        const SkTextBlobBuilder::RunBuffer* buf = NULL;
        switch (pos) {
        case kDefault_Positioning:
            buf = &blobBuilder.allocRun(font, glyphCount, offset.x(), offset.y(), &bounds);
            break;
        case kHorizontal_Positioning:
            buf = &blobBuilder.allocRunPosH(font, glyphCount, offset.y(), &bounds);
            break;
        case kFull_Positioning:
            buf = &blobBuilder.allocRunPos(font, glyphCount, &bounds);
            break;
        default:
            return NULL;
        }

        if (!reader.readByteArray(buf->glyphs, glyphCount * sizeof(uint16_t)) ||
            !reader.readByteArray(buf->pos, glyphCount * sizeof(SkScalar) * ScalarsPerGlyph(pos))) {
            return NULL;
        }
    }

    return blobBuilder.build();
}

SkTextBlob::RunIterator::RunIterator(const SkTextBlob* blob)
    : fBlob(blob)
    , fIndex(0) {
    SkASSERT(NULL != blob);
}

bool SkTextBlob::RunIterator::done() const {
    return NULL == fBlob->fRuns.get() || fIndex >= fBlob->fRuns->count();
}

void SkTextBlob::RunIterator::next() {
    SkASSERT(!this->done());
    fIndex++;
}

uint32_t SkTextBlob::RunIterator::glyphCount() const {
    SkASSERT(!this->done());
    return (*fBlob->fRuns)[fIndex].count;
}

const uint16_t* SkTextBlob::RunIterator::glyphs() const {
    SkASSERT(!this->done());
    return fBlob->fGlyphBuffer.get() + (*fBlob->fRuns)[fIndex].glyphStart;
}

const SkScalar* SkTextBlob::RunIterator::pos() const {
    SkASSERT(!this->done());
    return fBlob->fPosBuffer.get() + (*fBlob->fRuns)[fIndex].posStart;
}

const SkPoint& SkTextBlob::RunIterator::offset() const {
    SkASSERT(!this->done());
    return (*fBlob->fRuns)[fIndex].offset;
}

SkTextBlob::GlyphPositioning SkTextBlob::RunIterator::positioning() const {
    SkASSERT(!this->done());
    return (*fBlob->fRuns)[fIndex].positioning;
}

void SkTextBlob::RunIterator::applyFontToPaint(SkPaint* paint) const {
    SkASSERT(!this->done());

    const SkPaint& font = (*fBlob->fRuns)[fIndex].font;

    paint->setTypeface(font.getTypeface());
    paint->setTextEncoding(font.getTextEncoding());
    paint->setTextSize(font.getTextSize());
    paint->setTextScaleX(font.getTextScaleX());
    paint->setTextSkewX(font.getTextSkewX());
    paint->setHinting(font.getHinting());

    uint32_t flagsMask = SkPaint::kAntiAlias_Flag
                       | SkPaint::kUnderlineText_Flag
                       | SkPaint::kStrikeThruText_Flag
                       | SkPaint::kFakeBoldText_Flag
                       | SkPaint::kLinearText_Flag
                       | SkPaint::kSubpixelText_Flag
                       | SkPaint::kDevKernText_Flag
                       | SkPaint::kLCDRenderText_Flag
                       | SkPaint::kEmbeddedBitmapText_Flag
                       | SkPaint::kAutoHinting_Flag
                       | SkPaint::kVerticalText_Flag
                       | SkPaint::kGenA8FromLCD_Flag
                       | SkPaint::kDistanceFieldTextTEMP_Flag;
    paint->setFlags((paint->getFlags() & ~flagsMask) | (font.getFlags() & flagsMask));
}

SkTextBlobBuilder::SkTextBlobBuilder(unsigned runs)
    : fRuns(NULL)
    , fDeferredBounds(false) {

    if (runs > 0) {
        // if the number of runs is known, size our run storage accordingly.
        fRuns = SkNEW(SkTArray<SkTextBlob::Run>(runs));
    }
    fBounds.setEmpty();
}

SkTextBlobBuilder::~SkTextBlobBuilder() {
    // unused runs
    SkDELETE(fRuns);
}

void SkTextBlobBuilder::updateDeferredBounds() {
    SkASSERT(!fDeferredBounds || (NULL != fRuns && !fRuns->empty()));

    if (!fDeferredBounds) {
        return;
    }

    // FIXME: measure the current run & union bounds
    fDeferredBounds = false;
}

void SkTextBlobBuilder::ensureRun(const SkPaint& font, SkTextBlob::GlyphPositioning pos,
                                  const SkPoint& offset) {
    SkASSERT(SkPaint::kGlyphID_TextEncoding == font.getTextEncoding());

    if (NULL == fRuns) {
        fRuns = SkNEW(SkTArray<SkTextBlob::Run>());
    }

    // we can merge same-font/same-positioning runs in the following cases:
    //   * fully positioned run following another fully positioned run
    //   * horizontally postioned run following another horizontally positioned run with the same
    //     y-offset
    if (!fRuns->empty()
        && fRuns->back().positioning == pos
        && fRuns->back().font == font
        && (SkTextBlob::kFull_Positioning == fRuns->back().positioning
            || (SkTextBlob::kHorizontal_Positioning == fRuns->back().positioning
                && fRuns->back().offset.y() == offset.y()))){
        return;
    }

    this->updateDeferredBounds();

    // start a new run
    SkTextBlob::Run& newRun = fRuns->push_back();
    newRun.count = 0;
    newRun.glyphStart = fGlyphBuffer.count();
    newRun.posStart = fPosBuffer.count();
    newRun.offset = offset;
    newRun.font = font;
    newRun.positioning = pos;
}

void SkTextBlobBuilder::allocInternal(const SkPaint &font,
                                      SkTextBlob::GlyphPositioning positioning,
                                      int count, SkPoint offset, const SkRect* bounds) {
    SkASSERT(count > 0);

    this->ensureRun(font, positioning, offset);

    unsigned posScalarsPerGlyph = SkTextBlob::ScalarsPerGlyph(positioning);

    fGlyphBuffer.append(count);
    fPosBuffer.append(count * posScalarsPerGlyph);

    SkASSERT(NULL != fRuns && !fRuns->empty());
    SkTextBlob::Run& run = fRuns->back();

    run.count += count;

    // The current run might have been merged, so the start offset may point to prev run data.
    // Start from the back (which always points to the end of the current run buffers) instead.
    fCurrentRunBuffer.glyphs = fGlyphBuffer.isEmpty()
            ? NULL : fGlyphBuffer.end() - count;
    SkASSERT(NULL == fCurrentRunBuffer.glyphs || fCurrentRunBuffer.glyphs >= fGlyphBuffer.begin());
    fCurrentRunBuffer.pos = fPosBuffer.isEmpty()
            ? NULL : fPosBuffer.end() - count * posScalarsPerGlyph;
    SkASSERT(NULL == fCurrentRunBuffer.pos || fCurrentRunBuffer.pos >= fPosBuffer.begin());

    if (!fDeferredBounds) {
        if (NULL != bounds) {
            fBounds.join(*bounds);
        } else {
            fDeferredBounds = true;
        }
    }
}

const SkTextBlobBuilder::RunBuffer& SkTextBlobBuilder::allocRun(const SkPaint& font, int count,
                                                                SkScalar x, SkScalar y,
                                                                const SkRect* bounds) {
    this->allocInternal(font, SkTextBlob::kDefault_Positioning, count, SkPoint::Make(x, y), bounds);

    return fCurrentRunBuffer;
}

const SkTextBlobBuilder::RunBuffer& SkTextBlobBuilder::allocRunPosH(const SkPaint& font, int count,
                                                                    SkScalar y,
                                                                    const SkRect* bounds) {
    this->allocInternal(font, SkTextBlob::kHorizontal_Positioning, count, SkPoint::Make(0, y),
                        bounds);

    return fCurrentRunBuffer;
}

const SkTextBlobBuilder::RunBuffer& SkTextBlobBuilder::allocRunPos(const SkPaint& font, int count,
                                                                   const SkRect *bounds) {
    this->allocInternal(font, SkTextBlob::kFull_Positioning, count, SkPoint::Make(0, 0), bounds);

    return fCurrentRunBuffer;
}

const SkTextBlob* SkTextBlobBuilder::build() {
    const SkTextBlob* blob;

    if (fGlyphBuffer.count() > 0) {
        // we have some glyphs, construct a real blob
        SkASSERT(NULL != fRuns && !fRuns->empty());

        this->updateDeferredBounds();

        // ownership of all buffers is transferred to the blob
        blob = SkNEW_ARGS(SkTextBlob, (fGlyphBuffer.detach(),
                                       fPosBuffer.detach(),
                                       fRuns,
                                       fBounds));
        fRuns = NULL;
        fBounds.setEmpty();
    } else {
        // empty blob
        SkASSERT(NULL == fRuns || fRuns->empty());
        SkASSERT(fBounds.isEmpty());

        blob = SkNEW_ARGS(SkTextBlob, (NULL, NULL, NULL, SkRect::MakeEmpty()));
    }

    return blob;
}


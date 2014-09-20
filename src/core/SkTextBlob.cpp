/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextBlob.h"

#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

//
// Textblob data is laid out into externally-managed storage as follows:
//
//    -----------------------------------------------------------------------------
//   | SkTextBlob | RunRecord | Glyphs[] | Pos[] | RunRecord | Glyphs[] | Pos[] | ...
//    -----------------------------------------------------------------------------
//
//  Each run record describes a text blob run, and can be used to determine the (implicit)
//  location of the following record.

SkDEBUGCODE(static const unsigned kRunRecordMagic = 0xb10bcafe;)

class SkTextBlob::RunRecord {
public:
    RunRecord(uint32_t count, const SkPoint& offset, const SkPaint& font, GlyphPositioning pos)
        : fCount(count)
        , fOffset(offset)
        , fFont(font)
        , fPositioning(pos) {
        SkDEBUGCODE(fMagic = kRunRecordMagic);
    }

    uint32_t glyphCount() const {
        return fCount;
    }

    const SkPoint& offset() const {
        return fOffset;
    }

    const SkPaint& font() const {
        return fFont;
    }

    GlyphPositioning positioning() const {
        return fPositioning;
    }

    uint16_t* glyphBuffer() const {
        // Glyph are stored immediately following the record.
        return reinterpret_cast<uint16_t*>(const_cast<RunRecord*>(this) + 1);
    }

    SkScalar* posBuffer() const {
        // Position scalars follow the (aligned) glyph buffer.
        return reinterpret_cast<SkScalar*>(reinterpret_cast<uint8_t*>(this->glyphBuffer()) +
                                           SkAlign4(fCount * sizeof(uint16_t)));
    }

    static size_t StorageSize(int glyphCount, SkTextBlob::GlyphPositioning positioning) {
        // RunRecord object + (aligned) glyph buffer + position buffer
        return SkAlignPtr(sizeof(SkTextBlob::RunRecord)
                        + SkAlign4(glyphCount* sizeof(uint16_t))
                        + glyphCount * sizeof(SkScalar) * ScalarsPerGlyph(positioning));
    }

    static const RunRecord* First(const SkTextBlob* blob) {
        // The first record (if present) is stored following the blob object.
        return reinterpret_cast<const RunRecord*>(blob + 1);
    }

    static const RunRecord* Next(const RunRecord* run) {
        return reinterpret_cast<const RunRecord*>(reinterpret_cast<const uint8_t*>(run)
            + StorageSize(run->glyphCount(), run->positioning()));
    }

    void validate(uint8_t* storageTop) const {
        SkASSERT(kRunRecordMagic == fMagic);
        SkASSERT((uint8_t*)Next(this) <= storageTop);
        SkASSERT(glyphBuffer() + fCount <= (uint16_t*)posBuffer());
        SkASSERT(posBuffer() + fCount * ScalarsPerGlyph(fPositioning) <= (SkScalar*)Next(this));
    }

private:
    friend class SkTextBlobBuilder;

    void grow(uint32_t count) {
        SkScalar* initialPosBuffer = posBuffer();
        uint32_t initialCount = fCount;
        fCount += count;

        // Move the initial pos scalars to their new location.
        size_t copySize = initialCount * sizeof(SkScalar) * ScalarsPerGlyph(fPositioning);
        SkASSERT((uint8_t*)posBuffer() + copySize <= (uint8_t*)Next(this));

        // memmove, as the buffers may overlap
        memmove(posBuffer(), initialPosBuffer, copySize);
    }

    uint32_t         fCount;
    SkPoint          fOffset;
    SkPaint          fFont;
    GlyphPositioning fPositioning;

    SkDEBUGCODE(unsigned fMagic;)
};

SkTextBlob::SkTextBlob(int runCount, const SkRect& bounds)
    : fRunCount(runCount)
    , fBounds(bounds) {
}

SkTextBlob::~SkTextBlob() {
    const RunRecord* run = RunRecord::First(this);
    for (int i = 0; i < fRunCount; ++i) {
        const RunRecord* nextRun = RunRecord::Next(run);
        SkDEBUGCODE(run->validate((uint8_t*)this + fStorageSize);)
        run->~RunRecord();
        run = nextRun;
    }
}

void SkTextBlob::internal_dispose() const {
    // SkTextBlobs use externally-managed storage.
    this->internal_dispose_restore_refcnt_to_1();
    this->~SkTextBlob();
    sk_free(const_cast<SkTextBlob*>(this));
}

uint32_t SkTextBlob::uniqueID() const {
    static int32_t  gTextBlobGenerationID; // = 0;

    // loop in case our global wraps around, as we never want to return SK_InvalidGenID
    while (SK_InvalidGenID == fUniqueID) {
        fUniqueID = sk_atomic_inc(&gTextBlobGenerationID) + 1;
    }

    return fUniqueID;
}

void SkTextBlob::flatten(SkWriteBuffer& buffer) const {
    int runCount = fRunCount;

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
            !reader.readByteArray(buf->pos,
                                  glyphCount * sizeof(SkScalar) * ScalarsPerGlyph(pos))) {
            return NULL;
        }
    }

    return blobBuilder.build();
}

unsigned SkTextBlob::ScalarsPerGlyph(GlyphPositioning pos) {
    // GlyphPositioning values are directly mapped to scalars-per-glyph.
    SkASSERT(pos <= 2);
    return pos;
}

SkTextBlob::RunIterator::RunIterator(const SkTextBlob* blob)
    : fCurrentRun(RunRecord::First(blob))
    , fRemainingRuns(blob->fRunCount) {
    SkDEBUGCODE(fStorageTop = (uint8_t*)blob + blob->fStorageSize;)
}

bool SkTextBlob::RunIterator::done() const {
    return fRemainingRuns <= 0;
}

void SkTextBlob::RunIterator::next() {
    SkASSERT(!this->done());

    if (!this->done()) {
        SkDEBUGCODE(fCurrentRun->validate(fStorageTop);)
        fCurrentRun = RunRecord::Next(fCurrentRun);
        fRemainingRuns--;
    }
}

uint32_t SkTextBlob::RunIterator::glyphCount() const {
    SkASSERT(!this->done());
    return fCurrentRun->glyphCount();
}

const uint16_t* SkTextBlob::RunIterator::glyphs() const {
    SkASSERT(!this->done());
    return fCurrentRun->glyphBuffer();
}

const SkScalar* SkTextBlob::RunIterator::pos() const {
    SkASSERT(!this->done());
    return fCurrentRun->posBuffer();
}

const SkPoint& SkTextBlob::RunIterator::offset() const {
    SkASSERT(!this->done());
    return fCurrentRun->offset();
}

SkTextBlob::GlyphPositioning SkTextBlob::RunIterator::positioning() const {
    SkASSERT(!this->done());
    return fCurrentRun->positioning();
}

void SkTextBlob::RunIterator::applyFontToPaint(SkPaint* paint) const {
    SkASSERT(!this->done());

    const SkPaint& font = fCurrentRun->font();

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

SkTextBlobBuilder::SkTextBlobBuilder()
    : fStorageSize(0)
    , fStorageUsed(0)
    , fRunCount(0)
    , fDeferredBounds(false)
    , fLastRun(0) {
    fBounds.setEmpty();
}

SkTextBlobBuilder::~SkTextBlobBuilder() {
    if (NULL != fStorage.get()) {
        // We are abandoning runs and must destruct the associated font data.
        // The easiest way to accomplish that is to use the blob destructor.
        build()->unref();
    }
}

void SkTextBlobBuilder::updateDeferredBounds() {
    SkASSERT(!fDeferredBounds || fRunCount > 0);

    if (!fDeferredBounds) {
        return;
    }

    // FIXME: measure the current run & union bounds
    fDeferredBounds = false;
}

void SkTextBlobBuilder::reserve(size_t size) {
    // We don't currently pre-allocate, but maybe someday...
    if (fStorageUsed + size <= fStorageSize) {
        return;
    }

    if (0 == fRunCount) {
        SkASSERT(NULL == fStorage.get());
        SkASSERT(0 == fStorageSize);
        SkASSERT(0 == fStorageUsed);

        // the first allocation also includes blob storage
        fStorageUsed += sizeof(SkTextBlob);
    }

    fStorageSize = fStorageUsed + size;
    // FYI: This relies on everything we store being relocatable, particularly SkPaint.
    fStorage.realloc(fStorageSize);
}

bool SkTextBlobBuilder::mergeRun(const SkPaint &font, SkTextBlob::GlyphPositioning positioning,
                                 int count, SkPoint offset) {
    if (0 == fLastRun) {
        SkASSERT(0 == fRunCount);
        return false;
    }

    SkASSERT(fLastRun >= sizeof(SkTextBlob));
    SkTextBlob::RunRecord* run = reinterpret_cast<SkTextBlob::RunRecord*>(fStorage.get() +
                                                                          fLastRun);
    SkASSERT(run->glyphCount() > 0);

    if (run->positioning() != positioning
        || run->font() != font
        || (run->glyphCount() + count < run->glyphCount())) {
        return false;
    }

    // we can merge same-font/same-positioning runs in the following cases:
    //   * fully positioned run following another fully positioned run
    //   * horizontally postioned run following another horizontally positioned run with the same
    //     y-offset
    if (SkTextBlob::kFull_Positioning != positioning
        && (SkTextBlob::kHorizontal_Positioning != positioning
            || run->offset().y() != offset.y())) {
        return false;
    }

    size_t sizeDelta = SkTextBlob::RunRecord::StorageSize(run->glyphCount() + count, positioning) -
                       SkTextBlob::RunRecord::StorageSize(run->glyphCount(), positioning);
    this->reserve(sizeDelta);

    // reserve may have realloced
    run = reinterpret_cast<SkTextBlob::RunRecord*>(fStorage.get() + fLastRun);
    uint32_t preMergeCount = run->glyphCount();
    run->grow(count);

    // Callers expect the buffers to point at the newly added slice, ant not at the beginning.
    fCurrentRunBuffer.glyphs = run->glyphBuffer() + preMergeCount;
    fCurrentRunBuffer.pos = run->posBuffer()
                          + preMergeCount * SkTextBlob::ScalarsPerGlyph(positioning);

    fStorageUsed += sizeDelta;

    SkASSERT(fStorageUsed <= fStorageSize);
    run->validate(fStorage.get() + fStorageUsed);

    return true;
}

void SkTextBlobBuilder::allocInternal(const SkPaint &font,
                                      SkTextBlob::GlyphPositioning positioning,
                                      int count, SkPoint offset, const SkRect* bounds) {
    SkASSERT(count > 0);
    SkASSERT(SkPaint::kGlyphID_TextEncoding == font.getTextEncoding());

    if (!this->mergeRun(font, positioning, count, offset)) {
        updateDeferredBounds();

        size_t runSize = SkTextBlob::RunRecord::StorageSize(count, positioning);
        this->reserve(runSize);

        SkASSERT(fStorageUsed >= sizeof(SkTextBlob));
        SkASSERT(fStorageUsed + runSize <= fStorageSize);

        SkTextBlob::RunRecord* run = new (fStorage.get() + fStorageUsed)
                                         SkTextBlob::RunRecord(count, offset, font, positioning);

        fCurrentRunBuffer.glyphs = run->glyphBuffer();
        fCurrentRunBuffer.pos = run->posBuffer();

        fLastRun = fStorageUsed;
        fStorageUsed += runSize;
        fRunCount++;

        SkASSERT(fStorageUsed <= fStorageSize);
        run->validate(fStorage.get() + fStorageUsed);
    }

    if (!fDeferredBounds) {
        if (bounds) {
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
    SkASSERT((fRunCount > 0) == (NULL != fStorage.get()));

    this->updateDeferredBounds();

    if (0 == fRunCount) {
        SkASSERT(NULL == fStorage.get());
        fStorageUsed = sizeof(SkTextBlob);
        fStorage.realloc(fStorageUsed);
    }

    SkDEBUGCODE(
        size_t validateSize = sizeof(SkTextBlob);
        const SkTextBlob::RunRecord* run =
            SkTextBlob::RunRecord::First(reinterpret_cast<const SkTextBlob*>(fStorage.get()));
        for (int i = 0; i < fRunCount; ++i) {
            validateSize += SkTextBlob::RunRecord::StorageSize(run->fCount, run->fPositioning);
            run->validate(fStorage.get() + fStorageUsed);
            run = SkTextBlob::RunRecord::Next(run);
        }
        SkASSERT(validateSize == fStorageUsed);
    )

    const SkTextBlob* blob = new (fStorage.detach()) SkTextBlob(fRunCount, fBounds);
    SkDEBUGCODE(const_cast<SkTextBlob*>(blob)->fStorageSize = fStorageSize;)

    fStorageUsed = 0;
    fStorageSize = 0;
    fRunCount = 0;
    fLastRun = 0;
    fBounds.setEmpty();

    return blob;
}


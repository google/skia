/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextBlob.h"

#include "SkGlyphRun.h"
#include "SkPaintPriv.h"
#include "SkReadBuffer.h"
#include "SkSafeMath.h"
#include "SkTextBlobPriv.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"

#include <limits>
#include <new>

#if SK_SUPPORT_GPU
#include "text/GrTextBlobCache.h"
#endif

SkRunFont::SkRunFont(const SkPaint& paint)
        : fSize(paint.getTextSize())
        , fScaleX(paint.getTextScaleX())
        , fTypeface(SkPaintPriv::RefTypefaceOrDefault(paint))
        , fSkewX(paint.getTextSkewX())
        , fAlign(paint.getTextAlign())
        , fHinting(paint.getHinting())
        , fFlags(paint.getFlags() & kFlagsMask) { }

void SkRunFont::applyToPaint(SkPaint* paint) const {
    paint->setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    paint->setTypeface(fTypeface);
    paint->setTextSize(fSize);
    paint->setTextScaleX(fScaleX);
    paint->setTextSkewX(fSkewX);
    paint->setTextAlign(static_cast<SkPaint::Align>(fAlign));
    paint->setHinting(static_cast<SkPaint::Hinting>(fHinting));

    paint->setFlags((paint->getFlags() & ~kFlagsMask) | fFlags);
}

bool SkRunFont::operator==(const SkRunFont& other) const {
    return fTypeface == other.fTypeface
           && fSize == other.fSize
           && fScaleX == other.fScaleX
           && fSkewX == other.fSkewX
           && fAlign == other.fAlign
           && fHinting == other.fHinting
           && fFlags == other.fFlags;
}

namespace {
struct RunFontStorageEquivalent {
    SkScalar fSize, fScaleX;
    void*    fTypeface;
    SkScalar fSkewX;
    uint32_t fFlags;
};
static_assert(sizeof(SkRunFont) == sizeof(RunFontStorageEquivalent), "runfont_should_stay_packed");
}

size_t SkTextBlob::RunRecord::StorageSize(uint32_t glyphCount, uint32_t textSize,
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

const SkTextBlob::RunRecord* SkTextBlob::RunRecord::First(const SkTextBlob* blob) {
    // The first record (if present) is stored following the blob object.
    // (aligned up to make the RunRecord aligned too)
    return reinterpret_cast<const RunRecord*>(SkAlignPtr((uintptr_t)(blob + 1)));
}

const SkTextBlob::RunRecord* SkTextBlob::RunRecord::Next(const RunRecord* run) {
    return SkToBool(run->fFlags & kLast_Flag) ? nullptr : NextUnchecked(run);
}

namespace {
struct RunRecordStorageEquivalent {
    SkRunFont  fFont;
    SkPoint  fOffset;
    uint32_t fCount;
    uint32_t fFlags;
    SkDEBUGCODE(unsigned fMagic;)
};
}

void SkTextBlob::RunRecord::validate(const uint8_t* storageTop) const {
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

const SkTextBlob::RunRecord* SkTextBlob::RunRecord::NextUnchecked(const RunRecord* run) {
    SkSafeMath safe;
    auto res = reinterpret_cast<const RunRecord*>(
            reinterpret_cast<const uint8_t*>(run)
            + StorageSize(run->glyphCount(), run->textSize(), run->positioning(), &safe));
    SkASSERT(safe);
    return res;
}

size_t SkTextBlob::RunRecord::PosCount(uint32_t glyphCount,
                                       SkTextBlob::GlyphPositioning positioning,
                                       SkSafeMath* safe) {
    return safe->mul(glyphCount, ScalarsPerGlyph(positioning));
}

uint32_t* SkTextBlob::RunRecord::textSizePtr() const {
    // textSize follows the position buffer.
    SkASSERT(isExtended());
    SkSafeMath safe;
    auto res = (uint32_t*)(&this->posBuffer()[PosCount(fCount, positioning(), &safe)]);
    SkASSERT(safe);
    return res;
}

void SkTextBlob::RunRecord::grow(uint32_t count) {
    SkScalar* initialPosBuffer = posBuffer();
    uint32_t initialCount = fCount;
    fCount += count;

    // Move the initial pos scalars to their new location.
    size_t copySize = initialCount * sizeof(SkScalar) * ScalarsPerGlyph(positioning());
    SkASSERT((uint8_t*)posBuffer() + copySize <= (uint8_t*)NextUnchecked(this));

    // memmove, as the buffers may overlap
    memmove(posBuffer(), initialPosBuffer, copySize);
}

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

SkTextBlob::SkTextBlob(const SkRect& bounds)
    : fBounds(bounds)
    , fUniqueID(next_id())
    , fCacheID(SK_InvalidUniqueID) {}

SkTextBlob::~SkTextBlob() {
#if SK_SUPPORT_GPU
    if (SK_InvalidUniqueID != fCacheID.load()) {
        GrTextBlobCache::PostPurgeBlobMessage(fUniqueID, fCacheID);
    }
#endif

    const auto* run = RunRecord::First(this);
    do {
        const auto* nextRun = RunRecord::Next(run);
        SkDEBUGCODE(run->validate((uint8_t*)this + fStorageSize);)
        run->~RunRecord();
        run = nextRun;
    } while (run);
}

namespace {

union PositioningAndExtended {
    int32_t intValue;
    struct {
        uint8_t  positioning;
        uint8_t  extended;
        uint16_t padding;
    };
};

static_assert(sizeof(PositioningAndExtended) == sizeof(int32_t), "");

} // namespace

enum SkTextBlob::GlyphPositioning : uint8_t {
        kDefault_Positioning      = 0, // Default glyph advances -- zero scalars per glyph.
        kHorizontal_Positioning   = 1, // Horizontal positioning -- one scalar per glyph.
        kFull_Positioning         = 2  // Point positioning -- two scalars per glyph.
};

unsigned SkTextBlob::ScalarsPerGlyph(GlyphPositioning pos) {
    // GlyphPositioning values are directly mapped to scalars-per-glyph.
    SkASSERT(pos <= 2);
    return pos;
}

void SkTextBlob::operator delete(void* p) {
    sk_free(p);
}

void* SkTextBlob::operator new(size_t) {
    SK_ABORT("All blobs are created by placement new.");
    return sk_malloc_throw(0);
}

void* SkTextBlob::operator new(size_t, void* p) {
    return p;
}

SkTextBlobRunIterator::SkTextBlobRunIterator(const SkTextBlob* blob)
    : fCurrentRun(SkTextBlob::RunRecord::First(blob)) {
    SkDEBUGCODE(fStorageTop = (uint8_t*)blob + blob->fStorageSize;)
}

void SkTextBlobRunIterator::next() {
    SkASSERT(!this->done());

    if (!this->done()) {
        SkDEBUGCODE(fCurrentRun->validate(fStorageTop);)
        fCurrentRun = SkTextBlob::RunRecord::Next(fCurrentRun);
    }
}

SkTextBlobRunIterator::GlyphPositioning SkTextBlobRunIterator::positioning() const {
    SkASSERT(!this->done());
    static_assert(static_cast<GlyphPositioning>(SkTextBlob::kDefault_Positioning) ==
                  kDefault_Positioning, "");
    static_assert(static_cast<GlyphPositioning>(SkTextBlob::kHorizontal_Positioning) ==
                  kHorizontal_Positioning, "");
    static_assert(static_cast<GlyphPositioning>(SkTextBlob::kFull_Positioning) ==
                  kFull_Positioning, "");

    return SkTo<GlyphPositioning>(fCurrentRun->positioning());
}

void SkTextBlobRunIterator::applyFontToPaint(SkPaint* paint) const {
    SkASSERT(!this->done());

    fCurrentRun->font().applyToPaint(paint);
}

bool SkTextBlobRunIterator::isLCD() const {
    return SkToBool(fCurrentRun->font().flags() & SkPaint::kLCDRenderText_Flag);
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
    if (nullptr != fStorage.get()) {
        // We are abandoning runs and must destruct the associated font data.
        // The easiest way to accomplish that is to use the blob destructor.
        this->make();
    }
}

SkRect SkTextBlobBuilder::TightRunBounds(const SkTextBlob::RunRecord& run) {
    SkRect bounds;
    SkPaint paint;
    run.font().applyToPaint(&paint);

    if (SkTextBlob::kDefault_Positioning == run.positioning()) {
        paint.measureText(run.glyphBuffer(), run.glyphCount() * sizeof(uint16_t), &bounds);
        return bounds.makeOffset(run.offset().x(), run.offset().y());
    }

    SkAutoSTArray<16, SkRect> glyphBounds(run.glyphCount());
    paint.getTextWidths(run.glyphBuffer(),
                        run.glyphCount() * sizeof(uint16_t),
                        nullptr,
                        glyphBounds.get());

    SkASSERT(SkTextBlob::kFull_Positioning == run.positioning() ||
             SkTextBlob::kHorizontal_Positioning == run.positioning());
    // kFull_Positioning       => [ x, y, x, y... ]
    // kHorizontal_Positioning => [ x, x, x... ]
    //                            (const y applied by runBounds.offset(run->offset()) later)
    const SkScalar horizontalConstY = 0;
    const SkScalar* glyphPosX = run.posBuffer();
    const SkScalar* glyphPosY = (run.positioning() == SkTextBlob::kFull_Positioning) ?
                                                      glyphPosX + 1 : &horizontalConstY;
    const unsigned posXInc = SkTextBlob::ScalarsPerGlyph(run.positioning());
    const unsigned posYInc = (run.positioning() == SkTextBlob::kFull_Positioning) ?
                                                   posXInc : 0;

    bounds.setEmpty();
    for (unsigned i = 0; i < run.glyphCount(); ++i) {
        bounds.join(glyphBounds[i].makeOffset(*glyphPosX, *glyphPosY));
        glyphPosX += posXInc;
        glyphPosY += posYInc;
    }

    SkASSERT((void*)glyphPosX <= SkTextBlob::RunRecord::Next(&run));

    return bounds.makeOffset(run.offset().x(), run.offset().y());
}

SkRect SkTextBlobBuilder::ConservativeRunBounds(const SkTextBlob::RunRecord& run) {
    SkASSERT(run.glyphCount() > 0);
    SkASSERT(SkTextBlob::kFull_Positioning == run.positioning() ||
             SkTextBlob::kHorizontal_Positioning == run.positioning());

    SkPaint paint;
    run.font().applyToPaint(&paint);
    const SkRect fontBounds = paint.getFontBounds();
    if (fontBounds.isEmpty()) {
        // Empty font bounds are likely a font bug.  TightBounds has a better chance of
        // producing useful results in this case.
        return TightRunBounds(run);
    }

    // Compute the glyph position bbox.
    SkRect bounds;
    switch (run.positioning()) {
    case SkTextBlob::kHorizontal_Positioning: {
        const SkScalar* glyphPos = run.posBuffer();
        SkASSERT((void*)(glyphPos + run.glyphCount()) <= SkTextBlob::RunRecord::Next(&run));

        SkScalar minX = *glyphPos;
        SkScalar maxX = *glyphPos;
        for (unsigned i = 1; i < run.glyphCount(); ++i) {
            SkScalar x = glyphPos[i];
            minX = SkMinScalar(x, minX);
            maxX = SkMaxScalar(x, maxX);
        }

        bounds.setLTRB(minX, 0, maxX, 0);
    } break;
    case SkTextBlob::kFull_Positioning: {
        const SkPoint* glyphPosPts = reinterpret_cast<const SkPoint*>(run.posBuffer());
        SkASSERT((void*)(glyphPosPts + run.glyphCount()) <= SkTextBlob::RunRecord::Next(&run));

        bounds.setBounds(glyphPosPts, run.glyphCount());
    } break;
    default:
        SK_ABORT("unsupported positioning mode");
    }

    // Expand by typeface glyph bounds.
    bounds.fLeft   += fontBounds.left();
    bounds.fTop    += fontBounds.top();
    bounds.fRight  += fontBounds.right();
    bounds.fBottom += fontBounds.bottom();

    // Offset by run position.
    return bounds.makeOffset(run.offset().x(), run.offset().y());
}

void SkTextBlobBuilder::updateDeferredBounds() {
    SkASSERT(!fDeferredBounds || fRunCount > 0);

    if (!fDeferredBounds) {
        return;
    }

    SkASSERT(fLastRun >= SkAlignPtr(sizeof(SkTextBlob)));
    SkTextBlob::RunRecord* run = reinterpret_cast<SkTextBlob::RunRecord*>(fStorage.get() +
                                                                          fLastRun);

    // FIXME: we should also use conservative bounds for kDefault_Positioning.
    SkRect runBounds = SkTextBlob::kDefault_Positioning == run->positioning() ?
                       TightRunBounds(*run) : ConservativeRunBounds(*run);
    fBounds.join(runBounds);
    fDeferredBounds = false;
}

void SkTextBlobBuilder::reserve(size_t size) {
    SkSafeMath safe;

    // We don't currently pre-allocate, but maybe someday...
    if (safe.add(fStorageUsed, size) <= fStorageSize && safe) {
        return;
    }

    if (0 == fRunCount) {
        SkASSERT(nullptr == fStorage.get());
        SkASSERT(0 == fStorageSize);
        SkASSERT(0 == fStorageUsed);

        // the first allocation also includes blob storage
        // aligned up to a pointer alignment so SkTextBlob::RunRecords after it stay aligned.
        fStorageUsed = SkAlignPtr(sizeof(SkTextBlob));
    }

    fStorageSize = safe.add(fStorageUsed, size);

    // FYI: This relies on everything we store being relocatable, particularly SkPaint.
    //      Also, this is counting on the underlying realloc to throw when passed max().
    fStorage.realloc(safe ? fStorageSize : std::numeric_limits<size_t>::max());
}

bool SkTextBlobBuilder::mergeRun(const SkPaint &font, SkTextBlob::GlyphPositioning positioning,
                                 uint32_t count, SkPoint offset) {
    if (0 == fLastRun) {
        SkASSERT(0 == fRunCount);
        return false;
    }

    SkASSERT(fLastRun >= SkAlignPtr(sizeof(SkTextBlob)));
    SkTextBlob::RunRecord* run = reinterpret_cast<SkTextBlob::RunRecord*>(fStorage.get() +
                                                                          fLastRun);
    SkASSERT(run->glyphCount() > 0);

    if (run->textSize() != 0) {
        return false;
    }

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

    SkSafeMath safe;
    size_t sizeDelta =
        SkTextBlob::RunRecord::StorageSize(run->glyphCount() + count, 0, positioning, &safe) -
        SkTextBlob::RunRecord::StorageSize(run->glyphCount()        , 0, positioning, &safe);
    if (!safe) {
        return false;
    }

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
                                      int count, int textSize, SkPoint offset,
                                      const SkRect* bounds) {
    if (count <= 0 || textSize < 0 || font.getTextEncoding() != SkPaint::kGlyphID_TextEncoding) {
        fCurrentRunBuffer = { nullptr, nullptr, nullptr, nullptr };
        return;
    }

    if (textSize != 0 || !this->mergeRun(font, positioning, count, offset)) {
        this->updateDeferredBounds();

        SkSafeMath safe;
        size_t runSize = SkTextBlob::RunRecord::StorageSize(count, textSize, positioning, &safe);
        if (!safe) {
            fCurrentRunBuffer = { nullptr, nullptr, nullptr, nullptr };
            return;
        }

        this->reserve(runSize);

        SkASSERT(fStorageUsed >= SkAlignPtr(sizeof(SkTextBlob)));
        SkASSERT(fStorageUsed + runSize <= fStorageSize);

        SkTextBlob::RunRecord* run = new (fStorage.get() + fStorageUsed)
            SkTextBlob::RunRecord(count, textSize, offset, font, positioning);
        fCurrentRunBuffer.glyphs = run->glyphBuffer();
        fCurrentRunBuffer.pos = run->posBuffer();
        fCurrentRunBuffer.utf8text = run->textBuffer();
        fCurrentRunBuffer.clusters = run->clusterBuffer();

        fLastRun = fStorageUsed;
        fStorageUsed += runSize;
        fRunCount++;

        SkASSERT(fStorageUsed <= fStorageSize);
        run->validate(fStorage.get() + fStorageUsed);
    }
    SkASSERT(textSize > 0 || nullptr == fCurrentRunBuffer.utf8text);
    SkASSERT(textSize > 0 || nullptr == fCurrentRunBuffer.clusters);
    if (!fDeferredBounds) {
        if (bounds) {
            fBounds.join(*bounds);
        } else {
            fDeferredBounds = true;
        }
    }
}

const SkTextBlobBuilder::RunBuffer& SkTextBlobBuilder::allocRunText(const SkPaint& font, int count,
                                                                    SkScalar x, SkScalar y,
                                                                    int textByteCount,
                                                                    SkString lang,
                                                                    const SkRect* bounds) {
    this->allocInternal(font, SkTextBlob::kDefault_Positioning, count, textByteCount, SkPoint::Make(x, y), bounds);
    return fCurrentRunBuffer;
}

const SkTextBlobBuilder::RunBuffer& SkTextBlobBuilder::allocRunTextPosH(const SkPaint& font, int count,
                                                                        SkScalar y,
                                                                        int textByteCount,
                                                                        SkString lang,
                                                                        const SkRect* bounds) {
    this->allocInternal(font, SkTextBlob::kHorizontal_Positioning, count, textByteCount, SkPoint::Make(0, y),
                        bounds);

    return fCurrentRunBuffer;
}

const SkTextBlobBuilder::RunBuffer& SkTextBlobBuilder::allocRunTextPos(const SkPaint& font, int count,
                                                                       int textByteCount,
                                                                       SkString lang,
                                                                       const SkRect *bounds) {
    this->allocInternal(font, SkTextBlob::kFull_Positioning, count, textByteCount, SkPoint::Make(0, 0), bounds);

    return fCurrentRunBuffer;
}

sk_sp<SkTextBlob> SkTextBlobBuilder::make() {
    if (!fRunCount) {
        // We don't instantiate empty blobs.
        SkASSERT(!fStorage.get());
        SkASSERT(fStorageUsed == 0);
        SkASSERT(fStorageSize == 0);
        SkASSERT(fLastRun == 0);
        SkASSERT(fBounds.isEmpty());
        return nullptr;
    }

    this->updateDeferredBounds();

    // Tag the last run as such.
    auto* lastRun = reinterpret_cast<SkTextBlob::RunRecord*>(fStorage.get() + fLastRun);
    lastRun->fFlags |= SkTextBlob::RunRecord::kLast_Flag;

    SkTextBlob* blob = new (fStorage.release()) SkTextBlob(fBounds);
    SkDEBUGCODE(const_cast<SkTextBlob*>(blob)->fStorageSize = fStorageSize;)

    SkDEBUGCODE(
        SkSafeMath safe;
        size_t validateSize = SkAlignPtr(sizeof(SkTextBlob));
        for (const auto* run = SkTextBlob::RunRecord::First(blob); run;
             run = SkTextBlob::RunRecord::Next(run)) {
            validateSize += SkTextBlob::RunRecord::StorageSize(
                    run->fCount, run->textSize(), run->positioning(), &safe);
            run->validate(reinterpret_cast<const uint8_t*>(blob) + fStorageUsed);
            fRunCount--;
        }
        SkASSERT(validateSize == fStorageUsed);
        SkASSERT(fRunCount == 0);
        SkASSERT(safe);
    )

    fStorageUsed = 0;
    fStorageSize = 0;
    fRunCount = 0;
    fLastRun = 0;
    fBounds.setEmpty();

    return sk_sp<SkTextBlob>(blob);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkTextBlobPriv::Flatten(const SkTextBlob& blob, SkWriteBuffer& buffer) {
    buffer.writeRect(blob.fBounds);

    SkPaint runPaint;
    SkTextBlobRunIterator it(&blob);
    while (!it.done()) {
        SkASSERT(it.glyphCount() > 0);

        buffer.write32(it.glyphCount());
        PositioningAndExtended pe;
        pe.intValue = 0;
        pe.positioning = it.positioning();
        SkASSERT((int32_t)it.positioning() == pe.intValue);  // backwards compat.

        uint32_t textSize = it.textSize();
        pe.extended = textSize > 0;
        buffer.write32(pe.intValue);
        if (pe.extended) {
            buffer.write32(textSize);
        }
        buffer.writePoint(it.offset());
        // This should go away when switching to SkFont
        it.applyFontToPaint(&runPaint);
        buffer.writePaint(runPaint);

        buffer.writeByteArray(it.glyphs(), it.glyphCount() * sizeof(uint16_t));
        buffer.writeByteArray(it.pos(),
                              it.glyphCount() * sizeof(SkScalar) *
                              SkTextBlob::ScalarsPerGlyph(
                                  SkTo<SkTextBlob::GlyphPositioning>(it.positioning())));
        if (pe.extended) {
            buffer.writeByteArray(it.clusters(), sizeof(uint32_t) * it.glyphCount());
            buffer.writeByteArray(it.text(), it.textSize());
        }

        it.next();
    }

    // Marker for the last run (0 is not a valid glyph count).
    buffer.write32(0);
}

sk_sp<SkTextBlob> SkTextBlobPriv::MakeFromBuffer(SkReadBuffer& reader) {
    SkRect bounds;
    reader.readRect(&bounds);

    SkTextBlobBuilder blobBuilder;
    SkSafeMath safe;
    for (;;) {
        int glyphCount = reader.read32();
        if (glyphCount == 0) {
            // End-of-runs marker.
            break;
        }

        PositioningAndExtended pe;
        pe.intValue = reader.read32();
        const auto pos = SkTo<SkTextBlob::GlyphPositioning>(pe.positioning);
        if (glyphCount <= 0 || pos > SkTextBlob::kFull_Positioning) {
            return nullptr;
        }
        int textSize = pe.extended ? reader.read32() : 0;
        if (textSize < 0) {
            return nullptr;
        }

        SkPoint offset;
        reader.readPoint(&offset);
        SkPaint font;
        reader.readPaint(&font);

        // Compute the expected size of the buffer and ensure we have enough to deserialize
        // a run before allocating it.
        const size_t glyphSize = safe.mul(glyphCount, sizeof(uint16_t)),
                     posSize =
                             safe.mul(glyphCount, safe.mul(sizeof(SkScalar),
                             SkTextBlob::ScalarsPerGlyph(pos))),
                     clusterSize = pe.extended ? safe.mul(glyphCount, sizeof(uint32_t)) : 0;
        const size_t totalSize =
                safe.add(safe.add(glyphSize, posSize), safe.add(clusterSize, textSize));

        if (!reader.isValid() || !safe || totalSize > reader.available()) {
            return nullptr;
        }

        const SkTextBlobBuilder::RunBuffer* buf = nullptr;
        switch (pos) {
            case SkTextBlob::kDefault_Positioning:
                buf = &blobBuilder.allocRunText(font, glyphCount, offset.x(), offset.y(),
                                                textSize, SkString(), &bounds);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                buf = &blobBuilder.allocRunTextPosH(font, glyphCount, offset.y(),
                                                    textSize, SkString(), &bounds);
                break;
            case SkTextBlob::kFull_Positioning:
                buf = &blobBuilder.allocRunTextPos(font, glyphCount, textSize, SkString(), &bounds);
                break;
            default:
                return nullptr;
        }

        if (!buf->glyphs ||
            !buf->pos ||
            (pe.extended && (!buf->clusters || !buf->utf8text))) {
            return nullptr;
        }

        if (!reader.readByteArray(buf->glyphs, glyphSize) ||
            !reader.readByteArray(buf->pos, posSize)) {
            return nullptr;
            }

        if (pe.extended) {
            if (!reader.readByteArray(buf->clusters, clusterSize) ||
                !reader.readByteArray(buf->utf8text, textSize)) {
                return nullptr;
            }
        }
    }

    return blobBuilder.make();
}

sk_sp<SkTextBlob> SkTextBlob::MakeFromText(
        const void* text, size_t byteLength, const SkPaint& paint) {
    SkGlyphRunBuilder runBuilder;

    runBuilder.drawText(paint, text, byteLength, SkPoint::Make(0, 0));

    auto glyphRunList = runBuilder.useGlyphRunList();
    SkTextBlobBuilder blobBuilder;
    if (!glyphRunList.empty()) {
        auto run = glyphRunList[0];
        SkPaint blobPaint(paint);
        blobPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

        auto runData = blobBuilder.allocRunPos(blobPaint, run.runSize());
        run.filloutGlyphsAndPositions(runData.glyphs, (SkPoint *)runData.pos);
    }

    return blobBuilder.make();
}

sk_sp<SkData> SkTextBlob::serialize(const SkSerialProcs& procs) const {
    SkBinaryWriteBuffer buffer;
    buffer.setSerialProcs(procs);
    SkTextBlobPriv::Flatten(*this, buffer);

    size_t total = buffer.bytesWritten();
    sk_sp<SkData> data = SkData::MakeUninitialized(total);
    buffer.writeToMemory(data->writable_data());
    return data;
}

sk_sp<SkTextBlob> SkTextBlob::Deserialize(const void* data, size_t length,
                                          const SkDeserialProcs& procs) {
    SkReadBuffer buffer(data, length);
    buffer.setDeserialProcs(procs);
    return SkTextBlobPriv::MakeFromBuffer(buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkTextBlob::serialize(const SkSerialProcs& procs, void* memory, size_t memory_size) const {
    SkBinaryWriteBuffer buffer(memory, memory_size);
    buffer.setSerialProcs(procs);
    SkTextBlobPriv::Flatten(*this, buffer);
    return buffer.usingInitialStorage() ? buffer.bytesWritten() : 0u;
}

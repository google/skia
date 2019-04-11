/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyph_DEFINED
#define SkGlyph_DEFINED

#include "SkChecksum.h"
#include "SkFixed.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkTo.h"
#include "SkTypes.h"

class SkArenaAlloc;
class SkStrike;
class SkScalerContext;

// needs to be != to any valid SkMask::Format
#define MASK_FORMAT_UNKNOWN         (0xFF)
#define MASK_FORMAT_JUST_ADVANCE    MASK_FORMAT_UNKNOWN

#define kMaxGlyphWidth (1<<13)

/** (glyph-index or unicode-point) + subpixel-pos */
struct SkPackedID {
    static constexpr uint32_t kImpossibleID = ~0;
    enum {
        kSubBits = 2,
        kSubMask = ((1 << kSubBits) - 1),
        kSubShift = 24, // must be large enough for glyphs and unichars
        kCodeMask = ((1 << kSubShift) - 1),
        // relative offsets for X and Y subpixel bits
        kSubShiftX = kSubBits,
        kSubShiftY = 0
    };

    SkPackedID(uint32_t code) {
        SkASSERT(code <= kCodeMask);
        SkASSERT(code != kImpossibleID);
        fID = code;
    }

    SkPackedID(uint32_t code, SkFixed x, SkFixed y) {
        SkASSERT(code <= kCodeMask);
        x = FixedToSub(x);
        y = FixedToSub(y);
        uint32_t ID = (x << (kSubShift + kSubShiftX)) |
                      (y << (kSubShift + kSubShiftY)) |
                      code;
        SkASSERT(ID != kImpossibleID);
        fID = ID;
    }

    constexpr SkPackedID() : fID(kImpossibleID) {}

    bool operator==(const SkPackedID& that) const {
        return fID == that.fID;
    }
    bool operator!=(const SkPackedID& that) const {
        return !(*this == that);
    }
    bool operator<(SkPackedID that) const {
        return this->fID < that.fID;
    }

    uint32_t code() const {
        return fID & kCodeMask;
    }

    uint32_t value() const {
        return fID;
    }

    SkFixed getSubXFixed() const {
        return SubToFixed(ID2SubX(fID));
    }

    SkFixed getSubYFixed() const {
        return SubToFixed(ID2SubY(fID));
    }

    uint32_t hash() const {
        return SkChecksum::CheapMix(fID);
    }

    SkString dump() const {
        SkString str;
        str.appendf("code: %d, x: %d, y:%d", code(), getSubXFixed(), getSubYFixed());
        return str;
    }

private:
    static unsigned ID2SubX(uint32_t id) {
        return id >> (kSubShift + kSubShiftX);
    }

    static unsigned ID2SubY(uint32_t id) {
        return (id >> (kSubShift + kSubShiftY)) & kSubMask;
    }

    static unsigned FixedToSub(SkFixed n) {
        return (n >> (16 - kSubBits)) & kSubMask;
    }

    static SkFixed SubToFixed(unsigned sub) {
        SkASSERT(sub <= kSubMask);
        return sub << (16 - kSubBits);
    }

    uint32_t fID;
};

struct SkPackedGlyphID : public SkPackedID {
    SkPackedGlyphID(SkGlyphID code) : SkPackedID(code) { }
    SkPackedGlyphID(SkGlyphID code, SkFixed x, SkFixed y) : SkPackedID(code, x, y) { }
    SkPackedGlyphID(SkGlyphID code, SkIPoint pt) : SkPackedID(code, pt.x(), pt.y()) { }
    constexpr SkPackedGlyphID() = default;
    SkGlyphID code() const {
        return SkTo<SkGlyphID>(SkPackedID::code());
    }
};

class SkGlyph {
    struct PathData;

public:
    constexpr explicit SkGlyph(SkPackedGlyphID id) : fID{id} {}
    static constexpr SkFixed kSubpixelRound = SK_FixedHalf >> SkPackedID::kSubBits;

    bool isEmpty() const { return fWidth == 0 || fHeight == 0; }
    bool isJustAdvance() const { return MASK_FORMAT_JUST_ADVANCE == fMaskFormat; }
    bool isFullMetrics() const { return MASK_FORMAT_JUST_ADVANCE != fMaskFormat; }
    SkGlyphID getGlyphID() const { return fID.code(); }
    SkPackedGlyphID getPackedID() const { return fID; }
    SkFixed getSubXFixed() const { return fID.getSubXFixed(); }
    SkFixed getSubYFixed() const { return fID.getSubYFixed(); }

    size_t formatAlignment() const;
    size_t allocImage(SkArenaAlloc* alloc);
    size_t rowBytes() const;
    size_t computeImageSize() const;
    size_t rowBytesUsingFormat(SkMask::Format format) const;

    // Call this to set all of the metrics fields to 0 (e.g. if the scaler
    // encounters an error measuring a glyph). Note: this does not alter the
    // fImage, fPath, fID, fMaskFormat fields.
    void zeroMetrics();

    void toMask(SkMask* mask) const;

    SkMask mask(SkPoint position) const;

    SkPath* addPath(SkScalerContext*, SkArenaAlloc*);

    SkPath* path() const {
        return fPathData != nullptr && fPathData->fHasPath ? &fPathData->fPath : nullptr;
    }

    int maxDimension() const {
        // width and height are only defined if a metrics call was made.
        SkASSERT(fMaskFormat != MASK_FORMAT_UNKNOWN);

        return std::max(fWidth, fHeight);
    }

    // Returns the size allocated on the arena.
    size_t copyImageData(const SkGlyph& from, SkArenaAlloc* alloc);

    void*     fImage    = nullptr;

    // Path data has tricky state. If the glyph isEmpty, then fPathData should always be nullptr,
    // else if fPathData is not null, then a path has been requested. The fPath field of fPathData
    // may still be null after the request meaning that there is no path for this glyph.
    PathData* fPathData = nullptr;

    // The advance for this glyph.
    float     fAdvanceX = 0,
              fAdvanceY = 0;

    // The width and height of the glyph mask.
    uint16_t  fWidth  = 0,
              fHeight = 0;

    // The offset from the glyphs origin on the baseline to the top left of the glyph mask.
    int16_t   fTop  = 0,
              fLeft = 0;

    // Used by the GDI scaler to track state.
    int8_t    fForceBW = 0;

    // This is a combination of SkMask::Format and SkGlyph state. The SkGlyph can be in one of two
    // states, just the advances have been calculated, and all the metrics are available. The
    // illegal mask format is used to signal that only the advances are available.
    uint8_t   fMaskFormat = MASK_FORMAT_UNKNOWN;

private:

    // Support horizontal and vertical skipping strike-through / underlines.
    // The caller walks the linked list looking for a match. For a horizontal underline,
    // the fBounds contains the top and bottom of the underline. The fInterval pair contains the
    // beginning and end of of the intersection of the bounds and the glyph's path.
    // If interval[0] >= interval[1], no intesection was found.
    struct Intercept {
        Intercept* fNext;
        SkScalar   fBounds[2];    // for horz underlines, the boundaries in Y
        SkScalar   fInterval[2];  // the outside intersections of the axis and the glyph
    };

    struct PathData {
        Intercept* fIntercept{nullptr};
        SkPath     fPath;
        bool       fHasPath{false};
    };

    // TODO(herb) remove friend statement after SkStrike cleanup.
    friend class SkStrike;
    SkPackedGlyphID fID;
};

#endif

/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyph_DEFINED
#define SkGlyph_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkTypes.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTo.h"
#include "src/core/SkMask.h"

class SkArenaAlloc;
class SkStrike;
class SkScalerContext;

// needs to be != to any valid SkMask::Format
#define MASK_FORMAT_UNKNOWN         (0xFF)
#define MASK_FORMAT_JUST_ADVANCE    MASK_FORMAT_UNKNOWN

#define kMaxGlyphWidth (1<<13)

/** SkGlyphID + subpixel-pos */
struct SkPackedGlyphID {
    static constexpr uint32_t kImpossibleID = ~0u;
    enum {
        kSubBits = 2u,
        kSubMask = ((1u << kSubBits) - 1),
        kSubShift = 24u, // must be large enough for glyphs and unichars
        kCodeMask = ((1u << kSubShift) - 1),
        // relative offsets for X and Y subpixel bits
        kSubShiftX = kSubBits,
        kSubShiftY = 0
    };

    constexpr explicit SkPackedGlyphID(SkGlyphID glyphID)
            : fID{glyphID} { }

    constexpr SkPackedGlyphID(SkGlyphID glyphID, SkFixed x, SkFixed y)
            : fID {PackIDXY(glyphID, x, y)} {
        SkASSERT(fID != kImpossibleID);
    }

    constexpr SkPackedGlyphID(SkGlyphID code, SkIPoint pt)
        : SkPackedGlyphID(code, pt.fX, pt.fY) { }

    constexpr SkPackedGlyphID() : fID{kImpossibleID} {}

    bool operator==(const SkPackedGlyphID& that) const {
        return fID == that.fID;
    }
    bool operator!=(const SkPackedGlyphID& that) const {
        return !(*this == that);
    }
    bool operator<(SkPackedGlyphID that) const {
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
    static constexpr uint32_t PackIDXY(SkGlyphID glyphID, SkFixed x, SkFixed y) {
        return (FixedToSub(x) << (kSubShift + kSubShiftX))
             | (FixedToSub(y) << (kSubShift + kSubShiftY))
             | glyphID;
    }

    static constexpr unsigned ID2SubX(uint32_t id) {
        return id >> (kSubShift + kSubShiftX);
    }

    static constexpr unsigned ID2SubY(uint32_t id) {
        return (id >> (kSubShift + kSubShiftY)) & kSubMask;
    }

    static constexpr unsigned FixedToSub(SkFixed n) {
        return (n >> (16 - kSubBits)) & kSubMask;
    }

    static constexpr SkFixed SubToFixed(uint32_t sub) {
        SkASSERT(sub <= kSubMask);
        return sub << (16u - kSubBits);
    }

    uint32_t fID;
};

class SkGlyph {
    struct PathData;

public:
    constexpr explicit SkGlyph(SkPackedGlyphID id) : fID{id} {}
    static constexpr SkFixed kSubpixelRound = SK_FixedHalf >> SkPackedGlyphID::kSubBits;

    SkVector advanceVector() const { return SkVector{fAdvanceX, fAdvanceY}; }
    SkScalar advanceX() const { return fAdvanceX; }
    SkScalar advanceY() const { return fAdvanceY; }

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

    bool hasImage() const {
        SkASSERT(fMaskFormat != MASK_FORMAT_UNKNOWN);
        return fImage != nullptr;
    }

    SkMask mask() const;

    SkMask mask(SkPoint position) const;

    // If we haven't already tried to associate a path to this glyph
    // (i.e. setPathHasBeenCalled() returns false), then use the
    // SkScalerContext or SkPath argument to try to do so.  N.B. this
    // may still result in no path being associated with this glyph,
    // e.g. if you pass a null SkPath or the typeface is bitmap-only.
    //
    // This setPath() call is sticky... once you call it, the glyph
    // stays in its state permanently, ignoring any future calls.
    //
    // Returns true if this is the first time you called setPath()
    // and there actually is a path; call path() to get it.
    bool setPath(SkArenaAlloc* alloc, SkScalerContext* scalerContext);
    bool setPath(SkArenaAlloc* alloc, const SkPath* path);

    // Returns true if that path has been set.
    bool setPathHasBeenCalled() const { return fPathData != nullptr; }

    // Return a pointer to the path if it exists, otherwise return nullptr. Only works if the
    // path was previously set.
    const SkPath* path() const;

    int maxDimension() const {
        // width and height are only defined if a metrics call was made.
        SkASSERT(fMaskFormat != MASK_FORMAT_UNKNOWN);

        return std::max(fWidth, fHeight);
    }

    // Returns the size allocated on the arena.
    size_t copyImageData(const SkGlyph& from, SkArenaAlloc* alloc);

    // Make sure that the intercept information is on the glyph and return it, or return it if it
    // already exists.
    // * bounds - either end of the gap for the character.
    // * scale, xPos - information about how wide the gap is.
    // * array - accumulated gaps for many characters if not null.
    // * count - the number of gaps.
    void ensureIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                          SkScalar* array, int* count, SkArenaAlloc* alloc);

    void*     fImage    = nullptr;

    // The width and height of the glyph mask.
    uint16_t  fWidth  = 0,
              fHeight = 0;

    // The offset from the glyphs origin on the baseline to the top left of the glyph mask.
    int16_t   fTop  = 0,
              fLeft = 0;

    // This is a combination of SkMask::Format and SkGlyph state. The SkGlyph can be in one of two
    // states, just the advances have been calculated, and all the metrics are available. The
    // illegal mask format is used to signal that only the advances are available.
    uint8_t   fMaskFormat = MASK_FORMAT_UNKNOWN;

private:
    // There are two sides to an SkGlyph, the scaler side (things that create glyph data) have
    // access to all the fields. Scalers are assumed to maintain all the SkGlyph invariants. The
    // consumer side has a tighter interface.
    friend class SkScalerContext_FreeType;
    friend class SkScalerContext_DW;
    friend class SkScalerContext_GDI;
    friend class SkScalerContext_Mac;
    friend class SkStrikeClient;
    friend class SkTestScalerContext;
    friend class SkTestSVGScalerContext;
    friend class TestSVGTypeface;
    friend class TestTypeface;

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

    // path == nullptr indicates there is no path.
    void installPath(SkArenaAlloc* alloc, const SkPath* path);

    // Path data has tricky state. If the glyph isEmpty, then fPathData should always be nullptr,
    // else if fPathData is not null, then a path has been requested. The fPath field of fPathData
    // may still be null after the request meaning that there is no path for this glyph.
    PathData* fPathData = nullptr;

    // The advance for this glyph.
    float     fAdvanceX = 0,
              fAdvanceY = 0;

    // Used by the DirectWrite scaler to track state.
    int8_t    fForceBW = 0;

    // TODO(herb) remove friend statement after SkStrike cleanup.
    friend class SkStrike;
    SkPackedGlyphID fID;
};

#endif

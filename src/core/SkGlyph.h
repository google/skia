/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyph_DEFINED
#define SkGlyph_DEFINED

#include "SkArenaAlloc.h"
#include "SkChecksum.h"
#include "SkFixed.h"
#include "SkMask.h"
#include "SkTypes.h"


class SkPath;
class SkGlyphCache;

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

    uint32_t code() const {
        return fID & kCodeMask;
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

// FIXME - This is needed because the Android framework directly accesses fID.
// Remove when fID accesses are cleaned up.
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    operator uint32_t() const { return fID; }
#endif

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
    SkPackedGlyphID() : SkPackedID() { }
    SkGlyphID code() const {
        return SkTo<SkGlyphID>(SkPackedID::code());
    }
};

struct SkPackedUnicharID : public SkPackedID {
    SkPackedUnicharID(SkUnichar code) : SkPackedID(code) { }
    SkPackedUnicharID(SkUnichar code, SkFixed x, SkFixed y) : SkPackedID(code, x, y) { }
    SkPackedUnicharID() : SkPackedID() { }
    SkUnichar code() const {
        return SkTo<SkUnichar>(SkPackedID::code());
    }
};

SK_BEGIN_REQUIRE_DENSE
class SkGlyph {
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
        Intercept* fIntercept;
        SkPath*    fPath;
    };

public:
    static const SkFixed kSubpixelRound = SK_FixedHalf >> SkPackedID::kSubBits;
    void*       fImage;
    PathData*   fPathData;
    float       fAdvanceX, fAdvanceY;

    uint16_t    fWidth, fHeight;
    int16_t     fTop, fLeft;

    uint8_t     fMaskFormat;
    int8_t      fRsbDelta, fLsbDelta;  // used by auto-kerning
    int8_t      fForceBW;

    void initWithGlyphID(SkPackedGlyphID glyph_id) {
        fID             = glyph_id;
        fImage          = nullptr;
        fPathData       = nullptr;
        fMaskFormat     = MASK_FORMAT_UNKNOWN;
        fForceBW        = 0;
    }

    static size_t BitsToBytes(size_t bits) {
        return (bits + 7) >> 3;
    }

    /**
     *  Compute the rowbytes for the specified width and mask-format.
     */
    static unsigned ComputeRowBytes(unsigned width, SkMask::Format format) {
        unsigned rb = width;
        if (SkMask::kBW_Format == format) {
            rb = BitsToBytes(rb);
        } else if (SkMask::kARGB32_Format == format) {
            rb <<= 2;
        } else if (SkMask::kLCD16_Format == format) {
            rb = SkAlign4(rb << 1);
        } else {
            rb = SkAlign4(rb);
        }
        return rb;
    }

    size_t allocImage(SkArenaAlloc* alloc) {
        size_t allocSize;
        if (SkMask::kBW_Format == fMaskFormat) {
            allocSize = BitsToBytes(fWidth) * fHeight;
            fImage = alloc->makeArrayDefault<char>(allocSize);
        } else if (SkMask::kARGB32_Format == fMaskFormat) {
            allocSize = fWidth * fHeight;
            fImage = alloc->makeArrayDefault<uint32_t>(fWidth * fHeight);
            allocSize *= sizeof(uint32_t);
        } else if (SkMask::kLCD16_Format == fMaskFormat) {
            allocSize = SkAlign2(fWidth) * fHeight;
            fImage = alloc->makeArrayDefault<uint16_t>(allocSize);
            allocSize *= sizeof(uint16_t);
        } else {
            allocSize = SkAlign4(fWidth) * fHeight;
            fImage = alloc->makeArrayDefault<char>(allocSize);
        }
        return allocSize;
    }

    unsigned rowBytes() const {
        return ComputeRowBytes(fWidth, (SkMask::Format)fMaskFormat);
    }

    bool isJustAdvance() const {
        return MASK_FORMAT_JUST_ADVANCE == fMaskFormat;
    }

    bool isFullMetrics() const {
        return MASK_FORMAT_JUST_ADVANCE != fMaskFormat;
    }

    SkGlyphID getGlyphID() const {
        return fID.code();
    }

    SkPackedGlyphID getPackedID() const {
        return fID;
    }

    SkFixed getSubXFixed() const {
        return fID.getSubXFixed();
    }

    SkFixed getSubYFixed() const {
        return fID.getSubYFixed();
    }

    size_t computeImageSize() const;

    /** Call this to set all of the metrics fields to 0 (e.g. if the scaler
        encounters an error measuring a glyph). Note: this does not alter the
        fImage, fPath, fID, fMaskFormat fields.
     */
    void zeroMetrics();

    void toMask(SkMask* mask) const;

    class HashTraits {
    public:
        static SkPackedGlyphID GetKey(const SkGlyph& glyph) {
            return glyph.fID;
        }
        static uint32_t Hash(SkPackedGlyphID glyphId) {
            return glyphId.hash();
        }
    };

 private:
    // TODO(herb) remove friend statement after SkGlyphCache cleanup.
    friend class SkGlyphCache;

// FIXME - This is needed because the Android frame work directly accesses fID.
// Remove when fID accesses are cleaned up.
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
  public:
#endif
    SkPackedGlyphID fID;
};
SK_END_REQUIRE_DENSE

#endif

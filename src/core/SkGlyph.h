/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyph_DEFINED
#define SkGlyph_DEFINED

#include "SkTypes.h"
#include "SkFixed.h"
#include "SkMask.h"

class SkPath;

// needs to be != to any valid SkMask::Format
#define MASK_FORMAT_UNKNOWN         (0xFF)
#define MASK_FORMAT_JUST_ADVANCE    MASK_FORMAT_UNKNOWN

#define kMaxGlyphWidth (1<<13)

struct SkGlyph {
    void*       fImage;
    SkPath*     fPath;
    SkFixed     fAdvanceX, fAdvanceY;

    uint32_t    fID;
    uint16_t    fWidth, fHeight;
    int16_t     fTop, fLeft;

    void*       fDistanceField;
    uint8_t     fMaskFormat;
    int8_t      fRsbDelta, fLsbDelta;  // used by auto-kerning

    void init(uint32_t id) {
        fID             = id;
        fImage          = NULL;
        fPath           = NULL;
        fDistanceField  = NULL;
        fMaskFormat     = MASK_FORMAT_UNKNOWN;
    }

    /**
     *  Compute the rowbytes for the specified width and mask-format.
     */
    static unsigned ComputeRowBytes(unsigned width, SkMask::Format format) {
        unsigned rb = width;
        if (SkMask::kBW_Format == format) {
            rb = (rb + 7) >> 3;
        } else if (SkMask::kARGB32_Format == format ||
                   SkMask::kLCD32_Format == format)
        {
            rb <<= 2;
        } else if (SkMask::kLCD16_Format == format) {
            rb = SkAlign4(rb << 1);
        } else {
            rb = SkAlign4(rb);
        }
        return rb;
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

    uint16_t getGlyphID() const {
        return ID2Code(fID);
    }

    unsigned getGlyphID(unsigned baseGlyphCount) const {
        unsigned code = ID2Code(fID);
        SkASSERT(code >= baseGlyphCount);
        return code - baseGlyphCount;
    }

    unsigned getSubX() const {
        return ID2SubX(fID);
    }

    SkFixed getSubXFixed() const {
        return SubToFixed(ID2SubX(fID));
    }

    SkFixed getSubYFixed() const {
        return SubToFixed(ID2SubY(fID));
    }

    size_t computeImageSize() const;

    /** Call this to set all of the metrics fields to 0 (e.g. if the scaler
        encounters an error measuring a glyph). Note: this does not alter the
        fImage, fPath, fID, fMaskFormat fields.
     */
    void zeroMetrics();

    enum {
        kSubBits = 2,
        kSubMask = ((1 << kSubBits) - 1),
        kSubShift = 24, // must be large enough for glyphs and unichars
        kCodeMask = ((1 << kSubShift) - 1),
        // relative offsets for X and Y subpixel bits
        kSubShiftX = kSubBits,
        kSubShiftY = 0
    };

    static unsigned ID2Code(uint32_t id) {
        return id & kCodeMask;
    }

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

    static uint32_t MakeID(unsigned code) {
        return code;
    }

    static uint32_t MakeID(unsigned code, SkFixed x, SkFixed y) {
        SkASSERT(code <= kCodeMask);
        x = FixedToSub(x);
        y = FixedToSub(y);
        return (x << (kSubShift + kSubShiftX)) |
               (y << (kSubShift + kSubShiftY)) |
               code;
    }

    void toMask(SkMask* mask) const;
};

#endif

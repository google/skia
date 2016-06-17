/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureFlat_DEFINED
#define SkPictureFlat_DEFINED


#include "SkChecksum.h"
#include "SkChunkAlloc.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPtrRecorder.h"
#include "SkTDynamicHash.h"

enum DrawType {
    UNUSED,
    CLIP_PATH,
    CLIP_REGION,
    CLIP_RECT,
    CLIP_RRECT,
    CONCAT,
    DRAW_BITMAP,
    DRAW_BITMAP_MATRIX, // deprecated, M41 was last Chromium version to write this to an .skp
    DRAW_BITMAP_NINE,
    DRAW_BITMAP_RECT,
    DRAW_CLEAR,
    DRAW_DATA,
    DRAW_OVAL,
    DRAW_PAINT,
    DRAW_PATH,
    DRAW_PICTURE,
    DRAW_POINTS,
    DRAW_POS_TEXT,
    DRAW_POS_TEXT_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT
    DRAW_POS_TEXT_H,
    DRAW_POS_TEXT_H_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT_H
    DRAW_RECT,
    DRAW_RRECT,
    DRAW_SPRITE,
    DRAW_TEXT,
    DRAW_TEXT_ON_PATH,
    DRAW_TEXT_TOP_BOTTOM,   // fast variant of DRAW_TEXT
    DRAW_VERTICES,
    RESTORE,
    ROTATE,
    SAVE,
    SAVE_LAYER_SAVEFLAGS_DEPRECATED,
    SCALE,
    SET_MATRIX,
    SKEW,
    TRANSLATE,
    NOOP,
    BEGIN_COMMENT_GROUP, // deprecated (M44)
    COMMENT,             // deprecated (M44)
    END_COMMENT_GROUP,   // deprecated (M44)

    // new ops -- feel free to re-alphabetize on next version bump
    DRAW_DRRECT,
    PUSH_CULL,  // deprecated, M41 was last Chromium version to write this to an .skp
    POP_CULL,   // deprecated, M41 was last Chromium version to write this to an .skp

    DRAW_PATCH, // could not add in aphabetical order
    DRAW_PICTURE_MATRIX_PAINT,
    DRAW_TEXT_BLOB,
    DRAW_IMAGE,
    DRAW_IMAGE_RECT_STRICT, // deprecated (M45)
    DRAW_ATLAS,
    DRAW_IMAGE_NINE,
    DRAW_IMAGE_RECT,

    SAVE_LAYER_SAVELAYERFLAGS_DEPRECATED_JAN_2016,
    SAVE_LAYER_SAVELAYERREC,

    DRAW_ANNOTATION,
    DRAW_DRAWABLE,
    DRAW_DRAWABLE_MATRIX,

    LAST_DRAWTYPE_ENUM = DRAW_DRAWABLE_MATRIX,
};

// In the 'match' method, this constant will match any flavor of DRAW_BITMAP*
static const int kDRAW_BITMAP_FLAVOR = LAST_DRAWTYPE_ENUM+1;

enum DrawVertexFlags {
    DRAW_VERTICES_HAS_TEXS    = 0x01,
    DRAW_VERTICES_HAS_COLORS  = 0x02,
    DRAW_VERTICES_HAS_INDICES = 0x04,
    DRAW_VERTICES_HAS_XFER    = 0x08,
};

enum DrawAtlasFlags {
    DRAW_ATLAS_HAS_COLORS   = 1 << 0,
    DRAW_ATLAS_HAS_CULL     = 1 << 1,
};

enum SaveLayerRecFlatFlags {
    SAVELAYERREC_HAS_BOUNDS     = 1 << 0,
    SAVELAYERREC_HAS_PAINT      = 1 << 1,
    SAVELAYERREC_HAS_BACKDROP   = 1 << 2,
    SAVELAYERREC_HAS_FLAGS      = 1 << 3,
};

///////////////////////////////////////////////////////////////////////////////
// clipparams are packed in 5 bits
//  doAA:1 | regionOp:4

static inline uint32_t ClipParams_pack(SkRegion::Op op, bool doAA) {
    unsigned doAABit = doAA ? 1 : 0;
    return (doAABit << 4) | op;
}

static inline SkRegion::Op ClipParams_unpackRegionOp(uint32_t packed) {
    return (SkRegion::Op)(packed & 0xF);
}

static inline bool ClipParams_unpackDoAA(uint32_t packed) {
    return SkToBool((packed >> 4) & 1);
}

///////////////////////////////////////////////////////////////////////////////

class SkTypefacePlayback {
public:
    SkTypefacePlayback();
    virtual ~SkTypefacePlayback();

    int count() const { return fCount; }

    void reset(const SkRefCntSet*);

    void setCount(int count);
    SkRefCnt* set(int index, SkRefCnt*);

    void setupBuffer(SkReadBuffer& buffer) const {
        buffer.setTypefaceArray((SkTypeface**)fArray, fCount);
    }

protected:
    int fCount;
    SkRefCnt** fArray;
};

class SkFactoryPlayback {
public:
    SkFactoryPlayback(int count) : fCount(count) { fArray = new SkFlattenable::Factory[count]; }

    ~SkFactoryPlayback() { delete[] fArray; }

    SkFlattenable::Factory* base() const { return fArray; }

    void setupBuffer(SkReadBuffer& buffer) const {
        buffer.setFactoryPlayback(fArray, fCount);
    }

private:
    int fCount;
    SkFlattenable::Factory* fArray;
};

#endif

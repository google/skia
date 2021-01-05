/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureFlat_DEFINED
#define SkPictureFlat_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/private/SkChecksum.h"
#include "src/core/SkPtrRecorder.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTDynamicHash.h"
#include "src/core/SkWriteBuffer.h"

/*
 * Note: While adding new DrawTypes, it is necessary to add to the end of this list
 *       and update LAST_DRAWTYPE_ENUM to avoid having the code read older skps wrong.
 *       (which can cause segfaults)
 *
 *       Reordering can be done during version updates.
 */
enum DrawType {
    UNUSED,
    CLIP_PATH,
    CLIP_REGION,
    CLIP_RECT,
    CLIP_RRECT,
    CONCAT,
    DRAW_BITMAP_RETIRED_2016_REMOVED_2018,
    DRAW_BITMAP_MATRIX_RETIRED_2016_REMOVED_2018,
    DRAW_BITMAP_NINE_RETIRED_2016_REMOVED_2018,
    DRAW_BITMAP_RECT_RETIRED_2016_REMOVED_2018,
    DRAW_CLEAR,
    DRAW_DATA,
    DRAW_OVAL,
    DRAW_PAINT,
    DRAW_PATH,
    DRAW_PICTURE,
    DRAW_POINTS,
    DRAW_POS_TEXT_REMOVED_1_2019,
    DRAW_POS_TEXT_TOP_BOTTOM_REMOVED_1_2019,
    DRAW_POS_TEXT_H_REMOVED_1_2019,
    DRAW_POS_TEXT_H_TOP_BOTTOM_REMOVED_1_2019,
    DRAW_RECT,
    DRAW_RRECT,
    DRAW_SPRITE_RETIRED_2015_REMOVED_2018,
    DRAW_TEXT_REMOVED_1_2019,
    DRAW_TEXT_ON_PATH_RETIRED_08_2018_REMOVED_10_2018,
    DRAW_TEXT_TOP_BOTTOM_REMOVED_1_2019,
    DRAW_VERTICES_RETIRED_03_2017_REMOVED_01_2018,
    RESTORE,
    ROTATE,
    SAVE,
    SAVE_LAYER_SAVEFLAGS_DEPRECATED_2015_REMOVED_12_2020,
    SCALE,
    SET_MATRIX,
    SKEW,
    TRANSLATE,
    NOOP,
    BEGIN_COMMENT_GROUP_obsolete,
    COMMENT_obsolete,
    END_COMMENT_GROUP_obsolete,

    // new ops -- feel free to re-alphabetize on next version bump
    DRAW_DRRECT,
    PUSH_CULL,  // deprecated, M41 was last Chromium version to write this to an .skp
    POP_CULL,   // deprecated, M41 was last Chromium version to write this to an .skp

    DRAW_PATCH, // could not add in aphabetical order
    DRAW_PICTURE_MATRIX_PAINT,
    DRAW_TEXT_BLOB,
    DRAW_IMAGE,
    DRAW_IMAGE_RECT_STRICT_obsolete,
    DRAW_ATLAS,
    DRAW_IMAGE_NINE,
    DRAW_IMAGE_RECT,

    SAVE_LAYER_SAVELAYERFLAGS_DEPRECATED_JAN_2016_REMOVED_01_2018,
    SAVE_LAYER_SAVELAYERREC,

    DRAW_ANNOTATION,
    DRAW_DRAWABLE,
    DRAW_DRAWABLE_MATRIX,
    DRAW_TEXT_RSXFORM_DEPRECATED_DEC_2018,

    TRANSLATE_Z, // deprecated (M60)

    DRAW_SHADOW_REC,
    DRAW_IMAGE_LATTICE,
    DRAW_ARC,
    DRAW_REGION,
    DRAW_VERTICES_OBJECT,

    FLUSH,

    DRAW_EDGEAA_IMAGE_SET,

    SAVE_BEHIND,

    DRAW_EDGEAA_QUAD,

    DRAW_BEHIND_PAINT,
    CONCAT44,
    CLIP_SHADER_IN_PAINT,
    MARK_CTM,
    SET_M44,

    DRAW_IMAGE2,
    DRAW_IMAGE_RECT2,
    DRAW_IMAGE_LATTICE2,
    DRAW_EDGEAA_IMAGE_SET2,

    LAST_DRAWTYPE_ENUM = DRAW_EDGEAA_IMAGE_SET2,
};

enum DrawVertexFlags {
    DRAW_VERTICES_HAS_TEXS    = 0x01,
    DRAW_VERTICES_HAS_COLORS  = 0x02,
    DRAW_VERTICES_HAS_INDICES = 0x04,
    DRAW_VERTICES_HAS_XFER    = 0x08,
};

enum DrawAtlasFlags {
    DRAW_ATLAS_HAS_COLORS     = 1 << 0,
    DRAW_ATLAS_HAS_CULL       = 1 << 1,
    DRAW_ATLAS_HAS_SAMPLING   = 1 << 2,
};

enum DrawTextRSXformFlags {
    DRAW_TEXT_RSXFORM_HAS_CULL  = 1 << 0,
};

enum SaveLayerRecFlatFlags {
    SAVELAYERREC_HAS_BOUNDS     = 1 << 0,
    SAVELAYERREC_HAS_PAINT      = 1 << 1,
    SAVELAYERREC_HAS_BACKDROP   = 1 << 2,
    SAVELAYERREC_HAS_FLAGS      = 1 << 3,
    SAVELAYERREC_HAS_CLIPMASK_OBSOLETE   = 1 << 4,  // 6/13/2020
    SAVELAYERREC_HAS_CLIPMATRIX_OBSOLETE = 1 << 5,  // 6/13/2020
};

enum SaveBehindFlatFlags {
    SAVEBEHIND_HAS_SUBSET = 1 << 0,
};

///////////////////////////////////////////////////////////////////////////////
// clipparams are packed in 5 bits
//  doAA:1 | clipOp:4

static inline uint32_t ClipParams_pack(SkClipOp op, bool doAA) {
    unsigned doAABit = doAA ? 1 : 0;
    return (doAABit << 4) | static_cast<int>(op);
}

template <typename T> T asValidEnum(SkReadBuffer* buffer, uint32_t candidate) {

    if (buffer->validate(candidate <= static_cast<uint32_t>(T::kMax_EnumValue))) {
        return static_cast<T>(candidate);
    }

    return T::kMax_EnumValue;
}

static inline SkClipOp ClipParams_unpackRegionOp(SkReadBuffer* buffer, uint32_t packed) {
    return asValidEnum<SkClipOp>(buffer, packed & 0xF);
}

static inline bool ClipParams_unpackDoAA(uint32_t packed) {
    return SkToBool((packed >> 4) & 1);
}

///////////////////////////////////////////////////////////////////////////////

class SkTypefacePlayback {
public:
    SkTypefacePlayback() : fCount(0), fArray(nullptr) {}
    ~SkTypefacePlayback() = default;

    void setCount(size_t count);

    size_t count() const { return fCount; }

    sk_sp<SkTypeface>& operator[](size_t index) {
        SkASSERT(index < fCount);
        return fArray[index];
    }

    void setupBuffer(SkReadBuffer& buffer) const {
        buffer.setTypefaceArray(fArray.get(), fCount);
    }

protected:
    size_t fCount;
    std::unique_ptr<sk_sp<SkTypeface>[]> fArray;
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

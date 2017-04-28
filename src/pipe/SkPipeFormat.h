/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPipeFormat_DEFINED
#define SkPipeFormat_DEFINED

#include "SkTypes.h"

#define kDefinePicture_ExtPipeVerb  SkSetFourByteTag('s', 'k', 'p', 'i')

enum class SkPipeVerb : uint8_t {
    kSave,              // extra == 0
    kSaveLayer,
    kRestore,           // extra == 0
    kConcat,            // extra == SkMatrix::MaskType
    
    kClipRect,          // extra == (SkRegion::Op << 1) | isAntiAlias:1
    kClipRRect,         // extra == (SkRegion::Op << 1) | isAntiAlias:1
    kClipPath,          // extra == (SkRegion::Op << 1) | isAntiAlias:1
    kClipRegion,        // extra == (SkRegion::Op << 1)

    kDrawArc,           // extra == useCenter
    kDrawAtlas,         // extra == has_colors | has_cull | has_paint | mode
    kDrawDRRect,
    kDrawText,          // extra == byteLength:24 else next 32
    kDrawPosText,       // extra == byteLength:24 else next 32
    kDrawPosTextH,      // extra == byteLength:24 else next 32
    kDrawRegion,        // extra == size:24 of region, or 0 means next 32
    kDrawTextOnPath,
    kDrawTextBlob,
    kDrawTextRSXform,   // extra == (byteLength:23 << 1) else next 32 | has_cull_rect:1
    kDrawPatch,
    kDrawPaint,         // extra == 0
    kDrawPoints,        // extra == PointMode
    kDrawRect,          // extra == 0
    kDrawPath,          // extra == 0
    kDrawOval,          // extra == 0
    kDrawRRect,         // extra == 0
    
    kDrawImage,         // extra == has_paint:1
    kDrawImageRect,     // extra == constraint | has_src_rect | has_paint
    kDrawImageNine,     // extra == has_paint:1
    kDrawImageLattice,  // extra == x_count:8 | y_count:8 | has_paint:1
    
    kDrawVertices,
    
    kDrawPicture,       // extra == picture_index
    kDrawAnnotation,    // extra == (key_len_plus_1:23 << 1) else next 32 | has_data:1

    kDefineImage,       // extra == image_index
    kDefineTypeface,
    kDefineFactory,     // extra == factory_index (followed by padded getTypeName string)
    kDefinePicture,     // extra == 0 or forget_index + 1 (0 means we're defining a new picture)
    kEndPicture,        // extra == picture_index
    kWriteImage,        // extra == image_index
    kWritePicture,      // extra == picture_index
};

enum PaintUsage {
    kText_PaintUsage        = 1 << 0,
    kTextBlob_PaintUsage    = 1 << 1,
    kGeometry_PaintUsage    = 1 << 2,
    kImage_PaintUsage       = 1 << 3,
    kSaveLayer_PaintUsage   = 1 << 4,
    kDrawPaint_PaintUsage   = 1 << 5,
    kVertices_PaintUsage    = 1 << 6,
    kRespectsStroke_PaintUsage  = 1 << 7,
    kUnknown_PaintUsage     = 0xFF,
};

// must sum to <= 32
enum BitsPerField {
    kFlags_BPF      = 16,
    kFilter_BPF     = 2,
    kStyle_BPF      = 2,
    kCaps_BPF       = 2,
    kJoins_BPF      = 2,
    kHint_BPF       = 2,
    kAlign_BPF      = 2,
    kEncoding_BPF   = 2,
};

enum {
    kTextSize_NonDef    = 1 << 0,
    kTextScaleX_NonDef  = 1 << 1,
    kTextSkewX_NonDef   = 1 << 2,
    kStrokeWidth_NonDef = 1 << 3,
    kStrokeMiter_NonDef = 1 << 4,
    kColor_NonDef       = 1 << 5,
    kTypeface_NonDef    = 1 << 6,
    kPathEffect_NonDef  = 1 << 7,
    kShader_NonDef      = 1 << 8,
    kMaskFilter_NonDef  = 1 << 9,
    kColorFilter_NonDef = 1 << 10,
    kRasterizer_NonDef  = 1 << 11,
    kImageFilter_NonDef = 1 << 12,
    kDrawLooper_NonDef  = 1 << 13,
};

enum {
    kFlags_SaveLayerMask            = 0xFF,
    kHasBounds_SaveLayerMask        = 1 << 8,
    kHasPaint_SaveLayerMask         = 1 << 9,
    kHasBackdrop_SaveLayerMask      = 1 << 10,
    kDontClipToLayer_SaveLayerMask  = 1 << 11,
    kHasClipMask_SaveLayerMask      = 1 << 12,
    kHasClipMatrix_SaveLayerMask    = 1 << 13,
};

enum {
    kObjectDefinitionBits           = 20,
    kIndex_ObjectDefinitionMask     = ((1 << kObjectDefinitionBits) - 1),
    kUser_ObjectDefinitionMask      = 0x7 << kObjectDefinitionBits,
    kUndef_ObjectDefinitionMask     = 1 << 23,
    // (Undef:1 | User:3 | Index:20) must fit in extra:24
};

enum {
    kTypeMask_ConcatMask    = 0xF,
    kSetMatrix_ConcatMask   = 1 << 4,
};

enum {
    kMode_DrawAtlasMask         = 0xFF,
    kHasColors_DrawAtlasMask    = 1 << 8,
    kHasCull_DrawAtlasMask      = 1 << 9,
    kHasPaint_DrawAtlasMask     = 1 << 10,
};

enum {
    kHasPaint_DrawImageMask     = 1 << 0,
};

enum {
    kConstraint_DrawImageRectMask   = 1 << 0,
    kHasPaint_DrawImageRectMask     = 1 << 1,
    kHasSrcRect_DrawImageRectMask   = 1 << 2,
};

enum {
    kHasPaint_DrawImageNineMask     = 1 << 0,
};

enum {
    // picture_index takes the first kObjectDefinitionBits bits
    kHasMatrix_DrawPictureExtra = 1 << 21,
    kHasPaint_DrawPictureExtra  = 1 << 22,
};

enum {
    kIndex_DefineFactoryExtraBits       = 10,
    kNameLength_DefineFactoryExtraBits  = 14,   // includes trailing 0
    kNameLength_DefineFactoryExtraMask  = (1 << kNameLength_DefineFactoryExtraBits) - 1,
};

enum {
    kModeEnum_DrawPatchExtraMask        = 0xFF,
    kExplicitXfer_DrawPatchExtraValue   = 0xFF,
    kHasColors_DrawPatchExtraMask       = 0x100,
    kHasTexture_DrawPatchExtraMask      = 0x200,
};

enum {
    // if we store a zero for VCount, then read an int after the packedverb for the vcount
    kVCount_DrawVerticesMask            = (1 << 11) - 1,

    kVMode_DrawVerticesShift            = 11,
    kVMode_DrawVerticesMask             = 3 << kVMode_DrawVerticesShift,

    kXMode_DrawVerticesShift            = 13,
    kXMode_DrawVerticesMask             = 0xFF << kXMode_DrawVerticesShift,

    kHasTex_DrawVerticesMask            = 1 << 21,
    kHasColors_DrawVerticesMask         = 1 << 22,
    kHasIndices_DrawVerticesMask        = 1 << 23,
};

enum {
    kTextLength_DrawTextOnPathMask      = (1 << 16) - 1,
    kMatrixType_DrawTextOnPathShift     = 16,
    kMatrixType_DrawTextOnPathMask      = 0xF << kMatrixType_DrawTextOnPathShift,
};

enum {
    kHasPaint_DrawImageLatticeMask  = 1 << 0,
    kHasFlags_DrawImageLatticeMask  = 1 << 1,
    kXCount_DrawImageLatticeShift   = 2,        // bits  2:9  are xcount or FF means 32bits follow
    kYCount_DrawImageLatticeShift   = 10,       // bits 10:17 are ycount or FF means 32bits follow
    kCount_DrawImageLatticeMask     = 0xFF,     // sentinel for 32bits follow,
                                                // thus max inline count is 254
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool fits_in(int value, int bits) {
    return value >= 0 && value < (1 << bits);
}

static inline void ASSERT_FITS_IN(int value, int bits) {
    SkASSERT(fits_in(value, bits));
}

static inline uint32_t pack_verb(SkPipeVerb verb, unsigned extra = 0) {
    //SkDebugf("pack [%d] %d\n", verb, extra);
    ASSERT_FITS_IN((unsigned)verb, 8);
    ASSERT_FITS_IN(extra, 24);
    return ((uint32_t)verb << 24) | extra;
}

static inline SkPipeVerb unpack_verb(uint32_t data) {
    return (SkPipeVerb)(data >> 24);
}

static inline unsigned unpack_verb_extra(uint32_t data) {
    return data & 0xFFFFFF;
}

#endif


/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkGPipePriv_DEFINED
#define SkGPipePriv_DEFINED

#include "SkTypes.h"

#define UNIMPLEMENTED

// these must be contiguous, 0...N-1
enum PaintFlats {
    kColorFilter_PaintFlat,
    kDrawLooper_PaintFlat,
    kImageFilter_PaintFlat,
    kMaskFilter_PaintFlat,
    kPathEffect_PaintFlat,
    kRasterizer_PaintFlat,
    kShader_PaintFlat,
    kXfermode_PaintFlat,

    kLast_PaintFlat = kXfermode_PaintFlat
};
#define kCount_PaintFlats   (kLast_PaintFlat + 1)

enum DrawOps {
    kSkip_DrawOp,   // skip an addition N bytes (N == data)

    // these match Canvas apis
    kClipPath_DrawOp,
    kClipRegion_DrawOp,
    kClipRect_DrawOp,
    kClipRRect_DrawOp,
    kConcat_DrawOp,
    kDrawBitmap_DrawOp,
    kDrawBitmapMatrix_DrawOp,
    kDrawBitmapNine_DrawOp,
    kDrawBitmapRectToRect_DrawOp,
    kDrawClear_DrawOp,
    kDrawData_DrawOp,
    kDrawOval_DrawOp,
    kDrawPaint_DrawOp,
    kDrawPath_DrawOp,
    kDrawPicture_DrawOp,
    kDrawPoints_DrawOp,
    kDrawPosText_DrawOp,
    kDrawPosTextH_DrawOp,
    kDrawRect_DrawOp,
    kDrawRRect_DrawOp,
    kDrawSprite_DrawOp,
    kDrawText_DrawOp,
    kDrawTextOnPath_DrawOp,
    kDrawVertices_DrawOp,
    kRestore_DrawOp,
    kRotate_DrawOp,
    kSave_DrawOp,
    kSaveLayer_DrawOp,
    kScale_DrawOp,
    kSetMatrix_DrawOp,
    kSkew_DrawOp,
    kTranslate_DrawOp,

    kPaintOp_DrawOp,
    kSetTypeface_DrawOp,
    kSetAnnotation_DrawOp,

    kDef_Typeface_DrawOp,
    kDef_Flattenable_DrawOp,
    kDef_Bitmap_DrawOp,
    kDef_Factory_DrawOp,

    // these are signals to playback, not drawing verbs
    kReportFlags_DrawOp,
    kShareBitmapHeap_DrawOp,
    kDone_DrawOp,
};

/**
 *  DrawOp packs into a 32bit int as follows
 *
 *  DrawOp:8 - Flags:4 - Data:20
 *
 *  Flags and Data are called out separately, so we can reuse Data between
 *  different Ops that might have different Flags. e.g. Data might be a Paint
 *  index for both drawRect (no flags) and saveLayer (does have flags).
 *
 *  All Ops that take a SkPaint use their Data field to store the index to
 *  the paint (previously defined with kPaintOp_DrawOp).
 */

#define DRAWOPS_OP_BITS     8
#define DRAWOPS_FLAG_BITS   4
#define DRAWOPS_DATA_BITS   20

#define DRAWOPS_OP_MASK     ((1 << DRAWOPS_OP_BITS) - 1)
#define DRAWOPS_FLAG_MASK   ((1 << DRAWOPS_FLAG_BITS) - 1)
#define DRAWOPS_DATA_MASK   ((1 << DRAWOPS_DATA_BITS) - 1)

static inline unsigned DrawOp_unpackOp(uint32_t op32) {
    return (op32 >> (DRAWOPS_FLAG_BITS + DRAWOPS_DATA_BITS));
}

static inline unsigned DrawOp_unpackFlags(uint32_t op32) {
    return (op32 >> DRAWOPS_DATA_BITS) & DRAWOPS_FLAG_MASK;
}

static inline unsigned DrawOp_unpackData(uint32_t op32) {
    return op32 & DRAWOPS_DATA_MASK;
}

static inline uint32_t DrawOp_packOpFlagData(DrawOps op, unsigned flags, unsigned data) {
    SkASSERT(0 == (op & ~DRAWOPS_OP_MASK));
    SkASSERT(0 == (flags & ~DRAWOPS_FLAG_MASK));
    SkASSERT(0 == (data & ~DRAWOPS_DATA_MASK));

    return (op << (DRAWOPS_FLAG_BITS + DRAWOPS_DATA_BITS)) |
           (flags << DRAWOPS_DATA_BITS) |
            data;
}

/** DrawOp specific flag bits
 */

enum {
    kSaveLayer_HasBounds_DrawOpFlag = 1 << 0,
    kSaveLayer_HasPaint_DrawOpFlag = 1 << 1,
};
enum {
    kClear_HasColor_DrawOpFlag  = 1 << 0
};
enum {
    kDrawTextOnPath_HasMatrix_DrawOpFlag = 1 << 0
};
enum {
    kDrawVertices_HasTexs_DrawOpFlag     = 1 << 0,
    kDrawVertices_HasColors_DrawOpFlag   = 1 << 1,
    kDrawVertices_HasIndices_DrawOpFlag  = 1 << 2,
};
enum {
    kDrawBitmap_HasPaint_DrawOpFlag   = 1 << 0,
    // Specific to drawBitmapRect, but needs to be different from HasPaint,
    // which is used for all drawBitmap calls, so include it here.
    kDrawBitmap_HasSrcRect_DrawOpFlag = 1 << 1,
    // SkCanvas::DrawBitmapRectFlags::kBleed_DrawBitmapRectFlag is
    // converted into and out of this flag to save space
    kDrawBitmap_Bleed_DrawOpFlag      = 1 << 2,
};
enum {
    kClip_HasAntiAlias_DrawOpFlag = 1 << 0,
};
///////////////////////////////////////////////////////////////////////////////

class BitmapInfo : SkNoncopyable {
public:
    BitmapInfo(SkBitmap* bitmap, uint32_t genID, int toBeDrawnCount)
        : fBitmap(bitmap)
        , fGenID(genID)
        , fBytesAllocated(0)
        , fMoreRecentlyUsed(NULL)
        , fLessRecentlyUsed(NULL)
        , fToBeDrawnCount(toBeDrawnCount)
    {}

    ~BitmapInfo() {
        SkASSERT(0 == fToBeDrawnCount);
        SkDELETE(fBitmap);
    }

    void addDraws(int drawsToAdd) {
        if (0 == fToBeDrawnCount) {
            // The readers will only ever decrement the count, so once the
            // count is zero, the writer will be the only one modifying it,
            // so it does not need to be an atomic operation.
            fToBeDrawnCount = drawsToAdd;
        } else {
            sk_atomic_add(&fToBeDrawnCount, drawsToAdd);
        }
    }

    void decDraws() {
        sk_atomic_dec(&fToBeDrawnCount);
    }

    int drawCount() const {
        return fToBeDrawnCount;
    }

    SkBitmap* fBitmap;
    // Store the generation ID of the original bitmap, since copying does
    // not copy this field, so fBitmap's generation ID will not be useful
    // for comparing.
    // FIXME: Is it reasonable to make copying a bitmap/pixelref copy the
    // generation ID?
    uint32_t fGenID;
    // Keep track of the bytes allocated for this bitmap. When replacing the
    // bitmap or removing this BitmapInfo we know how much memory has been
    // reclaimed.
    size_t fBytesAllocated;
    // TODO: Generalize the LRU caching mechanism
    BitmapInfo* fMoreRecentlyUsed;
    BitmapInfo* fLessRecentlyUsed;
private:
    int      fToBeDrawnCount;
};

static inline bool shouldFlattenBitmaps(uint32_t flags) {
    return SkToBool(flags & SkGPipeWriter::kCrossProcess_Flag
            && !(flags & SkGPipeWriter::kSharedAddressSpace_Flag));
}

///////////////////////////////////////////////////////////////////////////////

enum PaintOps {
    kReset_PaintOp,     // no arg

    kFlags_PaintOp,     // arg inline
    kColor_PaintOp,     // arg 32
    kStyle_PaintOp,     // arg inline
    kJoin_PaintOp,      // arg inline
    kCap_PaintOp,       // arg inline
    kWidth_PaintOp,     // arg scalar
    kMiter_PaintOp,     // arg scalar

    kEncoding_PaintOp,  // arg inline - text
    kHinting_PaintOp,   // arg inline - text
    kAlign_PaintOp,     // arg inline - text
    kTextSize_PaintOp,  // arg scalar - text
    kTextScaleX_PaintOp,// arg scalar - text
    kTextSkewX_PaintOp, // arg scalar - text
    kTypeface_PaintOp,  // arg inline (index) - text

    kFlatIndex_PaintOp, // flags=paintflat, data=index
};

#define PAINTOPS_OP_BITS     8
#define PAINTOPS_FLAG_BITS   4
#define PAINTOPS_DATA_BITS   20

#define PAINTOPS_OP_MASK     ((1 << PAINTOPS_OP_BITS) - 1)
#define PAINTOPS_FLAG_MASK   ((1 << PAINTOPS_FLAG_BITS) - 1)
#define PAINTOPS_DATA_MASK   ((1 << PAINTOPS_DATA_BITS) - 1)

static inline unsigned PaintOp_unpackOp(uint32_t op32) {
    return (op32 >> (PAINTOPS_FLAG_BITS + PAINTOPS_DATA_BITS));
}

static inline unsigned PaintOp_unpackFlags(uint32_t op32) {
    return (op32 >> PAINTOPS_DATA_BITS) & PAINTOPS_FLAG_MASK;
}

static inline unsigned PaintOp_unpackData(uint32_t op32) {
    return op32 & PAINTOPS_DATA_MASK;
}

static inline uint32_t PaintOp_packOp(PaintOps op) {
    SkASSERT(0 == (op & ~PAINTOPS_OP_MASK));

    return op << (PAINTOPS_FLAG_BITS + PAINTOPS_DATA_BITS);
}

static inline uint32_t PaintOp_packOpData(PaintOps op, unsigned data) {
    SkASSERT(0 == (op & ~PAINTOPS_OP_MASK));
    SkASSERT(0 == (data & ~PAINTOPS_DATA_MASK));

    return (op << (PAINTOPS_FLAG_BITS + PAINTOPS_DATA_BITS)) | data;
}

static inline uint32_t PaintOp_packOpFlagData(PaintOps op, unsigned flags, unsigned data) {
    SkASSERT(0 == (op & ~PAINTOPS_OP_MASK));
    SkASSERT(0 == (flags & ~PAINTOPS_FLAG_MASK));
    SkASSERT(0 == (data & ~PAINTOPS_DATA_MASK));

    return (op << (PAINTOPS_FLAG_BITS + PAINTOPS_DATA_BITS)) |
    (flags << PAINTOPS_DATA_BITS) |
    data;
}

#endif

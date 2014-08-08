/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordOpts.h"

#include "SkRecordPattern.h"
#include "SkRecords.h"
#include "SkTDArray.h"

using namespace SkRecords;

void SkRecordOptimize(SkRecord* record) {
    // TODO(mtklein): fuse independent optimizations to reduce number of passes?
    SkRecordNoopSaveRestores(record);
    // TODO(mtklein): figure out why we draw differently and reenable
    //SkRecordNoopSaveLayerDrawRestores(record);

    SkRecordReduceDrawPosTextStrength(record);  // Helpful to run this before BoundDrawPosTextH.
}

// Most of the optimizations in this file are pattern-based.  These are all defined as structs with:
//   - a Pattern typedef
//   - a bool onMatch(SkRceord*, Pattern*, unsigned begin, unsigned end) method,
//     which returns true if it made changes and false if not.

// Run a pattern-based optimization once across the SkRecord, returning true if it made any changes.
// It looks for spans which match Pass::Pattern, and when found calls onMatch() with the pattern,
// record, and [begin,end) span of the commands that matched.
template <typename Pass>
static bool apply(Pass* pass, SkRecord* record) {
    typename Pass::Pattern pattern;
    bool changed = false;
    unsigned begin, end = 0;

    while (pattern.search(record, &begin, &end)) {
        changed |= pass->onMatch(record, &pattern, begin, end);
    }
    return changed;
}

// Turns the logical NoOp Save and Restore in Save-Draw*-Restore patterns into actual NoOps.
struct SaveOnlyDrawsRestoreNooper {
    typedef Pattern3<Is<Save>,
                     Star<Or<Is<NoOp>, IsDraw> >,
                     Is<Restore> >
        Pattern;

    bool onMatch(SkRecord* record, Pattern* pattern, unsigned begin, unsigned end) {
        record->replace<NoOp>(begin);  // Save
        record->replace<NoOp>(end-1);  // Restore
        return true;
    }
};
// Turns logical no-op Save-[non-drawing command]*-Restore patterns into actual no-ops.
struct SaveNoDrawsRestoreNooper {
    // Star matches greedily, so we also have to exclude Save and Restore.
    typedef Pattern3<Is<Save>,
                     Star<Not<Or3<Is<Save>,
                                  Is<Restore>,
                                  IsDraw> > >,
                     Is<Restore> >
        Pattern;

    bool onMatch(SkRecord* record, Pattern* pattern, unsigned begin, unsigned end) {
        // The entire span between Save and Restore (inclusively) does nothing.
        for (unsigned i = begin; i < end; i++) {
            record->replace<NoOp>(i);
        }
        return true;
    }
};
void SkRecordNoopSaveRestores(SkRecord* record) {
    SaveOnlyDrawsRestoreNooper onlyDraws;
    SaveNoDrawsRestoreNooper noDraws;

    // Run until they stop changing things.
    while (apply(&onlyDraws, record) || apply(&noDraws, record));
}

// For some SaveLayer-[drawing command]-Restore patterns, merge the SaveLayer's alpha into the
// draw, and no-op the SaveLayer and Restore.
struct SaveLayerDrawRestoreNooper {
    typedef Pattern3<Is<SaveLayer>, IsDraw, Is<Restore> > Pattern;

    bool onMatch(SkRecord* record, Pattern* pattern, unsigned begin, unsigned end) {
        SaveLayer* saveLayer = pattern->first<SaveLayer>();
        if (saveLayer->bounds != NULL) {
            // SaveLayer with bounds is too tricky for us.
            return false;
        }

        SkPaint* layerPaint = saveLayer->paint;
        if (NULL == layerPaint) {
            // There wasn't really any point to this SaveLayer at all.
            return KillSaveLayerAndRestore(record, begin);
        }

        SkPaint* drawPaint = pattern->second<SkPaint>();
        if (drawPaint == NULL) {
            // We can just give the draw the SaveLayer's paint.
            // TODO(mtklein): figure out how to do this clearly
            return false;
        }

        const uint32_t layerColor = layerPaint->getColor();
        const uint32_t  drawColor =  drawPaint->getColor();
        if (!IsOnlyAlpha(layerColor)  || !IsOpaque(drawColor) ||
            HasAnyEffect(*layerPaint) || HasAnyEffect(*drawPaint)) {
            // Too fancy for us.  Actually, as long as layerColor is just an alpha
            // we can blend it into drawColor's alpha; drawColor doesn't strictly have to be opaque.
            return false;
        }

        drawPaint->setColor(SkColorSetA(drawColor, SkColorGetA(layerColor)));
        return KillSaveLayerAndRestore(record, begin);
    }

    static bool KillSaveLayerAndRestore(SkRecord* record, unsigned saveLayerIndex) {
        record->replace<NoOp>(saveLayerIndex);    // SaveLayer
        record->replace<NoOp>(saveLayerIndex+2);  // Restore
        return true;
    }

    static bool HasAnyEffect(const SkPaint& paint) {
        return paint.getPathEffect()  ||
               paint.getShader()      ||
               paint.getXfermode()    ||
               paint.getMaskFilter()  ||
               paint.getColorFilter() ||
               paint.getRasterizer()  ||
               paint.getLooper()      ||
               paint.getImageFilter();
    }

    static bool IsOpaque(SkColor color) {
        return SkColorGetA(color) == SK_AlphaOPAQUE;
    }
    static bool IsOnlyAlpha(SkColor color) {
        return SK_ColorTRANSPARENT == SkColorSetA(color, SK_AlphaTRANSPARENT);
    }
};
void SkRecordNoopSaveLayerDrawRestores(SkRecord* record) {
    SaveLayerDrawRestoreNooper pass;
    apply(&pass, record);
}


// Replaces DrawPosText with DrawPosTextH when all Y coordinates are equal.
struct StrengthReducer {
    typedef Pattern1<Is<DrawPosText> > Pattern;

    bool onMatch(SkRecord* record, Pattern* pattern, unsigned begin, unsigned end) {
        SkASSERT(end == begin + 1);
        DrawPosText* draw = pattern->first<DrawPosText>();

        const unsigned points = draw->paint.countText(draw->text, draw->byteLength);
        if (points == 0) {
            return false;  // No point (ha!).
        }

        const SkScalar firstY = draw->pos[0].fY;
        for (unsigned i = 1; i < points; i++) {
            if (draw->pos[i].fY != firstY) {
                return false;  // Needs full power of DrawPosText.
            }
        }
        // All ys are the same.  We can replace DrawPosText with DrawPosTextH.

        // draw->pos is points SkPoints, [(x,y),(x,y),(x,y),(x,y), ... ].
        // We're going to squint and look at that as 2*points SkScalars, [x,y,x,y,x,y,x,y, ...].
        // Then we'll rearrange things so all the xs are in order up front, clobbering the ys.
        SK_COMPILE_ASSERT(sizeof(SkPoint) == 2 * sizeof(SkScalar), SquintingIsNotSafe);
        SkScalar* scalars = &draw->pos[0].fX;
        for (unsigned i = 0; i < 2*points; i += 2) {
            scalars[i/2] = scalars[i];
        }

        // Extend lifetime of draw to the end of the loop so we can copy its paint.
        Adopted<DrawPosText> adopted(draw);
        SkNEW_PLACEMENT_ARGS(record->replace<DrawPosTextH>(begin, adopted),
                             DrawPosTextH,
                             (draw->paint, draw->text, draw->byteLength, scalars, firstY));
        return true;
    }
};
void SkRecordReduceDrawPosTextStrength(SkRecord* record) {
    StrengthReducer pass;
    apply(&pass, record);
}


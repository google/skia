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
    SkRecordNoopCulls(record);
    SkRecordNoopSaveRestores(record);
    // TODO(mtklein): figure out why we draw differently and reenable
    //SkRecordNoopSaveLayerDrawRestores(record);

    SkRecordAnnotateCullingPairs(record);
    SkRecordReduceDrawPosTextStrength(record);  // Helpful to run this before BoundDrawPosTextH.
    SkRecordBoundDrawPosTextH(record);
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

struct CullNooper {
    typedef Pattern3<Is<PushCull>, Star<Is<NoOp> >, Is<PopCull> > Pattern;

    bool onMatch(SkRecord* record, Pattern* pattern, unsigned begin, unsigned end) {
        record->replace<NoOp>(begin);  // PushCull
        record->replace<NoOp>(end-1);  // PopCull
        return true;
    }
};

void SkRecordNoopCulls(SkRecord* record) {
    CullNooper pass;
    while (apply(&pass, record));
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

// Tries to replace DrawPosTextH with BoundedDrawPosTextH, which knows conservative upper and lower
// bounds to use with SkCanvas::quickRejectY.
struct TextBounder {
    typedef Pattern1<Is<DrawPosTextH> > Pattern;

    bool onMatch(SkRecord* record, Pattern* pattern, unsigned begin, unsigned end) {
        SkASSERT(end == begin + 1);
        DrawPosTextH* draw = pattern->first<DrawPosTextH>();

        // If we're drawing vertical text, none of the checks we're about to do make any sense.
        // We'll need to call SkPaint::computeFastBounds() later, so bail if that's not possible.
        if (draw->paint.isVerticalText() || !draw->paint.canComputeFastBounds()) {
            return false;
        }

        // Rather than checking the top and bottom font metrics, we guess.  Actually looking up the
        // top and bottom metrics is slow, and this overapproximation should be good enough.
        const SkScalar buffer = draw->paint.getTextSize() * 1.5f;
        SkDEBUGCODE(SkPaint::FontMetrics metrics;)
        SkDEBUGCODE(draw->paint.getFontMetrics(&metrics);)
        SkASSERT(-buffer <= metrics.fTop);
        SkASSERT(+buffer >= metrics.fBottom);

        // Let the paint adjust the text bounds.  We don't care about left and right here, so we use
        // 0 and 1 respectively just so the bounds rectangle isn't empty.
        SkRect bounds;
        bounds.set(0, draw->y - buffer, SK_Scalar1, draw->y + buffer);
        SkRect adjusted = draw->paint.computeFastBounds(bounds, &bounds);

        Adopted<DrawPosTextH> adopted(draw);
        SkNEW_PLACEMENT_ARGS(record->replace<BoundedDrawPosTextH>(begin, adopted),
                             BoundedDrawPosTextH,
                             (&adopted, adjusted.fTop, adjusted.fBottom));
        return true;
    }
};
void SkRecordBoundDrawPosTextH(SkRecord* record) {
    TextBounder pass;
    apply(&pass, record);
}

// Replaces PushCull with PairedPushCull, which lets us skip to the paired PopCull when the canvas
// can quickReject the cull rect.
// There's no efficient way (yet?) to express this one as a pattern, so we write a custom pass.
class CullAnnotator {
public:
    // Do nothing to most ops.
    template <typename T> void operator()(T*) {}

    void operator()(PushCull* push) {
        Pair pair = { fIndex, push };
        fPushStack.push(pair);
    }

    void operator()(PopCull* pop) {
        Pair push = fPushStack.top();
        fPushStack.pop();

        SkASSERT(fIndex > push.index);
        unsigned skip = fIndex - push.index;

        Adopted<PushCull> adopted(push.command);
        SkNEW_PLACEMENT_ARGS(fRecord->replace<PairedPushCull>(push.index, adopted),
                             PairedPushCull, (&adopted, skip));
    }

    void apply(SkRecord* record) {
        for (fRecord = record, fIndex = 0; fIndex < record->count(); fIndex++) {
            fRecord->mutate<void>(fIndex, *this);
        }
    }

private:
    struct Pair {
        unsigned index;
        PushCull* command;
    };

    SkTDArray<Pair> fPushStack;
    SkRecord* fRecord;
    unsigned fIndex;
};
void SkRecordAnnotateCullingPairs(SkRecord* record) {
    CullAnnotator pass;
    pass.apply(record);
}

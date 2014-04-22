/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordOpts.h"

#include "SkRecords.h"
#include "SkTDArray.h"

void SkRecordOptimize(SkRecord* record) {
    // TODO(mtklein): fuse independent optimizations to reduce number of passes?
    SkRecordNoopSaveRestores(record);
    SkRecordAnnotateCullingPairs(record);
    SkRecordReduceDrawPosTextStrength(record);  // Helpful to run this before BoundDrawPosTextH.
    SkRecordBoundDrawPosTextH(record);
}

// Streamline replacing one command with another.
#define REPLACE(record, index, T, ...) \
    SkNEW_PLACEMENT_ARGS(record->replace<SkRecords::T>(index), SkRecords::T, (__VA_ARGS__))

namespace {

// Convenience base class to share some common implementation code.
class Common : SkNoncopyable {
public:
    explicit Common(SkRecord* record) : fRecord(record), fIndex(0) {}

    unsigned index() const { return fIndex; }
    void next() { ++fIndex; }

protected:
    SkRecord* fRecord;
    unsigned fIndex;
};

// Turns logical no-op Save-[non-drawing command]*-Restore patterns into actual no-ops.
// TODO(mtklein): state machine diagram
class SaveRestoreNooper : public Common {
public:
    explicit SaveRestoreNooper(SkRecord* record)
        : Common(record), fSave(kInactive), fChanged(false) {}

    // Most drawing commands reset to inactive state without nooping anything.
    template <typename T>
    void operator()(T*) { fSave = kInactive; }

    bool changed() const { return fChanged; }

private:
    static const unsigned kInactive = ~0;
    unsigned fSave;
    bool fChanged;
};

// If the command doesn't draw anything, that doesn't reset the state back to inactive.
// TODO(mtklein): do this with some sort of template-based trait mechanism instead of macros
#define DOESNT_DRAW(T) template <> void SaveRestoreNooper::operator()(SkRecords::T*) {}
DOESNT_DRAW(NoOp)
DOESNT_DRAW(Concat)
DOESNT_DRAW(SetMatrix)
DOESNT_DRAW(ClipRect)
DOESNT_DRAW(ClipRRect)
DOESNT_DRAW(ClipPath)
DOESNT_DRAW(ClipRegion)
DOESNT_DRAW(PairedPushCull)
DOESNT_DRAW(PushCull)
DOESNT_DRAW(PopCull)
#undef DOESNT_DRAW

template <>
void SaveRestoreNooper::operator()(SkRecords::Save* r) {
    fSave = SkCanvas::kMatrixClip_SaveFlag == r->flags ? this->index() : kInactive;
}

template <>
void SaveRestoreNooper::operator()(SkRecords::Restore* r) {
    if (fSave != kInactive) {
        // Remove everything between the save and restore, inclusive on both sides.
        fChanged = true;
        SkRecord::Destroyer destroyer;
        for (unsigned i = fSave; i <= this->index(); i++) {
            fRecord->mutate(i, destroyer);
            REPLACE(fRecord, i, NoOp);
        }
        fSave = kInactive;
    }
}


// Tries to replace PushCull with PairedPushCull, which lets us skip to the paired PopCull
// when the canvas can quickReject the cull rect.
class CullAnnotator : public Common {
public:
    explicit CullAnnotator(SkRecord* record) : Common(record) {}

    // Do nothing to most ops.
    template <typename T>
    void operator()(T*) {}

private:
    struct Pair {
        unsigned index;
        SkRecords::PushCull* command;
    };

    SkTDArray<Pair> fPushStack;
};

template <>
void CullAnnotator::operator()(SkRecords::PushCull* push) {
    Pair pair = { this->index(), push };
    fPushStack.push(pair);
}

template <>
void CullAnnotator::operator()(SkRecords::PopCull* pop) {
    Pair push = fPushStack.top();
    fPushStack.pop();

    SkASSERT(this->index() > push.index);
    unsigned skip = this->index() - push.index;

    // PairedPushCull adopts push.command.
    REPLACE(fRecord, push.index, PairedPushCull, push.command, skip);
}

// Replaces DrawPosText with DrawPosTextH when all Y coordinates are equal.
class StrengthReducer : public Common {
public:
    explicit StrengthReducer(SkRecord* record) : Common(record) {}

    // Do nothing to most ops.
    template <typename T>
    void operator()(T*) {}
};

template <>
void StrengthReducer::operator()(SkRecords::DrawPosText* r) {
    const unsigned points = r->paint.countText(r->text, r->byteLength);
    if (points == 0) {
        // No point (ha!).
        return;
    }

    const SkScalar firstY = r->pos[0].fY;
    for (unsigned i = 1; i < points; i++) {
        if (r->pos[i].fY != firstY) {
            // Needs the full strength of DrawPosText.
            return;
        }
    }
    // All ys are the same.  We can replace DrawPosText with DrawPosTextH.

    // r->pos is points SkPoints, [(x,y),(x,y),(x,y),(x,y), ... ].
    // We're going to squint and look at that as 2*points SkScalars, [x,y,x,y,x,y,x,y, ...].
    // Then we'll rearrange things so all the xs are in order up front, clobbering the ys.
    SK_COMPILE_ASSERT(sizeof(SkPoint) == 2 * sizeof(SkScalar), SquintingIsNotSafe);
    SkScalar* scalars = &r->pos[0].fX;
    for (unsigned i = 0; i < 2*points; i += 2) {
        scalars[i/2] = scalars[i];
    }

    SkRecord::Destroyer destroyer;
    fRecord->mutate(this->index(), destroyer);
    REPLACE(fRecord, this->index(),
            DrawPosTextH, (char*)r->text, r->byteLength, scalars, firstY, r->paint);
}


// Tries to replace DrawPosTextH with BoundedDrawPosTextH, which knows conservative upper and lower
// bounds to use with SkCanvas::quickRejectY.
class TextBounder : public Common {
public:
    explicit TextBounder(SkRecord* record) : Common(record) {}

    // Do nothing to most ops.
    template <typename T>
    void operator()(T*) {}
};

template <>
void TextBounder::operator()(SkRecords::DrawPosTextH* r) {
    // If we're drawing vertical text, none of the checks we're about to do make any sense.
    // We'll need to call SkPaint::computeFastBounds() later, so bail if that's not possible.
    if (r->paint.isVerticalText() || !r->paint.canComputeFastBounds()) {
        return;
    }

    // Rather than checking the top and bottom font metrics, we guess.  Actually looking up the
    // top and bottom metrics is slow, and this overapproximation should be good enough.
    const SkScalar buffer = r->paint.getTextSize() * 1.5f;
    SkDEBUGCODE(SkPaint::FontMetrics metrics;)
    SkDEBUGCODE(r->paint.getFontMetrics(&metrics);)
    SkASSERT(-buffer <= metrics.fTop);
    SkASSERT(+buffer >= metrics.fBottom);

    // Let the paint adjust the text bounds.  We don't care about left and right here, so we use
    // 0 and 1 respectively just so the bounds rectangle isn't empty.
    SkRect bounds;
    bounds.set(0, r->y - buffer, SK_Scalar1, r->y + buffer);
    SkRect adjusted = r->paint.computeFastBounds(bounds, &bounds);

    // BoundedDrawPosTextH adopts r.
    REPLACE(fRecord, this->index(), BoundedDrawPosTextH, r, adjusted.fTop, adjusted.fBottom);
}

template <typename Pass>
static void run_pass(Pass& pass, SkRecord* record) {
    for (; pass.index() < record->count(); pass.next()) {
        record->mutate(pass.index(), pass);
    }
}

}  // namespace


void SkRecordNoopSaveRestores(SkRecord* record) {
    // Run SaveRestoreNooper until it doesn't make any more changes.
    bool changed;
    do {
        SaveRestoreNooper nooper(record);
        run_pass(nooper, record);
        changed = nooper.changed();
    } while (changed);
}

void SkRecordAnnotateCullingPairs(SkRecord* record) {
    CullAnnotator annotator(record);
    run_pass(annotator, record);
}

void SkRecordReduceDrawPosTextStrength(SkRecord* record) {
    StrengthReducer reducer(record);
    run_pass(reducer, record);
}

void SkRecordBoundDrawPosTextH(SkRecord* record) {
    TextBounder bounder(record);
    run_pass(bounder, record);
}

#undef REPLACE

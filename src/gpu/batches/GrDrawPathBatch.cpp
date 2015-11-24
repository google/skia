/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawPathBatch.h"

SkString GrDrawPathBatch::dumpInfo() const {
    SkString string;
    string.printf("PATH: 0x%p", fPath.get());
    return string;
}

void GrDrawPathBatch::onDraw(GrBatchFlushState* state) {
    GrProgramDesc  desc;

    SkAutoTUnref<GrPathProcessor> pathProc(GrPathProcessor::Create(this->color(),
                                                                   this->overrides(),
                                                                   this->viewMatrix()));
    state->gpu()->buildProgramDesc(&desc, *pathProc, *this->pipeline());
    GrPathRendering::DrawPathArgs args(pathProc, this->pipeline(),
                                        &desc, &this->stencilSettings());
    state->gpu()->pathRendering()->drawPath(args, fPath.get());
}

GrDrawPathRangeBatch::~GrDrawPathRangeBatch() {
    for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
        (*iter.get())->unref();
    }
}

SkString GrDrawPathRangeBatch::dumpInfo() const {
    SkString string;
    string.printf("RANGE: 0x%p COUNTS: [", *fDraws.head());
    for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
        string.appendf("%d ,", (*iter.get())->count());
    }
    string.remove(string.size() - 2, 2);
    string.append("]");
    return string;
}

bool GrDrawPathRangeBatch::isWinding() const {
    static const GrStencilSettings::Face pathFace = GrStencilSettings::kFront_Face;
    bool isWinding = kInvert_StencilOp != this->stencilSettings().passOp(pathFace);
    if (isWinding) {
        // Double check that it is in fact winding.
        SkASSERT(kIncClamp_StencilOp == this->stencilSettings().passOp(pathFace));
        SkASSERT(kIncClamp_StencilOp == this->stencilSettings().failOp(pathFace));
        SkASSERT(0x1 != this->stencilSettings().writeMask(pathFace));
        SkASSERT(!this->stencilSettings().isTwoSided());
    }
    return isWinding;
}

GrDrawPathRangeBatch::GrDrawPathRangeBatch(const SkMatrix& viewMatrix, const SkMatrix& localMatrix,
                                           GrColor color, GrPathRange* range, GrPathRangeDraw* draw,
                                           const SkRect& bounds)
    : INHERITED(ClassID(), viewMatrix, color)
    , fPathRange(range)
    , fLocalMatrix(localMatrix) {
    SkDEBUGCODE(draw->fUsedInBatch = true;)
    fDraws.addToHead(SkRef(draw));
    fTotalPathCount = draw->count();
    fBounds = bounds;
}

bool GrDrawPathRangeBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    GrDrawPathRangeBatch* that = t->cast<GrDrawPathRangeBatch>();
    if (this->fPathRange.get() != that->fPathRange.get()) {
        return false;
    }
    if (!GrPathRangeDraw::CanMerge(**this->fDraws.head(), **that->fDraws.head())) {
        return false;
    }
    if (!GrPipeline::AreEqual(*this->pipeline(), *that->pipeline(), false)) {
        return false;
    }
    if (this->color() != that->color() ||
        !this->viewMatrix().cheapEqualTo(that->viewMatrix()) ||
        !fLocalMatrix.cheapEqualTo(that->fLocalMatrix)) {
        return false;
    }
    // TODO: Check some other things here. (winding, opaque, pathProc color, vm, ...)
    // Try to combine this call with the previous DrawPaths. We do this by stenciling all the
    // paths together and then covering them in a single pass. This is not equivalent to two
    // separate draw calls, so we can only do it if there is no blending (no overlap would also
    // work). Note that it's also possible for overlapping paths to cancel each other's winding
    // numbers, and we only partially account for this by not allowing even/odd paths to be
    // combined. (Glyphs in the same font tend to wind the same direction so it works out OK.)
    if (!this->isWinding() ||
        this->stencilSettings() != that->stencilSettings() ||
        this->overrides().willColorBlendWithDst()) {
        return false;
    }
    SkASSERT(!that->overrides().willColorBlendWithDst());
    fTotalPathCount += that->fTotalPathCount;
    while (GrPathRangeDraw** head = that->fDraws.head()) {
        fDraws.addToTail(*head);
        // We're stealing that's refs, so pop without unreffing.
        that->fDraws.popHead();
    }
    return true;
}

void GrDrawPathRangeBatch::onDraw(GrBatchFlushState* state) {
    GrProgramDesc  desc;
    SkAutoTUnref<GrPathProcessor> pathProc(GrPathProcessor::Create(this->color(),
                                                                   this->overrides(),
                                                                   this->viewMatrix(),
                                                                   fLocalMatrix));
    state->gpu()->buildProgramDesc(&desc, *pathProc, *this->pipeline());
    GrPathRendering::DrawPathArgs args(pathProc, this->pipeline(),
                                        &desc, &this->stencilSettings());
    if (fDraws.count() == 1) {
        const GrPathRangeDraw& draw = **fDraws.head();
        state->gpu()->pathRendering()->drawPaths(args, fPathRange.get(), draw.indices(),
            GrPathRange::kU16_PathIndexType, draw.transforms(), draw.transformType(),
            draw.count());
        return;
    }

    GrPathRendering::PathTransformType transformType = (*fDraws.head())->transformType();
    int floatsPerTransform = GrPathRendering::PathTransformSize(transformType);
    SkAutoSTMalloc<512, float> transformStorage(floatsPerTransform * fTotalPathCount);
    SkAutoSTMalloc<256, uint16_t> indexStorage(fTotalPathCount);
    uint16_t* indices = indexStorage.get();
    float* transforms = transformStorage.get();
    for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
        SkASSERT((*iter.get())->transformType() == transformType);
        int cnt = (*iter.get())->count();
        memcpy(indices, (*iter.get())->indices(), cnt * sizeof(uint16_t));
        indices += cnt;
        memcpy(transforms, (*iter.get())->transforms(), cnt * floatsPerTransform * sizeof(float));
        transforms += cnt * floatsPerTransform;
    }
    SkASSERT(indices - indexStorage.get() == fTotalPathCount);
    state->gpu()->pathRendering()->drawPaths(args, fPathRange.get(), indexStorage.get(),
        GrPathRange::kU16_PathIndexType, transformStorage.get(), transformType,
        fTotalPathCount);
}

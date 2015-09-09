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
    state->gpu()->buildProgramDesc(&desc, *this->pathProcessor(),
                                    *this->pipeline(), *this->tracker());
    GrPathRendering::DrawPathArgs args(this->pathProcessor(), this->pipeline(),
                                        &desc, this->tracker(), &this->stencilSettings());
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

GrDrawPathRangeBatch::GrDrawPathRangeBatch(const GrPathProcessor* pathProc,
                                           GrPathRangeDraw* pathRangeDraw)
    : INHERITED(pathProc)
    , fDraws(4) {
    SkDEBUGCODE(pathRangeDraw->fUsedInBatch = true;)
    this->initClassID<GrDrawPathRangeBatch>();
    fDraws.addToHead(SkRef(pathRangeDraw));
    fTotalPathCount = pathRangeDraw->count();
    // Don't compute a bounding box. For dst copy texture, we'll opt instead for it to just copy
    // the entire dst. Realistically this is a moot point, because any context that supports
    // NV_path_rendering will also support NV_blend_equation_advanced.
    // For clipping we'll just skip any optimizations based on the bounds.
    fBounds.setLargest();
}

bool GrDrawPathRangeBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    GrDrawPathRangeBatch* that = t->cast<GrDrawPathRangeBatch>();
    if (!GrPathRangeDraw::CanMerge(**this->fDraws.head(), **that->fDraws.head())) {
        return false;
    }
    if (!GrPipeline::AreEqual(*this->pipeline(), *that->pipeline(), false)) {
        return false;
    }
    if (!this->pathProcessor()->isEqual(*this->tracker(), *that->pathProcessor(),
                                        *that->tracker())) {
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
        this->opts().willColorBlendWithDst()) {
        return false;
    }
    SkASSERT(!that->opts().willColorBlendWithDst());
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
    state->gpu()->buildProgramDesc(&desc, *this->pathProcessor(), *this->pipeline(),
                                    *this->tracker());
    GrPathRendering::DrawPathArgs args(this->pathProcessor(), this->pipeline(),
                                        &desc, this->tracker(), &this->stencilSettings());
    if (fDraws.count() == 1) {
        const GrPathRangeDraw& draw = **fDraws.head();
        state->gpu()->pathRendering()->drawPaths(args, draw.range(), draw.indices(),
            GrPathRange::kU16_PathIndexType, draw.transforms(), draw.transformType(),
            draw.count());
        return;
    }

    const GrPathRange* range = (*fDraws.head())->range();
    GrPathRendering::PathTransformType transformType = (*fDraws.head())->transformType();
    int floatsPerTransform = GrPathRendering::PathTransformSize(transformType);
    SkAutoSTMalloc<512, float> transformStorage(floatsPerTransform * fTotalPathCount);
    SkAutoSTMalloc<256, uint16_t> indexStorage(fTotalPathCount);
    uint16_t* indices = indexStorage.get();
    float* transforms = transformStorage.get();
    for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
        SkASSERT((*iter.get())->transformType() == transformType);
        SkASSERT((*iter.get())->range() == range);
        int cnt = (*iter.get())->count();
        memcpy(indices, (*iter.get())->indices(), cnt * sizeof(uint16_t));
        indices += cnt;
        memcpy(transforms, (*iter.get())->transforms(), cnt * floatsPerTransform * sizeof(float));
        transforms += cnt * floatsPerTransform;
    }
    SkASSERT(indices - indexStorage.get() == fTotalPathCount);
    state->gpu()->pathRendering()->drawPaths(args, range, indexStorage.get(),
        GrPathRange::kU16_PathIndexType, transformStorage.get(), transformType,
        fTotalPathCount);
}

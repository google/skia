/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawPathBatch.h"

static void pre_translate_transform_values(const float* xforms,
                                           GrPathRendering::PathTransformType type, int count,
                                           SkScalar x, SkScalar y, float* dst);

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

SkString GrDrawPathRangeBatch::dumpInfo() const {
    SkString string;
    string.printf("RANGE: 0x%p COUNTS: [", fPathRange.get());
    for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
        string.appendf("%d, ", iter.get()->fInstanceData->count());
    }
    string.remove(string.size() - 2, 2);
    string.append("]");
    return string;
}

GrDrawPathRangeBatch::GrDrawPathRangeBatch(const SkMatrix& viewMatrix, SkScalar scale, SkScalar x,
                                           SkScalar y, GrColor color,
                                           GrPathRendering::FillType fill, GrPathRange* range,
                                           const InstanceData* instanceData, const SkRect& bounds)
    : INHERITED(ClassID(), viewMatrix, color, fill)
    , fPathRange(range)
    , fTotalPathCount(instanceData->count())
    , fScale(scale) {
    fDraws.addToHead()->set(instanceData, x, y);
    fBounds = bounds;
}

bool GrDrawPathRangeBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    GrDrawPathRangeBatch* that = t->cast<GrDrawPathRangeBatch>();
    if (this->fPathRange.get() != that->fPathRange.get() ||
        this->transformType() != that->transformType() ||
        this->fScale != that->fScale ||
        this->color() != that->color() ||
        !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return false;
    }
    if (!GrPipeline::AreEqual(*this->pipeline(), *that->pipeline(), false)) {
        return false;
    }
    switch (fDraws.head()->fInstanceData->transformType()) {
        case GrPathRendering::kNone_PathTransformType:
            if (this->fDraws.head()->fX != that->fDraws.head()->fX ||
                this->fDraws.head()->fY != that->fDraws.head()->fY) {
                return false;
            }
            break;
        case GrPathRendering::kTranslateX_PathTransformType:
            if (this->fDraws.head()->fY != that->fDraws.head()->fY) {
                return false;
            }
            break;
        case GrPathRendering::kTranslateY_PathTransformType:
            if (this->fDraws.head()->fX != that->fDraws.head()->fX) {
                return false;
            }
            break;
        default: break;
    }
    // TODO: Check some other things here. (winding, opaque, pathProc color, vm, ...)
    // Try to combine this call with the previous DrawPaths. We do this by stenciling all the
    // paths together and then covering them in a single pass. This is not equivalent to two
    // separate draw calls, so we can only do it if there is no blending (no overlap would also
    // work). Note that it's also possible for overlapping paths to cancel each other's winding
    // numbers, and we only partially account for this by not allowing even/odd paths to be
    // combined. (Glyphs in the same font tend to wind the same direction so it works out OK.)
    if (GrPathRendering::kWinding_FillType != this->fillType() ||
        this->stencilSettings() != that->stencilSettings() ||
        this->overrides().willColorBlendWithDst()) {
        return false;
    }
    SkASSERT(!that->overrides().willColorBlendWithDst());
    fTotalPathCount += that->fTotalPathCount;
    while (Draw* head = that->fDraws.head()) {
        Draw* draw = fDraws.addToTail();
        draw->fInstanceData.reset(head->fInstanceData.detach());
        draw->fX = head->fX;
        draw->fY = head->fY;
        that->fDraws.popHead();
    }
    return true;
}

void GrDrawPathRangeBatch::onDraw(GrBatchFlushState* state) {
    const Draw& head = *fDraws.head();

    SkMatrix drawMatrix(this->viewMatrix());
    drawMatrix.preScale(fScale, fScale);
    drawMatrix.preTranslate(head.fX, head.fY);

    SkMatrix localMatrix;
    localMatrix.setScale(fScale, fScale);
    localMatrix.preTranslate(head.fX, head.fY);

    SkAutoTUnref<GrPathProcessor> pathProc(GrPathProcessor::Create(this->color(),
                                                                   this->overrides(),
                                                                   drawMatrix,
                                                                   localMatrix));

    GrProgramDesc  desc;
    state->gpu()->buildProgramDesc(&desc, *pathProc, *this->pipeline());
    GrPathRendering::DrawPathArgs args(pathProc, this->pipeline(),
                                       &desc, &this->stencilSettings());

    if (fDraws.count() == 1) {
        const InstanceData& instances = *head.fInstanceData;
        state->gpu()->pathRendering()->drawPaths(args, fPathRange.get(), instances.indices(),
                                                 GrPathRange::kU16_PathIndexType,
                                                 instances.transformValues(),
                                                 instances.transformType(),
                                                 instances.count());
    } else {
        int floatsPerTransform = GrPathRendering::PathTransformSize(this->transformType());
#if defined(GOOGLE3)
        //Stack frame size is limited in GOOGLE3.
        SkAutoSTMalloc<512, float> transformStorage(floatsPerTransform * fTotalPathCount);
        SkAutoSTMalloc<256, uint16_t> indexStorage(fTotalPathCount);
#else
        SkAutoSTMalloc<4096, float> transformStorage(floatsPerTransform * fTotalPathCount);
        SkAutoSTMalloc<2048, uint16_t> indexStorage(fTotalPathCount);
#endif
        int idx = 0;
        for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
            const Draw& draw = *iter.get();
            const InstanceData& instances = *draw.fInstanceData;
            memcpy(&indexStorage[idx], instances.indices(), instances.count() * sizeof(uint16_t));
            pre_translate_transform_values(instances.transformValues(), this->transformType(),
                                           instances.count(),
                                           draw.fX - head.fX, draw.fY - head.fY,
                                           &transformStorage[floatsPerTransform * idx]);
            idx += instances.count();

            // TODO: Support mismatched transform types if we start using more types other than 2D.
            SkASSERT(instances.transformType() == this->transformType());
        }
        SkASSERT(idx == fTotalPathCount);

        state->gpu()->pathRendering()->drawPaths(args, fPathRange.get(), indexStorage,
                                                 GrPathRange::kU16_PathIndexType, transformStorage,
                                                 this->transformType(), fTotalPathCount);
    }
}

inline void pre_translate_transform_values(const float* xforms,
                                           GrPathRendering::PathTransformType type, int count,
                                           SkScalar x, SkScalar y, float* dst) {
    if (0 == x && 0 == y) {
        memcpy(dst, xforms, count * GrPathRendering::PathTransformSize(type) * sizeof(float));
        return;
    }
    switch (type) {
        case GrPathRendering::kNone_PathTransformType:
            SkFAIL("Cannot pre-translate kNone_PathTransformType.");
            break;
        case GrPathRendering::kTranslateX_PathTransformType:
            SkASSERT(0 == y);
            for (int i = 0; i < count; i++) {
                dst[i] = xforms[i] + x;
            }
            break;
        case GrPathRendering::kTranslateY_PathTransformType:
            SkASSERT(0 == x);
            for (int i = 0; i < count; i++) {
                dst[i] = xforms[i] + y;
            }
            break;
        case GrPathRendering::kTranslate_PathTransformType:
            for (int i = 0; i < 2 * count; i += 2) {
                dst[i] = xforms[i] + x;
                dst[i + 1] = xforms[i + 1] + y;
            }
            break;
        case GrPathRendering::kAffine_PathTransformType:
            for (int i = 0; i < 6 * count; i += 6) {
                dst[i] = xforms[i];
                dst[i + 1] = xforms[i + 1];
                dst[i + 2] = xforms[i] * x + xforms[i + 1] * y + xforms[i + 2];
                dst[i + 3] = xforms[i + 3];
                dst[i + 4] = xforms[i + 4];
                dst[i + 5] = xforms[i + 3] * x + xforms[i + 4] * y + xforms[i + 5];
            }
            break;
        default:
            SkFAIL("Unknown transform type.");
            break;
    }
}

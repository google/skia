/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "src/utils/SkCanvasStack.h"

SkCanvasStack::SkCanvasStack(int width, int height)
        : INHERITED(width, height) {}

SkCanvasStack::~SkCanvasStack() {
    this->removeAll();
}

void SkCanvasStack::pushCanvas(std::unique_ptr<SkCanvas> canvas, const SkIPoint& origin) {
    if (canvas) {
        // compute the bounds of this canvas
        const SkIRect canvasBounds = SkIRect::MakeSize(canvas->getBaseLayerSize());

        // push the canvas onto the stack
        this->INHERITED::addCanvas(canvas.get());

        // push the canvas data onto the stack
        CanvasData* data = &fCanvasData.push_back();
        data->origin = origin;
        data->requiredClip.setRect(canvasBounds);
        data->ownedCanvas = std::move(canvas);

        // subtract this region from the canvas objects already on the stack.
        // This ensures they do not draw into the space occupied by the layers
        // above them.
        for (int i = fList.count() - 1; i > 0; --i) {
            SkIRect localBounds = canvasBounds;
            localBounds.offset(origin - fCanvasData[i-1].origin);

            fCanvasData[i-1].requiredClip.op(localBounds, SkRegion::kDifference_Op);
            fList[i-1]->clipRegion(fCanvasData[i-1].requiredClip);
        }
    }
    SkASSERT(fList.count() == fCanvasData.count());
}

void SkCanvasStack::removeAll() {
    this->INHERITED::removeAll();   // call the baseclass *before* we actually delete the canvases
    fCanvasData.reset();
}

/**
 * Traverse all canvases (e.g. layers) the stack and ensure that they are clipped
 * to their bounds and that the area covered by any canvas higher in the stack is
 * also clipped out.
 */
void SkCanvasStack::clipToZOrderedBounds() {
    SkASSERT(fList.count() == fCanvasData.count());
    for (int i = 0; i < fList.count(); ++i) {
        fList[i]->clipRegion(fCanvasData[i].requiredClip);
    }
}

////////////////////////////////////////////////////////////////////////////////

/**
 * We need to handle setMatrix specially as it overwrites the matrix in each
 * canvas unlike all other matrix operations (i.e. translate, scale, etc) which
 * just pre-concatenate with the existing matrix.
 */
void SkCanvasStack::didSetMatrix(const SkMatrix& matrix) {
    SkASSERT(fList.count() == fCanvasData.count());
    for (int i = 0; i < fList.count(); ++i) {

        SkMatrix tempMatrix = matrix;
        tempMatrix.postTranslate(SkIntToScalar(-fCanvasData[i].origin.x()),
                                 SkIntToScalar(-fCanvasData[i].origin.y()));
        fList[i]->setMatrix(tempMatrix);
    }
    this->SkCanvas::didSetMatrix(matrix);
}

void SkCanvasStack::onClipRect(const SkRect& r, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRect(r, op, edgeStyle);
    this->clipToZOrderedBounds();
}

void SkCanvasStack::onClipRRect(const SkRRect& rr, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRRect(rr, op, edgeStyle);
    this->clipToZOrderedBounds();
}

void SkCanvasStack::onClipPath(const SkPath& p, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipPath(p, op, edgeStyle);
    this->clipToZOrderedBounds();
}

void SkCanvasStack::onClipShader(sk_sp<SkShader> cs, SkClipOp op) {
    this->INHERITED::onClipShader(std::move(cs), op);
    // we don't change the "bounds" of the clip, so we don't need to update zorder
}

void SkCanvasStack::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    SkASSERT(fList.count() == fCanvasData.count());
    for (int i = 0; i < fList.count(); ++i) {
        SkRegion tempRegion;
        deviceRgn.translate(-fCanvasData[i].origin.x(),
                            -fCanvasData[i].origin.y(), &tempRegion);
        tempRegion.op(fCanvasData[i].requiredClip, SkRegion::kIntersect_Op);
        fList[i]->clipRegion(tempRegion, op);
    }
    this->SkCanvas::onClipRegion(deviceRgn, op);
}

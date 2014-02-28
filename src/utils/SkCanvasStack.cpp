
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkCanvasStack.h"

SkCanvasStack::SkCanvasStack(int width, int height)
        : INHERITED(width, height) {}

SkCanvasStack::~SkCanvasStack() {
    this->removeAll();
}

void SkCanvasStack::pushCanvas(SkCanvas* canvas, const SkIPoint& origin) {
    if (canvas) {
        // compute the bounds of this canvas
        const SkIRect canvasBounds = SkIRect::MakeSize(canvas->getDeviceSize());

        // push the canvas onto the stack
        this->INHERITED::addCanvas(canvas);

        // push the canvas data onto the stack
        CanvasData* data = &fCanvasData.push_back();
        data->origin = origin;
        data->requiredClip.setRect(canvasBounds);

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
    fCanvasData.reset();
    this->INHERITED::removeAll();
}

/**
 * Traverse all canvases (e.g. layers) the stack and ensure that they are clipped
 * to their bounds and that the area covered by any canvas higher in the stack is
 * also clipped out.
 */
void SkCanvasStack::clipToZOrderedBounds() {
    SkASSERT(fList.count() == fCanvasData.count());
    for (int i = 0; i < fList.count(); ++i) {
        fList[i]->clipRegion(fCanvasData[i].requiredClip, SkRegion::kIntersect_Op);
    }
}

////////////////////////////////////////////////////////////////////////////////

/**
 * We need to handle setMatrix specially as it overwrites the matrix in each
 * canvas unlike all other matrix operations (i.e. translate, scale, etc) which
 * just pre-concatenate with the existing matrix.
 */
void SkCanvasStack::setMatrix(const SkMatrix& matrix) {
    SkASSERT(fList.count() == fCanvasData.count());
    for (int i = 0; i < fList.count(); ++i) {

        SkMatrix tempMatrix = matrix;
        tempMatrix.postTranslate(SkIntToScalar(-fCanvasData[i].origin.x()),
                                 SkIntToScalar(-fCanvasData[i].origin.y()));
        fList[i]->setMatrix(tempMatrix);
    }
    this->SkCanvas::setMatrix(matrix);
}

bool SkCanvasStack::clipRect(const SkRect& r, SkRegion::Op op, bool aa) {
    bool result = this->INHERITED::clipRect(r, op, aa);
    this->clipToZOrderedBounds();
    return result;
}

bool SkCanvasStack::clipRRect(const SkRRect& rr, SkRegion::Op op, bool aa) {
    bool result = this->INHERITED::clipRRect(rr, op, aa);
    this->clipToZOrderedBounds();
    return result;
}

bool SkCanvasStack::clipPath(const SkPath& p, SkRegion::Op op, bool aa) {
    bool result = this->INHERITED::clipPath(p, op, aa);
    this->clipToZOrderedBounds();
    return result;
}

bool SkCanvasStack::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    SkASSERT(fList.count() == fCanvasData.count());
    for (int i = 0; i < fList.count(); ++i) {
        SkRegion tempRegion;
        deviceRgn.translate(-fCanvasData[i].origin.x(),
                            -fCanvasData[i].origin.y(), &tempRegion);
        tempRegion.op(fCanvasData[i].requiredClip, SkRegion::kIntersect_Op);
        fList[i]->clipRegion(tempRegion, op);
    }
    return this->SkCanvas::clipRegion(deviceRgn, op);
}

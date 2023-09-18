/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkClipStackDevice.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkMatrixPriv.h"

#include <utility>

class SkRRect;
enum class SkClipOp;

SkIRect SkClipStackDevice::devClipBounds() const {
    SkIRect r = fClipStack.bounds(this->imageInfo().bounds()).roundOut();
    if (!r.isEmpty()) {
        SkASSERT(this->imageInfo().bounds().contains(r));
    }
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkClipStackDevice::pushClipStack() {
    fClipStack.save();
}

void SkClipStackDevice::popClipStack() {
    fClipStack.restore();
}

void SkClipStackDevice::clipRect(const SkRect& rect, SkClipOp op, bool aa) {
    fClipStack.clipRect(rect, this->localToDevice(), op, aa);
}

void SkClipStackDevice::clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    fClipStack.clipRRect(rrect, this->localToDevice(), op, aa);
}

void SkClipStackDevice::clipPath(const SkPath& path, SkClipOp op, bool aa) {
    fClipStack.clipPath(path, this->localToDevice(), op, aa);
}

void SkClipStackDevice::onClipShader(sk_sp<SkShader> shader) {
    fClipStack.clipShader(std::move(shader));
}

void SkClipStackDevice::clipRegion(const SkRegion& rgn, SkClipOp op) {
    SkIPoint origin = this->getOrigin();
    SkRegion tmp;
    SkPath path;
    rgn.getBoundaryPath(&path);
    path.transform(SkMatrix::Translate(-origin));
    fClipStack.clipPath(path, SkMatrix::I(), op, false);
}

void SkClipStackDevice::replaceClip(const SkIRect& rect) {
    SkRect deviceRect = SkMatrixPriv::MapRect(this->globalToDevice(), SkRect::Make(rect));
    fClipStack.replaceClip(deviceRect, /*doAA=*/false);
}

bool SkClipStackDevice::isClipAntiAliased() const {
    SkClipStack::B2TIter        iter(fClipStack);
    const SkClipStack::Element* element;

    while ((element = iter.next()) != nullptr) {
        if (element->isAA()) {
            return true;
        }
    }
    return false;
}

bool SkClipStackDevice::isClipWideOpen() const {
    return fClipStack.quickContains(SkRect::MakeIWH(this->width(), this->height()));
}

bool SkClipStackDevice::isClipEmpty() const {
    return fClipStack.isEmpty(SkIRect::MakeWH(this->width(), this->height()));
}

bool SkClipStackDevice::isClipRect() const {
    if (this->isClipWideOpen()) {
        return true;
    } else if (this->isClipEmpty()) {
        return false;
    }

    SkClipStack::BoundsType boundType;
    bool isIntersectionOfRects;
    SkRect bounds;
    fClipStack.getBounds(&bounds, &boundType, &isIntersectionOfRects);
    return isIntersectionOfRects && boundType == SkClipStack::kNormal_BoundsType;
}

void SkClipStackDevice::android_utils_clipAsRgn(SkRegion* rgn) const {
    SkClipStack::BoundsType boundType;
    bool isIntersectionOfRects;
    SkRect bounds;
    fClipStack.getBounds(&bounds, &boundType, &isIntersectionOfRects);
    if (isIntersectionOfRects && SkClipStack::kNormal_BoundsType == boundType) {
        rgn->setRect(bounds.round());
    } else {
        SkRegion boundsRgn({0, 0, this->width(), this->height()});
        SkPath tmpPath;

        *rgn = boundsRgn;
        SkClipStack::B2TIter iter(fClipStack);
        while (auto elem = iter.next()) {
            tmpPath.rewind();
            elem->asDeviceSpacePath(&tmpPath);
            SkRegion tmpRgn;
            tmpRgn.setPath(tmpPath, boundsRgn);
            if (elem->isReplaceOp()) {
                // All replace elements are rectangles
                // TODO: SkClipStack can be simplified to be I,D,R ops now, which means element
                // iteration can be from top of the stack to the most recent replace element.
                // When that's done, this loop will be simplifiable.
                rgn->setRect(elem->getDeviceSpaceRect().round());
            } else {
                rgn->op(tmpRgn, static_cast<SkRegion::Op>(elem->getOp()));
            }
        }
    }
}

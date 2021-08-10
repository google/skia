/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkClipStackDevice.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"

SkIRect SkClipStackDevice::onDevClipBounds() const {
    SkIRect r = fClipStack.bounds(this->imageInfo().bounds()).roundOut();
    if (!r.isEmpty()) {
        SkASSERT(this->imageInfo().bounds().contains(r));
    }
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkClipStackDevice::onSave() {
    fClipStack.save();
}

void SkClipStackDevice::onRestore() {
    fClipStack.restore();
}

void SkClipStackDevice::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
    fClipStack.clipRect(rect, this->localToDevice(), op, aa);
}

void SkClipStackDevice::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    fClipStack.clipRRect(rrect, this->localToDevice(), op, aa);
}

void SkClipStackDevice::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    fClipStack.clipPath(path, this->localToDevice(), op, aa);
}

void SkClipStackDevice::onClipShader(sk_sp<SkShader> shader) {
    fClipStack.clipShader(std::move(shader));
}

void SkClipStackDevice::onClipRegion(const SkRegion& rgn, SkClipOp op) {
    SkIPoint origin = this->getOrigin();
    SkRegion tmp;
    SkPath path;
    rgn.getBoundaryPath(&path);
    path.transform(SkMatrix::Translate(-origin));
    fClipStack.clipPath(path, SkMatrix::I(), op, false);
}

void SkClipStackDevice::onReplaceClip(const SkIRect& rect) {
    SkRect deviceRect = SkMatrixPriv::MapRect(this->globalToDevice(), SkRect::Make(rect));
    fClipStack.replaceClip(deviceRect, /*doAA=*/false);
}

bool SkClipStackDevice::onClipIsAA() const {
    SkClipStack::B2TIter        iter(fClipStack);
    const SkClipStack::Element* element;

    while ((element = iter.next()) != nullptr) {
        if (element->isAA()) {
            return true;
        }
    }
    return false;
}

bool SkClipStackDevice::onClipIsWideOpen() const {
    return fClipStack.quickContains(SkRect::MakeIWH(this->width(), this->height()));
}

void SkClipStackDevice::onAsRgnClip(SkRegion* rgn) const {
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

SkBaseDevice::ClipType SkClipStackDevice::onGetClipType() const {
    if (fClipStack.isWideOpen()) {
        return ClipType::kRect;
    }
    if (fClipStack.isEmpty(SkIRect::MakeWH(this->width(), this->height()))) {
        return ClipType::kEmpty;
    } else {
        SkClipStack::BoundsType boundType;
        bool isIntersectionOfRects;
        SkRect bounds;
        fClipStack.getBounds(&bounds, &boundType, &isIntersectionOfRects);
        if (isIntersectionOfRects && SkClipStack::kNormal_BoundsType == boundType) {
            return ClipType::kRect;
        } else {
            return ClipType::kComplex;
        }
    }
}

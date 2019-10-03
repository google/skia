/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkClipStackDevice.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"

SkIRect SkClipStackDevice::devClipBounds() const {
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
    fClipStack.clipRect(rect, this->ctm(), op, aa);
}

void SkClipStackDevice::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    fClipStack.clipRRect(rrect, this->ctm(), op, aa);
}

void SkClipStackDevice::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    fClipStack.clipPath(path, this->ctm(), op, aa);
}

void SkClipStackDevice::onClipRegion(const SkRegion& rgn, SkClipOp op) {
    SkIPoint origin = this->getOrigin();
    SkRegion tmp;
    const SkRegion* ptr = &rgn;
    if (origin.fX | origin.fY) {
        // translate from "global/canvas" coordinates to relative to this device
        rgn.translate(-origin.fX, -origin.fY, &tmp);
        ptr = &tmp;
    }
    fClipStack.clipDevRect(ptr->getBounds(), op);
}

void SkClipStackDevice::onSetDeviceClipRestriction(SkIRect* clipRestriction) {
    if (clipRestriction->isEmpty()) {
        fClipStack.setDeviceClipRestriction(*clipRestriction);
    } else {
        SkIPoint origin = this->getOrigin();
        SkIRect rect = clipRestriction->makeOffset(-origin);
        fClipStack.setDeviceClipRestriction(rect);
        fClipStack.clipDevRect(rect, SkClipOp::kIntersect);
    }
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

void SkClipStackDevice::onAsRgnClip(SkRegion* rgn) const {
    SkClipStack::BoundsType boundType;
    bool isIntersectionOfRects;
    SkRect bounds;
    fClipStack.getBounds(&bounds, &boundType, &isIntersectionOfRects);
    if (isIntersectionOfRects && SkClipStack::kNormal_BoundsType == boundType) {
        rgn->setRect(bounds.round());
    } else {
        SkPath path;
        fClipStack.asPath(&path);
        rgn->setPath(path, SkRegion(SkIRect::MakeWH(this->width(), this->height())));
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

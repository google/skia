/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkClipStackDevice.h"

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
        SkIRect rect = clipRestriction->makeOffset(-origin.x(), -origin.y());
        fClipStack.setDeviceClipRestriction(rect);
        fClipStack.clipDevRect(rect, SkClipOp::kIntersect);
    }
}

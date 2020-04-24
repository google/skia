/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/xform/SkXform.h"

static std::atomic<uint32_t> gGenID{1};
Xform::GenID Xform::NextGenID() {
    return gGenID++;
}

#ifdef SK_DEBUG
void Xform::debugValidate() const {
    if (this->isCached() && fParent) {
        SkASSERT(fParent->isCached());
    }
    for (auto c : fChildren) {
        SkASSERT(c->parent() == this);
        c->debugValidate();
    }
}
#endif

void Xform::setParent(sk_sp<Xform> parent) {
    if (parent == fParent) {
        return;
    }

    if (fParent) {
        fParent->internalRemoveChild(this);
    }
    if (parent) {
        parent->internalAddChild(this);
    }
    fParent = std::move(parent);

    // Potentially we could skip this if knew that our old and new parents
    // were both cached, and they started us in the same state...
    // For now, we conservatively always inval
    this->invalidateCaches();

    this->debugValidate();
}

void Xform::internalAddChild(Xform* child) {
    SkASSERT(fChildren.find(child) < 0);
    fChildren.push_back(child);
}

void Xform::internalRemoveChild(Xform* child) {
    int index = fChildren.find(child);
    SkASSERT(index >= 0);
    fChildren.removeShuffle(index);
}

void Xform::invalidateCaches() {
    fGenID = 0;
    if (this->isCached()) {
        this->internalInvalidateCaches();
        for (auto c : fChildren) {
            c->invalidateCaches();
        }
    }
}

void Xform::visit(XformResolver* resolver) {
    this->onVisit(resolver);
}

void Xform::setCache(const SkMatrix& ctm, sk_sp<ClipCache> clip) {
    fCTM = ctm;
    fClip = std::move(clip);
    fGenID = NextGenID();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void MatrixXF::onVisit(XformResolver* resolver) {
    resolver->concat(fLocalMatrix);
}

void ClipXF::onVisit(XformResolver* resolver) {
    resolver->clipRect(fRect, fOp);
}

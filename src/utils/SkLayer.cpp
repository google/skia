
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkLayer.h"
#include "SkCanvas.h"

//#define DEBUG_DRAW_LAYER_BOUNDS
//#define DEBUG_TRACK_NEW_DELETE

#ifdef DEBUG_TRACK_NEW_DELETE
    static int gLayerAllocCount;
#endif

///////////////////////////////////////////////////////////////////////////////

SkLayer::SkLayer() {
    fParent = NULL;
    m_opacity = SK_Scalar1;
    m_size.set(0, 0);
    m_position.set(0, 0);
    m_anchorPoint.set(SK_ScalarHalf, SK_ScalarHalf);

    fMatrix.reset();
    fChildrenMatrix.reset();
    fFlags = 0;

#ifdef DEBUG_TRACK_NEW_DELETE
    gLayerAllocCount += 1;
    SkDebugf("SkLayer new:    %d\n", gLayerAllocCount);
#endif
}

SkLayer::SkLayer(const SkLayer& src) : INHERITED() {
    fParent = NULL;
    m_opacity = src.m_opacity;
    m_size = src.m_size;
    m_position = src.m_position;
    m_anchorPoint = src.m_anchorPoint;

    fMatrix = src.fMatrix;
    fChildrenMatrix = src.fChildrenMatrix;
    fFlags = src.fFlags;

#ifdef DEBUG_TRACK_NEW_DELETE
    gLayerAllocCount += 1;
    SkDebugf("SkLayer copy:   %d\n", gLayerAllocCount);
#endif
}

SkLayer::~SkLayer() {
    this->removeChildren();

#ifdef DEBUG_TRACK_NEW_DELETE
    gLayerAllocCount -= 1;
    SkDebugf("SkLayer delete: %d\n", gLayerAllocCount);
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool SkLayer::isInheritFromRootTransform() const {
    return (fFlags & kInheritFromRootTransform_Flag) != 0;
}

void SkLayer::setInheritFromRootTransform(bool doInherit) {
    if (doInherit) {
        fFlags |= kInheritFromRootTransform_Flag;
    } else {
        fFlags &= ~kInheritFromRootTransform_Flag;
    }
}

void SkLayer::setMatrix(const SkMatrix& matrix) {
    fMatrix = matrix;
}

void SkLayer::setChildrenMatrix(const SkMatrix& matrix) {
    fChildrenMatrix = matrix;
}

///////////////////////////////////////////////////////////////////////////////

int SkLayer::countChildren() const {
    return m_children.count();
}

SkLayer* SkLayer::getChild(int index) const {
    if ((unsigned)index < (unsigned)m_children.count()) {
        SkASSERT(m_children[index]->fParent == this);
        return m_children[index];
    }
    return NULL;
}

SkLayer* SkLayer::addChild(SkLayer* child) {
    SkASSERT(this != child);
    child->ref();
    child->detachFromParent();
    SkASSERT(child->fParent == NULL);
    child->fParent = this;

    *m_children.append() = child;
    return child;
}

void SkLayer::detachFromParent() {
    if (fParent) {
        int index = fParent->m_children.find(this);
        SkASSERT(index >= 0);
        fParent->m_children.remove(index);
        fParent = NULL;
        this->unref();  // this call might delete us
    }
}

void SkLayer::removeChildren() {
    int count = m_children.count();
    for (int i = 0; i < count; i++) {
        SkLayer* child = m_children[i];
        SkASSERT(child->fParent == this);
        child->fParent = NULL;  // in case it has more than one owner
        child->unref();
    }
    m_children.reset();
}

SkLayer* SkLayer::getRootLayer() const {
    const SkLayer* root = this;
    while (root->fParent != NULL) {
        root = root->fParent;
    }
    return const_cast<SkLayer*>(root);
}

///////////////////////////////////////////////////////////////////////////////

void SkLayer::getLocalTransform(SkMatrix* matrix) const {
    matrix->setTranslate(m_position.fX, m_position.fY);

    SkScalar tx = SkScalarMul(m_anchorPoint.fX, m_size.width());
    SkScalar ty = SkScalarMul(m_anchorPoint.fY, m_size.height());
    matrix->preTranslate(tx, ty);
    matrix->preConcat(this->getMatrix());
    matrix->preTranslate(-tx, -ty);
}

void SkLayer::localToGlobal(SkMatrix* matrix) const {
    this->getLocalTransform(matrix);

    if (this->isInheritFromRootTransform()) {
        matrix->postConcat(this->getRootLayer()->getMatrix());
        return;
    }

    const SkLayer* layer = this;
    while (layer->fParent != NULL) {
        layer = layer->fParent;

        SkMatrix tmp;
        layer->getLocalTransform(&tmp);
        tmp.preConcat(layer->getChildrenMatrix());
        matrix->postConcat(tmp);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkLayer::onDraw(SkCanvas*, SkScalar opacity) {
//    SkDebugf("----- no onDraw for %p\n", this);
}

#include "SkString.h"

void SkLayer::draw(SkCanvas* canvas, SkScalar opacity) {
#if 0
    SkString str1, str2;
 //   this->getMatrix().toDumpString(&str1);
 //   this->getChildrenMatrix().toDumpString(&str2);
    SkDebugf("--- drawlayer %p opacity %g size [%g %g] pos [%g %g] matrix %s children %s\n",
             this, opacity * this->getOpacity(), m_size.width(), m_size.height(),
             m_position.fX, m_position.fY, str1.c_str(), str2.c_str());
#endif

    opacity = SkScalarMul(opacity, this->getOpacity());
    if (opacity <= 0) {
//        SkDebugf("---- abort drawing %p opacity %g\n", this, opacity);
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);

    // apply our local transform
    {
        SkMatrix tmp;
        this->getLocalTransform(&tmp);
        if (this->isInheritFromRootTransform()) {
            // should we also apply the root's childrenMatrix?
            canvas->setMatrix(getRootLayer()->getMatrix());
        }
        canvas->concat(tmp);
    }

    this->onDraw(canvas, opacity);

#ifdef DEBUG_DRAW_LAYER_BOUNDS
    {
        SkRect r = SkRect::MakeSize(this->getSize());
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(SkIntToScalar(2));
        p.setColor(0xFFFF44DD);
        canvas->drawRect(r, p);
        canvas->drawLine(r.fLeft, r.fTop, r.fRight, r.fBottom, p);
        canvas->drawLine(r.fLeft, r.fBottom, r.fRight, r.fTop, p);
    }
#endif

    int count = this->countChildren();
    if (count > 0) {
        canvas->concat(this->getChildrenMatrix());
        for (int i = 0; i < count; i++) {
            this->getChild(i)->draw(canvas, opacity);
        }
    }
}

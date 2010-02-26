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

#ifdef DEBUG_TRACK_NEW_DELETE
    gLayerAllocCount += 1;
    SkDebugf("SkLayer new:    %d\n", gLayerAllocCount);
#endif
}

SkLayer::SkLayer(const SkLayer& src) {
    fParent = NULL;
    m_opacity = src.m_opacity;
    m_size = src.m_size;
    m_position = src.m_position;
    m_anchorPoint = src.m_anchorPoint;

    fMatrix = src.fMatrix;
    fChildrenMatrix = src.fChildrenMatrix;

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
        return m_children[index];
    }
    return NULL;
}

SkLayer* SkLayer::addChild(SkLayer* child) {
    child->ref();
    if (child->fParent) {
        child->fParent->removeChild(child);
    }
    SkASSERT(child->fParent == NULL);
    child->fParent = this;

    *m_children.append() = child;
    return child;
}

bool SkLayer::removeChild(SkLayer* child) {
    int index = m_children.find(child);
    if (index >= 0) {
        SkASSERT(child->fParent == this);
        child->fParent = NULL;
        child->unref();
        m_children.remove(index);
        return true;
    }
    return false;
}

void SkLayer::removeChildren() {
    m_children.unrefAll();
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
        canvas->translate(m_position.fX, m_position.fY);
        
        SkScalar tx = SkScalarMul(m_anchorPoint.fX, m_size.width());
        SkScalar ty = SkScalarMul(m_anchorPoint.fY, m_size.height());
        canvas->translate(tx, ty);
        canvas->concat(this->getMatrix());
        canvas->translate(-tx, -ty);
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


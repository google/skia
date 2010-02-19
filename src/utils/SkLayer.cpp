#include "SkLayer.h"
#include "SkCanvas.h"

SkLayer::SkLayer() {
    m_opacity = 1;
    m_size.set(0, 0);
    m_position.set(0, 0);
    m_anchorPoint.set(0.5, 0.5);

    fMatrix.reset();
    fChildrenMatrix.reset();
}

SkLayer::SkLayer(const SkLayer& src) {
    m_opacity = src.m_opacity;
    m_size = src.m_size;
    m_position = src.m_position;
    m_anchorPoint = src.m_anchorPoint;

    fMatrix = src.fMatrix;
    fChildrenMatrix = src.fChildrenMatrix;
}

SkLayer::~SkLayer() {
    this->removeChildren();
}

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
    *m_children.append() = child;
    return child;
}

void SkLayer::removeChildren() {
    m_children.unrefAll();
    m_children.reset();
}

///////////////////////////////////////////////////////////////////////////////

void SkLayer::setMatrix(const SkMatrix& matrix) {
    fMatrix = matrix;
}

void SkLayer::setChildrenMatrix(const SkMatrix& matrix) {
    fChildrenMatrix = matrix;
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
    if (opacity <= 0 || this->getSize().isEmpty()) {
#if 0
        SkDebugf("---- abort drawing %p opacity %g size [%g %g]\n",
                 this, opacity, m_size.width(), m_size.height());
#endif
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);

    // update the matrix
    {
        SkScalar tx = m_position.fX;
        SkScalar ty = m_position.fY;
        canvas->translate(tx, ty);
        
        // now apply our matrix about the anchorPoint
        tx = SkScalarMul(m_anchorPoint.fX, m_size.width());
        ty = SkScalarMul(m_anchorPoint.fY, m_size.height());
        canvas->translate(tx, ty);
        canvas->concat(this->getMatrix());
        canvas->translate(-tx, -ty);
    }

    this->onDraw(canvas, opacity);

    int count = this->countChildren();
    if (count > 0) {
        canvas->concat(this->getChildrenMatrix());
        for (int i = 0; i < count; i++) {
            this->getChild(i)->draw(canvas, opacity);
        }
    }
}


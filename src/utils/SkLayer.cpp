#include "SkLayer.h"
#include "SkCanvas.h"

SkLayer::SkLayer() {
    m_doRotation = false;
    m_isFixed = false;
    m_backgroundColorSet = false;

    m_angleTransform = 0;
    m_opacity = 1;

    m_size.set(0, 0);
    m_position.set(0, 0);
    m_translation.set(0, 0);
    m_anchorPoint.set(0.5, 0.5);
    m_scale.set(1, 1);

    m_backgroundColor = 0;

    fMatrix.reset();
    fChildrenMatrix.reset();
}

SkLayer::SkLayer(const SkLayer& src) {
    m_doRotation = src.m_doRotation;
    m_isFixed = src.m_isFixed;
    m_backgroundColorSet = src.m_backgroundColorSet;

    m_angleTransform = src.m_angleTransform;
    m_opacity = src.m_opacity;
    m_size = src.m_size;
    m_position = src.m_position;
    m_translation = src.m_translation;
    m_anchorPoint = src.m_anchorPoint;
    m_scale = src.m_scale;

    m_fixedLeft = src.m_fixedLeft;
    m_fixedTop = src.m_fixedTop;
    m_fixedRight = src.m_fixedRight;
    m_fixedBottom = src.m_fixedBottom;

    m_backgroundColor = src.m_backgroundColor;

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

void SkLayer::onSetupCanvas(SkCanvas* canvas, SkScalar, const SkRect*) {
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

void SkLayer::onDraw(SkCanvas*, SkScalar opacity, const SkRect* viewport) {}

void SkLayer::draw(SkCanvas* canvas, SkScalar opacity, const SkRect* viewport) {
#if 0
    SkDebugf("--- drawlayer %p anchor [%g %g] scale [%g %g]\n", this, m_anchorPoint.fX, m_anchorPoint.fY,
             m_scale.fX, m_scale.fY);
#endif

    opacity = SkScalarMul(opacity, this->getOpacity());
    if (opacity <= 0 || this->getSize().isEmpty()) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, false);
    canvas->save(SkCanvas::kMatrix_SaveFlag);

    this->onSetupCanvas(canvas, opacity, viewport);
    this->onDraw(canvas, opacity, viewport);

    int count = this->countChildren();
    if (count > 0) {
        canvas->concat(this->getChildrenMatrix());
        for (int i = 0; i < count; i++) {
            this->getChild(i)->draw(canvas, opacity, viewport);
        }
    }
}


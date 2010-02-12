#include "SkLayer.h"

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



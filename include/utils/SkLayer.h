/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkLayer_DEFINED
#define SkLayer_DEFINED

#include "SkRefCnt.h"
#include "SkTDArray.h"
#include "SkColor.h"
#include "SkMatrix.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkSize.h"

class SkCanvas;
class SkPicture;


struct SkLength {
  enum SkLengthType { Undefined, Auto, Relative, Percent, Fixed, Static, Intrinsic, MinIntrinsic };
  SkLengthType type;
  SkScalar value;
  SkLength() {
    type = Undefined;
    value = 0;
  }
  bool defined() const {
    if (type == Undefined)
      return false;
    return true;
  }
  float calcFloatValue(float max) const {
    switch (type) {
      case Percent:
        return (max * value) / 100.0f;
      case Fixed:
        return value;
      default:
        return value;
    }
  }
};

class SkLayer : public SkRefCnt {

public:
    SkLayer();
    SkLayer(const SkLayer&);
    virtual ~SkLayer();

    // deprecated
    void setTranslation(SkScalar x, SkScalar y) { m_translation.set(x, y); }
    void setRotation(SkScalar a) { m_angleTransform = a; m_doRotation = true; }
    void setScale(SkScalar x, SkScalar y) { m_scale.set(x, y); }
    SkPoint position() const { return m_position; }
    SkPoint translation() const { return m_translation; }
    SkSize  size() const { return m_size; }
    SkRect  bounds() const {
        SkRect rect;
        rect.set(m_position.fX, m_position.fY,
                 m_position.fX + m_size.width(),
                 m_position.fY + m_size.height());
        rect.offset(m_translation.fX, m_translation.fY);
        return rect;
    }
    
    const SkSize& getSize() const { return m_size; }
    void setSize(SkScalar w, SkScalar h) { m_size.set(w, h); }

    SkScalar getOpacity() const { return m_opacity; }
    void setOpacity(SkScalar opacity) { m_opacity = opacity; }

    const SkPoint& getPosition() const { return m_position; }
    void setPosition(SkScalar x, SkScalar y) { m_position.set(x, y); }

    const SkPoint& getAnchorPoint() const { return m_anchorPoint; }
    void setAnchorPoint(SkScalar x, SkScalar y) { m_anchorPoint.set(x, y); }

    virtual void setBackgroundColor(SkColor color) { m_backgroundColor = color; m_backgroundColorSet = true; }

    void setFixedPosition(SkLength left, SkLength top, SkLength right, SkLength bottom) {
      m_fixedLeft = left;
      m_fixedTop = top;
      m_fixedRight = right;
      m_fixedBottom = bottom;
      m_isFixed = true;
    }

    const SkMatrix& getMatrix() const { return fMatrix; }
    void setMatrix(const SkMatrix&);

    const SkMatrix& getChildrenMatrix() const { return fChildrenMatrix; }
    void setChildrenMatrix(const SkMatrix&);

    // children

    int countChildren() const;
    SkLayer* getChild(int index) const;
    SkLayer* addChild(SkLayer* child);
    void removeChildren();

    // paint method

    virtual void draw(SkCanvas*, SkScalar opacity, const SkRect* viewPort);

protected:
    virtual void onSetupCanvas(SkCanvas*, SkScalar opacity, const SkRect*);
    virtual void onDraw(SkCanvas*, SkScalar opacity, const SkRect* viewPort);

private:
    SkMatrix    fMatrix;
    SkMatrix    fChildrenMatrix;

public:

    bool m_doRotation;
    bool m_isFixed;
    bool m_backgroundColorSet;

    // layers properties

    SkScalar m_angleTransform;
    SkScalar m_opacity;

    SkSize m_size;
    SkPoint m_position;
    SkPoint m_translation;
    SkPoint m_anchorPoint;
    SkPoint m_scale;

    SkLength m_fixedLeft;
    SkLength m_fixedTop;
    SkLength m_fixedRight;
    SkLength m_fixedBottom;

    SkColor m_backgroundColor;

    SkTDArray<SkLayer*> m_children;
};

#endif

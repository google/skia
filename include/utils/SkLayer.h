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

class SkLayer : public SkRefCnt {

public:
    SkLayer();
    SkLayer(const SkLayer&);
    virtual ~SkLayer();

    SkScalar getOpacity() const { return m_opacity; }
    const SkSize& getSize() const { return m_size; }
    const SkPoint& getPosition() const { return m_position; }
    const SkPoint& getAnchorPoint() const { return m_anchorPoint; }
    const SkMatrix& getMatrix() const { return fMatrix; }
    const SkMatrix& getChildrenMatrix() const { return fChildrenMatrix; }

    SkScalar getWidth() const { return m_size.width(); }
    SkScalar getHeight() const { return m_size.height(); }

    void setOpacity(SkScalar opacity) { m_opacity = opacity; }
    void setSize(SkScalar w, SkScalar h) { m_size.set(w, h); }
    void setPosition(SkScalar x, SkScalar y) { m_position.set(x, y); }
    void setAnchorPoint(SkScalar x, SkScalar y) { m_anchorPoint.set(x, y); }
    void setMatrix(const SkMatrix&);
    void setChildrenMatrix(const SkMatrix&);

    // children

    int countChildren() const;
    SkLayer* getChild(int index) const;
    SkLayer* addChild(SkLayer* child);
    void removeChildren();

    // paint method

    void draw(SkCanvas*, SkScalar opacity);
    void draw(SkCanvas* canvas) {
        this->draw(canvas, SK_Scalar1);
    }

protected:
    virtual void onDraw(SkCanvas*, SkScalar opacity);

private:
    SkScalar    m_opacity;
    SkSize      m_size;
    SkPoint     m_position;
    SkPoint     m_anchorPoint;
    SkMatrix    fMatrix;
    SkMatrix    fChildrenMatrix;

    SkTDArray<SkLayer*> m_children;
};

#endif

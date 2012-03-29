
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRectShape_DEFINED
#define SkRectShape_DEFINED

#include "SkShape.h"
#include "SkPaint.h"
#include "SkSize.h"

class SkPaintShape : public SkShape {
public:
    SkPaintShape();

    SkPaint& paint() { return fPaint; }
    const SkPaint& paint() const { return fPaint; }

protected:
    SkPaintShape(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    
private:
    SkPaint fPaint;
    
    typedef SkShape INHERITED;
};

class SkRectShape : public SkPaintShape {
public:
    SkRectShape();
    
    void setRect(const SkRect&);
    void setOval(const SkRect&);
    void setCircle(SkScalar x, SkScalar y, SkScalar radius);
    void setRRect(const SkRect&, SkScalar rx, SkScalar ry);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRectShape)

protected:
    SkRectShape(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    // overrides
    virtual void onDraw(SkCanvas*);

private:
    SkRect  fBounds;
    SkSize  fRadii;

    typedef SkPaintShape INHERITED;
};

#endif

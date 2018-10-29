/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SGCanvas_DEFINED
#define SGCanvas_DEFINED

#include "SkClipOp.h"
#include "SkMatrix.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRRect.h"

class SkImage;
class SkPaint;
class SkPath;
class SkTextBlob;

class SGCanvas {
public:
    virtual ~SGCanvas() {}

    void restoreToCount(int);

    void clipRect(const SkRect& rect, bool aa = false) {
        this->clipRect(rect, SkClipOp::kIntersect, aa);
    }
    void clipRect(const SkRect&, SkClipOp, bool aa);
    void clipRRect(const SkRRect&, SkClipOp, bool aa);
    void clipPath(const SkPath&, bool aa);
    void clipPath(const SkPath&, SkClipOp, bool aa);

    void translate(float x, float y);
    void concat(const SkMatrix& m) { this->onConcat(m); }

    virtual int getSaveCount() const = 0;
    virtual void save() = 0;
    virtual void saveLayer(const SkRect&, const SkPaint*) = 0;
    virtual void restore() = 0;

    virtual void drawPaint(const SkPaint&) = 0;
    virtual void drawPath(const SkPath&, const SkPaint&) = 0;
    virtual void drawRect(const SkRect&, const SkPaint&) = 0;
    virtual void drawRRect(const SkRRect&, const SkPaint&) = 0;

    void drawImage(const SkImage*, float x, float y, const SkPaint*);
    void drawImage(sk_sp<SkImage>, float x, float y, const SkPaint*);

    void drawTextBlob(SkTextBlob* blob, const SkPaint& paint) {
        this->onDrawTextBlob(blob, paint);
    }
    void drawTextBlob(SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint&);
    void drawTextBlob(sk_sp<SkTextBlob> blob, SkScalar x, SkScalar y, const SkPaint&);

protected:
    virtual void onConcat(const SkMatrix&) = 0;
    virtual void onDrawImage(const SkImage*, float x, float y, const SkPaint*) = 0;
    virtual void onClipPath(const SkPath&, SkClipOp, bool aa) = 0;
    virtual void onDrawTextBlob(SkTextBlob*, const SkPaint&) = 0;
};

class SGAutoCanvasRestore {
public:
    SGAutoCanvasRestore(SGCanvas* canvas, bool doSave) : fCanvas(canvas) {
        fSaveCount = canvas->getSaveCount();
        if (doSave) {
            canvas->save();
        }
    }

    ~SGAutoCanvasRestore() {
        fCanvas->restoreToCount(fSaveCount);
    }

private:
    SGCanvas*   fCanvas;
    int         fSaveCount;
};

#endif


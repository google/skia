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

class SkPaint;
class SkPath;
class SkTextBlob;

class SGCanvas {
public:
    virtual ~SGCanvas() {}

    void restoreToCount(int);

    void clipPath(const SkPath&, bool aa);
    void clipPath(const SkPath&, SkClipOp, bool aa);

    virtual int getSaveCount() const = 0;
    virtual void save() = 0;
    virtual void saveLayer(const SkRect&, const SkPaint*) = 0;
    virtual void restore() = 0;

    virtual void concat(const SkMatrix&) = 0;

    virtual void drawPaint(const SkPaint&) = 0;
    virtual void drawPath(const SkPath&, const SkPaint&) = 0;
    virtual void drawRect(const SkRect&, const SkPaint&) = 0;
    virtual void drawTextBlob(SkTextBlob*, const SkPaint&) = 0;

protected:
    virtual void onClipPath(const SkPath&, SkClipOp, bool aa) = 0;
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


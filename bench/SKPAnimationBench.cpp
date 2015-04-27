/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SKPAnimationBench.h"
#include "SkCommandLineFlags.h"
#include "SkMultiPictureDraw.h"
#include "SkSurface.h"

SKPAnimationBench::SKPAnimationBench(const char* name, const SkPicture* pic,
                                     const SkIRect& clip, SkMatrix animationMatrix, int steps)
    : INHERITED(name, pic, clip, 1.0, false)
    , fSteps(steps)
    , fAnimationMatrix(animationMatrix)
    , fName(name) {
    fUniqueName.printf("%s_animation", name);
}

const char* SKPAnimationBench::onGetName() {
    return fName.c_str();
}

const char* SKPAnimationBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

void SKPAnimationBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    INHERITED::onPerCanvasPreDraw(canvas);
    SkIRect bounds;
    SkAssertResult(canvas->getClipDeviceBounds(&bounds));

    fCenter.set((bounds.fRight - bounds.fLeft) / 2.0f,
                (bounds.fBottom - bounds.fTop) / 2.0f);
}

void SKPAnimationBench::drawPicture() {
    SkMatrix frameMatrix = SkMatrix::MakeTrans(-fCenter.fX, -fCenter.fY);
    frameMatrix.postConcat(fAnimationMatrix);
    SkMatrix reverseTranslate = SkMatrix::MakeTrans(fCenter.fX, fCenter.fY);
    frameMatrix.postConcat(reverseTranslate);

    SkMatrix currentMatrix = frameMatrix;
    for (int i = 0; i < fSteps; i++) {
        for (int j = 0; j < this->tileRects().count(); ++j) {
            SkMatrix trans = SkMatrix::MakeTrans(-1.f * this->tileRects()[j].fLeft,
                                                 -1.f * this->tileRects()[j].fTop);
            SkMatrix tileMatrix = currentMatrix;
            tileMatrix.postConcat(trans);
            this->surfaces()[j]->getCanvas()->drawPicture(this->picture(), &tileMatrix, NULL);
        }

        for (int j = 0; j < this->tileRects().count(); ++j) {
           this->surfaces()[j]->getCanvas()->flush();
        }
        currentMatrix.postConcat(frameMatrix);
    }
}

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottySlide.h"

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "Skotty.h"

SkottySlide::SkottySlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

void SkottySlide::load(SkScalar, SkScalar) {
    fAnimation  = skotty::Animation::MakeFromFile(fPath.c_str());
    fTimeBase   = 0; // force a time reset

    if (fAnimation) {
        fAnimation->setShowInval(fShowAnimationInval);
        SkDebugf("loaded Bodymovin animation v: %s, size: [%f %f], fr: %f\n",
                 fAnimation->version().c_str(),
                 fAnimation->size().width(),
                 fAnimation->size().height(),
                 fAnimation->frameRate());
    } else {
        SkDebugf("failed to load Bodymovin animation: %s\n", fPath.c_str());
    }
}

void SkottySlide::unload() {
    fAnimation.reset();
}

SkISize SkottySlide::getDimensions() const {
    return fAnimation? fAnimation->size().toCeil() : SkISize::Make(0, 0);
}

void SkottySlide::draw(SkCanvas* canvas) {
    if (fAnimation) {
        SkAutoCanvasRestore acr(canvas, true);
        const SkRect dstR = SkRect::Make(canvas->imageInfo().bounds());
        fAnimation->render(canvas, &dstR);
    }
}

bool SkottySlide::animate(const SkAnimTimer& timer) {
    if (fTimeBase == 0) {
        // Reset the animation time.
        fTimeBase = timer.msec();
    }

    if (fAnimation) {
        auto t = timer.msec() - fTimeBase;
        fAnimation->animationTick(t);
    }
    return true;
}

bool SkottySlide::onChar(SkUnichar c) {
    switch (c) {
    case 'I':
        if (fAnimation) {
            fShowAnimationInval = !fShowAnimationInval;
            fAnimation->setShowInval(fShowAnimationInval);
        }
        break;
    default:
        break;
    }

    return INHERITED::onChar(c);
}

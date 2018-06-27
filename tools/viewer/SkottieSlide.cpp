/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieSlide.h"

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "Skottie.h"

SkottieSlide::SkottieSlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

void SkottieSlide::load(SkScalar w, SkScalar h) {
    fAnimation = skottie::Animation::MakeFromFile(fPath.c_str());
    fWinSize   = SkSize::Make(w, h);
    fTimeBase  = 0; // force a time reset

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

void SkottieSlide::unload() {
    fAnimation.reset();
}

SkISize SkottieSlide::getDimensions() const {
    // We always scale to fill the window.
    return fWinSize.toCeil();
}

void SkottieSlide::draw(SkCanvas* canvas) {
    if (fAnimation) {
        SkAutoCanvasRestore acr(canvas, true);
        const auto dstR = SkRect::MakeSize(fWinSize);
        fAnimation->render(canvas, &dstR);
    }
}

bool SkottieSlide::animate(const SkAnimTimer& timer) {
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

bool SkottieSlide::onChar(SkUnichar c) {
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

bool SkottieSlide::onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state, uint32_t) {
    switch (state) {
    case sk_app::Window::kUp_InputState:
        fShowAnimationInval = !fShowAnimationInval;
        fAnimation->setShowInval(fShowAnimationInval);
        break;
    default:
        break;
    }

    return false;
}

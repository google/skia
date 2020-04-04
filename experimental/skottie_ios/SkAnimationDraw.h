// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkAnimationDraw_DEFINED
#define SkAnimationDraw_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

#include "modules/skottie/include/Skottie.h"

class SkAnimationDraw {
public:
    SkAnimationDraw() = default;
    ~SkAnimationDraw() = default;

    explicit operator bool() const { return fAnimation != nullptr; }

    void draw(SkSize size, SkCanvas* canvas) {
        if (size.width() != fSize.width() || size.height() != fSize.height()) {
            // Cache the current matrix; change only if size changes.
            if (fAnimationSize.width() > 0 && fAnimationSize.height() > 0) {
                float scale = std::min(size.width() / fAnimationSize.width(),
                                       size.height() / fAnimationSize.height());
                fMatrix.setScaleTranslate(
                        scale, scale,
                        (size.width()  - fAnimationSize.width()  * scale) * 0.5f,
                        (size.height() - fAnimationSize.height() * scale) * 0.5f);
            } else {
                fMatrix = SkMatrix();
            }
            fSize = size;
        }
        canvas->concat(fMatrix);
        SkRect rect = {0, 0, fAnimationSize.width(), fAnimationSize.height()};
        canvas->drawRect(rect, SkPaint(SkColors::kWhite));
        fAnimation->render(canvas);
    }

    void load(const void* data, size_t length) {
        skottie::Animation::Builder builder;
        fAnimation = builder.make((const char*)data, (size_t)length);
        fSize = {0, 0};
        fAnimationSize = fAnimation ? fAnimation->size() : SkSize{0, 0};
    }

    void seek(double time) { if (fAnimation) { fAnimation->seekFrameTime(time, nullptr); } }

    float duration() { return fAnimation ? fAnimation->duration() : 0; }

    SkSize size() { return fAnimationSize; }

private:
    sk_sp<skottie::Animation> fAnimation; // owner
    SkSize fSize;
    SkSize fAnimationSize;
    SkMatrix fMatrix;

    SkAnimationDraw(const SkAnimationDraw&) = delete;
    SkAnimationDraw& operator=(const SkAnimationDraw&) = delete;
};
#endif  // SkAnimationDraw_DEFINED

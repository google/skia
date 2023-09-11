// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkottieViewController.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "modules/skottie/include/Skottie.h"
#include "src/base/SkTime.h"

#include <cmath>

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

class SkTimeKeeper {
private:
    double fStartTime = 0; // used when running
    float fAnimationMoment = 0; // when paused.
    float fDuration = 0;
    bool fPaused = false;
    bool fStopAtEnd = false;

public:
    void setStopAtEnd(bool s) { fStopAtEnd = s; }

    float currentTime() {
        if (0 == fDuration) {
            return 0;
        }
        if (fPaused) {
            return fAnimationMoment;
        }
        double time = 1e-9 * (SkTime::GetNSecs() - fStartTime);
        if (fStopAtEnd && time >= fDuration) {
            fPaused = true;
            fAnimationMoment = fDuration;
            return fAnimationMoment;
        }
        return std::fmod(time, fDuration);
    }

    void setDuration(float d) {
        fDuration = d;
        fStartTime = SkTime::GetNSecs();
        fAnimationMoment = 0;
    }

    bool paused() const { return fPaused; }

    float duration() const { return fDuration; }

    void seek(float seconds) {
        if (fPaused) {
            fAnimationMoment = std::fmod(seconds, fDuration);
        } else {
            fStartTime = SkTime::GetNSecs() - 1e9 * seconds;
        }
    }

    void togglePaused() {
        if (fPaused) {
            double offset = (fAnimationMoment >= fDuration) ? 0 : -1e9 * fAnimationMoment;
            fStartTime = SkTime::GetNSecs() + offset;
            fPaused = false;
        } else {
            fAnimationMoment = this->currentTime();
            fPaused = true;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

@implementation SkottieViewController {
    SkAnimationDraw fDraw;
    SkTimeKeeper fClock;
}

- (bool)loadAnimation:(NSData*) data {
    fDraw.load((const void*)[data bytes], (size_t)[data length]);
    fClock.setDuration(fDraw.duration());
    return (bool)fDraw;
}

- (void)setStopAtEnd:(bool)stop { fClock.setStopAtEnd(stop); }

- (float)animationDurationSeconds { return fClock.duration(); }

- (float)currentTime { return fDraw ? fClock.currentTime() : 0; }

- (void)seek:(float)seconds {
    if (fDraw) {
        fClock.seek(seconds);
    }
}

- (CGSize)size { return {(CGFloat)fDraw.size().width(), (CGFloat)fDraw.size().height()}; }

- (bool)togglePaused {
    fClock.togglePaused();
    return fClock.paused();
}

- (bool)isPaused { return fClock.paused(); }

- (void)draw:(CGRect)rect toCanvas:(SkCanvas*)canvas atSize:(CGSize)size {
    // TODO(halcanary): Use the rect and the InvalidationController to speed up rendering.
    if (rect.size.width > 0 && rect.size.height > 0 && fDraw && canvas) {
        if (!fClock.paused()) {
            fDraw.seek(fClock.currentTime());
        }
        fDraw.draw(SkSize{(float)size.width, (float)size.height}, canvas);
    }
}

@end

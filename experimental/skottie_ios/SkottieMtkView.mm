 //Copyright 2019 Google LLC.
 //Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/skottie_ios/SkottieMtkView.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "modules/skottie/include/Skottie.h"

#include "experimental/skottie_ios/SkMetalViewBridge.h"

@implementation SkottieMtkView {
    sk_sp<skottie::Animation> fAnimation; // owner
    CGSize fSize;
    double fStartTime; // used when running
    float fAnimationMoment; // when paused.
    SkMatrix fMatrix;
    SkSize fAnimationSize;
    bool fPaused;
}

-(void)dealloc {
    fAnimation = nullptr;
    [super dealloc];
}

- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    // TODO(halcanary): Use the rect and the InvalidationController to speed up rendering.
    if (!fAnimation || ![[self currentDrawable] texture] || ![self grContext]) {
        return;
    }
    CGSize size = [self drawableSize];
    if (size.width != fSize.width || size.height != fSize.height) {
        // Cache the current matrix; change only if size changes.
        if (fAnimationSize.width() > 0 && fAnimationSize.height() > 0) {
            float scale = std::min(size.width / fAnimationSize.width(),
                                   size.height / fAnimationSize.height());
            fMatrix.setScaleTranslate(
                    scale, scale,
                    ((float)size.width  - fAnimationSize.width()  * scale) * 0.5f,
                    ((float)size.height - fAnimationSize.height() * scale) * 0.5f);
        } else {
            fMatrix = SkMatrix();
        }
        fSize = size;
    }
    SkPaint whitePaint(SkColors::kWhite);
    if (!fPaused) {
        fAnimation->seekFrameTime([self currentTime], nullptr);
    }
    sk_sp<SkSurface> surface = SkMtkViewToSurface(self, [self grContext]);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->concat(fMatrix);
    canvas->drawRect(SkRect{0, 0, fAnimationSize.width(), fAnimationSize.height()}, whitePaint);
    fAnimation->render(canvas);
    surface->flush();
    surface = nullptr;
    [[self currentDrawable] present];
}

- (BOOL)loadAnimation:(NSData*) data {
    skottie::Animation::Builder builder;
    fAnimation = builder.make((const char*)[data bytes], (size_t)[data length]);
    fStartTime = SkTime::GetNSecs();
    fAnimationMoment = 0;
    fSize = {0, 0};
    fAnimationSize = fAnimation ? fAnimation->size() : SkSize{0, 0};
    return fAnimation != nullptr;
}

- (float)animationDurationSeconds {
    return fAnimation ? fAnimation->duration() : 0;
}

- (float)currentTime {
    if (!fAnimation) {
        return 0;
    }
    if (fPaused) {
        return fAnimationMoment;
    }
    return std::fmod(1e-9 * (SkTime::GetNSecs() - fStartTime), fAnimation->duration());
}

- (void)seek:(float)seconds {
    if (fAnimation) {
        if (fPaused) {
            fAnimationMoment = std::fmod(seconds, fAnimation->duration());
            fAnimation->seekFrameTime(fAnimationMoment);
        } else {
            fStartTime = SkTime::GetNSecs() - 1e9 * seconds;
        }
    }
}

- (CGSize)size { return {(CGFloat)fAnimationSize.width(), (CGFloat)fAnimationSize.height()}; }

- (BOOL)togglePaused {
    if (fPaused) {
        fStartTime = SkTime::GetNSecs() - 1e9 * fAnimationMoment;
    } else {
        fAnimationMoment = [self currentTime];
    }
    fPaused = !fPaused;
    return fPaused;
}
@end

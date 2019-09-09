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
    double fStartTime;
    double fTime;
    SkMatrix fMatrix;
    SkRect fAnimRect;
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
        float aw = fAnimRect.right(),
              ah = fAnimRect.bottom();
        if (aw > 0 && ah > 0) {
            float scale = std::min(size.width / aw, size.height / ah);
            fMatrix.setScaleTranslate(scale,
                                      scale,
                                      ((float)size.width  - aw * scale) * 0.5f,
                                      ((float)size.height - ah * scale) * 0.5f);
        } else {
            fMatrix = SkMatrix();
        }
        fSize = size;
    }
    SkPaint whitePaint(SkColors::kWhite);
    if (!fPaused) {
        fTime = SkTime::GetNSecs();
        fAnimation->seekFrameTime(std::fmod(1e-9 * (fTime - fStartTime),
                                            fAnimation->duration()), nullptr);
    }
    sk_sp<SkSurface> surface = SkMtkViewToSurface(self, [self grContext]);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->concat(fMatrix);
    canvas->drawRect(fAnimRect, whitePaint);
    fAnimation->render(canvas);
    surface->flush();
    surface = nullptr;
    [[self currentDrawable] present];
}

- (BOOL)loadAnimation:(NSData*) data {
    skottie::Animation::Builder builder;
    fAnimation = builder.make((const char*)[data bytes], (size_t)[data length]);
    fTime = fStartTime = SkTime::GetNSecs();
    fSize = {0, 0};
    fAnimRect = fAnimation ? SkRect::MakeSize(fAnimation->size()) : SkRect{0, 0, 0, 0};
    return fAnimation != nullptr;
}

- (CGSize)size {
    if (fAnimation) {
        const SkSize& s = fAnimation->size();
        return {(CGFloat)s.width(), (CGFloat)s.height()};
    }
    return {0, 0};
}

- (BOOL)togglePaused {
    fPaused = !fPaused;
    if (!fPaused) {
        fStartTime += (SkTime::GetNSecs() - fTime);
    }
    return fPaused;
}
@end

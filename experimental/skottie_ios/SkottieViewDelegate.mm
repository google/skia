// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/skottie_ios/SkottieViewDelegate.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "modules/skottie/include/Skottie.h"

#include "experimental/skottie_ios/SkMetalViewBridge.h"

@implementation SkottieViewDelegate {
    sk_sp<skottie::Animation> fAnimation; // owner
    double fStartTime;
    double fTime;
    float fScale;
    SkPoint fOffset;
    bool fPaused;
}

-(void)dealloc {
    fAnimation = nullptr;
    [super dealloc];
}

- (BOOL)loadAnimation:(NSData*) data {
    skottie::Animation::Builder builder;
    fAnimation = builder.make((const char*)[data bytes], (size_t)[data length]);
    fTime = fStartTime = SkTime::GetNSecs();
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

- (void)drawInMTKView:(nonnull MTKView*)view {
    if (![[view currentDrawable] texture]) {
        return;
    }
    if (![self grContext]) {
        NSLog(@"error: no context");
        return;
    }
    if (0 == fScale) {
        [self mtkView:view drawableSizeWillChange:[view drawableSize]];
    }
    if (!fPaused && fAnimation) {
        fTime = SkTime::GetNSecs();
        fAnimation->seekFrameTime(std::fmod(1e-9 * (fTime - fStartTime),
                                            fAnimation->duration()), nullptr);
    }
    sk_sp<SkSurface> surface = SkMtkViewToSurface(view, [self grContext]);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate(fOffset.x(), fOffset.y());
    canvas->scale(fScale, fScale);
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawRect(SkRect::MakeSize(fAnimation->size()), SkPaint(SkColors::kWhite));
    fAnimation->render(canvas);
    surface->flush();
    [[view currentDrawable] present];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    if (fAnimation) {
        const SkSize& animSize = fAnimation->size();
        fScale = std::min(size.width  / animSize.width(),
                          size.height / animSize.height());
        fOffset = {((float)size.width  - animSize.width()  * fScale) * 0.5f,
                   ((float)size.height - animSize.height() * fScale) * 0.5f};
    }
}
@end

////////////////////////////////////////////////////////////////////////////////


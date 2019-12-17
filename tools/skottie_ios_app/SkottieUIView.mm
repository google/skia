// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkottieUIView.h"

#include "tools/skottie_ios_app/SkAnimationDraw.h"
#include "tools/skottie_ios_app/SkTimeKeeper.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/utils/mac/SkCGUtils.h"

#include "modules/skottie/include/Skottie.h"

@implementation SkottieUIView {
    SkAnimationDraw fDraw;
    SkTimeKeeper fClock;
    SkBitmap fBackBuffer;
}

- (void)drawRect:(CGRect)rect {
    double next = fClock.paused() ? 0 : (1.0 / 30.0) + SkTime::GetNSecs() * 1e-9;
    [super drawRect:rect];
    // TODO(halcanary): Use the rect and the InvalidationController to speed up rendering.
    if (rect.size.width > 0 && rect.size.height > 0 && fDraw) {
        CGSize size = [self bounds].size;
        SkISize iSize = {(int)size.width, (int)size.height};

        if (fBackBuffer.drawsNothing() || iSize != fBackBuffer.dimensions()) {
            fBackBuffer.allocN32Pixels(iSize.fWidth, iSize.fHeight);
        }
        fBackBuffer.eraseColor(SK_ColorTRANSPARENT);
        {
            SkCanvas canvas(fBackBuffer);
            if (!fClock.paused()) {
                fDraw.seek(fClock.currentTime());
            }
            fDraw.draw({(float)size.width, (float)size.height}, &canvas);
        }
        SkCGDrawBitmap(UIGraphicsGetCurrentContext(), fBackBuffer, 0, 0);
    }
    if (next) {
        [NSTimer scheduledTimerWithTimeInterval:std::max(0.0, next - SkTime::GetNSecs() * 1e-9)
                 target:self
                 selector:@selector(setNeedsDisplay)
                 userInfo:nil
                 repeats:NO];
    }
}

- (BOOL)loadAnimation:(NSData*) data {
    fDraw.load((const void*)[data bytes], (size_t)[data length]);
    fClock.setDuration(fDraw.duration());
    [self setNeedsDisplay];
    return (bool)fDraw;
}

- (void)setStopAtEnd:(BOOL)stop{ fClock.setStopAtEnd(stop); }

- (float)animationDurationSeconds { return fClock.duration(); }

- (float)currentTime { return fDraw ? fClock.currentTime() : 0; }

- (void)seek:(float)seconds {
    if (fDraw) {
        fClock.seek(seconds);
        [self setNeedsDisplay];
    }
}

- (CGSize)size { return {(CGFloat)fDraw.size().width(), (CGFloat)fDraw.size().height()}; }

- (BOOL)togglePaused {
    fClock.togglePaused();
    [self setNeedsDisplay];
    return fClock.paused();
}

- (BOOL)isPaused { return fClock.paused(); }
@end

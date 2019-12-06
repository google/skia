// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/skottie_ios/SkottieUiView.h"

#include "experimental/skottie_ios/SkAnimationDraw.h"
#include "experimental/skottie_ios/SkTimeKeeper.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/utils/mac/SkCGUtils.h"

#include "modules/skottie/include/Skottie.h"

@implementation SkottieUiView {
    SkAnimationDraw fDraw;
    SkTimeKeeper fClock;
    SkBitmap fBackBuffer;
    double fLastDraw;
}

- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    // TODO(halcanary): Use the rect and the InvalidationController to speed up rendering.
    if (!fDraw) {
        return;
    }
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
        fDraw.draw(SkSize{(float)size.width, (float)size.height}, &canvas);
    }

    SkCGDrawPixmap(UIGraphicsGetCurrentContext(), fBackBuffer.pixmap(), 0, 0);
    double now = SkTime::GetNSecs() * 1e-9;
    if (!fClock.paused()) {
        static const double kInterval = 1.0 / 30.0;
        double next = std::min(kInterval, std::max(0.0, fLastDraw + kInterval - now));
        [NSTimer scheduledTimerWithTimeInterval:next
                 target:self
                 selector:@selector(setNeedsDisplay)
                 userInfo:nil
                 repeats:NO];
    }
    fLastDraw = now;
}

- (BOOL)loadAnimation:(NSData*) data {
    fDraw.load((const void*)[data bytes], (size_t)[data length]);
    fClock.setDuration(fDraw.duration());
    [self setNeedsDisplay];
    fLastDraw = 0;
    return (BOOL)fDraw;
}

- (void)setStopAtEnd:stop{ fClock.setStopAtEnd(stop); }

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

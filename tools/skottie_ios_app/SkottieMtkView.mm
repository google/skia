 //Copyright 2019 Google LLC.
 //Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkottieMtkView.h"

#include "tools/skottie_ios_app/SkAnimationDraw.h"
#include "tools/skottie_ios_app/SkTimeKeeper.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "modules/skottie/include/Skottie.h"

#include "tools/skottie_ios_app/SkMetalViewBridge.h"

@implementation SkottieMtkView {
    SkAnimationDraw fDraw;
    SkTimeKeeper fClock;
}

- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    // TODO(halcanary): Use the rect and the InvalidationController to speed up rendering.
    if (!fDraw || ![[self currentDrawable] texture] || ![self grContext]) {
        return;
    }
    CGSize size = [self drawableSize];
    if (!fClock.paused()) {
        fDraw.seek(fClock.currentTime());
    }
    sk_sp<SkSurface> surface = SkMtkViewToSurface(self, [self grContext]);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    fDraw.draw(SkSize{(float)size.width, (float)size.height}, surface->getCanvas());
    surface->flush();
    surface = nullptr;

    id<MTLCommandBuffer> commandBuffer = [[self queue] commandBuffer];
    [commandBuffer presentDrawable:[self currentDrawable]];
    [commandBuffer commit];
}

- (BOOL)loadAnimation:(NSData*) data {
    fDraw.load((const void*)[data bytes], (size_t)[data length]);
    fClock.setDuration(fDraw.duration());
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

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaMtkView.h"

#include "include/core/SkSurface.h"
#include "tools/skottie_ios_app/SkMetalViewBridge.h"

@implementation SkiaMtkView {
    id<MTLCommandQueue> fQueue;
    GrContext* fGrContext;
}

- (instancetype)initWithFrame:(CGRect)frameRect
                device:(id<MTLDevice>)mtlDevice
                queue:(id<MTLCommandQueue>)queue
                grDevice:(GrContext*)grContext {
    self = [super initWithFrame:frameRect device:mtlDevice];
    fQueue = queue;
    fGrContext = grContext;
    SkMtkViewConfigForSkia(self);
    return self;
}

- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    // TODO(halcanary): Use the rect and the InvalidationController to speed up rendering.
    SkiaViewController* viewController = [self controller];
    if (!viewController || ![[self currentDrawable] texture] || !fGrContext) {
        return;
    }
    CGSize size = [self drawableSize];
    sk_sp<SkSurface> surface = SkMtkViewToSurface(self, fGrContext);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    [viewController draw:rect toCanvas:surface->getCanvas() atSize:size];
    surface->flush();
    surface = nullptr;

    id<MTLCommandBuffer> commandBuffer = [fQueue commandBuffer];
    [commandBuffer presentDrawable:[self currentDrawable]];
    [commandBuffer commit];

    bool paused = [viewController isPaused];
    [self setEnableSetNeedsDisplay:paused];
    [self setPaused:paused];
}
@end

// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "tools/skottie_ios_app/SkMetalViewBridge.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

// A UIView that uses a Metal-backed SkSurface to draw.
@interface SkiaMtkView : MTKView
    @property (strong) SkiaViewController* controller;

    // Override of the MTKView interface.  Uses Skia+Metal to draw.
    - (void)drawRect:(CGRect)rect;

    // Required initializer.
    - (instancetype)initWithFrame:(CGRect)frameRect
                    device:(id<MTLDevice>)device
                    queue:(id<MTLCommandQueue>)queue
                    grDevice:(GrContext*)grContext;
@end

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

@interface SkiaMetalContext : SkiaContext
    @property (strong) id<MTLDevice> metalDevice;
    @property (strong) id<MTLCommandQueue> metalQueue;
    - (instancetype) init;
    - (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame;
    - (SkiaViewController*) getViewController:(UIView*)view;
@end

@implementation SkiaMetalContext {
    sk_sp<GrContext> fGrContext;
}

- (instancetype) init {
    self = [super init];
    [self setMetalDevice:MTLCreateSystemDefaultDevice()];
    if(![self metalDevice]) {
        NSLog(@"Metal is not supported on this device");
        return nil;
    }
    [self setMetalQueue:[[self metalDevice] newCommandQueue]];
    fGrContext = GrContext::MakeMetal((__bridge void*)[self metalDevice],
                                      (__bridge void*)[self metalQueue],
                                      GrContextOptions());

    if (!fGrContext) {
        NSLog(@"GrContext::MakeMetal failed");
        return nil;
    }
    return self;
}

- (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame {
    SkiaMtkView* skiaView = [[SkiaMtkView alloc] initWithFrame:frame
                                                 device:[self metalDevice]
                                                 queue:[self metalQueue]
                                                 grDevice:fGrContext.get()];
    [skiaView setPreferredFramesPerSecond:30];
    [skiaView setController:vc];
    return skiaView;
}

- (SkiaViewController*) getViewController:(UIView*)view {
    return [view isKindOfClass:[SkiaMtkView class]] ? [(SkiaMtkView*)view controller] : nil;
}
@end

SkiaContext* MakeSkiaMetalContext() { return [[SkiaMetalContext alloc] init]; }

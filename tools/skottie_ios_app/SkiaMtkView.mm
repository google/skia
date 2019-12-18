 //Copyright 2019 Google LLC.
 //Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaMtkView.h"

#include "include/core/SkSurface.h"
#include "tools/skottie_ios_app/SkMetalViewBridge.h"

@implementation SkiaMtkView
- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    // TODO(halcanary): Use the rect and the InvalidationController to speed up rendering.
    SkiaViewController* viewController = [self controller];
    if (!viewController || ![[self currentDrawable] texture] || ![self grContext]) {
        return;
    }
    CGSize size = [self drawableSize];
    sk_sp<SkSurface> surface = SkMtkViewToSurface(self, [self grContext]);
    if (!surface) {
        NSLog(@"error: no sksurface");
        return;
    }
    [viewController draw:rect toCanvas:surface->getCanvas() atSize:size];
    surface->flush();
    surface = nullptr;

    id<MTLCommandBuffer> commandBuffer = [[self queue] commandBuffer];
    [commandBuffer presentDrawable:[self currentDrawable]];
    [commandBuffer commit];
}
@end

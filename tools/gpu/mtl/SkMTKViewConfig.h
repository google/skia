#ifndef SkMTKViewConfig_DEFINED
#define SkMTKViewConfig_DEFINED

#include "include/core/SkTypes.h"

#import <MetalKit/MetalKit.h>

class GrContext;
class SkSurface;
template <typename T> class sk_sp;

SK_API sk_sp<GrContext> SkConfigureMTKView(MTKView*);
SK_API sk_sp<SkSurface> SkMTKViewSurface(MTKView*, GrContext*);

/*
Example Use:

    ////////////////
    // Renderer.h //
    /////////////////
    #import <MetalKit/MetalKit.h>
    @interface Renderer : NSObject <MTKViewDelegate>
    -(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
    @end

    /////////////////
    // Renderer.mm //
    /////////////////
    #import "Renderer.h"
    #include "include/gpu/GrContext.h"
    #include "tools/gpu/mtl/SkMTKViewConfig.h"
    #include "include/core/SkPaint.h"
    #include "include/core/SkSurface.h"
    #include "include/core/SkCanvas.h"
    #include "include/effects/SkGradientShader.h"
    #include "include/core/SkTime.h"
    @implementation Renderer {
        sk_sp<GrContext> fGrContext;
        SkPaint fPaint;
        CGSize fSize;
    }
    -(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view {
        self = [super init];
        fGrContext = SkConfigureMTKView(view);
        view.delegate = self;
        return self;
    }
    - (void)drawInMTKView:(nonnull MTKView *)view {
        if (!fPaint.getShader()) {
            SkColor4f colors[2] = {SkColors::kGreen, SkColors::kMagenta};
            SkPoint points[2] = {{0, -1024}, {0, 1024}};
            fPaint.setShader(SkGradientShader::MakeLinear(
                    points, colors, nullptr, nullptr, 2,
                    SkTileMode::kClamp, 0, nullptr));
        }
        float time = (float)(180 * 1e-9 * SkTime::GetNSecs());
        // peform as much work as possible before creating surface.
        if (auto surface = SkMTKViewSurface(view, fGrContext.get())) {
            SkCanvas* c = surface->getCanvas();
            c->translate(surface->width() * 0.5f, surface->height() * 0.5f);
            c->rotate(time);
            c->drawPaint(fPaint);
            // flush and present are necessary:
            surface->flush();
            [view.currentDrawable present];
        }
    }
    - (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
        fSize = size;
    }
    @end
*/
#endif  // SkMTKViewConfig_DEFINED

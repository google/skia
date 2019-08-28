#ifndef SkMTKViewConfig_DEFINED
#define SkMTKViewConfig_DEFINED

#include "include/core/SkTypes.h"

#import <MetalKit/MetalKit.h>

namespace skui { class ViewLayer; }
class GrContext;
template <typename T> class sk_sp;

SK_API void SkPaintMetalSurface(id<CAMetalDrawable>, id<MTLCommandQueue>, CGSize,
                                int sampleCount, GrContext*, skui::ViewLayer*);
SK_API sk_sp<GrContext> SkConfigureMTKView(MTKView* view, id<MTLCommandQueue>);

/*
Example use of these functions:

    /////////////////////////
    // SkMTKViewDelegate.h //
    /////////////////////////
    #import <MetalKit/MetalKit.h>
    namespace skui { class ViewLayer; }
    @interface SkMTKViewDelegate : NSObject<MTKViewDelegate>
    - (nonnull instancetype)init:(nonnull MTKView *)view
                        withView:(nonnull skui::ViewLayer*)delegate;
    @end

    //////////////////////////
    // SkMTKViewDelegate.mm //
    //////////////////////////
    #include "include/gpu/GrContext.h"
    #include "tools/skui/ViewLayer.h"
    #import "tools/gpu/mtl/SkMTKViewConfig.h"
    #import "SkMTKViewDelegate.h"
    @implementation SkMTKViewDelegate {
        id <MTLCommandQueue> fCommandQueue;
        sk_sp<GrContext> fGrContext;
        skui::ViewLayer* fDelegate;
    }
    - (nonnull instancetype)init:(nonnull MTKView *)view
                        withView:(nonnull skui::ViewLayer*)delegate {
        self = [super init];
        self->fCommandQueue = [view.device newCommandQueue];
        self->fGrContext = SkConfigureMTKView(view, self->fCommandQueue);
        self->fDelegate = delegate;
        view.delegate = self;
        [self mtkView:view drawableSizeWillChange:view.bounds.size];
        return self;
    }
    - (void)drawInMTKView:(nonnull MTKView *)view {
        SkPaintMetalSurface(view.currentDrawable, fCommandQueue, view.drawableSize,
                            (int)view.sampleCount, fGrContext.get(), fDelegate);
    }
    - (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
        fDelegate->onResize((int)size.width, (int)size.height);
    }
    @end

    //////////////////
    // MySkiaView.h //
    //////////////////
    #include "tools/skui/ViewLayer.h"
    class MySkiaView : public skui::ViewLayer {
        void onPaint(SkSurface*) override;
    };

    ///////////////////
    // MySkiaView.cc //
    ///////////////////
    #include "MySkiaView.h"
    #include "include/core/SkSurface.h"
    #include "include/core/SkCanvas.h"
    void MySkiaView::onPaint(SkSurface* s) {
        s->getCanvas()->drawColor(SK_ColorCYAN, SkBlendMode::kSrc);
    }

    ///////////////////////////////
    // ExampleUIViewController.h //
    ///////////////////////////////
    #import <UIKit/UIKit.h>
    @interface ExampleUIViewController : UIViewController
    @end

    ////////////////////////////////
    // ExampleUIViewController.mm //
    ////////////////////////////////
    #import "ExampleUIViewController.h"
    @implementation ExampleUIViewController {
        MySkiaView fSkiaView;
        SkMTKViewDelegate* fDelegate;
    }
    - (void)viewDidLoad {
        [super viewDidLoad];
        if (MTKView* view = (MTKView *)self.view) {
            view.device = MTLCreateSystemDefaultDevice();
            if (view.device) {
                fDelegate = [[SkMTKViewDelegate alloc] init:view
                                                   withView:&fSkiaView];
                view.delegate = fDelegate;
                [fDelegate mtkView:view drawableSizeWillChange:view.bounds.size];
            }
        }
    }
    @end
*/
#endif  // SkMTKViewConfig_DEFINED

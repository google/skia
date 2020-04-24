
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "tools/ToolUtils.h"
#include "tools/sk_app/GLWindowContext.h"
#include "tools/sk_app/ios/WindowContextFactory_ios.h"

#import <OpenGLES/ES3/gl.h>
#import <UIKit/UIKit.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::IOSWindowInfo;
using sk_app::GLWindowContext;

@interface RasterView : MainView
@end

@implementation RasterView
+ (Class) layerClass {
    return [CAEAGLLayer class];
}
@end

namespace {

// TODO: This still uses GL to handle the update rather than using a purely raster backend,
// for historical reasons. Writing a pure raster backend would be better in the long run.

class RasterWindowContext_ios : public GLWindowContext {
public:
    RasterWindowContext_ios(const IOSWindowInfo&, const DisplayParams&);

    ~RasterWindowContext_ios() override;

    sk_sp<SkSurface> getBackbufferSurface() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    sk_app::Window_ios*  fWindow;
    UIViewController*    fViewController;
    RasterView*          fRasterView;
    EAGLContext*         fGLContext;
    GLuint               fFramebuffer;
    GLuint               fRenderbuffer;
    sk_sp<SkSurface>     fBackbufferSurface;

    typedef GLWindowContext INHERITED;
};

RasterWindowContext_ios::RasterWindowContext_ios(const IOSWindowInfo& info,
                                                 const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow)
    , fViewController(info.fViewController)
    , fGLContext(nil) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

RasterWindowContext_ios::~RasterWindowContext_ios() {
    this->destroyContext();
    [fRasterView removeFromSuperview];
    [fRasterView release];
}

sk_sp<const GrGLInterface> RasterWindowContext_ios::onInitializeContext() {
    SkASSERT(nil != fViewController);
    SkASSERT(!fGLContext);

    CGRect frameRect = [fViewController.view frame];
    fRasterView = [[[RasterView alloc] initWithFrame:frameRect] initWithWindow:fWindow];
    [fViewController.view addSubview:fRasterView];

    fGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];

    if (!fGLContext)
    {
        SkDebugf("Could Not Create OpenGL ES Context\n");
        return nullptr;
    }

    if (![EAGLContext setCurrentContext:fGLContext]) {
        SkDebugf("Could Not Set OpenGL ES Context As Current\n");
        this->onDestroyContext();
        return nullptr;
    }

    // Set up EAGLLayer
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)fRasterView.layer;
    eaglLayer.drawableProperties = @{kEAGLDrawablePropertyRetainedBacking : @NO,
                                     kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8 };
    eaglLayer.opaque = YES;
    eaglLayer.frame = frameRect;
    eaglLayer.contentsGravity = kCAGravityTopLeft;

    // Set up framebuffer
    glGenFramebuffers(1, &fFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, fFramebuffer);

    glGenRenderbuffers(1, &fRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fRenderbuffer);

    [fGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        SkDebugf("Invalid Framebuffer\n");
        this->onDestroyContext();
        return nullptr;
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 255);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    fStencilBits = 8;
    fSampleCount = 1; // TODO: handle multisampling

    fWidth = fViewController.view.frame.size.width;
    fHeight = fViewController.view.frame.size.height;

    glViewport(0, 0, fWidth, fHeight);

    // make the offscreen image
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight, fDisplayParams.fColorType,
                                         kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info);
    return GrGLMakeNativeInterface();
}

void RasterWindowContext_ios::onDestroyContext() {
    glDeleteFramebuffers(1, &fFramebuffer);
    glDeleteRenderbuffers(1, &fRenderbuffer);
    [EAGLContext setCurrentContext:nil];
    [fGLContext release];
    fGLContext = nil;
}

sk_sp<SkSurface> RasterWindowContext_ios::getBackbufferSurface() {
    return fBackbufferSurface;
}

void RasterWindowContext_ios::onSwapBuffers() {
    if (fBackbufferSurface) {
        // We made/have an off-screen surface. Get the contents as an SkImage:
        sk_sp<SkImage> snapshot = fBackbufferSurface->makeImageSnapshot();

        sk_sp<SkSurface> gpuSurface = INHERITED::getBackbufferSurface();
        SkCanvas* gpuCanvas = gpuSurface->getCanvas();
        gpuCanvas->drawImage(snapshot, 0, 0);
        gpuCanvas->flush();
        glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
        [fGLContext presentRenderbuffer:GL_RENDERBUFFER];
    }
}

void RasterWindowContext_ios::resize(int w, int h) {
    // TODO: handle rotation
    // [fGLContext update];
     INHERITED::resize(w, h);
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeRasterForIOS(const IOSWindowInfo& info,
                                                const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_ios(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

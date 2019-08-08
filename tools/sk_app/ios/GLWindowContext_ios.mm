
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "tools/sk_app/GLWindowContext.h"
#include "tools/sk_app/ios/WindowContextFactory_ios.h"

#include <OpenGLES/ES3/gl.h>
#include <UIKit/UIKit.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::IOSWindowInfo;
using sk_app::GLWindowContext;

@interface GLView : MainView
@end

@implementation GLView
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}
@end

namespace {

class GLWindowContext_ios : public GLWindowContext {
public:
    GLWindowContext_ios(const IOSWindowInfo&, const DisplayParams&);

    ~GLWindowContext_ios() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    sk_app::Window_ios*  fWindow;
    UIViewController*    fViewController;
    GLView*              fGLView;
    EAGLContext*         fGLContext;
    GLuint                fFramebuffer;
    GLuint                fRenderbuffer;

    typedef GLWindowContext INHERITED;
};

GLWindowContext_ios::GLWindowContext_ios(const IOSWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow)
    , fViewController(info.fViewController)
    , fGLContext(nil) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_ios::~GLWindowContext_ios() {
    [fGLContext release];
    fGLContext = nil;
    [fGLView removeFromSuperview];
}

sk_sp<const GrGLInterface> GLWindowContext_ios::onInitializeContext() {
    SkASSERT(nil != fViewController);
    SkASSERT(!fGLContext);

    CGRect frameRect = [fViewController.view frame];
    fGLView = [[[GLView alloc] initWithFrame:frameRect] initWithWindow:fWindow];
    [fViewController.view addSubview:fGLView];

    fGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];

    if (!fGLContext)
    {
        SkDebugf("Could Not Create OpenGL ES Context\n");
        return nullptr;
    }

    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)fGLView.layer;

    eaglLayer.drawableProperties = @{kEAGLDrawablePropertyRetainedBacking : @NO,
                                     kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8 };
    eaglLayer.opaque = YES;

//        eaglLayer.drawableSize = frameRect.size;
    eaglLayer.frame = frameRect;

//        BOOL useVsync = fDisplayParams.fDisableVsync ? NO : YES;
//        eaglLayer.displaySyncEnabled = useVsync;  // TODO: need solution for 10.12 or lower
//        eaglLayer.layoutManager = [CAConstraintLayoutManager layoutManager];
//        eaglLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
    eaglLayer.contentsGravity = kCAGravityTopLeft;

    if (![EAGLContext setCurrentContext:fGLContext]) {
        return nullptr;
    }

    glGenFramebuffers(1, &fFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, fFramebuffer);

    glGenRenderbuffers(1, &fRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fRenderbuffer);

    [fGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return nullptr;
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 255);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    fStencilBits = 8;
    fSampleCount = 1; // for now

    fWidth = fViewController.view.frame.size.width;
    fHeight = fViewController.view.frame.size.height;

    glViewport(0, 0, fWidth, fHeight);

    return GrGLMakeNativeInterface();

//    if (!fGLContext) {
//        // set up pixel format
//        constexpr int kMaxAttributes = 18;
//        NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
//        int numAttributes = 0;
//        attributes[numAttributes++] = NSOpenGLPFAAccelerated;
//        attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
//        attributes[numAttributes++] = NSOpenGLPFADoubleBuffer;
//        attributes[numAttributes++] = NSOpenGLPFAOpenGLProfile;
//        attributes[numAttributes++] = NSOpenGLProfileVersion3_2Core;
//        attributes[numAttributes++] = NSOpenGLPFAColorSize;
//        attributes[numAttributes++] = 24;
//        attributes[numAttributes++] = NSOpenGLPFAAlphaSize;
//        attributes[numAttributes++] = 8;
//        attributes[numAttributes++] = NSOpenGLPFADepthSize;
//        attributes[numAttributes++] = 0;
//        attributes[numAttributes++] = NSOpenGLPFAStencilSize;
//        attributes[numAttributes++] = 8;
//        if (fDisplayParams.fMSAASampleCount > 1) {
//            attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
//            attributes[numAttributes++] = 1;
//            attributes[numAttributes++] = NSOpenGLPFASamples;
//            attributes[numAttributes++] = fDisplayParams.fMSAASampleCount;
//        } else {
//            attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
//            attributes[numAttributes++] = 0;
//        }
//        attributes[numAttributes++] = 0;
//        SkASSERT(numAttributes <= kMaxAttributes);
//
//        fPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
//        if (nil == fPixelFormat) {
//            return nullptr;
//        }
//
//        // create context
//        fGLContext = [[NSOpenGLContext alloc] initWithFormat:fPixelFormat shareContext:nil];
//        if (nil == fGLContext) {
//            [fPixelFormat release];
//            fPixelFormat = nil;
//            return nullptr;
//        }
//
//        // TODO: support Retina displays
//        [fMainView setWantsBestResolutionOpenGLSurface:NO];
//        [fGLContext setView:fMainView];
//    }
//
//    GLint swapInterval = fDisplayParams.fDisableVsync ? 0 : 1;
//    [fGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
//
//    // make context current
//    [fGLContext makeCurrentContext];
//
//    glClearStencil(0);
//    glClearColor(0, 0, 0, 255);
//    glStencilMask(0xffffffff);
//    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//
//    GLint stencilBits;
//    [fPixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
//    fStencilBits = stencilBits;
//    GLint sampleCount;
//    [fPixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
//    fSampleCount = sampleCount;
//    fSampleCount = SkTMax(fSampleCount, 1);
//
//    const NSRect viewportRect = [fMainView frame];
//    fWidth = viewportRect.size.width;
//    fHeight = viewportRect.size.height;
//
//    glViewport(0, 0, fWidth, fHeight);
//
//    return GrGLMakeNativeInterface();
}

void GLWindowContext_ios::onDestroyContext() {
    [fGLContext release];
    fGLContext = nil;
}

void GLWindowContext_ios::onSwapBuffers() {
//    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [fGLContext presentRenderbuffer:GL_RENDERBUFFER];
}

void GLWindowContext_ios::resize(int w, int h) {
//*** I guess we need this for rotation
//    [fGLContext update];
//    INHERITED::resize(w, h);
}


}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeGLForIOS(const IOSWindowInfo& info,
                                            const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GLWindowContext_ios(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

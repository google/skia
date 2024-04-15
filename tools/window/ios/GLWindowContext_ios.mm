
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "tools/window/GLWindowContext.h"
#include "tools/window/ios/WindowContextFactory_ios.h"
#include "include/gpu/ganesh/gl/ios/GrGLMakeIOSInterface.h"

#import <OpenGLES/ES3/gl.h>
#import <UIKit/UIKit.h>

using skwindow::DisplayParams;
using skwindow::IOSWindowInfo;
using skwindow::internal::GLWindowContext;

@interface GLView : MainView
@end

@implementation GLView
+ (Class) layerClass {
    return [CAEAGLLayer class];
}
@end

namespace {

class GLWindowContext_ios : public GLWindowContext {
public:
    GLWindowContext_ios(const IOSWindowInfo&, const DisplayParams&);

    ~GLWindowContext_ios() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    void onSwapBuffers() override;

    sk_app::Window_ios*  fWindow;
    UIViewController*    fViewController;
    GLView*              fGLView;
    EAGLContext*         fGLContext;
    GLuint               fFramebuffer;
    GLuint               fRenderbuffer;
};

GLWindowContext_ios::GLWindowContext_ios(const IOSWindowInfo& info, const DisplayParams& params)
        : GLWindowContext(params)
        , fWindow(info.fWindow)
        , fViewController(info.fViewController)
        , fGLContext(nil) {

    // iOS test apps currently ignore MSAA settings.

    this->initializeContext();
}

GLWindowContext_ios::~GLWindowContext_ios() {
    this->destroyContext();
    [fGLView removeFromSuperview];
    [fGLView release];
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

    if (![EAGLContext setCurrentContext:fGLContext]) {
        SkDebugf("Could Not Set OpenGL ES Context As Current\n");
        this->onDestroyContext();
        return nullptr;
    }

    // Set up EAGLLayer
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)fGLView.layer;
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

    return GrGLInterfaces::MakeIOS();
}

void GLWindowContext_ios::onDestroyContext() {
    glDeleteFramebuffers(1, &fFramebuffer);
    glDeleteRenderbuffers(1, &fRenderbuffer);
    [EAGLContext setCurrentContext:nil];
    [fGLContext release];
    fGLContext = nil;
}

void GLWindowContext_ios::onSwapBuffers() {
    glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
    [fGLContext presentRenderbuffer:GL_RENDERBUFFER];
}

void GLWindowContext_ios::resize(int w, int h) {
    GLWindowContext::resize(w, h);
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGLForIOS(const IOSWindowInfo& info,
                                            const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GLWindowContext_ios(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "SkSampleUIView.h"

//#define SKGL_CONFIG         kEAGLColorFormatRGB565
#define SKGL_CONFIG         kEAGLColorFormatRGBA8

#define FORCE_REDRAW

#include "SkCanvas.h"
#include "SkCGUtils.h"
#include "SkSurface.h"
#include "SampleApp.h"

#if SK_SUPPORT_GPU
//#define USE_GL_1
#define USE_GL_2

#include "gl/GrGLInterface.h"
#include "GrContext.h"
#include "SkGpuDevice.h"
#endif

class SkiOSDeviceManager : public SampleWindow::DeviceManager {
public:
    SkiOSDeviceManager(GLint layerFBO) {
#if SK_SUPPORT_GPU
        fCurContext = NULL;
        fCurIntf = NULL;
        fCurRenderTarget = NULL;
        fMSAASampleCount = 0;
        fLayerFBO = layerFBO;
#endif
        fBackend = SkOSWindow::kNone_BackEndType;
    }
    
    virtual ~SkiOSDeviceManager() {
#if SK_SUPPORT_GPU
        SkSafeUnref(fCurContext);
        SkSafeUnref(fCurIntf);
        SkSafeUnref(fCurRenderTarget);
#endif
    }
    
    void setUpBackend(SampleWindow* win, int msaaSampleCount) override {
        SkASSERT(SkOSWindow::kNone_BackEndType == fBackend);
        
        fBackend = SkOSWindow::kNone_BackEndType;
        
#if SK_SUPPORT_GPU
        switch (win->getDeviceType()) {
            // these two don't use GL
            case SampleWindow::kRaster_DeviceType:
            case SampleWindow::kPicture_DeviceType:
                break;
            // these guys use the native backend
            case SampleWindow::kGPU_DeviceType:
                fBackend = SkOSWindow::kNativeGL_BackEndType;
                break;
            default:
                SkASSERT(false);
                break;
        }
        SkOSWindow::AttachmentInfo info;
        bool result = win->attach(fBackend, msaaSampleCount, &info);
        if (!result) {
            SkDebugf("Failed to initialize GL");
            return;
        }
        fMSAASampleCount = msaaSampleCount;
        
        SkASSERT(NULL == fCurIntf);
        switch (win->getDeviceType()) {
            // these two don't use GL
            case SampleWindow::kRaster_DeviceType:
            case SampleWindow::kPicture_DeviceType:
                fCurIntf = NULL;
                break;
            case SampleWindow::kGPU_DeviceType:
                fCurIntf = GrGLCreateNativeInterface();
                break;
            default:
                SkASSERT(false);
                break;
        }
        
        SkASSERT(NULL == fCurContext);
        if (SkOSWindow::kNone_BackEndType != fBackend) {
            fCurContext = GrContext::Create(kOpenGL_GrBackend,
                                            (GrBackendContext) fCurIntf);
        }
        
        if ((NULL == fCurContext || NULL == fCurIntf) &&
            SkOSWindow::kNone_BackEndType != fBackend) {
            // We need some context and interface to see results if we're using a GL backend
            SkSafeUnref(fCurContext);
            SkSafeUnref(fCurIntf);
            SkDebugf("Failed to setup 3D");
            win->detach();
        }
#endif // SK_SUPPORT_GPU
        // call windowSizeChanged to create the render target
        this->windowSizeChanged(win);
    }
    
    void tearDownBackend(SampleWindow *win) override {
#if SK_SUPPORT_GPU
        SkSafeUnref(fCurContext);
        fCurContext = NULL;
        
        SkSafeUnref(fCurIntf);
        fCurIntf = NULL;
        
        SkSafeUnref(fCurRenderTarget);
        fCurRenderTarget = NULL;
#endif
        win->detach();
        fBackend = SampleWindow::kNone_BackEndType;
    }

    SkSurface* createSurface(SampleWindow::DeviceType dType, SampleWindow* win) override{
#if SK_SUPPORT_GPU
        if (SampleWindow::IsGpuDeviceType(dType) && fCurContext) {
            SkSurfaceProps props(win->getSurfaceProps());
            return SkSurface::NewRenderTargetDirect(fCurRenderTarget, &props);
        }
#endif
        return NULL;
    }

    virtual void publishCanvas(SampleWindow::DeviceType dType,
                               SkCanvas* canvas,
                               SampleWindow* win) override {
#if SK_SUPPORT_GPU
        if (NULL != fCurContext) {
            fCurContext->flush();
        }
#endif
        win->present();
    }
    
    void windowSizeChanged(SampleWindow* win) override {
#if SK_SUPPORT_GPU
        if (NULL != fCurContext) {
            SkOSWindow::AttachmentInfo info;

            win->attach(fBackend, fMSAASampleCount, &info);
            
            glBindFramebuffer(GL_FRAMEBUFFER, fLayerFBO);
            GrBackendRenderTargetDesc desc;
            desc.fWidth = SkScalarRoundToInt(win->width());
            desc.fHeight = SkScalarRoundToInt(win->height());
            desc.fConfig = kSkia8888_GrPixelConfig;
            desc.fRenderTargetHandle = fLayerFBO;
            desc.fSampleCnt = info.fSampleCount;
            desc.fStencilBits = info.fStencilBits;

            SkSafeUnref(fCurRenderTarget);
            fCurRenderTarget = fCurContext->wrapBackendRenderTarget(desc);
        }
#endif
    }
    
    GrContext* getGrContext() override {
#if SK_SUPPORT_GPU
        return fCurContext;
#else
        return NULL;
#endif
    }
    
    GrRenderTarget* getGrRenderTarget() override {
#if SK_SUPPORT_GPU
        return fCurRenderTarget;
#else
        return NULL;
#endif
    }
    
    bool isUsingGL() const { return SkOSWindow::kNone_BackEndType != fBackend; }
    
private:
   
#if SK_SUPPORT_GPU
    GrContext*              fCurContext;
    const GrGLInterface*    fCurIntf;
    GrRenderTarget*         fCurRenderTarget;
    int                     fMSAASampleCount;
    GLint                   fLayerFBO;
#endif
    
    SkOSWindow::SkBackEndTypes fBackend;
    
    typedef SampleWindow::DeviceManager INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
@implementation SkSampleUIView

@synthesize fTitle, fRasterLayer, fGLLayer;

#include "SkApplication.h"
#include "SkEvent.h"
#include "SkWindow.h"

struct FPSState {
    static const int FRAME_COUNT = 60;
    
    CFTimeInterval fNow0, fNow1;
    CFTimeInterval fTime0, fTime1, fTotalTime;
    int fFrameCounter;
    SkString str;
    FPSState() {
        fTime0 = fTime1 = fTotalTime = 0;
        fFrameCounter = 0;
    }
    
    void startDraw() {
        fNow0 = CACurrentMediaTime();
    }
    
    void endDraw() {
        fNow1 = CACurrentMediaTime();
    }
    
    void flush(SkOSWindow* hwnd) {
        CFTimeInterval now2 = CACurrentMediaTime();
        
        fTime0 += fNow1 - fNow0;
        fTime1 += now2 - fNow1;
        
        if (++fFrameCounter == FRAME_COUNT) {
            CFTimeInterval totalNow = CACurrentMediaTime();
            fTotalTime = totalNow - fTotalTime;
            
            //SkMSec ms0 = (int)(1000 * fTime0 / FRAME_COUNT);
            //SkMSec msTotal = (int)(1000 * fTotalTime / FRAME_COUNT);
            //str.printf(" ms: %d [%d], fps: %3.1f", msTotal, ms0,
            //           FRAME_COUNT / fTotalTime);
            str.printf(" fps:%3.1f", FRAME_COUNT / fTotalTime);
            hwnd->setTitle(NULL);
            fTotalTime = totalNow;
            fTime0 = fTime1 = 0;
            fFrameCounter = 0;
        }
    }
};

static FPSState gFPS;

#define FPS_StartDraw() gFPS.startDraw()
#define FPS_EndDraw()   gFPS.endDraw()
#define FPS_Flush(wind) gFPS.flush(wind)

///////////////////////////////////////////////////////////////////////////////

- (id)initWithDefaults {
    if (self = [super initWithDefaults]) {
        fRedrawRequestPending = false;
        fFPSState = new FPSState;
        
#ifdef USE_GL_1
        fGL.fContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#else
        fGL.fContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#endif
        
        if (!fGL.fContext || ![EAGLContext setCurrentContext:fGL.fContext])
        {
            [self release];
            return nil;
        }
        
        // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
        glGenFramebuffers(1, &fGL.fFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, fGL.fFramebuffer);
        
        glGenRenderbuffers(1, &fGL.fRenderbuffer);
        glGenRenderbuffers(1, &fGL.fStencilbuffer);
        
        glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fGL.fRenderbuffer);
        
        glBindRenderbuffer(GL_RENDERBUFFER, fGL.fStencilbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fGL.fStencilbuffer);
        
        self.fGLLayer = [CAEAGLLayer layer];
        fGLLayer.bounds = self.bounds;
        fGLLayer.anchorPoint = CGPointMake(0, 0);
        fGLLayer.opaque = TRUE;
        [self.layer addSublayer:fGLLayer];
        fGLLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                       [NSNumber numberWithBool:NO],
                                       kEAGLDrawablePropertyRetainedBacking,
                                       SKGL_CONFIG,
                                       kEAGLDrawablePropertyColorFormat,
                                       nil];
        
        self.fRasterLayer = [CALayer layer];
        fRasterLayer.anchorPoint = CGPointMake(0, 0);
        fRasterLayer.opaque = TRUE;
        [self.layer addSublayer:fRasterLayer];
        
        NSMutableDictionary *newActions = [[NSMutableDictionary alloc] initWithObjectsAndKeys:[NSNull null], @"onOrderIn",
                                           [NSNull null], @"onOrderOut",
                                           [NSNull null], @"sublayers",
                                           [NSNull null], @"contents",
                                           [NSNull null], @"bounds",
                                           nil];
        fGLLayer.actions = newActions;
        fRasterLayer.actions = newActions;
        [newActions release];
        
        fDevManager = new SkiOSDeviceManager(fGL.fFramebuffer);
        static char* kDummyArgv = const_cast<char*>("dummyExecutableName");
        fWind = new SampleWindow(self, 1, &kDummyArgv, fDevManager);

        fWind->resize(self.frame.size.width, self.frame.size.height,
                      kN32_SkColorType);
    }
    return self;
}

- (void)dealloc {
    delete fDevManager;
    delete fFPSState;
    self.fRasterLayer = nil;
    self.fGLLayer = nil;
    [fGL.fContext release];
    [super dealloc];
}

- (void)layoutSubviews {
    int W, H;
    
    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
    [fGL.fContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:fGLLayer];
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &fGL.fWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &fGL.fHeight);
    
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fStencilbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fGL.fWidth, fGL.fHeight);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    
    if (fDevManager->isUsingGL()) {
        W = fGL.fWidth;
        H = fGL.fHeight;
        CGRect rect = CGRectMake(0, 0, W, H);
        fGLLayer.bounds = rect;
    }
    else {
        CGRect rect = self.bounds;
        W = (int)CGRectGetWidth(rect);
        H = (int)CGRectGetHeight(rect);
        fRasterLayer.bounds = rect;
    }
    
    printf("---- layoutSubviews %d %d\n", W, H);
    fWind->resize(W, H);
    fWind->inval(NULL);
}

///////////////////////////////////////////////////////////////////////////////

- (void)drawWithCanvas:(SkCanvas*)canvas {
    fRedrawRequestPending = false;
    fFPSState->startDraw();
    fWind->draw(canvas);
    fFPSState->endDraw();
#ifdef FORCE_REDRAW
    fWind->inval(NULL);
#endif
    fFPSState->flush(fWind);
}

- (void)drawInGL {
    // This application only creates a single context which is already set current at this point.
    // This call is redundant, but needed if dealing with multiple contexts.
    [EAGLContext setCurrentContext:fGL.fContext];
    
    // This application only creates a single default framebuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple framebuffers.
    glBindFramebuffer(GL_FRAMEBUFFER, fGL.fFramebuffer);
    
    GLint scissorEnable;
    glGetIntegerv(GL_SCISSOR_TEST, &scissorEnable);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (scissorEnable) {
        glEnable(GL_SCISSOR_TEST);
    }
    glViewport(0, 0, fGL.fWidth, fGL.fHeight);
    
   
    SkAutoTUnref<SkSurface> surface(fWind->createSurface());
    SkCanvas* canvas = surface->getCanvas();

    // if we're not "retained", then we have to always redraw everything.
    // This call forces us to ignore the fDirtyRgn, and draw everywhere.
    // If we are "retained", we can skip this call (as the raster case does)
    fWind->forceInvalAll();

    [self drawWithCanvas:canvas];

    // This application only creates a single color renderbuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple renderbuffers.
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
    [fGL.fContext presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)drawInRaster {
    SkAutoTUnref<SkSurface> surface(fWind->createSurface());
    SkCanvas* canvas = surface->getCanvas();
    [self drawWithCanvas:canvas];
    CGImageRef cgimage = SkCreateCGImageRef(fWind->getBitmap());
    fRasterLayer.contents = (id)cgimage;
    CGImageRelease(cgimage);
}

- (void)forceRedraw {
    if (fDevManager->isUsingGL())
        [self drawInGL];
    else 
        [self drawInRaster];
}

///////////////////////////////////////////////////////////////////////////////

- (void)setSkTitle:(const char *)title {
    NSString* text = [NSString stringWithUTF8String:title];
    if ([text length] > 0)
        self.fTitle = text;
    
    if (fTitleItem && fTitle) {
        fTitleItem.title = [NSString stringWithFormat:@"%@%@", fTitle, 
                            [NSString stringWithUTF8String:fFPSState->str.c_str()]];
    }
}

- (void)postInvalWithRect:(const SkIRect*)r {
    if (!fRedrawRequestPending) {
        fRedrawRequestPending = true;
        bool gl = fDevManager->isUsingGL();
        [CATransaction begin];
        [CATransaction setAnimationDuration:0];
        fRasterLayer.hidden = gl;
        fGLLayer.hidden = !gl;
        [CATransaction commit];
        if (gl) {
            [self performSelector:@selector(drawInGL) withObject:nil afterDelay:0];
        }
        else {
            [self performSelector:@selector(drawInRaster) withObject:nil afterDelay:0];
            [self setNeedsDisplay];
        }
    }
}

- (void)getAttachmentInfo:(SkOSWindow::AttachmentInfo*)info {
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER,
                                 GL_RENDERBUFFER_STENCIL_SIZE,
                                 &info->fStencilBits);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER,
                                 GL_RENDERBUFFER_SAMPLES_APPLE,
                                 &info->fSampleCount);
}

@end

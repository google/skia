#import "SkSampleUIView.h"

//#define SKWIND_CONFIG       SkBitmap::kRGB_565_Config
#define SKWIND_CONFIG       SkBitmap::kARGB_8888_Config
#define SKGL_CONFIG         kEAGLColorFormatRGB565
//#define SKGL_CONFIG         kEAGLColorFormatRGBA8

#define FORCE_REDRAW

//#define USE_GL_1
#define USE_GL_2

#include "SkCanvas.h"
#include "GrContext.h"
#include "gl/GrGLInterface.h"
#include "SkGpuDevice.h"
#include "SkCGUtils.h"
class SkiOSDeviceManager : public SampleWindow::DeviceManager {
public:
    SkiOSDeviceManager() {
        fGrContext = NULL;
        fGrRenderTarget = NULL;
        usingGL = false;
    }
    virtual ~SkiOSDeviceManager() {
        SkSafeUnref(fGrContext);
        SkSafeUnref(fGrRenderTarget);
    }
    
    virtual void init(SampleWindow* win) {
        win->attach(kNativeGL_BackEndType);
        if (NULL == fGrContext) {
#ifdef USE_GL_1
            fGrContext = GrContext::Create(kOpenGL_Fixed_GrEngine, NULL);
#else
            fGrContext = GrContext::Create(kOpenGL_Shaders_GrEngine, NULL);
#endif
        }
        
        if (NULL == fGrContext) {
            SkDebugf("Failed to setup 3D");
            win->detachGL();
        }
    }        
    
    virtual bool supportsDeviceType(SampleWindow::DeviceType dType) {
        switch (dType) {
            case SampleWindow::kRaster_DeviceType:
            case SampleWindow::kPicture_DeviceType: // fallthru
                return true;
            case SampleWindow::kGPU_DeviceType:
                return NULL != fGrContext;
            default:
                return false;
        }
    }
    virtual bool prepareCanvas(SampleWindow::DeviceType dType,
                               SkCanvas* canvas,
                               SampleWindow* win) {
        if (SampleWindow::kGPU_DeviceType == dType) {
            canvas->setDevice(new SkGpuDevice(fGrContext, fGrRenderTarget))->unref();
            usingGL = true;
        }
        else {
            //The clip needs to be applied with a device attached to the canvas
            canvas->setBitmapDevice(win->getBitmap());
            usingGL = false;
        }
        return true;
    }
    virtual void publishCanvas(SampleWindow::DeviceType dType,
                               SkCanvas* canvas,
                               SampleWindow* win) {
        if (SampleWindow::kGPU_DeviceType == dType) {
            fGrContext->flush();
        }
        else {
            //CGContextRef cg = UIGraphicsGetCurrentContext();
            //SkCGDrawBitmap(cg, win->getBitmap(), 0, 0);
        }
        win->presentGL();
    }
    
    virtual void windowSizeChanged(SampleWindow* win) {
        if (fGrContext) {
            win->attach(kNativeGL_BackEndType);
            
            GrPlatformSurfaceDesc desc;
            desc.reset();
            desc.fSurfaceType = kRenderTarget_GrPlatformSurfaceType;
            desc.fWidth = SkScalarRound(win->width());
            desc.fHeight = SkScalarRound(win->height());
            desc.fConfig = kSkia8888_PM_GrPixelConfig;
            const GrGLInterface* gl = GrGLGetDefaultGLInterface();
            GrAssert(NULL != gl);
            GR_GL_GetIntegerv(gl, GR_GL_STENCIL_BITS, &desc.fStencilBits);
            GR_GL_GetIntegerv(gl, GR_GL_SAMPLES, &desc.fSampleCnt);
            GrGLint buffer;
            GR_GL_GetIntegerv(gl, GR_GL_FRAMEBUFFER_BINDING, &buffer);
            desc.fPlatformRenderTarget = buffer;
            
            SkSafeUnref(fGrRenderTarget);
            fGrRenderTarget = static_cast<GrRenderTarget*>(
                                                           fGrContext->createPlatformSurface(desc));
        }
    }
    
    bool isUsingGL() { return usingGL; }
    
    virtual GrContext* getGrContext() { return fGrContext; }

    virtual GrRenderTarget* getGrRenderTarget() SK_OVERRIDE {
        return fGrRenderTarget;
    }
private:
    bool usingGL;
    GrContext* fGrContext;
    GrRenderTarget* fGrRenderTarget;
};

////////////////////////////////////////////////////////////////////////////////
@implementation SkSampleUIView

@synthesize fTitle, fRasterLayer, fGLLayer;

#include "SkApplication.h"
#include "SkEvent.h"
#include "SkWindow.h"

#define kREDRAW_UIVIEW_GL "sk_redraw_uiview_gl_iOS"

extern bool gDoTraceDraw;
#define DO_TRACE_DRAW_MAX   100

struct FPSState {
    static const int FRAME_COUNT = 60;
    
    CFTimeInterval fNow0, fNow1;
    CFTimeInterval fTime0, fTime1, fTotalTime;
    int fFrameCounter;
    int fDrawCounter;
    SkString str;
    FPSState() {
        fTime0 = fTime1 = fTotalTime = 0;
        fFrameCounter = 0;
    }
    
    void startDraw() {
        fNow0 = CACurrentMediaTime();
        
        if (0 == fDrawCounter && false) {
            gDoTraceDraw = true;
            SkDebugf("\n");
        }
    }
    
    void endDraw() {
        fNow1 = CACurrentMediaTime();
        
        if (0 == fDrawCounter) {
            gDoTraceDraw = true;
        }
        if (DO_TRACE_DRAW_MAX == ++fDrawCounter) {
            fDrawCounter = 0;
        }
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
        
        NSMutableDictionary *newActions = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNull null], @"onOrderIn",
                                           [NSNull null], @"onOrderOut",
                                           [NSNull null], @"sublayers",
                                           [NSNull null], @"contents",
                                           [NSNull null], @"bounds",
                                           nil];
        fGLLayer.actions = newActions;
        fRasterLayer.actions = newActions;
        [newActions release];
        
        fDevManager = new SkiOSDeviceManager;
        fWind = new SampleWindow(self, NULL, NULL, fDevManager);
        fWind->resize(self.frame.size.width, self.frame.size.height, SKWIND_CONFIG);
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
    
    
    GrContext* ctx = fDevManager->getGrContext();
    SkASSERT(NULL != ctx);
    
    SkCanvas canvas;
    
    // if we're not "retained", then we have to always redraw everything.
    // This call forces us to ignore the fDirtyRgn, and draw everywhere.
    // If we are "retained", we can skip this call (as the raster case does)
    fWind->forceInvalAll();
    
    [self drawWithCanvas:&canvas];
    
    // This application only creates a single color renderbuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple renderbuffers.
    glBindRenderbuffer(GL_RENDERBUFFER, fGL.fRenderbuffer);
    [fGL.fContext presentRenderbuffer:GL_RENDERBUFFER];
    
#if GR_COLLECT_STATS
    //    static int frame = 0;
    //    if (!(frame % 100)) {
    //        ctx->printStats();
    //    }
    //    ctx->resetStats();
    //    ++frame;
#endif
}

- (void)drawInRaster {
    SkCanvas canvas;
    [self drawWithCanvas:&canvas];
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

@end

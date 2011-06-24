#import "SkUIView.h"
#include <QuartzCore/QuartzCore.h>

#include "SkGpuCanvas.h"
#include "SkCGUtils.h"
#include "GrContext.h"

#define SKWIND_CONFIG       SkBitmap::kRGB_565_Config
//#define SKWIND_CONFIG       SkBitmap::kARGB_8888_Config
#define SKGL_CONFIG         kEAGLColorFormatRGB565
//#define SKGL_CONFIG         kEAGLColorFormatRGBA8

#define SHOW_FPS
#define FORCE_REDRAW
//#define DUMP_FPS_TO_PRINTF

//#define USE_ACCEL_TO_ROTATE

//#define SHOULD_COUNTER_INIT 334
static int gShouldCounter;
static bool should_draw() {
    if (--gShouldCounter == 0) {
   //     printf("\n");
    }
    return true;
    return gShouldCounter >= 0;
}
#ifdef SHOULD_COUNTER_INIT
    bool (*gShouldDrawProc)() = should_draw;
#else
    bool (*gShouldDrawProc)() = NULL;
#endif

//#define USE_GL_1
#define USE_GL_2

#if defined(USE_GL_1) || defined(USE_GL_2)
    #define USE_GL
#endif

@implementation SkUIView


@synthesize fWind;
@synthesize fTitleLabel;
@synthesize fBackend;
@synthesize fComplexClip;
@synthesize fUseWarp;

#include "SkWindow.h"
#include "SkEvent.h"

static float gScreenScale = 1;

extern SkOSWindow* create_sk_window(void* hwnd);

#define kREDRAW_UIVIEW_GL "sk_redraw_uiview_gl_iOS"

#define TITLE_HEIGHT  44

static const float SCALE_FOR_ZOOM_LENS = 4.0;
#define Y_OFFSET_FOR_ZOOM_LENS           200
#define SIZE_FOR_ZOOM_LENS               250

static const float MAX_ZOOM_SCALE = 4.0;
static const float MIN_ZOOM_SCALE = 2.0 / MAX_ZOOM_SCALE;

extern bool gDoTraceDraw;
#define DO_TRACE_DRAW_MAX   100

#ifdef SHOW_FPS
struct FPSState {
    static const int FRAME_COUNT = 60;

    CFTimeInterval fNow0, fNow1;
    CFTimeInterval fTime0, fTime1, fTotalTime;
    int fFrameCounter;
    int fDrawCounter;

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
        
    void flush(SkOSWindow* wind) {
        CFTimeInterval now2 = CACurrentMediaTime();

        fTime0 += fNow1 - fNow0;
        fTime1 += now2 - fNow1;
        
        if (++fFrameCounter == FRAME_COUNT) {
            CFTimeInterval totalNow = CACurrentMediaTime();
            fTotalTime = totalNow - fTotalTime;
            
            SkMSec ms0 = (int)(1000 * fTime0 / FRAME_COUNT);
            SkMSec msTotal = (int)(1000 * fTotalTime / FRAME_COUNT);
            
            SkString str;
            str.printf("ms: %d [%d], fps: %3.1f", msTotal, ms0,
                       FRAME_COUNT / fTotalTime);
#ifdef DUMP_FPS_TO_PRINTF
            SkDebugf("%s\n", str.c_str());
#else
            wind->setTitle(str.c_str());
#endif

            fTotalTime = totalNow;
            fTime0 = fTime1 = 0;
            fFrameCounter = 0;
        }
    }
};

static FPSState gFPS;

    #define FPS_StartDraw() gFPS.startDraw()
    #define FPS_EndDraw()   gFPS.endDraw()
    #define FPS_Flush(wind)     gFPS.flush(wind)
#else
    #define FPS_StartDraw()
    #define FPS_EndDraw()
    #define FPS_Flush(wind)
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_GL
+ (Class) layerClass
{
	return [CAEAGLLayer class];
}
#endif

- (id)initWithMyDefaults {
    fBackend = kGL_Backend;
    fUseWarp = false;
    fRedrawRequestPending = false;
    fWind = create_sk_window(self);
    fWind->setConfig(SKWIND_CONFIG);
    fMatrix.reset();
    fLocalMatrix.reset();
    fNeedGestureEnded = false;
    fNeedFirstPinch = true;
    fZoomAround = false;
    fComplexClip = false;

    [self initGestures];

#ifdef USE_GL
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    eaglLayer.opaque = TRUE;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO],
                                        kEAGLDrawablePropertyRetainedBacking,
                                        SKGL_CONFIG,
                                        kEAGLDrawablePropertyColorFormat,
                                        nil];

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
    glGenFramebuffersOES(1, &fGL.fFramebuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, fGL.fFramebuffer);

    glGenRenderbuffersOES(1, &fGL.fRenderbuffer);
    glGenRenderbuffersOES(1, &fGL.fStencilbuffer);

    glBindRenderbufferOES(GL_RENDERBUFFER_OES, fGL.fRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, fGL.fRenderbuffer);

    glBindRenderbufferOES(GL_RENDERBUFFER_OES, fGL.fStencilbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, fGL.fStencilbuffer);
#endif

#ifdef USE_ACCEL_TO_ROTATE
    fRotateMatrix.reset();
    [UIAccelerometer sharedAccelerometer].delegate = self;
    [UIAccelerometer sharedAccelerometer].updateInterval = 1 / 30.0;
#endif
    return self;
}

- (id)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        self = [self initWithMyDefaults];
    }
    return self;
}

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        self = [self initWithMyDefaults];
    }
    return self;
}

#include "SkImageDecoder.h"
#include "SkStream_NSData.h"

static void zoom_around(SkCanvas* canvas, float cx, float cy, float zoom) {
    float clipW = SIZE_FOR_ZOOM_LENS;
    float clipH = SIZE_FOR_ZOOM_LENS;

    SkRect r;
    r.set(0, 0, clipW, clipH);
    r.offset(cx - clipW/2, cy - clipH/2);

    SkPaint paint;
    paint.setColor(0xFF66AAEE);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);

    // draw our "frame" around the zoom lens
    canvas->drawRect(r, paint);

    // now clip and scale the lens
    canvas->clipRect(r);
    canvas->translate(cx, cy);
    canvas->scale(zoom, zoom);
    canvas->translate(-cx, -cy);
}

- (void)drawWithCanvas:(SkCanvas*)canvas {
    if (fComplexClip) {
        canvas->drawColor(SK_ColorBLUE);

        SkPath path;
        static const SkRect r[] = {
            { 50, 50, 250, 250 },
            { 150, 150, 500, 600 }
        };
        for (size_t i = 0; i < GR_ARRAY_COUNT(r); i++) {
            path.addRect(r[i]);
        }
        canvas->clipPath(path);
    }
    
    // This is to consolidate multiple inval requests
    fRedrawRequestPending = false;

    if (fFlingState.isActive()) {
        if (!fFlingState.evaluateMatrix(&fLocalMatrix)) {
            [self flushLocalMatrix];
        }
    }

    SkMatrix localMatrix = fLocalMatrix;
#ifdef USE_ACCEL_TO_ROTATE
    localMatrix.preConcat(fRotateMatrix);
#endif

    SkMatrix matrix;
    matrix.setConcat(localMatrix, fMatrix);

    const SkMatrix* localM = NULL;
    if (localMatrix.getType() & SkMatrix::kScale_Mask) {
        localM = &localMatrix;
    }
#ifdef USE_ACCEL_TO_ROTATE
    localM = &localMatrix;
#endif
    canvas->setExternalMatrix(localM);

#ifdef SHOULD_COUNTER_INIT
    gShouldCounter = SHOULD_COUNTER_INIT;
#endif

    {
        int saveCount = canvas->save();
        canvas->concat(matrix);
//        SkRect r = { 10, 10, 500, 600 }; canvas->clipRect(r);
        fWind->draw(canvas);
        canvas->restoreToCount(saveCount);
    }

    if (fZoomAround) {
        zoom_around(canvas, fZoomAroundX, fZoomAroundY, SCALE_FOR_ZOOM_LENS);
        canvas->concat(matrix);
        fWind->draw(canvas);
    }
    
#ifdef FORCE_REDRAW
    fWind->inval(NULL);
#endif
}

///////////////////////////////////////////////////////////////////////////////

- (void)layoutSubviews {
    int W, H;

    gScreenScale = [UIScreen mainScreen].scale;

#ifdef USE_GL
    
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
    if ([self respondsToSelector:@selector(setContentScaleFactor:)]) {
        self.contentScaleFactor = gScreenScale;
    }
    // Allocate color buffer backing based on the current layer size
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, fGL.fRenderbuffer);
    [fGL.fContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:eaglLayer];
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &fGL.fWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &fGL.fHeight);

    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
    {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
    }

    glBindRenderbufferOES(GL_RENDERBUFFER_OES, fGL.fStencilbuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_STENCIL_INDEX8_OES, fGL.fWidth, fGL.fHeight);

    W = fGL.fWidth;
    H = fGL.fHeight;
#else
    CGRect rect = [self bounds];
    W = (int)CGRectGetWidth(rect);
    H = (int)CGRectGetHeight(rect) - TITLE_HEIGHT;
#endif

    printf("---- layoutSubviews %d %d\n", W, H);
    fWind->resize(W, H);
    fWind->inval(NULL);
}

#ifdef USE_GL

static GrContext* gCtx;
static GrContext* get_global_grctx() {
    // should be pthread-local at least
    if (NULL == gCtx) {        
#ifdef USE_GL_1
        gCtx = GrContext::Create(GrGpu::kOpenGL_Fixed_Engine, 0);
#else
        gCtx = GrContext::Create(GrGpu::kOpenGL_Shaders_Engine, 0);
#endif
    }
    return gCtx;
}

#include "SkDevice.h"
#include "SkShader.h"
#include "SkGrTexturePixelRef.h"
#include "GrMesh.h"
#include "SkRandom.h"

#include "GrAtlas.h"
#include "GrTextStrike.h"

static void show_fontcache(GrContext* ctx, SkCanvas* canvas) {
#if 0
    SkPaint paint;
    const int SIZE = 64;
    GrAtlas* plot[64][64];

    paint.setAntiAlias(true);
    paint.setTextSize(24);
    paint.setTextAlign(SkPaint::kCenter_Align);

    Gr_bzero(plot, sizeof(plot));

    GrFontCache* cache = ctx->getFontCache();
    GrTextStrike* strike = cache->getHeadStrike();
    int count = 0;
    while (strike) {
        GrAtlas* atlas = strike->getAtlas();
        while (atlas) {
            int x = atlas->getPlotX();
            int y = atlas->getPlotY();

            SkRandom rand((intptr_t)strike);
            SkColor c = rand.nextU() | 0x80808080;
            paint.setColor(c);
            paint.setAlpha(0x80);

            SkRect r;
            r.set(x * SIZE, y * SIZE, (x + 1)*SIZE, (y+1)*SIZE);
            r.inset(1, 1);
            canvas->drawRect(r, paint);
            
            paint.setColor(0xFF660000);
            SkString label;
            label.printf("%d", count);
            canvas->drawText(label.c_str(), label.size(), r.centerX(),
                             r.fTop + r.height() * 2 / 3, paint);
            
            atlas = atlas->nextAtlas();
        }
        strike = strike->fNext;
        count += 1;
    }
#endif
}

void test_patch(SkCanvas* canvas, const SkBitmap& bm, SkScalar scale);

static void draw_mesh(SkCanvas* canvas, const SkBitmap& bm) {
    GrMesh fMesh;

    SkRect r;
    r.set(0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()));
    
    //        fMesh.init(bounds, fBitmap.width() / 40, fBitmap.height() / 40, texture);
    fMesh.init(r, bm.width()/16, bm.height()/16, r);

    SkPaint paint;
    SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
    paint.setShader(s)->unref();
    fMesh.draw(canvas, paint);
}

static void scale_about(SkCanvas* canvas, float sx, float sy, float px, float py) {
    canvas->translate(px, py);
    canvas->scale(sx, sy);
    canvas->translate(-px, -py);
}

static float grInterp(float v0, float v1, float percent) {
    return v0 + percent * (v1 - v0);
}

static void draw_device(SkCanvas* canvas, SkDevice* dev, float w, float h, float warp) {
    canvas->save();
    float s = grInterp(1, 0.8, warp);
    scale_about(canvas, s, s, w/2, h/2);
    test_patch(canvas, dev->accessBitmap(false), warp);
    canvas->restore();
}

- (void)drawInGL {
//    printf("------ drawInGL\n");
    // This application only creates a single context which is already set current at this point.
    // This call is redundant, but needed if dealing with multiple contexts.
    [EAGLContext setCurrentContext:fGL.fContext];
    
    // This application only creates a single default framebuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple framebuffers.
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, fGL.fFramebuffer);
    
    GLint scissorEnable;
    glGetIntegerv(GL_SCISSOR_TEST, &scissorEnable);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (scissorEnable) {
        glEnable(GL_SCISSOR_TEST);
    }
    
    GrContext* ctx = get_global_grctx();
    SkGpuCanvas origCanvas(ctx);
    origCanvas.setBitmapDevice(fWind->getBitmap());
    
    //    gl->reset();
    SkGpuCanvas glCanvas(ctx);
    SkCanvas   rasterCanvas;

    SkCanvas* canvas;
    SkDevice* dev = NULL;
    
    switch (fBackend) {
        case kRaster_Backend:
            canvas = &rasterCanvas;
            break;
        case kGL_Backend:
            canvas = &glCanvas;
            break;
    }

    if (fUseWarp || fWarpState.isActive()) {
        if (kGL_Backend == fBackend) {
            dev = origCanvas.createDevice(fWind->getBitmap(), true);
            canvas->setDevice(dev)->unref();
        } else {
            canvas->setBitmapDevice(fWind->getBitmap());
            dev = canvas->getDevice();
        }
    } else {
        canvas->setBitmapDevice(fWind->getBitmap());
        dev = NULL;
    }
    
    canvas->translate(0, TITLE_HEIGHT);

    // if we're not "retained", then we have to always redraw everything.
    // This call forces us to ignore the fDirtyRgn, and draw everywhere.
    // If we are "retained", we can skip this call (as the raster case does)
    fWind->forceInvalAll();

    FPS_StartDraw();
    [self drawWithCanvas:canvas];
    FPS_EndDraw();

    if (dev) {
        draw_device(&origCanvas, dev, fWind->width(), fWind->height(),
                    fWarpState.evaluate());
    } else {
        if (kRaster_Backend == fBackend) {
            origCanvas.drawBitmap(fWind->getBitmap(), 0, 0, NULL);
        }
        // else GL - we're already on screen
    }

    show_fontcache(ctx, canvas);
    ctx->flush(false);
        
    // This application only creates a single color renderbuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple renderbuffers.
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, fGL.fRenderbuffer);
    [fGL.fContext presentRenderbuffer:GL_RENDERBUFFER_OES];
    
#if GR_COLLECT_STATS
    static int frame = 0;
    if (!(frame % 100)) {
        get_global_grctx()->printStats();
    }
    get_global_grctx()->resetStats();
    ++frame;
#endif
    
    FPS_Flush(fWind);

#if 0
    gCtx->deleteAllTextures(GrTextureCache::kAbandonTexture_DeleteMode);
    gCtx->unref();
    gCtx = NULL;
#endif
}

#else   // raster case

- (void)drawRect:(CGRect)rect {
    CGContextRef cg = UIGraphicsGetCurrentContext();
    SkCanvas* canvas = NULL;

    FPS_StartDraw();
    [self drawWithCanvas:canvas];

    FPS_EndDraw();
    SkCGDrawBitmap(cg, fWind->getBitmap(), 0, TITLE_HEIGHT);

    FPS_Flush(fWind);

}
#endif

- (void)setWarpState:(bool)useWarp {
    fWarpState.stop();  // we should reverse from where we are if active...

    const float duration = 0.5;
    fUseWarp = useWarp;
    if (useWarp) {
        fWarpState.start(0, 1, duration);
    } else {
        fWarpState.start(1, 0, duration);
    }
}

///////////////////////////////////////////////////////////////////////////////

- (void)flushLocalMatrix {
    fMatrix.postConcat(fLocalMatrix);
    fLocalMatrix.reset();
    fFlingState.stop();
    fNeedGestureEnded = false;
    fNeedFirstPinch = true;
}

- (void)localMatrixWithGesture:(UIGestureRecognizer*)gesture {
    fNeedGestureEnded = true;

    switch (gesture.state) {
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
            [self flushLocalMatrix];
            break;
        case UIGestureRecognizerStateChanged: {
            SkMatrix matrix;
            matrix.setConcat(fLocalMatrix, fMatrix);
        } break;
        default:
            break;
    }
}

- (void)commonHandleGesture:(UIGestureRecognizer*)sender {
    if (fFlingState.isActive()) {
        [self flushLocalMatrix];
    }

    switch (sender.state) {
        case UIGestureRecognizerStateBegan:
            [self flushLocalMatrix];
            break;
        default:
            break;
    }
}

- (void)handleDTapGesture:(UIGestureRecognizer*)sender {
    [self flushLocalMatrix];
    fMatrix.reset();
}

static float discretize(float x) {
    return (int)x;
}

- (void)handlePanGesture:(UIPanGestureRecognizer*)sender {
    [self commonHandleGesture:sender];

    CGPoint delta = [sender translationInView:self];
    delta.x *= gScreenScale;
    delta.y *= gScreenScale;
    // avoid flickering where the drawing might toggle in and out of a pixel
    // center if translated by a fractional value
    delta.x = discretize(delta.x);
    delta.y = discretize(delta.y);
    fLocalMatrix.setTranslate(delta.x, delta.y);
    [self localMatrixWithGesture:sender];

    if (UIGestureRecognizerStateEnded == sender.state) {
        CGPoint velocity = [sender velocityInView:self];
        fFlingState.reset(velocity.x, velocity.y);
        fNeedGestureEnded = true;
    }
}

- (float)limitTotalZoom:(float)scale {
    // this query works 'cause we know that we're square-scale w/ no skew/rotation
    const float curr = fMatrix[0];

    if (scale > 1 && curr * scale > MAX_ZOOM_SCALE) {
        scale = MAX_ZOOM_SCALE / curr;
    } else if (scale < 1 && curr * scale < MIN_ZOOM_SCALE) {
        scale = MIN_ZOOM_SCALE / curr;
    }
    return scale;
}

- (void)handleScaleGesture:(UIPinchGestureRecognizer*)sender {
    [self commonHandleGesture:sender];

    if ([sender numberOfTouches] == 2) {
        float scale = sender.scale;
        CGPoint p0 = [sender locationOfTouch:0 inView:self];
        CGPoint p1 = [sender locationOfTouch:0 inView:self];
        float cx = (p0.x + p1.x) * 0.5;
        float cy = (p0.y + p1.y) * 0.5;

        if (fNeedFirstPinch) {
            fFirstPinchX = cx;
            fFirstPinchY = cy;
            fNeedFirstPinch = false;
        }

        scale = [self limitTotalZoom:scale];

        fLocalMatrix.setTranslate(-fFirstPinchX, -fFirstPinchY);
        fLocalMatrix.postScale(scale, scale);
        fLocalMatrix.postTranslate(cx, cy);
        [self localMatrixWithGesture:sender];
    } else {
        [self flushLocalMatrix];
    }
}

- (void)handleLongPressGesture:(UILongPressGestureRecognizer*)sender {
    [self commonHandleGesture:sender];

    if ([sender numberOfTouches] == 0) {
        fZoomAround = false;
        return;
    }

    CGPoint pt = [sender locationOfTouch:0 inView:self];
    switch (sender.state) {
        case UIGestureRecognizerStateBegan:
        case UIGestureRecognizerStateChanged:
            fZoomAround = true;
            fZoomAroundX = pt.x;
            fZoomAroundY = pt.y - Y_OFFSET_FOR_ZOOM_LENS;
            break;
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled:
            fZoomAround = false;
            break;
        default:
            break;
    }
}

- (void)addAndReleaseGesture:(UIGestureRecognizer*)gesture {
    [self addGestureRecognizer:gesture];
    [gesture release];
}

- (void)initGestures {
    UITapGestureRecognizer* tapG = [UITapGestureRecognizer alloc];
    [tapG initWithTarget:self action:@selector(handleDTapGesture:)];
    tapG.numberOfTapsRequired = 2;
    [self addAndReleaseGesture:tapG];

    UIPanGestureRecognizer* panG = [UIPanGestureRecognizer alloc];
    [panG initWithTarget:self action:@selector(handlePanGesture:)];
    [self addAndReleaseGesture:panG];
    
    UIPinchGestureRecognizer* pinchG = [UIPinchGestureRecognizer alloc];
    [pinchG initWithTarget:self action:@selector(handleScaleGesture:)];
    [self addAndReleaseGesture:pinchG];

    UILongPressGestureRecognizer* longG = [UILongPressGestureRecognizer alloc];
    [longG initWithTarget:self action:@selector(handleLongPressGesture:)];
    [self addAndReleaseGesture:longG];
}

///////////////////////////////////////////////////////////////////////////////

static float abs(float x) { return x < 0 ? -x : x; }

static bool normalize(UIAcceleration* acc, float xy[]) {
    float mag2 = acc.x*acc.x + acc.y*acc.y + acc.z*acc.z;
    if (mag2 < 0.000001) {
        return false;
    }
    if (abs((float)acc.z) > 0.9 * sqrt(mag2)) {
        return false;
    }

    mag2 = acc.x*acc.x + acc.y*acc.y;
    if (mag2 < 0.000001) {
        return false;
    }
    float scale = 1 / sqrt(mag2);
    xy[0] = acc.x * scale;
    xy[1] = acc.y * scale;
    return true;
}

static void normalize(float xy[]) {
    float scale = 1 / sqrt(xy[0]*xy[0] + xy[1]*xy[1]);
    xy[0] *= scale;
    xy[1] *= scale;
}

static float weighted_average(float newv, float oldv) {
    return newv * 0.25 + oldv * 0.75;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acc {

    float norm[2];
    if (normalize(acc, norm)) {
        float sinv = -norm[0];
        float cosv = -norm[1];
        // smooth
        norm[0] = weighted_average(sinv, -fRotateMatrix[1]);
        norm[1] = weighted_average(cosv, fRotateMatrix[0]);
        normalize(norm);
        fRotateMatrix.setSinCos(norm[0], norm[1], 400, 400);
    }
#if 0
    NSDate *now = [NSDate date];
    NSTimeInterval intervalDate = [now timeIntervalSinceDate:now_prev];
    
    velX += (acceleration.x * intervalDate);
    distX += (velX * intervalDate);
    //do other axis here too
    
    // setup for next UIAccelerometer event
    now_prev = now;
#endif
}

///////////////////////////////////////////////////////////////////////////////

- (void)setSkTitle:(const char *)title {
    if (fTitleLabel) {
        fTitleLabel.text = [NSString stringWithUTF8String:title];
        [fTitleLabel setNeedsDisplay];
    }
}

- (BOOL)onHandleEvent:(const SkEvent&)evt {
    if (evt.isType(kREDRAW_UIVIEW_GL)) {
        [self drawInGL];
        return true;
    }
    return false;
}

- (void)postInvalWithRect:(const SkIRect*)r {
#ifdef USE_GL

#if 1
    if (!fRedrawRequestPending) {
        fRedrawRequestPending = true;
        /*
            performSelectorOnMainThread seems to starve updating other views
            (e.g. our FPS view in the titlebar), so we use the afterDelay
            version
         */
        if (0) {
            [self performSelectorOnMainThread:@selector(drawInGL) withObject:nil waitUntilDone:NO];
        } else {
            [self performSelector:@selector(drawInGL) withObject:nil afterDelay:0];
        }
    }
#else
    if (!fRedrawRequestPending) {
        SkEvent* evt = new SkEvent(kREDRAW_UIVIEW_GL);
        evt->post(fWind->getSinkID());
        fRedrawRequestPending = true;
    }
#endif

#else
    if (r) {
        [self setNeedsDisplayInRect:CGRectMake(r->fLeft, r->fTop,
                                               r->width(), r->height())];
    } else {
        [self setNeedsDisplay];
    }
#endif
}

@end

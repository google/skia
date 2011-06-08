#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGpuCanvas.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTime.h"
#include "SkWindow.h"

#include "SampleCode.h"
#include "GrContext.h"
#include "SkTouchGesture.h"
#include "SkTypeface.h"

#define TEST_GPIPEx

#ifdef  TEST_GPIPE
#define PIPE_FILE
#define FILE_PATH "/path/to/drawing.data"
#endif

#define USE_ARROWS_FOR_ZOOM true
//#define DEFAULT_TO_GPU

extern SkView* create_overview(int, const SkViewFactory[]);

#define SK_SUPPORT_GL

#define ANIMATING_EVENTTYPE "nextSample"
#define ANIMATING_DELAY     750

#ifdef SK_DEBUG
    #define FPS_REPEAT_MULTIPLIER   1
#else
    #define FPS_REPEAT_MULTIPLIER   10
#endif
#define FPS_REPEAT_COUNT    (10 * FPS_REPEAT_MULTIPLIER)

#ifdef SK_SUPPORT_GL
    #include "GrGLConfig.h"
#endif

///////////////
static const char view_inval_msg[] = "view-inval-msg";

static void postInvalDelay(SkEventSinkID sinkID) {
    SkEvent* evt = new SkEvent(view_inval_msg);
    evt->post(sinkID, 1);
}

static bool isInvalEvent(const SkEvent& evt) {
    return evt.isType(view_inval_msg);
}
//////////////////

SkViewRegister* SkViewRegister::gHead;
SkViewRegister::SkViewRegister(SkViewFactory fact) : fFact(fact) {
    static bool gOnce;
    if (!gOnce) {
        gHead = NULL;
        gOnce = true;
    }

    fChain = gHead;
    gHead = this;
}

#if defined(SK_SUPPORT_GL)
    #define SK_USE_SHADERS
#endif

#ifdef SK_BUILD_FOR_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFURLAccess.h>

static void testpdf() {
    CFStringRef path = CFStringCreateWithCString(NULL, "/test.pdf",
                                                 kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithFileSystemPath(NULL, path,
                                              kCFURLPOSIXPathStyle,
                                              false);
    CFRelease(path);
    CGRect box = CGRectMake(0, 0, 8*72, 10*72);
    CGContextRef cg = CGPDFContextCreateWithURL(url, &box, NULL);
    CFRelease(url);

    CGContextBeginPage(cg, &box);
    CGRect r = CGRectMake(10, 10, 40 + 0.5, 50 + 0.5);
    CGContextFillEllipseInRect(cg, r);
    CGContextEndPage(cg);
    CGContextRelease(cg);

    if (false) {
        SkBitmap bm;
        bm.setConfig(SkBitmap::kA8_Config, 64, 64);
        bm.allocPixels();
        bm.eraseColor(0);

        SkCanvas canvas(bm);

    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

enum FlipAxisEnum {
    kFlipAxis_X = (1 << 0),
    kFlipAxis_Y = (1 << 1)
};

enum SkTriState {
    kFalse_SkTriState,
    kTrue_SkTriState,
    kUnknown_SkTriState,
};

static SkTriState cycle_tristate(SkTriState state) {
    static const SkTriState gCycle[] = {
        /* kFalse_SkTriState   -> */  kUnknown_SkTriState,
        /* kTrue_SkTriState    -> */  kFalse_SkTriState,
        /* kUnknown_SkTriState -> */  kTrue_SkTriState,
    };
    return gCycle[state];
}

#include "SkDrawFilter.h"

class FlagsDrawFilter : public SkDrawFilter {
public:
    FlagsDrawFilter(SkTriState lcd, SkTriState aa, SkTriState filter,
                    SkTriState hinting) :
        fLCDState(lcd), fAAState(aa), fFilterState(filter), fHintingState(hinting) {}

    virtual void filter(SkPaint* paint, Type t) {
        if (kText_Type == t && kUnknown_SkTriState != fLCDState) {
            paint->setLCDRenderText(kTrue_SkTriState == fLCDState);
        }
        if (kUnknown_SkTriState != fAAState) {
            paint->setAntiAlias(kTrue_SkTriState == fAAState);
        }
        if (kUnknown_SkTriState != fFilterState) {
            paint->setFilterBitmap(kTrue_SkTriState == fFilterState);
        }
        if (kUnknown_SkTriState != fHintingState) {
            paint->setHinting(kTrue_SkTriState == fHintingState ?
                              SkPaint::kNormal_Hinting :
                              SkPaint::kSlight_Hinting);
        }
    }

private:
    SkTriState  fLCDState;
    SkTriState  fAAState;
    SkTriState  fFilterState;
    SkTriState  fHintingState;
};

//////////////////////////////////////////////////////////////////////////////

#define MAX_ZOOM_LEVEL  8
#define MIN_ZOOM_LEVEL  -8

static const char gCharEvtName[] = "SampleCode_Char_Event";
static const char gKeyEvtName[] = "SampleCode_Key_Event";
static const char gTitleEvtName[] = "SampleCode_Title_Event";
static const char gPrefSizeEvtName[] = "SampleCode_PrefSize_Event";
static const char gFastTextEvtName[] = "SampleCode_FastText_Event";

bool SampleCode::CharQ(const SkEvent& evt, SkUnichar* outUni) {
    if (evt.isType(gCharEvtName, sizeof(gCharEvtName) - 1)) {
        if (outUni) {
            *outUni = evt.getFast32();
        }
        return true;
    }
    return false;
}

bool SampleCode::KeyQ(const SkEvent& evt, SkKey* outKey) {
    if (evt.isType(gKeyEvtName, sizeof(gKeyEvtName) - 1)) {
        if (outKey) {
            *outKey = (SkKey)evt.getFast32();
        }
        return true;
    }
    return false;
}

bool SampleCode::TitleQ(const SkEvent& evt) {
    return evt.isType(gTitleEvtName, sizeof(gTitleEvtName) - 1);
}

void SampleCode::TitleR(SkEvent* evt, const char title[]) {
    SkASSERT(evt && TitleQ(*evt));
    evt->setString(gTitleEvtName, title);
}

bool SampleCode::PrefSizeQ(const SkEvent& evt) {
    return evt.isType(gPrefSizeEvtName, sizeof(gPrefSizeEvtName) - 1);
}

void SampleCode::PrefSizeR(SkEvent* evt, SkScalar width, SkScalar height) {
    SkASSERT(evt && PrefSizeQ(*evt));
    SkScalar size[2];
    size[0] = width;
    size[1] = height;
    evt->setScalars(gPrefSizeEvtName, 2, size);
}

bool SampleCode::FastTextQ(const SkEvent& evt) {
    return evt.isType(gFastTextEvtName, sizeof(gFastTextEvtName) - 1);
}

///////////////////////////////////////////////////////////////////////////////

static SkMSec gAnimTime;
static SkMSec gAnimTimePrev;

SkMSec SampleCode::GetAnimTime() { return gAnimTime; }
SkMSec SampleCode::GetAnimTimeDelta() { return gAnimTime - gAnimTimePrev; }
SkScalar SampleCode::GetAnimSecondsDelta() {
    return SkDoubleToScalar(GetAnimTimeDelta() / 1000.0);
}

SkScalar SampleCode::GetAnimScalar(SkScalar speed, SkScalar period) {
    // since gAnimTime can be up to 32 bits, we can't convert it to a float
    // or we'll lose the low bits. Hence we use doubles for the intermediate
    // calculations
    double seconds = (double)gAnimTime / 1000.0;
    double value = SkScalarToDouble(speed) * seconds;
    if (period) {
        value = ::fmod(value, SkScalarToDouble(period));
    }
    return SkDoubleToScalar(value);
}

//////////////////////////////////////////////////////////////////////////////

static SkView* curr_view(SkWindow* wind) {
    SkView::F2BIter iter(wind);
    return iter.next();
}

class SampleWindow : public SkOSWindow {
    SkTDArray<SkViewFactory> fSamples;
public:
    SampleWindow(void* hwnd);
    virtual ~SampleWindow();

    virtual void draw(SkCanvas* canvas);

protected:
    virtual void onDraw(SkCanvas* canvas);
    virtual bool onHandleKey(SkKey key);
    virtual bool onHandleChar(SkUnichar);
    virtual void onSizeChange();

    virtual SkCanvas* beforeChildren(SkCanvas*);
    virtual void afterChildren(SkCanvas*);
    virtual void beforeChild(SkView* child, SkCanvas* canvas);
    virtual void afterChild(SkView* child, SkCanvas* canvas);

    virtual bool onEvent(const SkEvent& evt);
    virtual bool onQuery(SkEvent* evt);

    virtual bool onDispatchClick(int x, int y, Click::State);
    virtual bool onClick(Click* click);
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y);

#if 0
    virtual bool handleChar(SkUnichar uni);
    virtual bool handleEvent(const SkEvent& evt);
    virtual bool handleKey(SkKey key);
    virtual bool handleKeyUp(SkKey key);
    virtual bool onHandleKeyUp(SkKey key);
#endif

private:
    int fCurrIndex;

    SkPicture* fPicture;
    SkGpuCanvas* fGpuCanvas;
    GrContext* fGrContext;
    SkPath fClipPath;

    SkTouchGesture fGesture;
    int      fZoomLevel;
    SkScalar fZoomScale;

    enum CanvasType {
        kRaster_CanvasType,
        kPicture_CanvasType,
        kGPU_CanvasType
    };
    CanvasType fCanvasType;

    bool fUseClip;
    bool fNClip;
    bool fRepeatDrawing;
    bool fAnimating;
    bool fRotate;
    bool fScale;
    bool fRequestGrabImage;
    bool fUsePipe;
    bool fMeasureFPS;
    SkMSec fMeasureFPS_Time;

    // The following are for the 'fatbits' drawing
    // Latest position of the mouse.
    int fMouseX, fMouseY;
    int fFatBitsScale;
    // Used by the text showing position and color values.
    SkTypeface* fTypeface;
    bool fShowZoomer;

    SkTriState fLCDState;
    SkTriState fAAState;
    SkTriState fFilterState;
    SkTriState fHintingState;
    unsigned   fFlipAxis;

    int fScrollTestX, fScrollTestY;

    bool make3DReady();
    void changeZoomLevel(int delta);

    void loadView(SkView*);
    void updateTitle();
    bool nextSample();

    void toggleZoomer();
    bool zoomIn();
    bool zoomOut();
    void updatePointer(int x, int y);
    void showZoomer(SkCanvas* canvas);

    void postAnimatingEvent() {
        if (fAnimating) {
            SkEvent* evt = new SkEvent(ANIMATING_EVENTTYPE);
            evt->post(this->getSinkID(), ANIMATING_DELAY);
        }
    }


    static CanvasType cycle_canvastype(CanvasType);

    typedef SkOSWindow INHERITED;
};

bool SampleWindow::zoomIn()
{
    // Arbitrarily decided
    if (fFatBitsScale == 25) return false;
    fFatBitsScale++;
    this->inval(NULL);
    return true;
}

bool SampleWindow::zoomOut()
{
    if (fFatBitsScale == 1) return false;
    fFatBitsScale--;
    this->inval(NULL);
    return true;
}

void SampleWindow::toggleZoomer()
{
    fShowZoomer = !fShowZoomer;
    this->inval(NULL);
}

void SampleWindow::updatePointer(int x, int y)
{
    fMouseX = x;
    fMouseY = y;
    if (fShowZoomer) {
        this->inval(NULL);
    }
}

bool SampleWindow::make3DReady() {

#if defined(SK_SUPPORT_GL)
    if (attachGL()) {
        if (NULL != fGrContext) {
        // various gr lifecycle tests
        #if   0
            fGrContext->freeGpuResources();
        #elif 0
            // this will leak resources.
            fGrContext->contextLost();
        #elif 0
            GrAssert(1 == fGrContext->refcnt());
            fGrContext->unref();
            fGrContext = NULL;
        #endif
        }

        if (NULL == fGrContext) {
        #if defined(SK_USE_SHADERS)
            fGrContext = GrContext::Create(kOpenGL_Shaders_GrEngine, NULL);
        #else
            fGrContext = GrContext::Create(kOpenGL_Fixed_GrEngine, NULL);
        #endif
            SkDebugf("---- constructor\n");
        }

        if (NULL != fGrContext) {
            return true;
        } else {
            detachGL();
        }
    }
#endif
    SkDebugf("Failed to setup 3D");
    return false;
}

SampleWindow::CanvasType SampleWindow::cycle_canvastype(CanvasType ct) {
    static const CanvasType gCT[] = {
        kPicture_CanvasType,
        kGPU_CanvasType,
        kRaster_CanvasType
    };
    return gCT[ct];
}

SampleWindow::SampleWindow(void* hwnd) : INHERITED(hwnd) {
#ifdef  PIPE_FILE
    //Clear existing file or create file if it doesn't exist
    FILE* f = fopen(FILE_PATH, "wb");
    fclose(f);
#endif
     
    fPicture = NULL;
    fGpuCanvas = NULL;

    fGrContext = NULL;

#ifdef DEFAULT_TO_GPU
    fCanvasType = kGPU_CanvasType;
#else
    fCanvasType = kRaster_CanvasType;
#endif
    fUseClip = false;
    fNClip = false;
    fRepeatDrawing = false;
    fAnimating = false;
    fRotate = false;
    fScale = false;
    fRequestGrabImage = false;
    fUsePipe = false;
    fMeasureFPS = false;
    fLCDState = kUnknown_SkTriState;
    fAAState = kUnknown_SkTriState;
    fFilterState = kUnknown_SkTriState;
    fHintingState = kUnknown_SkTriState;
    fFlipAxis = 0;
    fScrollTestX = fScrollTestY = 0;

    fMouseX = fMouseY = 0;
    fFatBitsScale = 8;
    fTypeface = SkTypeface::CreateFromTypeface(NULL, SkTypeface::kBold);
    fShowZoomer = false;

    fZoomLevel = 0;
    fZoomScale = SK_Scalar1;

//    this->setConfig(SkBitmap::kRGB_565_Config);
    this->setConfig(SkBitmap::kARGB_8888_Config);
    this->setVisibleP(true);
    this->setClipToBounds(false);

    {
        const SkViewRegister* reg = SkViewRegister::Head();
        while (reg) {
            *fSamples.append() = reg->factory();
            reg = reg->next();
        }
    }
    fCurrIndex = 0;
    this->loadView(fSamples[fCurrIndex]());

#ifdef SK_BUILD_FOR_MAC
    testpdf();
#endif
}

SampleWindow::~SampleWindow() {
    delete fPicture;
    delete fGpuCanvas;
    if (NULL != fGrContext) {
        fGrContext->unref();
    }
    fTypeface->unref();
}

static SkBitmap capture_bitmap(SkCanvas* canvas) {
    SkBitmap bm;
    const SkBitmap& src = canvas->getDevice()->accessBitmap(false);
    src.copyTo(&bm, src.config());
    return bm;
}

static bool bitmap_diff(SkCanvas* canvas, const SkBitmap& orig,
                        SkBitmap* diff) {
    const SkBitmap& src = canvas->getDevice()->accessBitmap(false);

    SkAutoLockPixels alp0(src);
    SkAutoLockPixels alp1(orig);
    for (int y = 0; y < src.height(); y++) {
        const void* srcP = src.getAddr(0, y);
        const void* origP = orig.getAddr(0, y);
        size_t bytes = src.width() * src.bytesPerPixel();
        if (memcmp(srcP, origP, bytes)) {
            SkDebugf("---------- difference on line %d\n", y);
            return true;
        }
    }
    return false;
}

static void drawText(SkCanvas* canvas, SkString string, SkScalar left, SkScalar top, SkPaint& paint)
{
    SkColor desiredColor = paint.getColor();
    paint.setColor(SK_ColorWHITE);
    const char* c_str = string.c_str();
    size_t size = string.size();
    SkRect bounds;
    paint.measureText(c_str, size, &bounds);
    bounds.offset(left, top);
    SkScalar inset = SkIntToScalar(-2);
    bounds.inset(inset, inset);
    canvas->drawRect(bounds, paint);
    if (desiredColor != SK_ColorBLACK) {
        paint.setColor(SK_ColorBLACK);
        canvas->drawText(c_str, size, left + SK_Scalar1, top + SK_Scalar1, paint);
    }
    paint.setColor(desiredColor);
    canvas->drawText(c_str, size, left, top, paint);
}

#define XCLIP_N  8
#define YCLIP_N  8

void SampleWindow::draw(SkCanvas* canvas) {
    // update the animation time
    gAnimTimePrev = gAnimTime;
    gAnimTime = SkTime::GetMSecs();

    SkScalar cx = SkScalarHalf(this->width());
    SkScalar cy = SkScalarHalf(this->height());

    if (fZoomLevel) {
        SkMatrix m;
        SkPoint center;
        m = canvas->getTotalMatrix();//.invert(&m);
        m.mapXY(cx, cy, &center);
        cx = center.fX;
        cy = center.fY;

        m.setTranslate(-cx, -cy);
        m.postScale(fZoomScale, fZoomScale);
        m.postTranslate(cx, cy);

        canvas->concat(m);
    }

    if (fFlipAxis) {
        SkMatrix m;
        m.setTranslate(cx, cy);
        if (fFlipAxis & kFlipAxis_X) {
            m.preScale(-SK_Scalar1, SK_Scalar1);
        }
        if (fFlipAxis & kFlipAxis_Y) {
            m.preScale(SK_Scalar1, -SK_Scalar1);
        }
        m.preTranslate(-cx, -cy);
        canvas->concat(m);
    }

    // Apply any gesture matrix
    if (true) {
        const SkMatrix& localM = fGesture.localM();
        if (localM.getType() & SkMatrix::kScale_Mask) {
            canvas->setExternalMatrix(&localM);
        }
        canvas->concat(localM);
        canvas->concat(fGesture.globalM());

        if (fGesture.isActive()) {
            this->inval(NULL);
        }
    }

    if (fNClip) {
        this->INHERITED::draw(canvas);
        SkBitmap orig = capture_bitmap(canvas);

        const SkScalar w = this->width();
        const SkScalar h = this->height();
        const SkScalar cw = w / XCLIP_N;
        const SkScalar ch = h / YCLIP_N;
        for (int y = 0; y < YCLIP_N; y++) {
            SkRect r;
            r.fTop = y * ch;
            r.fBottom = (y + 1) * ch;
            if (y == YCLIP_N - 1) {
                r.fBottom = h;
            }
            for (int x = 0; x < XCLIP_N; x++) {
                SkAutoCanvasRestore acr(canvas, true);
                r.fLeft = x * cw;
                r.fRight = (x + 1) * cw;
                if (x == XCLIP_N - 1) {
                    r.fRight = w;
                }
                canvas->clipRect(r);
                this->INHERITED::draw(canvas);
            }
        }

        SkBitmap diff;
        if (bitmap_diff(canvas, orig, &diff)) {
        }
    } else {
        this->INHERITED::draw(canvas);
    }
    if (fShowZoomer && fCanvasType != kGPU_CanvasType) {
        // In the GPU case, INHERITED::draw calls beforeChildren, which
        // creates an SkGpuCanvas.  All further draw calls are directed
        // at that canvas, which is deleted in afterChildren (which is
        // also called by draw), so we cannot show the zoomer here.
        // Instead, we call it inside afterChildren.
        showZoomer(canvas);
    }
}

void SampleWindow::showZoomer(SkCanvas* canvas) {
        int count = canvas->save();
        canvas->resetMatrix();
        // Ensure the mouse position is on screen.
        int width = SkScalarRound(this->width());
        int height = SkScalarRound(this->height());
        if (fMouseX >= width) fMouseX = width - 1;
        else if (fMouseX < 0) fMouseX = 0;
        if (fMouseY >= height) fMouseY = height - 1;
        else if (fMouseY < 0) fMouseY = 0;

        SkBitmap bitmap = capture_bitmap(canvas);
        bitmap.lockPixels();

        // Find the size of the zoomed in view, forced to be odd, so the examined pixel is in the middle.
        int zoomedWidth = (width >> 1) | 1;
        int zoomedHeight = (height >> 1) | 1;
        SkIRect src;
        src.set(0, 0, zoomedWidth / fFatBitsScale, zoomedHeight / fFatBitsScale);
        src.offset(fMouseX - (src.width()>>1), fMouseY - (src.height()>>1));
        SkRect dest;
        dest.set(0, 0, SkIntToScalar(zoomedWidth), SkIntToScalar(zoomedHeight));
        dest.offset(SkIntToScalar(width - zoomedWidth), SkIntToScalar(height - zoomedHeight));
        SkPaint paint;
        // Clear the background behind our zoomed in view
        paint.setColor(SK_ColorWHITE);
        canvas->drawRect(dest, paint);
        canvas->drawBitmapRect(bitmap, &src, dest);
        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kStroke_Style);
        // Draw a border around the pixel in the middle
        SkRect originalPixel;
        originalPixel.set(SkIntToScalar(fMouseX), SkIntToScalar(fMouseY), SkIntToScalar(fMouseX + 1), SkIntToScalar(fMouseY + 1));
        SkMatrix matrix;
        SkRect scalarSrc;
        scalarSrc.set(src);
        SkColor color = bitmap.getColor(fMouseX, fMouseY);
        if (matrix.setRectToRect(scalarSrc, dest, SkMatrix::kFill_ScaleToFit)) {
            SkRect pixel;
            matrix.mapRect(&pixel, originalPixel);
            // TODO Perhaps measure the values and make the outline white if it's "dark"
            if (color == SK_ColorBLACK) {
                paint.setColor(SK_ColorWHITE);
            }
            canvas->drawRect(pixel, paint);
        }
        paint.setColor(SK_ColorBLACK);
        // Draw a border around the destination rectangle
        canvas->drawRect(dest, paint);
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
        // Identify the pixel and its color on screen
        paint.setTypeface(fTypeface);
        paint.setAntiAlias(true);
        SkScalar lineHeight = paint.getFontMetrics(NULL);
        SkString string;
        string.appendf("(%i, %i)", fMouseX, fMouseY);
        SkScalar left = dest.fLeft + SkIntToScalar(3);
        SkScalar i = SK_Scalar1;
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Alpha
        i += SK_Scalar1;
        string.reset();
        string.appendf("A: %X", SkColorGetA(color));
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Red
        i += SK_Scalar1;
        string.reset();
        string.appendf("R: %X", SkColorGetR(color));
        paint.setColor(SK_ColorRED);
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Green
        i += SK_Scalar1;
        string.reset();
        string.appendf("G: %X", SkColorGetG(color));
        paint.setColor(SK_ColorGREEN);
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        // Blue
        i += SK_Scalar1;
        string.reset();
        string.appendf("B: %X", SkColorGetB(color));
        paint.setColor(SK_ColorBLUE);
        drawText(canvas, string, left, SkScalarMulAdd(lineHeight, i, dest.fTop), paint);
        canvas->restoreToCount(count);
}

void SampleWindow::onDraw(SkCanvas* canvas) {
    if (fRepeatDrawing) {
        this->inval(NULL);
    }
}

#include "SkColorPriv.h"

static void reverseRedAndBlue(const SkBitmap& bm) {
    SkASSERT(bm.config() == SkBitmap::kARGB_8888_Config);
    uint8_t* p = (uint8_t*)bm.getPixels();
    uint8_t* stop = p + bm.getSize();
    while (p < stop) {
        // swap red/blue (to go from ARGB(int) to RGBA(memory) and premultiply
        unsigned scale = SkAlpha255To256(p[3]);
        unsigned r = p[2];
        unsigned b = p[0];
        p[0] = SkAlphaMul(r, scale);
        p[1] = SkAlphaMul(p[1], scale);
        p[2] = SkAlphaMul(b, scale);
        p += 4;
    }
}

SkCanvas* SampleWindow::beforeChildren(SkCanvas* canvas) {
    if (kGPU_CanvasType != fCanvasType) {
#ifdef SK_SUPPORT_GL
        detachGL();
#endif
    }

    switch (fCanvasType) {
        case kRaster_CanvasType:
            canvas = this->INHERITED::beforeChildren(canvas);
            break;
        case kPicture_CanvasType:
            fPicture = new SkPicture;
            canvas = fPicture->beginRecording(9999, 9999);
            break;
        case kGPU_CanvasType: {
            if (make3DReady()) {
                SkDevice* device = canvas->getDevice();
                const SkBitmap& bitmap = device->accessBitmap(true);

                GrRenderTarget* renderTarget;
                renderTarget = fGrContext->createRenderTargetFrom3DApiState();
                fGpuCanvas = new SkGpuCanvas(fGrContext, renderTarget);
                renderTarget->unref();

                device = fGpuCanvas->createDevice(SkBitmap::kARGB_8888_Config,
                                                  bitmap.width(), bitmap.height(),
                                                  false, false);
                fGpuCanvas->setDevice(device)->unref();

                fGpuCanvas->concat(canvas->getTotalMatrix());
                canvas = fGpuCanvas;

            } else {
                canvas = this->INHERITED::beforeChildren(canvas);
            }
            break;
        }
    }

    if (fUseClip) {
        canvas->drawColor(0xFFFF88FF);
        canvas->clipPath(fClipPath);
    }

    return canvas;
}

static void paint_rgn(const SkBitmap& bm, const SkIRect& r,
                      const SkRegion& rgn) {
    SkCanvas    canvas(bm);
    SkRegion    inval(rgn);

    inval.translate(r.fLeft, r.fTop);
    canvas.clipRegion(inval);
    canvas.drawColor(0xFFFF8080);
}

void SampleWindow::afterChildren(SkCanvas* orig) {
    if (fRequestGrabImage) {
        fRequestGrabImage = false;

        SkCanvas* canvas = fGpuCanvas ? fGpuCanvas : orig;
        SkDevice* device = canvas->getDevice();
        SkBitmap bmp;
        if (device->accessBitmap(false).copyTo(&bmp, SkBitmap::kARGB_8888_Config)) {
            static int gSampleGrabCounter;
            SkString name;
            name.printf("sample_grab_%d", gSampleGrabCounter++);
            SkImageEncoder::EncodeFile(name.c_str(), bmp,
                                       SkImageEncoder::kPNG_Type, 100);
        }
    }

    switch (fCanvasType) {
        case kRaster_CanvasType:
            break;
        case kPicture_CanvasType:
            if (true) {
                SkPicture* pict = new SkPicture(*fPicture);
                fPicture->unref();
                orig->drawPicture(*pict);
                pict->unref();
            } else if (true) {
                SkDynamicMemoryWStream ostream;
                fPicture->serialize(&ostream);
                fPicture->unref();

                SkMemoryStream istream(ostream.getStream(), ostream.getOffset());
                SkPicture pict(&istream);
                orig->drawPicture(pict);
            } else {
                fPicture->draw(orig);
                fPicture->unref();
            }
            fPicture = NULL;
            break;
#ifdef SK_SUPPORT_GL
        case kGPU_CanvasType:
            if (fShowZoomer && fGpuCanvas) {
                this->showZoomer(fGpuCanvas);
            }
            delete fGpuCanvas;
            fGpuCanvas = NULL;
            presentGL();
            break;
#endif
    }

    // Do this after presentGL and other finishing, rather than in afterChild
    if (fMeasureFPS && fMeasureFPS_Time) {
        fMeasureFPS_Time = SkTime::GetMSecs() - fMeasureFPS_Time;
        this->updateTitle();
        postInvalDelay(this->getSinkID());
    }

    //    if ((fScrollTestX | fScrollTestY) != 0)
    if (false) {
        const SkBitmap& bm = orig->getDevice()->accessBitmap(true);
        int dx = fScrollTestX * 7;
        int dy = fScrollTestY * 7;
        SkIRect r;
        SkRegion inval;

        r.set(50, 50, 50+100, 50+100);
        bm.scrollRect(&r, dx, dy, &inval);
        paint_rgn(bm, r, inval);
    }        
}

void SampleWindow::beforeChild(SkView* child, SkCanvas* canvas) {
    if (fScale) {
        SkScalar scale = SK_Scalar1 * 7 / 10;
        SkScalar cx = this->width() / 2;
        SkScalar cy = this->height() / 2;
        canvas->translate(cx, cy);
        canvas->scale(scale, scale);
        canvas->translate(-cx, -cy);
    }
    if (fRotate) {
        SkScalar cx = this->width() / 2;
        SkScalar cy = this->height() / 2;
        canvas->translate(cx, cy);
        canvas->rotate(SkIntToScalar(30));
        canvas->translate(-cx, -cy);
    }

    canvas->setDrawFilter(new FlagsDrawFilter(fLCDState, fAAState,
                                       fFilterState, fHintingState))->unref();

    if (fMeasureFPS) {
        fMeasureFPS_Time = 0;   // 0 means the child is not aware of repeat-draw
        if (SampleView::SetRepeatDraw(child, FPS_REPEAT_COUNT)) {
            fMeasureFPS_Time = SkTime::GetMSecs();
        }
    } else {
        (void)SampleView::SetRepeatDraw(child, 1);
    }
    (void)SampleView::SetUsePipe(child, fUsePipe);
}

void SampleWindow::afterChild(SkView* child, SkCanvas* canvas) {
    canvas->setDrawFilter(NULL);
}

static SkBitmap::Config gConfigCycle[] = {
    SkBitmap::kNo_Config,           // none -> none
    SkBitmap::kNo_Config,           // a1 -> none
    SkBitmap::kNo_Config,           // a8 -> none
    SkBitmap::kNo_Config,           // index8 -> none
    SkBitmap::kARGB_4444_Config,    // 565 -> 4444
    SkBitmap::kARGB_8888_Config,    // 4444 -> 8888
    SkBitmap::kRGB_565_Config       // 8888 -> 565
};

static SkBitmap::Config cycle_configs(SkBitmap::Config c) {
    return gConfigCycle[c];
}

void SampleWindow::changeZoomLevel(int delta) {
    fZoomLevel += delta;
    if (fZoomLevel > 0) {
        fZoomLevel = SkMin32(fZoomLevel, MAX_ZOOM_LEVEL);
        fZoomScale = SkIntToScalar(fZoomLevel + 1);
    } else if (fZoomLevel < 0) {
        fZoomLevel = SkMax32(fZoomLevel, MIN_ZOOM_LEVEL);
        fZoomScale = SK_Scalar1 / (1 - fZoomLevel);
    } else {
        fZoomScale = SK_Scalar1;
    }

    this->inval(NULL);
}

bool SampleWindow::nextSample() {
    fCurrIndex = (fCurrIndex + 1) % fSamples.count();
    this->loadView(fSamples[fCurrIndex]());
    return true;
}

bool SampleWindow::onEvent(const SkEvent& evt) {
    if (evt.isType(ANIMATING_EVENTTYPE)) {
        if (fAnimating) {
            this->nextSample();
            this->postAnimatingEvent();
        }
        return true;
    }
    if (evt.isType("set-curr-index")) {
        fCurrIndex = evt.getFast32() % fSamples.count();
        this->loadView(fSamples[fCurrIndex]());
        return true;
    }
    if (isInvalEvent(evt)) {
        this->inval(NULL);
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

bool SampleWindow::onQuery(SkEvent* query) {
    if (query->isType("get-slide-count")) {
        query->setFast32(fSamples.count());
        return true;
    }
    if (query->isType("get-slide-title")) {
        SkView* view = fSamples[query->getFast32()]();
        SkEvent evt(gTitleEvtName);
        if (view->doQuery(&evt)) {
            query->setString("title", evt.findString(gTitleEvtName));
        }
        SkSafeUnref(view);
        return true;
    }
    if (query->isType("use-fast-text")) {
        SkEvent evt(gFastTextEvtName);
        return curr_view(this)->doQuery(&evt);
    }
    return this->INHERITED::onQuery(query);
}

static void cleanup_for_filename(SkString* name) {
    char* str = name->writable_str();
    for (size_t i = 0; i < name->size(); i++) {
        switch (str[i]) {
            case ':': str[i] = '-'; break;
            case '/': str[i] = '-'; break;
            case ' ': str[i] = '_'; break;
            default: break;
        }
    }
}

bool SampleWindow::onHandleChar(SkUnichar uni) {
    {
        SkView* view = curr_view(this);
        if (view) {
            SkEvent evt(gCharEvtName);
            evt.setFast32(uni);
            if (view->doQuery(&evt)) {
                return true;
            }
        }
    }

    int dx = 0xFF;
    int dy = 0xFF;

    switch (uni) {
        case '5': dx =  0; dy =  0; break;
        case '8': dx =  0; dy = -1; break;
        case '6': dx =  1; dy =  0; break;
        case '2': dx =  0; dy =  1; break;
        case '4': dx = -1; dy =  0; break;
        case '7': dx = -1; dy = -1; break;
        case '9': dx =  1; dy = -1; break;
        case '3': dx =  1; dy =  1; break;
        case '1': dx = -1; dy =  1; break;

        default:
            break;
    }

    if (0xFF != dx && 0xFF != dy) {
        if ((dx | dy) == 0) {
            fScrollTestX = fScrollTestY = 0;
        } else {
            fScrollTestX += dx;
            fScrollTestY += dy;
        }
        this->inval(NULL);
        return true;
    }

    switch (uni) {
        case 'a':
            fAnimating = !fAnimating;
            this->postAnimatingEvent();
            this->updateTitle();
            return true;
        case 'b':
            fAAState = cycle_tristate(fAAState);
            this->updateTitle();
            this->inval(NULL);
            break;
        case 'c':
            fUseClip = !fUseClip;
            this->inval(NULL);
            this->updateTitle();
            return true;
        case 'd':
            SkGraphics::SetFontCacheUsed(0);
            return true;
        case 'f':
            fMeasureFPS = !fMeasureFPS;
            this->inval(NULL);
            break;
        case 'g':
            fRequestGrabImage = true;
            this->inval(NULL);
            break;
        case 'h':
            fHintingState = cycle_tristate(fHintingState);
            this->updateTitle();
            this->inval(NULL);
            break;
        case 'i':
            this->zoomIn();
            break;
        case 'l':
            fLCDState = cycle_tristate(fLCDState);
            this->updateTitle();
            this->inval(NULL);
            break;
        case 'n':
            fFilterState = cycle_tristate(fFilterState);
            this->updateTitle();
            this->inval(NULL);
            break;
        case 'o':
            this->zoomOut();
            break;
        case 'p':
            fUsePipe = !fUsePipe;
            this->updateTitle();
            this->inval(NULL);
            break;
        case 'r':
            fRotate = !fRotate;
            this->inval(NULL);
            this->updateTitle();
            return true;
        case 's':
            fScale = !fScale;
            this->inval(NULL);
            this->updateTitle();
            return true;
        case 'x':
            fFlipAxis ^= kFlipAxis_X;
            this->updateTitle();
            this->inval(NULL);
            break;
        case 'y':
            fFlipAxis ^= kFlipAxis_Y;
            this->updateTitle();
            this->inval(NULL);
            break;
        case 'z':
            this->toggleZoomer();
            break;
        default:
            break;
    }

    return this->INHERITED::onHandleChar(uni);
}

#include "SkDumpCanvas.h"

bool SampleWindow::onHandleKey(SkKey key) {
    {
        SkView* view = curr_view(this);
        if (view) {
            SkEvent evt(gKeyEvtName);
            evt.setFast32(key);
            if (view->doQuery(&evt)) {
                return true;
            }
        }
    }

    switch (key) {
        case kRight_SkKey:
            if (this->nextSample()) {
                return true;
            }
            break;
        case kLeft_SkKey:
            fCanvasType = cycle_canvastype(fCanvasType);
            this->updateTitle();
            this->inval(NULL);
            return true;
        case kUp_SkKey:
            if (USE_ARROWS_FOR_ZOOM) {
                this->changeZoomLevel(1);
            } else {
                fNClip = !fNClip;
                this->inval(NULL);
            }
            this->updateTitle();
            return true;
        case kDown_SkKey:
            if (USE_ARROWS_FOR_ZOOM) {
                this->changeZoomLevel(-1);
            } else {
                this->setConfig(cycle_configs(this->getBitmap().config()));
            }
            this->updateTitle();
            return true;
        case kOK_SkKey:
            if (false) {
                SkDebugfDumper dumper;
                SkDumpCanvas dc(&dumper);
                this->draw(&dc);
            } else {
                fRepeatDrawing = !fRepeatDrawing;
                if (fRepeatDrawing) {
                    this->inval(NULL);
                }
            }
            return true;
        case kBack_SkKey:
            this->loadView(NULL);
            return true;
        default:
            break;
    }
    return this->INHERITED::onHandleKey(key);
}

///////////////////////////////////////////////////////////////////////////////

static const char gGestureClickType[] = "GestureClickType";

bool SampleWindow::onDispatchClick(int x, int y, Click::State state) {
    if (Click::kMoved_State == state) {
        updatePointer(x, y);
    }
    int w = SkScalarRound(this->width());
    int h = SkScalarRound(this->height());

    // check for the resize-box
    if (w - x < 16 && h - y < 16) {
        return false;   // let the OS handle the click
    } else {
        return this->INHERITED::onDispatchClick(x, y, state);
    }
}

class GestureClick : public SkView::Click {
public:
    GestureClick(SkView* target) : SkView::Click(target) {
        this->setType(gGestureClickType);
    }

    static bool IsGesture(Click* click) {
        return click->isType(gGestureClickType);
    }
};

SkView::Click* SampleWindow::onFindClickHandler(SkScalar x, SkScalar y) {
    return new GestureClick(this);
}

bool SampleWindow::onClick(Click* click) {
    if (GestureClick::IsGesture(click)) {
        float x = SkScalarToFloat(click->fCurr.fX);
        float y = SkScalarToFloat(click->fCurr.fY);
        switch (click->fState) {
            case SkView::Click::kDown_State:
                fGesture.touchBegin(click, x, y);
                break;
            case SkView::Click::kMoved_State:
                fGesture.touchMoved(click, x, y);
                this->inval(NULL);
                break;
            case SkView::Click::kUp_State:
                fGesture.touchEnd(click);
                this->inval(NULL);
                break;
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void SampleWindow::loadView(SkView* view) {
    SkView::F2BIter iter(this);
    SkView* prev = iter.next();
    if (prev) {
        prev->detachFromParent();
    }

    if (NULL == view) {
        view = create_overview(fSamples.count(), fSamples.begin());
    }
    view->setVisibleP(true);
    view->setClipToBounds(false);
    this->attachChildToFront(view)->unref();
    view->setSize(this->width(), this->height());

    this->updateTitle();
}

static const char* gConfigNames[] = {
    "unknown config",
    "A1",
    "A8",
    "Index8",
    "565",
    "4444",
    "8888"
};

static const char* configToString(SkBitmap::Config c) {
    return gConfigNames[c];
}

static const char* gCanvasTypePrefix[] = {
    "raster: ",
    "picture: ",
    "opengl: "
};

static const char* trystate_str(SkTriState state,
                                const char trueStr[], const char falseStr[]) {
    if (kTrue_SkTriState == state) {
        return trueStr;
    } else if (kFalse_SkTriState == state) {
        return falseStr;
    }
    return NULL;
}

void SampleWindow::updateTitle() {
    SkString title;

    SkView::F2BIter iter(this);
    SkView* view = iter.next();
    SkEvent evt(gTitleEvtName);
    if (view->doQuery(&evt)) {
        title.set(evt.findString(gTitleEvtName));
    }
    if (title.size() == 0) {
        title.set("<unknown>");
    }

    title.prepend(gCanvasTypePrefix[fCanvasType]);

    title.prepend(" ");
    title.prepend(configToString(this->getBitmap().config()));

    if (fAnimating) {
        title.prepend("<A> ");
    }
    if (fScale) {
        title.prepend("<S> ");
    }
    if (fRotate) {
        title.prepend("<R> ");
    }
    if (fNClip) {
        title.prepend("<C> ");
    }

    title.prepend(trystate_str(fLCDState, "LCD ", "lcd "));
    title.prepend(trystate_str(fAAState, "AA ", "aa "));
    title.prepend(trystate_str(fFilterState, "H ", "h "));
    title.prepend(fFlipAxis & kFlipAxis_X ? "X " : NULL);
    title.prepend(fFlipAxis & kFlipAxis_Y ? "Y " : NULL);

    if (fZoomLevel) {
        title.prependf("{%d} ", fZoomLevel);
    }
    
    if (fMeasureFPS) {
        title.appendf(" %6.1f ms", fMeasureFPS_Time / (float)FPS_REPEAT_MULTIPLIER);
    }
    if (fUsePipe && SampleView::IsSampleView(view)) {
        title.prepend("<P> ");
    }
    if (SampleView::IsSampleView(view)) {
        title.prepend("! ");
    }

    this->setTitle(title.c_str());
}

void SampleWindow::onSizeChange() {
    this->INHERITED::onSizeChange();

    SkView::F2BIter iter(this);
    SkView* view = iter.next();
    view->setSize(this->width(), this->height());

    // rebuild our clippath
    {
        const SkScalar W = this->width();
        const SkScalar H = this->height();

        fClipPath.reset();
#if 0
        for (SkScalar y = SK_Scalar1; y < H; y += SkIntToScalar(32)) {
            SkRect r;
            r.set(SK_Scalar1, y, SkIntToScalar(30), y + SkIntToScalar(30));
            for (; r.fLeft < W; r.offset(SkIntToScalar(32), 0))
                fClipPath.addRect(r);
        }
#else
        SkRect r;
        r.set(0, 0, W, H);
        fClipPath.addRect(r, SkPath::kCCW_Direction);
        r.set(W/4, H/4, W*3/4, H*3/4);
        fClipPath.addRect(r, SkPath::kCW_Direction);
#endif
    }

    this->updateTitle();    // to refresh our config
}

///////////////////////////////////////////////////////////////////////////////

static const char is_sample_view_tag[] = "sample-is-sample-view";
static const char repeat_count_tag[] = "sample-set-repeat-count";
static const char set_use_pipe_tag[] = "sample-set-use-pipe";

bool SampleView::IsSampleView(SkView* view) {
    SkEvent evt(is_sample_view_tag);
    return view->doQuery(&evt);
}

bool SampleView::SetRepeatDraw(SkView* view, int count) {
    SkEvent evt(repeat_count_tag);
    evt.setFast32(count);
    return view->doEvent(evt);
}

bool SampleView::SetUsePipe(SkView* view, bool pred) {
    SkEvent evt(set_use_pipe_tag);
    evt.setFast32(pred);
    return view->doEvent(evt);
}

bool SampleView::onEvent(const SkEvent& evt) {
    if (evt.isType(repeat_count_tag)) {
        fRepeatCount = evt.getFast32();
        return true;
    }
    if (evt.isType(set_use_pipe_tag)) {
        fUsePipe = !!evt.getFast32();
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

bool SampleView::onQuery(SkEvent* evt) {
    if (evt->isType(is_sample_view_tag)) {
        return true;
    }
    return this->INHERITED::onQuery(evt);
}

#ifdef TEST_GPIPE
    #include "SkGPipe.h"

class SimplePC : public SkGPipeController {
public:
    SimplePC(SkCanvas* target);
    ~SimplePC();

    virtual void* requestBlock(size_t minRequest, size_t* actual);
    virtual void notifyWritten(size_t bytes);

private:
    SkGPipeReader   fReader;
    void*           fBlock;
    size_t          fBlockSize;
    size_t          fBytesWritten;
    int             fAtomsWritten;
    SkGPipeReader::Status   fStatus;

    size_t        fTotalWritten;
};

SimplePC::SimplePC(SkCanvas* target) : fReader(target) {
    fBlock = NULL;
    fBlockSize = fBytesWritten = 0;
    fStatus = SkGPipeReader::kDone_Status;
    fTotalWritten = 0;
    fAtomsWritten = 0;
}

SimplePC::~SimplePC() {
//    SkASSERT(SkGPipeReader::kDone_Status == fStatus);
    sk_free(fBlock);

    if (fTotalWritten) {
        SkDebugf("--- %d bytes %d atoms, status %d\n", fTotalWritten,
                 fAtomsWritten, fStatus);
    }
}

void* SimplePC::requestBlock(size_t minRequest, size_t* actual) {
    sk_free(fBlock);

    fBlockSize = minRequest * 4;
    fBlock = sk_malloc_throw(fBlockSize);
    fBytesWritten = 0;
    *actual = fBlockSize;
    return fBlock;
}

void SimplePC::notifyWritten(size_t bytes) {
    SkASSERT(fBytesWritten + bytes <= fBlockSize);
    
#ifdef  PIPE_FILE
    //File is open in append mode
    FILE* f = fopen(FILE_PATH, "ab");
    SkASSERT(f != NULL);
    fwrite((const char*)fBlock + fBytesWritten, 1, bytes, f);
    fclose(f);
#endif
    
    fStatus = fReader.playback((const char*)fBlock + fBytesWritten, bytes);
    SkASSERT(SkGPipeReader::kError_Status != fStatus);
    fBytesWritten += bytes;
    fTotalWritten += bytes;

    fAtomsWritten += 1;
}

#endif


void SampleView::onDraw(SkCanvas* canvas) {
#ifdef TEST_GPIPE
    SimplePC controller(canvas);
    SkGPipeWriter writer;
    if (fUsePipe) {
        uint32_t flags = SkGPipeWriter::kCrossProcess_Flag;
//        flags = 0;
        canvas = writer.startRecording(&controller, flags);
    }
#endif

    this->onDrawBackground(canvas);

    for (int i = 0; i < fRepeatCount; i++) {
        SkAutoCanvasRestore acr(canvas, true);
        this->onDrawContent(canvas);
    }
}

void SampleView::onDrawBackground(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T> void SkTBSort(T array[], int count) {
    for (int i = 1; i < count - 1; i++) {
        bool didSwap = false;
        for (int j = count - 1; j > i; --j) {
            if (array[j] < array[j-1]) {
                T tmp(array[j-1]);
                array[j-1] = array[j];
                array[j] = tmp;
                didSwap = true;
            }
        }
        if (!didSwap) {
            break;
        }
    }

    for (int k = 0; k < count - 1; k++) {
        SkASSERT(!(array[k+1] < array[k]));
    }
}

#include "SkRandom.h"

static void rand_rect(SkIRect* rect, SkRandom& rand) {
    int bits = 8;
    int shift = 32 - bits;
    rect->set(rand.nextU() >> shift, rand.nextU() >> shift,
              rand.nextU() >> shift, rand.nextU() >> shift);
    rect->sort();
}

static void dumpRect(const SkIRect& r) {
    SkDebugf(" { %d, %d, %d, %d },\n",
             r.fLeft, r.fTop,
             r.fRight, r.fBottom);
}

static void test_rects(const SkIRect rect[], int count) {
    SkRegion rgn0, rgn1;

    for (int i = 0; i < count; i++) {
        rgn0.op(rect[i], SkRegion::kUnion_Op);
     //   dumpRect(rect[i]);
    }
    rgn1.setRects(rect, count);

    if (rgn0 != rgn1) {
        SkDebugf("\n");
        for (int i = 0; i < count; i++) {
            dumpRect(rect[i]);
        }
        SkDebugf("\n");
    }
}

static void test() {
    size_t i;

    const SkIRect r0[] = {
        { 0, 0, 1, 1 },
        { 2, 2, 3, 3 },
    };
    const SkIRect r1[] = {
        { 0, 0, 1, 3 },
        { 1, 1, 2, 2 },
        { 2, 0, 3, 3 },
    };
    const SkIRect r2[] = {
        { 0, 0, 1, 2 },
        { 2, 1, 3, 3 },
        { 4, 0, 5, 1 },
        { 6, 0, 7, 4 },
    };

    static const struct {
        const SkIRect* fRects;
        int            fCount;
    } gRecs[] = {
        { r0, SK_ARRAY_COUNT(r0) },
        { r1, SK_ARRAY_COUNT(r1) },
        { r2, SK_ARRAY_COUNT(r2) },
    };

    for (i = 0; i < SK_ARRAY_COUNT(gRecs); i++) {
        test_rects(gRecs[i].fRects, gRecs[i].fCount);
    }

    SkRandom rand;
    for (i = 0; i < 10000; i++) {
        SkRegion rgn0, rgn1;

        const int N = 8;
        SkIRect rect[N];
        for (int j = 0; j < N; j++) {
            rand_rect(&rect[j], rand);
        }
        test_rects(rect, N);
    }
}

SkOSWindow* create_sk_window(void* hwnd) {
//    test();
    return new SampleWindow(hwnd);
}

void get_preferred_size(int* x, int* y, int* width, int* height) {
    *x = 10;
    *y = 50;
    *width = 640;
    *height = 480;
}

void application_init() {
//    setenv("ANDROID_ROOT", "../../../data", 0);
#ifdef SK_BUILD_FOR_MAC
    setenv("ANDROID_ROOT", "/android/device/data", 0);
#endif
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}

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

//#define DEFAULT_TO_GPU

extern SkView* create_overview(int, const SkViewFactory[]);

#define SK_SUPPORT_GL

#define ANIMATING_EVENTTYPE "nextSample"
#define ANIMATING_DELAY     750

#ifdef SK_SUPPORT_GL
    #include "GrGLConfig.h"
#endif

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

#if 1
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

#include "SkDrawFilter.h"

class LCDTextDrawFilter : public SkDrawFilter {
public:
    enum Mode {
        kNeutral_Mode,
        kForceOn_Mode,
        kForceOff_Mode
    };

    LCDTextDrawFilter(Mode mode) : fMode(mode) {}

    virtual bool filter(SkCanvas*, SkPaint* paint, Type t) {
        if (kText_Type == t && kNeutral_Mode != fMode) {
            fPrevLCD = paint->isLCDRenderText();
            paint->setLCDRenderText(kForceOn_Mode == fMode);
        }
        return true;
    }

    /** If filter() returned true, then restore() will be called to restore the
     canvas/paint to their previous states
     */
    virtual void restore(SkCanvas*, SkPaint* paint, Type t) {
        if (kText_Type == t && kNeutral_Mode != fMode) {
            paint->setLCDRenderText(fPrevLCD);
        }
    }

private:
    Mode    fMode;
    bool    fPrevLCD;
};

LCDTextDrawFilter::Mode cycle_lcdmode(LCDTextDrawFilter::Mode mode) {
    static const LCDTextDrawFilter::Mode gCycle[] = {
        /* kNeutral_Mode  -> */  LCDTextDrawFilter::kForceOn_Mode,
        /* kForceOn_Mode  -> */  LCDTextDrawFilter::kForceOff_Mode,
        /* kForceOff_Mode -> */  LCDTextDrawFilter::kNeutral_Mode
    };
    return gCycle[mode];
}

//////////////////////////////////////////////////////////////////////////////

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

#if 0
    virtual bool handleChar(SkUnichar uni);
    virtual bool handleEvent(const SkEvent& evt);
    virtual bool handleKey(SkKey key);
    virtual bool handleKeyUp(SkKey key);

    virtual bool onClick(Click* click);
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y);
    virtual bool onHandleKeyUp(SkKey key);
#endif

private:
    int fCurrIndex;

    SkPicture* fPicture;
    SkGpuCanvas* fGpuCanvas;
    GrContext* fGrContext;
    SkPath fClipPath;

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

    LCDTextDrawFilter::Mode fLCDMode;

    int fScrollTestX, fScrollTestY;

    bool make3DReady();

    void loadView(SkView*);
    void updateTitle();
    bool nextSample();

    void postAnimatingEvent() {
        if (fAnimating) {
            SkEvent* evt = new SkEvent(ANIMATING_EVENTTYPE);
            evt->post(this->getSinkID(), ANIMATING_DELAY);
        }
    }


    static CanvasType cycle_canvastype(CanvasType);

    typedef SkOSWindow INHERITED;
};

bool SampleWindow::make3DReady() {

#if defined(SK_SUPPORT_GL)
    if (attachGL()) {
        if (NULL == fGrContext) {
        #if defined(SK_USE_SHADERS)
            fGrContext = GrContext::Create(GrGpu::kOpenGL_Shaders_Engine, NULL);
        #else
            fGrContext = GrContext::Create(GrGpu::kOpenGL_Fixed_Engine, NULL);
        #endif
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
    fLCDMode = LCDTextDrawFilter::kNeutral_Mode;
    fScrollTestX = fScrollTestY = 0;

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

    testpdf();
}

SampleWindow::~SampleWindow() {
    delete fPicture;
    delete fGpuCanvas;
    if (NULL != fGrContext) {
        fGrContext->unref();
    }
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

#define XCLIP_N  8
#define YCLIP_N  8

void SampleWindow::draw(SkCanvas* canvas) {
    // update the animation time
    gAnimTimePrev = gAnimTime;
    gAnimTime = SkTime::GetMSecs();

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
    SkIPoint viewport;
    bool alreadyGPU = canvas->getViewport(&viewport);

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
            if (!alreadyGPU && make3DReady()) {
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
        SkBitmap bitmap;
        SkIRect bounds = {
            0, 0,
            SkScalarRound(this->width()),
            SkScalarRound(this->height())
        };
        if (device->readPixels(bounds, &bitmap)) {
            static int gSampleGrabCounter;
            SkString name;
            name.printf("sample_grab_%d", gSampleGrabCounter++);
            SkImageEncoder::EncodeFile(name.c_str(), bitmap,
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
            delete fGpuCanvas;
            fGpuCanvas = NULL;
            presentGL();
            break;
#endif
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

    if (LCDTextDrawFilter::kNeutral_Mode != fLCDMode) {
        canvas->setDrawFilter(new LCDTextDrawFilter(fLCDMode))->unref();
    }
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
        case 'f': {
            const char* title = this->getTitle();
            if (title[0] == 0) {
                title = "sampleapp";
            }
            SkString name(title);
            cleanup_for_filename(&name);
            name.append(".png");
            if (SkImageEncoder::EncodeFile(name.c_str(), this->getBitmap(),
                                           SkImageEncoder::kPNG_Type, 100)) {
                SkDebugf("Created %s\n", name.c_str());
            }
            return true;
        }
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
        case 'c':
            fUseClip = !fUseClip;
            this->inval(NULL);
            this->updateTitle();
            return true;
        case 'd':
            SkGraphics::SetFontCacheUsed(0);
            return true;
        case 'g':
            fRequestGrabImage = true;
            this->inval(NULL);
            break;
        case 'l':
            fLCDMode = cycle_lcdmode(fLCDMode);
            this->updateTitle();
            this->inval(NULL);
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
            fNClip = !fNClip;
            this->updateTitle();
            this->inval(NULL);
            return true;
        case kDown_SkKey:
            this->setConfig(cycle_configs(this->getBitmap().config()));
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
    if (LCDTextDrawFilter::kForceOn_Mode == fLCDMode) {
        title.prepend("LCD ");
    } else if (LCDTextDrawFilter::kForceOff_Mode == fLCDMode) {
        title.prepend("lcd ");
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

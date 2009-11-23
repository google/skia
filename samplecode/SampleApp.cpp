#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGLCanvas.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTime.h"
#include "SkWindow.h"

#include "SampleCode.h"

SkView* create_overview(int, const SkViewFactory*);

//#define SK_SUPPORT_GL

#ifdef SK_SUPPORT_GL
#include <AGL/agl.h>
#include <OpenGL/gl.h>
#endif

#define ANIMATING_EVENTTYPE "nextSample"
#define ANIMATING_DELAY     750

#define USE_OFFSCREEN

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

#ifdef SK_SUPPORT_GL
static AGLContext   gAGLContext;

static void init_gl(WindowRef wref) {
    GLint major, minor;
    
    aglGetVersion(&major, &minor);
    SkDebugf("---- agl version %d %d\n", major, minor);
    
    const GLint pixelAttrs[] = {
        AGL_RGBA,
        AGL_DEPTH_SIZE, 32,
        AGL_OFFSCREEN,
        AGL_NONE
    };
    
    AGLPixelFormat format = aglCreatePixelFormat(pixelAttrs);
    SkDebugf("----- agl format %p\n", format);
    gAGLContext = aglCreateContext(format, NULL);
    SkDebugf("----- agl context %p\n", gAGLContext);
    aglDestroyPixelFormat(format);

    aglEnable(gAGLContext, GL_BLEND);
    aglEnable(gAGLContext, GL_LINE_SMOOTH);
    aglEnable(gAGLContext, GL_POINT_SMOOTH);
    aglEnable(gAGLContext, GL_POLYGON_SMOOTH);
    
    aglSetCurrentContext(gAGLContext);
}

static void setup_offscreen_gl(const SkBitmap& offscreen, WindowRef wref) {
    GLboolean success = true;

#ifdef USE_OFFSCREEN
    success = aglSetOffScreen(gAGLContext,
                                        offscreen.width(),
                                        offscreen.height(),
                                        offscreen.rowBytes(),
                                        offscreen.getPixels());
#else
    success = aglSetWindowRef(gAGLContext, wref);
#endif

    GLenum err = aglGetError();
    if (err) {
        SkDebugf("---- setoffscreen %d %d %s [%d %d]\n", success, err,
                 aglErrorString(err), offscreen.width(), offscreen.height());
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_TEXTURE_2D);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}
#endif

//////////////////////////////////////////////////////////////////////////////

static const char gTitleEvtName[] = "SampleCode_Title_Event";
static const char gPrefSizeEvtName[] = "SampleCode_PrefSize_Event";

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

static SkMSec gAnimTime;
SkMSec SampleCode::GetAnimTime() { return gAnimTime; }

SkScalar SampleCode::GetAnimScalar(SkScalar speed, SkScalar period) {
    SkScalar seconds = SkFloatToScalar(gAnimTime / 1000.0f);
    SkScalar value = SkScalarMul(speed, seconds);
    if (period) {
        value = SkScalarMod(value, period);
    }
    return value;
}

//////////////////////////////////////////////////////////////////////////////

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
    SkGLCanvas* fGLCanvas;
    SkPath fClipPath;
    
    enum CanvasType {
        kRaster_CanvasType,
        kPicture_CanvasType,
        kOpenGL_CanvasType
    };
    CanvasType fCanvasType;

    bool fUseClip;
    bool fNClip;
    bool fRepeatDrawing;
    bool fAnimating;
    bool fRotate;
    bool fScale;
    
    int fScrollTestX, fScrollTestY;
    
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

SampleWindow::CanvasType SampleWindow::cycle_canvastype(CanvasType ct) {
    static const CanvasType gCT[] = {
        kPicture_CanvasType,
        kOpenGL_CanvasType,
        kRaster_CanvasType
    };
    return gCT[ct];
}

SampleWindow::SampleWindow(void* hwnd) : INHERITED(hwnd) {
#ifdef SK_SUPPORT_GL
    init_gl((WindowRef)hwnd);
#endif

    fPicture = NULL;
    fGLCanvas = NULL;

    fCanvasType = kRaster_CanvasType;
    fUseClip = false;
    fNClip = false;
    fRepeatDrawing = false;
    fAnimating = false;
    fRotate = false;
    fScale = false;

    fScrollTestX = fScrollTestY = 0;

//	this->setConfig(SkBitmap::kRGB_565_Config);
	this->setConfig(SkBitmap::kARGB_8888_Config);
	this->setVisibleP(true);

    {
        const SkViewRegister* reg = SkViewRegister::Head();
        while (reg) {
            *fSamples.append() = reg->factory();
            reg = reg->next();
        }
    }
    fCurrIndex = 0;
    this->loadView(fSamples[fCurrIndex]());
}

SampleWindow::~SampleWindow() {
    delete fPicture;
    delete fGLCanvas;
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
#ifdef SK_SUPPORT_GL
#ifndef USE_OFFSCREEN
    aglSetWindowRef(gAGLContext, NULL);
#endif
#endif
    switch (fCanvasType) {
        case kRaster_CanvasType:
            canvas = this->INHERITED::beforeChildren(canvas);
            break;
        case kPicture_CanvasType:
            fPicture = new SkPicture;
            canvas = fPicture->beginRecording(9999, 9999);
            break;
#ifdef SK_SUPPORT_GL
        case kOpenGL_CanvasType: {
            //SkGLCanvas::DeleteAllTextures();  // just for testing
            SkDevice* device = canvas->getDevice();
            const SkBitmap& bitmap = device->accessBitmap(true);
            // first clear the raster bitmap, so we don't see any leftover bits
            bitmap.eraseColor(0);
            // now setup our glcanvas
            setup_offscreen_gl(bitmap, (WindowRef)this->getHWND());
            fGLCanvas = new SkGLCanvas;
            fGLCanvas->setViewport(bitmap.width(), bitmap.height());
            canvas = fGLCanvas;
            break;
        }
#endif
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
        case kOpenGL_CanvasType:
            glFlush();
            delete fGLCanvas;
            fGLCanvas = NULL;
#ifdef USE_OFFSCREEN
            reverseRedAndBlue(orig->getDevice()->accessBitmap(true));
#endif
            break;
#endif
    }
    
//    if ((fScrollTestX | fScrollTestY) != 0)
    {
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
}

void SampleWindow::afterChild(SkView* child, SkCanvas* canvas) {
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

static void cleanup_for_filename(SkString* name) {
    char* str = name->writable_str();
    for (int i = 0; i < name->size(); i++) {
        switch (str[i]) {
            case ':': str[i] = '-'; break;
            case '/': str[i] = '-'; break;
            case ' ': str[i] = '_'; break;
            default: break;
        }
    }
}

bool SampleWindow::onHandleChar(SkUnichar uni) {
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
        default:
            break;
    }
    
    return this->INHERITED::onHandleChar(uni);
}

#include "SkDumpCanvas.h"

bool SampleWindow::onHandleKey(SkKey key) {
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
            if (true) {
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

SkOSWindow* create_sk_window(void* hwnd) {
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
    setenv("ANDROID_ROOT", "/android/device/data", 0);
	SkGraphics::Init();
	SkEvent::Init();
}

void application_term() {
	SkEvent::Term();
	SkGraphics::Term();
}

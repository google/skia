/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleApp.h"

#include "SkData.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTime.h"
#include "SkWindow.h"

#include "SampleCode.h"
#include "GrContext.h"
#include "SkTypeface.h"

#include "gl/GrGLInterface.h"
#include "GrRenderTarget.h"

#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkStream.h"

#define TEST_GPIPE

#ifdef  TEST_GPIPE
#define PIPE_FILEx
#ifdef  PIPE_FILE
#define FILE_PATH "/path/to/drawing.data"
#endif

#define PIPE_NETx
#ifdef  PIPE_NET
#include "SkSockets.h"
SkTCPServer gServer;
#endif

#define DEBUGGERx
#ifdef  DEBUGGER
extern SkView* create_debugger(const char* data, size_t size);
extern bool is_debugger(SkView* view);
SkTDArray<char> gTempDataStore;
#endif

#endif

#define USE_ARROWS_FOR_ZOOM true
//#define DEFAULT_TO_GPU

extern SkView* create_overview(int, const SkViewFactory*[]);
extern bool is_overview(SkView* view);
extern SkView* create_transition(SkView*, SkView*, int);
extern bool is_transition(SkView* view);


#define ANIMATING_EVENTTYPE "nextSample"
#define ANIMATING_DELAY     750

#ifdef SK_DEBUG
    #define FPS_REPEAT_MULTIPLIER   1
#else
    #define FPS_REPEAT_MULTIPLIER   10
#endif
#define FPS_REPEAT_COUNT    (10 * FPS_REPEAT_MULTIPLIER)

static SampleWindow* gSampleWindow;

static void postEventToSink(SkEvent* evt, SkEventSink* sink) {
    evt->setTargetID(sink->getSinkID())->post();
}

///////////////////////////////////////////////////////////////////////////////

static const char* skip_until(const char* str, const char* skip) {
    if (!str) {
        return NULL;
    }
    return strstr(str, skip);
}

static const char* skip_past(const char* str, const char* skip) {
    const char* found = skip_until(str, skip);
    if (!found) {
        return NULL;
    }
    return found + strlen(skip);
}

static const char* gPrefFileName = "sampleapp_prefs.txt";

static bool readTitleFromPrefs(SkString* title) {
    SkFILEStream stream(gPrefFileName);
    if (!stream.isValid()) {
        return false;
    }

    int len = stream.getLength();
    SkString data(len);
    stream.read(data.writable_str(), len);
    const char* s = data.c_str();

    s = skip_past(s, "curr-slide-title");
    s = skip_past(s, "=");
    s = skip_past(s, "\"");
    const char* stop = skip_until(s, "\"");
    if (stop > s) {
        title->set(s, stop - s);
        return true;
    }
    return false;
}

static void writeTitleToPrefs(const char* title) {
    SkFILEWStream stream(gPrefFileName);
    SkString data;
    data.printf("curr-slide-title = \"%s\"\n", title);
    stream.write(data.c_str(), data.size());
}

///////////////////////////////////////////////////////////////////////////////

class SampleWindow::DefaultDeviceManager : public SampleWindow::DeviceManager {
public:

    DefaultDeviceManager() {
        fGrRenderTarget = NULL;
        fGrContext = NULL;
        fGL = NULL;
        fNullGrContext = NULL;
        fNullGrRenderTarget = NULL;
    }

    virtual ~DefaultDeviceManager() {
        SkSafeUnref(fGrRenderTarget);
        SkSafeUnref(fGrContext);
        SkSafeUnref(fGL);
        SkSafeUnref(fNullGrContext);
        SkSafeUnref(fNullGrRenderTarget);
    }

    virtual void init(SampleWindow* win) {
        if (!win->attachGL()) {
            SkDebugf("Failed to initialize GL");
        }
        if (NULL == fGL) {
            fGL = GrGLCreateNativeInterface();
            GrAssert(NULL == fGrContext);
            fGrContext = GrContext::Create(kOpenGL_Shaders_GrEngine,
                                           (GrPlatform3DContext) fGL);
        }
        if (NULL == fGrContext || NULL == fGL) {
            SkSafeUnref(fGrContext);
            SkSafeUnref(fGL);
            SkDebugf("Failed to setup 3D");
            win->detachGL();
        }
        if (NULL == fNullGrContext) {
            const GrGLInterface* nullGL = GrGLCreateNullInterface();
            fNullGrContext = GrContext::Create(kOpenGL_Shaders_GrEngine,
                                               (GrPlatform3DContext) nullGL);
            nullGL->unref();
        }
    }

    virtual bool supportsDeviceType(SampleWindow::DeviceType dType) {
        switch (dType) {
            case kRaster_DeviceType:
            case kPicture_DeviceType: // fallthru
                return true;
            case kGPU_DeviceType:
                return NULL != fGrContext && NULL != fGrRenderTarget;
            case kNullGPU_DeviceType:
                return NULL != fNullGrContext && NULL != fNullGrRenderTarget;
            default:
                return false;
        }
    }

    virtual bool prepareCanvas(SampleWindow::DeviceType dType,
                               SkCanvas* canvas,
                               SampleWindow* win) {
        switch (dType) {
            case kGPU_DeviceType:
                if (fGrContext) {
                    canvas->setDevice(new SkGpuDevice(fGrContext,
                                                    fGrRenderTarget))->unref();
                } else {
                    return false;
                }
                break;
            case kNullGPU_DeviceType:
                if (fNullGrContext) {
                    canvas->setDevice(new SkGpuDevice(fNullGrContext,
                                                      fNullGrRenderTarget))->unref();
                } else {
                    return false;
                }
                break;
            case kRaster_DeviceType:
            case kPicture_DeviceType:
                break;
        }
        return true;
    }

    virtual void publishCanvas(SampleWindow::DeviceType dType,
                               SkCanvas* canvas,
                               SampleWindow* win) {
        if (fGrContext) {
            // in case we have queued drawing calls
            fGrContext->flush();
            if (NULL != fNullGrContext) {
                fNullGrContext->flush();
            }
            if (dType != kGPU_DeviceType &&
                dType != kNullGPU_DeviceType) {
                // need to send the raster bits to the (gpu) window
                fGrContext->setRenderTarget(fGrRenderTarget);
                const SkBitmap& bm = win->getBitmap();
                fGrRenderTarget->writePixels(0, 0, bm.width(), bm.height(),
                                             kSkia8888_PM_GrPixelConfig,
                                             bm.getPixels(),
                                             bm.rowBytes());
            }
        }
        win->presentGL();
    }

    virtual void windowSizeChanged(SampleWindow* win) {
        if (fGrContext) {
            win->attachGL();

            GrPlatformRenderTargetDesc desc;
            desc.fWidth = SkScalarRound(win->width());
            desc.fHeight = SkScalarRound(win->height());
            desc.fConfig = kSkia8888_PM_GrPixelConfig;
            GR_GL_GetIntegerv(fGL, GR_GL_SAMPLES, &desc.fSampleCnt);
            GR_GL_GetIntegerv(fGL, GR_GL_STENCIL_BITS, &desc.fStencilBits);
            GrGLint buffer;
            GR_GL_GetIntegerv(fGL, GR_GL_FRAMEBUFFER_BINDING, &buffer);
            desc.fRenderTargetHandle = buffer;

            SkSafeUnref(fGrRenderTarget);
            fGrRenderTarget = fGrContext->createPlatformRenderTarget(desc);
        }
        if (NULL != fNullGrContext) {
            GrPlatformRenderTargetDesc desc;
            desc.fWidth = SkScalarRound(win->width());
            desc.fHeight = SkScalarRound(win->height());
            desc.fConfig = kSkia8888_PM_GrPixelConfig;
            desc.fStencilBits = 8;
            desc.fSampleCnt = 0;
            desc.fRenderTargetHandle = 0;
            fNullGrRenderTarget = fNullGrContext->createPlatformRenderTarget(desc);
        }
    }

    virtual GrContext* getGrContext(SampleWindow::DeviceType dType) {
        if (kNullGPU_DeviceType == dType) {
            return fNullGrContext;
        } else {
            return fGrContext;
        }
    }
private:
    GrContext* fGrContext;
    const GrGLInterface* fGL;
    GrRenderTarget* fGrRenderTarget;
    GrContext* fNullGrContext;
    GrRenderTarget* fNullGrRenderTarget;
};

///////////////
static const char view_inval_msg[] = "view-inval-msg";

void SampleWindow::postInvalDelay() {
    (new SkEvent(view_inval_msg, this->getSinkID()))->postDelay(1);
}

static bool isInvalEvent(const SkEvent& evt) {
    return evt.isType(view_inval_msg);
}
//////////////////

SkFuncViewFactory::SkFuncViewFactory(SkViewCreateFunc func)
    : fCreateFunc(func) {
}

SkView* SkFuncViewFactory::operator() () const {
    return (*fCreateFunc)();
}

#include "GMSampleView.h"

SkGMSampleViewFactory::SkGMSampleViewFactory(GMFactoryFunc func)
    : fFunc(func) {
}

SkView* SkGMSampleViewFactory::operator() () const {
    return new GMSampleView(fFunc(NULL));
}

SkViewRegister* SkViewRegister::gHead;
SkViewRegister::SkViewRegister(SkViewFactory* fact) : fFact(fact) {
    fFact->ref();
    fChain = gHead;
    gHead = this;
}

SkViewRegister::SkViewRegister(SkViewCreateFunc func) {
    fFact = new SkFuncViewFactory(func);
    fChain = gHead;
    gHead = this;
}

SkViewRegister::SkViewRegister(GMFactoryFunc func) {
    fFact = new SkGMSampleViewFactory(func);
    fChain = gHead;
    gHead = this;
}

class AutoUnrefArray {
public:
    AutoUnrefArray() {}
    ~AutoUnrefArray() {
        int count = fObjs.count();
        for (int i = 0; i < count; ++i) {
            fObjs[i]->unref();
        }
    }
    SkRefCnt*& push_back() { return *fObjs.append(); }
    
private:
    SkTDArray<SkRefCnt*> fObjs;
};

// registers GMs as Samples
// This can't be performed during static initialization because it could be
// run before GMRegistry has been fully built.
void SkGMRegistyToSampleRegistry() {
    static bool gOnce;
    static AutoUnrefArray fRegisters; 

    if (!gOnce) {
        const skiagm::GMRegistry* gmreg = skiagm::GMRegistry::Head();
        while (gmreg) {
            fRegisters.push_back() = new SkViewRegister(gmreg->factory());
            gmreg = gmreg->next();
        }
        gOnce = true;
    }
}

#if 0
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

#include "SkDrawFilter.h"

class FlagsDrawFilter : public SkDrawFilter {
public:
    FlagsDrawFilter(SkOSMenu::TriState lcd, SkOSMenu::TriState aa, SkOSMenu::TriState filter,
                    SkOSMenu::TriState hinting) :
        fLCDState(lcd), fAAState(aa), fFilterState(filter), fHintingState(hinting) {}

    virtual void filter(SkPaint* paint, Type t) {
        if (kText_Type == t && SkOSMenu::kMixedState != fLCDState) {
            paint->setLCDRenderText(SkOSMenu::kOnState == fLCDState);
        }
        if (SkOSMenu::kMixedState != fAAState) {
            paint->setAntiAlias(SkOSMenu::kOnState == fAAState);
        }
        if (SkOSMenu::kMixedState != fFilterState) {
            paint->setFilterBitmap(SkOSMenu::kOnState == fFilterState);
        }
        if (SkOSMenu::kMixedState != fHintingState) {
            paint->setHinting(SkOSMenu::kOnState == fHintingState ?
                              SkPaint::kNormal_Hinting :
                              SkPaint::kSlight_Hinting);
        }
    }

private:
    SkOSMenu::TriState  fLCDState;
    SkOSMenu::TriState  fAAState;
    SkOSMenu::TriState  fFilterState;
    SkOSMenu::TriState  fHintingState;
};

//////////////////////////////////////////////////////////////////////////////

#define MAX_ZOOM_LEVEL  8
#define MIN_ZOOM_LEVEL  -8

static const char gCharEvtName[] = "SampleCode_Char_Event";
static const char gKeyEvtName[] = "SampleCode_Key_Event";
static const char gTitleEvtName[] = "SampleCode_Title_Event";
static const char gPrefSizeEvtName[] = "SampleCode_PrefSize_Event";
static const char gFastTextEvtName[] = "SampleCode_FastText_Event";
static const char gUpdateWindowTitleEvtName[] = "SampleCode_UpdateWindowTitle";

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

bool SampleCode::RequestTitle(SkView* view, SkString* title) {
    SkEvent evt(gTitleEvtName);
    if (view->doQuery(&evt)) {
        title->set(evt.findString(gTitleEvtName));
        return true;
    }
    return false;
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

GrContext* SampleCode::GetGr() {
    return gSampleWindow ? gSampleWindow->getGrContext() : NULL;
}

// some GMs rely on having a skiagm::GetGr function defined
namespace skiagm {
    GrContext* GetGr() { return SampleCode::GetGr(); }
}

//////////////////////////////////////////////////////////////////////////////

static SkView* curr_view(SkWindow* wind) {
    SkView::F2BIter iter(wind);
    return iter.next();
}

static bool curr_title(SkWindow* wind, SkString* title) {
    SkView* view = curr_view(wind);
    if (view) {
        SkEvent evt(gTitleEvtName);
        if (view->doQuery(&evt)) {
            title->set(evt.findString(gTitleEvtName));
            return true;
        }
    }
    return false;
}

void SampleWindow::setZoomCenter(float x, float y)
{
    fZoomCenterX = SkFloatToScalar(x);
    fZoomCenterY = SkFloatToScalar(y);
}

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

void SampleWindow::updatePointer(int x, int y)
{
    fMouseX = x;
    fMouseY = y;
    if (fShowZoomer) {
        this->inval(NULL);
    }
}

static inline SampleWindow::DeviceType cycle_devicetype(SampleWindow::DeviceType ct) {
    static const SampleWindow::DeviceType gCT[] = {
        SampleWindow::kPicture_DeviceType,
        SampleWindow::kGPU_DeviceType,
        SampleWindow::kRaster_DeviceType, // skip the null gpu device in normal cycling
        SampleWindow::kRaster_DeviceType
    };
    return gCT[ct];
}

static void usage(const char * argv0) {
    SkDebugf("%s [sampleName] [-i resourcePath]\n", argv0);
    SkDebugf("    sampleName: sample at which to start.\n");
    SkDebugf("    resourcePath: directory that stores image resources.\n");
}

SampleWindow::SampleWindow(void* hwnd, int argc, char** argv, DeviceManager* devManager) : INHERITED(hwnd) {

    const char* resourcePath = NULL;
    fCurrIndex = -1;

    const char* const commandName = argv[0];
    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-i") == 0) {
            argv++;
            if (argv < stop && **argv) {
                resourcePath = *argv;
            }
        } else {
            fCurrIndex = findByTitle(*argv);
            if (fCurrIndex < 0) {
                fprintf(stderr, "Unknown sample \"%s\"\n", *argv);
            }
        }
    }

    if (fCurrIndex < 0) {
        SkString title;
        if (readTitleFromPrefs(&title)) {
            fCurrIndex = findByTitle(title.c_str());
        }
    }

    if (fCurrIndex < 0) {
        fCurrIndex = 0;
    }

    gSampleWindow = this;

#ifdef  PIPE_FILE
    //Clear existing file or create file if it doesn't exist
    FILE* f = fopen(FILE_PATH, "wb");
    fclose(f);
#endif
     
    fPicture = NULL;
    
#ifdef DEFAULT_TO_GPU
    fDeviceType = kGPU_DeviceType;
#else
    fDeviceType = kRaster_DeviceType;
#endif
    fUseClip = false;
    fNClip = false;
    fAnimating = false;
    fRotate = false;
    fPerspAnim = false;
    fPerspAnimTime = 0;
    fScale = false;
    fRequestGrabImage = false;
    fUsePipe = false;
    fMeasureFPS = false;
    fLCDState = SkOSMenu::kMixedState;
    fAAState = SkOSMenu::kMixedState;
    fFilterState = SkOSMenu::kMixedState;
    fHintingState = SkOSMenu::kMixedState;
    fFlipAxis = 0;
    fScrollTestX = fScrollTestY = 0;

    fMouseX = fMouseY = 0;
    fFatBitsScale = 8;
    fTypeface = SkTypeface::CreateFromTypeface(NULL, SkTypeface::kBold);
    fShowZoomer = false;
    
    fZoomLevel = 0;
    fZoomScale = SK_Scalar1;
    
    fMagnify = false;
    fDebugger = false;
    
    fSaveToPdf = false;
    fPdfCanvas = NULL;

    fTransitionNext = 6;
    fTransitionPrev = 2;
    
    int sinkID = this->getSinkID();
    fAppMenu.setTitle("Global Settings");
    int itemID;
    
    itemID =fAppMenu.appendList("Device Type", "Device Type", sinkID, 0, 
                                "Raster", "Picture", "OpenGL", NULL);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'd');
    itemID = fAppMenu.appendTriState("AA", "AA", sinkID, fAAState);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'b');
    itemID = fAppMenu.appendTriState("LCD", "LCD", sinkID, fLCDState);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'l');
    itemID = fAppMenu.appendTriState("Filter", "Filter", sinkID, fFilterState);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'n');
    itemID = fAppMenu.appendTriState("Hinting", "Hinting", sinkID, fHintingState);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'h');
    fUsePipeMenuItemID = fAppMenu.appendSwitch("Pipe", "Pipe" , sinkID, fUsePipe);    
    fAppMenu.assignKeyEquivalentToItem(fUsePipeMenuItemID, 'p');
#ifdef DEBUGGER
    itemID = fAppMenu.appendSwitch("Debugger", "Debugger", sinkID, fDebugger);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'q');
#endif
    itemID = fAppMenu.appendSwitch("Slide Show", "Slide Show" , sinkID, false);    
    fAppMenu.assignKeyEquivalentToItem(itemID, 'a');    
    itemID = fAppMenu.appendSwitch("Clip", "Clip" , sinkID, fUseClip);    
    fAppMenu.assignKeyEquivalentToItem(itemID, 'c');
    itemID = fAppMenu.appendSwitch("Flip X", "Flip X" , sinkID, false); 
    fAppMenu.assignKeyEquivalentToItem(itemID, 'x');
    itemID = fAppMenu.appendSwitch("Flip Y", "Flip Y" , sinkID, false);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'y');
    itemID = fAppMenu.appendSwitch("Zoomer", "Zoomer" , sinkID, fShowZoomer);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'z');
    itemID = fAppMenu.appendSwitch("Magnify", "Magnify" , sinkID, fMagnify);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'm');
    itemID =fAppMenu.appendList("Transition-Next", "Transition-Next", sinkID, 
                                fTransitionNext, "Up", "Up and Right", "Right", 
                                "Down and Right", "Down", "Down and Left", 
                                "Left", "Up and Left", NULL);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'j');
    itemID =fAppMenu.appendList("Transition-Prev", "Transition-Prev", sinkID, 
                                fTransitionPrev, "Up", "Up and Right", "Right", 
                                "Down and Right", "Down", "Down and Left", 
                                "Left", "Up and Left", NULL);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'k');
    itemID = fAppMenu.appendAction("Save to PDF", sinkID);
    fAppMenu.assignKeyEquivalentToItem(itemID, 'e');
    
    this->addMenu(&fAppMenu);
    this->addMenu(&fSlideMenu);
    
//    this->setConfig(SkBitmap::kRGB_565_Config);
    this->setConfig(SkBitmap::kARGB_8888_Config);
    this->setVisibleP(true);
    this->setClipToBounds(false);

    skiagm::GM::SetResourcePath(resourcePath);

    SkGMRegistyToSampleRegistry();
    {
        const SkViewRegister* reg = SkViewRegister::Head();
        while (reg) {
            *fSamples.append() = reg->factory();
            reg = reg->next();
        }
    }

    this->loadView((*fSamples[fCurrIndex])());
    
    fPDFData = NULL;

    if (NULL == devManager) {
        fDevManager = new DefaultDeviceManager();
    } else {
        devManager->ref();
        fDevManager = devManager;
    }
    fDevManager->init(this);

    // If another constructor set our dimensions, ensure that our
    // onSizeChange gets called.
    if (this->height() && this->width()) {
        this->onSizeChange();
    }

    // can't call this synchronously, since it may require a subclass to
    // to implement, or the caller may need us to have returned from the
    // constructor first. Hence we post an event to ourselves.
//    this->updateTitle();
    postEventToSink(new SkEvent(gUpdateWindowTitleEvtName), this);
}

SampleWindow::~SampleWindow() {
    delete fPicture;
    delete fPdfCanvas;
    fTypeface->unref();

    SkSafeUnref(fDevManager);
}

int SampleWindow::findByTitle(const char title[]) {
    int i, count = fSamples.count();
    for (i = 0; i < count; i++) {
        if (getSampleTitle(i).equals(title)) {
            return i;
        }
    }
    return -1;
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
    if (!fDevManager->prepareCanvas(fDeviceType, canvas, this)) {
        return;
    }
    // update the animation time
    if (!gAnimTimePrev && !gAnimTime) {
        // first time make delta be 0
        gAnimTime = SkTime::GetMSecs();
        gAnimTimePrev = gAnimTime;
    } else {
        gAnimTimePrev = gAnimTime;
        gAnimTime = SkTime::GetMSecs();
    }
    
    const SkMatrix& localM = fGesture.localM();
    if (localM.getType() & SkMatrix::kScale_Mask) {
        canvas->setExternalMatrix(&localM);
    }
    if (fGesture.isActive()) {
        this->updateMatrix();
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
    if (fShowZoomer && !fSaveToPdf) {
        showZoomer(canvas);
    }
    if (fMagnify && !fSaveToPdf) {
        magnify(canvas);
    }
    
    // do this last
    fDevManager->publishCanvas(fDeviceType, canvas, this);
}

static float clipW = 200;
static float clipH = 200;
void SampleWindow::magnify(SkCanvas* canvas) {
    SkRect r;
    int count = canvas->save();
    
    SkMatrix m = canvas->getTotalMatrix();
    m.invert(&m);
    SkPoint offset, center;
    SkScalar mouseX = fMouseX * SK_Scalar1;
    SkScalar mouseY = fMouseY * SK_Scalar1;
    m.mapXY(mouseX - clipW/2, mouseY - clipH/2, &offset);
    m.mapXY(mouseX, mouseY, &center);
    
    r.set(0, 0, clipW * m.getScaleX(), clipH * m.getScaleX());
    r.offset(offset.fX, offset.fY);
    
    SkPaint paint;
    paint.setColor(0xFF66AAEE);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10.f * m.getScaleX());
    //lense offset
    //canvas->translate(0, -250);
    canvas->drawRect(r, paint);
    canvas->clipRect(r);
    
    m = canvas->getTotalMatrix();
    m.setTranslate(-center.fX, -center.fY);
    m.postScale(0.5f * fFatBitsScale, 0.5f * fFatBitsScale);
    m.postTranslate(center.fX, center.fY);
    canvas->concat(m);
    
    this->INHERITED::draw(canvas);
    
    canvas->restoreToCount(count);
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

void SampleWindow::saveToPdf()
{
    fSaveToPdf = true;
    this->inval(NULL);
}

SkCanvas* SampleWindow::beforeChildren(SkCanvas* canvas) {
    if (fSaveToPdf) {
        const SkBitmap& bmp = canvas->getDevice()->accessBitmap(false);
        SkISize size = SkISize::Make(bmp.width(), bmp.height());
        SkPDFDevice* pdfDevice = new SkPDFDevice(size, size,
                canvas->getTotalMatrix());
        fPdfCanvas = new SkCanvas(pdfDevice);
        pdfDevice->unref();
        canvas = fPdfCanvas;
    } else {
        switch (fDeviceType) {
            case kRaster_DeviceType:
            case kGPU_DeviceType:
                canvas = this->INHERITED::beforeChildren(canvas);
                break;
            case kPicture_DeviceType:
                fPicture = new SkPicture;
                canvas = fPicture->beginRecording(9999, 9999);
                break;
            case kNullGPU_DeviceType:
                break;
        }
    }

    if (fUseClip) {
        canvas->drawColor(0xFFFF88FF);
        canvas->clipPath(fClipPath, SkRegion::kIntersect_Op, true);
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
#include "SkData.h"
void SampleWindow::afterChildren(SkCanvas* orig) {
    if (fSaveToPdf) {
        fSaveToPdf = false;
        if (fShowZoomer) {
            showZoomer(fPdfCanvas);
        }
        SkString name;
        name.printf("%s.pdf", this->getTitle());
        SkPDFDocument doc;
        SkPDFDevice* device = static_cast<SkPDFDevice*>(fPdfCanvas->getDevice());
        doc.appendPage(device);
#ifdef SK_BUILD_FOR_ANDROID
        name.prepend("/sdcard/");
#endif
        
#ifdef SK_BUILD_FOR_IOS
        SkDynamicMemoryWStream mstream;
        doc.emitPDF(&mstream);
        fPDFData = mstream.copyToData();
#endif
        SkFILEWStream stream(name.c_str());
        if (stream.isValid()) {
            doc.emitPDF(&stream);
            const char* desc = "File saved from Skia SampleApp";
            this->onPDFSaved(this->getTitle(), desc, name.c_str());
        }
        
        delete fPdfCanvas;
        fPdfCanvas = NULL;

        // We took over the draw calls in order to create the PDF, so we need
        // to redraw.
        this->inval(NULL);
        return;
    }
    
    if (fRequestGrabImage) {
        fRequestGrabImage = false;

        SkDevice* device = orig->getDevice();
        SkBitmap bmp;
        if (device->accessBitmap(false).copyTo(&bmp, SkBitmap::kARGB_8888_Config)) {
            static int gSampleGrabCounter;
            SkString name;
            name.printf("sample_grab_%d", gSampleGrabCounter++);
            SkImageEncoder::EncodeFile(name.c_str(), bmp,
                                       SkImageEncoder::kPNG_Type, 100);
        }
    }

    if (kPicture_DeviceType == fDeviceType) {
        if (true) {
            SkPicture* pict = new SkPicture(*fPicture);
            fPicture->unref();
            this->installDrawFilter(orig);
            orig->drawPicture(*pict);
            pict->unref();
        } else if (true) {
            SkDynamicMemoryWStream ostream;
            fPicture->serialize(&ostream);
            fPicture->unref();

            SkAutoDataUnref data(ostream.copyToData());
            SkMemoryStream istream(data.data(), data.size());
            SkPicture pict(&istream);
            orig->drawPicture(pict);
        } else {
            fPicture->draw(orig);
            fPicture->unref();
        }
        fPicture = NULL;
    }

    // Do this after presentGL and other finishing, rather than in afterChild
    if (fMeasureFPS && fMeasureFPS_Time) {
        fMeasureFPS_Time = SkTime::GetMSecs() - fMeasureFPS_Time;
        this->updateTitle();
        this->postInvalDelay();
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
#ifdef DEBUGGER
    SkView* curr = curr_view(this);
    if (fDebugger && !is_debugger(curr) && !is_transition(curr) && !is_overview(curr)) {
        //Stop Pipe when fDebugger is active
        fUsePipe = false;
        (void)SampleView::SetUsePipe(curr, false);
        fAppMenu.getItemByID(fUsePipeMenuItemID)->setBool(fUsePipe);
        this->onUpdateMenu(&fAppMenu);
        
        //Reset any transformations
        fGesture.stop();
        fGesture.reset();
        
        this->loadView(create_debugger(gTempDataStore.begin(), 
                                       gTempDataStore.count()));
    }
#endif
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
    if (fPerspAnim) {
        fPerspAnimTime += SampleCode::GetAnimSecondsDelta();

        static const SkScalar gAnimPeriod = 10 * SK_Scalar1;
        static const SkScalar gAnimMag = SK_Scalar1 / 1000;
        SkScalar t = SkScalarMod(fPerspAnimTime, gAnimPeriod);
        if (SkScalarFloorToInt(SkScalarDiv(fPerspAnimTime, gAnimPeriod)) & 0x1) {
            t = gAnimPeriod - t;
        }
        t = 2 * t - gAnimPeriod;
        t = SkScalarMul(SkScalarDiv(t, gAnimPeriod), gAnimMag);
        SkMatrix m;
        m.reset();
        m.setPerspY(t);
        canvas->concat(m);
    }

    this->installDrawFilter(canvas);

    if (fMeasureFPS) {
        fMeasureFPS_Time = 0;   // 0 means the child is not aware of repeat-draw
        if (SampleView::SetRepeatDraw(child, FPS_REPEAT_COUNT)) {
            fMeasureFPS_Time = SkTime::GetMSecs();
        }
    } else {
        (void)SampleView::SetRepeatDraw(child, 1);
    }
    if (fPerspAnim) {
        this->inval(NULL);
    }
    //(void)SampleView::SetUsePipe(child, fUsePipe);
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

void SampleWindow::changeZoomLevel(float delta) {
    fZoomLevel += SkFloatToScalar(delta);
    if (fZoomLevel > 0) {
        fZoomLevel = SkMinScalar(fZoomLevel, MAX_ZOOM_LEVEL);
        fZoomScale = fZoomLevel + SK_Scalar1;
    } else if (fZoomLevel < 0) {
        fZoomLevel = SkMaxScalar(fZoomLevel, MIN_ZOOM_LEVEL);
        fZoomScale = SK_Scalar1 / (SK_Scalar1 - fZoomLevel);
    } else {
        fZoomScale = SK_Scalar1;
    }
    this->updateMatrix();
}

void SampleWindow::updateMatrix(){
    SkMatrix m;
    m.reset();
    if (fZoomLevel) {
        SkPoint center;
        //m = this->getLocalMatrix();//.invert(&m);
        m.mapXY(fZoomCenterX, fZoomCenterY, &center);
        SkScalar cx = center.fX;
        SkScalar cy = center.fY;
        
        m.setTranslate(-cx, -cy);
        m.postScale(fZoomScale, fZoomScale);
        m.postTranslate(cx, cy);
    }
    
    if (fFlipAxis) {
        m.preTranslate(fZoomCenterX, fZoomCenterY);
        if (fFlipAxis & kFlipAxis_X) {
            m.preScale(-SK_Scalar1, SK_Scalar1);
        }
        if (fFlipAxis & kFlipAxis_Y) {
            m.preScale(SK_Scalar1, -SK_Scalar1);
        }
        m.preTranslate(-fZoomCenterX, -fZoomCenterY);
        //canvas->concat(m);
    }
    // Apply any gesture matrix
    m.preConcat(fGesture.localM());
    m.preConcat(fGesture.globalM());
    
    this->setLocalMatrix(m);
    
    this->updateTitle();
    this->inval(NULL);
}
bool SampleWindow::previousSample() {
    fCurrIndex = (fCurrIndex - 1 + fSamples.count()) % fSamples.count();
    this->loadView(create_transition(curr_view(this), (*fSamples[fCurrIndex])(), 
                                     fTransitionPrev));
    return true;
}

bool SampleWindow::nextSample() {
    fCurrIndex = (fCurrIndex + 1) % fSamples.count();
    this->loadView(create_transition(curr_view(this), (*fSamples[fCurrIndex])(), 
                                     fTransitionNext));
    return true;
}

bool SampleWindow::goToSample(int i) {
    fCurrIndex = (i) % fSamples.count();
    this->loadView(create_transition(curr_view(this),(*fSamples[fCurrIndex])(), 6));
    return true;
}

SkString SampleWindow::getSampleTitle(int i) {
    SkView* view = (*fSamples[i])();
    SkString title;
    SampleCode::RequestTitle(view, &title);
    view->unref();
    return title;
}

int SampleWindow::sampleCount() {
    return fSamples.count();
}

void SampleWindow::showOverview() {
    this->loadView(create_transition(curr_view(this), 
                                     create_overview(fSamples.count(), fSamples.begin()), 
                                     4));
}

void SampleWindow::installDrawFilter(SkCanvas* canvas) {
    canvas->setDrawFilter(new FlagsDrawFilter(fLCDState, fAAState,
                                              fFilterState, fHintingState))->unref();
}

void SampleWindow::postAnimatingEvent() {
    if (fAnimating) {
        (new SkEvent(ANIMATING_EVENTTYPE, this->getSinkID()))->postDelay(ANIMATING_DELAY);
    }
}

bool SampleWindow::onEvent(const SkEvent& evt) {
    if (evt.isType(gUpdateWindowTitleEvtName)) {
        this->updateTitle();
        return true;
    }
    if (evt.isType(ANIMATING_EVENTTYPE)) {
        if (fAnimating) {
            this->nextSample();
            this->postAnimatingEvent();
        }
        return true;
    }
    if (evt.isType("replace-transition-view")) {
        this->loadView((SkView*)SkEventSink::FindSink(evt.getFast32()));
        return true;
    }
    if (evt.isType("set-curr-index")) {
        this->goToSample(evt.getFast32());
        return true;
    }
    if (isInvalEvent(evt)) {
        this->inval(NULL);
        return true;
    }
    int selected = -1;
    if (SkOSMenu::FindListIndex(evt, "Device Type", &selected)) {
        this->setDeviceType((DeviceType)selected);
        return true; 
    }
    if (SkOSMenu::FindSwitchState(evt, "Pipe", &fUsePipe)) {
#ifdef PIPE_NET
        if (!fUsePipe)
            gServer.disconnectAll();
#endif
        (void)SampleView::SetUsePipe(curr_view(this), fUsePipe);
        this->updateTitle();
        this->inval(NULL);
        return true;
    }
    if (SkOSMenu::FindSwitchState(evt, "Slide Show", NULL)) {
        this->toggleSlideshow();
        return true;
    }
    if (SkOSMenu::FindTriState(evt, "AA", &fAAState) ||
        SkOSMenu::FindTriState(evt, "LCD", &fLCDState) ||
        SkOSMenu::FindTriState(evt, "Filter", &fFilterState) ||
        SkOSMenu::FindTriState(evt, "Hinting", &fHintingState) ||
        SkOSMenu::FindSwitchState(evt, "Clip", &fUseClip) ||
        SkOSMenu::FindSwitchState(evt, "Zoomer", &fShowZoomer) ||
        SkOSMenu::FindSwitchState(evt, "Magnify", &fMagnify) ||
        SkOSMenu::FindListIndex(evt, "Transition-Next", &fTransitionNext) ||
        SkOSMenu::FindListIndex(evt, "Transition-Prev", &fTransitionPrev)) {
        this->inval(NULL);
        this->updateTitle();
        return true;
    }
    if (SkOSMenu::FindSwitchState(evt, "Flip X", NULL)) {
        fFlipAxis ^= kFlipAxis_X;
        this->updateMatrix();
        return true;
    }
    if (SkOSMenu::FindSwitchState(evt, "Flip Y", NULL)) {
        fFlipAxis ^= kFlipAxis_Y;
        this->updateMatrix();
        return true;
    }
    if (SkOSMenu::FindAction(evt,"Save to PDF")) {
        this->saveToPdf();
        return true;
    } 
#ifdef DEBUGGER
    if (SkOSMenu::FindSwitchState(evt, "Debugger", &fDebugger)) {
        if (fDebugger) {
            fUsePipe = true;
            (void)SampleView::SetUsePipe(curr_view(this), true);
        } else {
            this->loadView(fSamples[fCurrIndex]());
        }
        this->inval(NULL);
        return true;
    }
#endif
    return this->INHERITED::onEvent(evt);
}

bool SampleWindow::onQuery(SkEvent* query) {
    if (query->isType("get-slide-count")) {
        query->setFast32(fSamples.count());
        return true;
    }
    if (query->isType("get-slide-title")) {
        SkView* view = (*fSamples[query->getFast32()])();
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
    if (query->isType("ignore-window-bitmap")) {
        query->setFast32(this->getGrContext() != NULL);
        return true;
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
        case 'f':
            // only 
            toggleFPS();
            break;
        case 'g':
            fRequestGrabImage = true;
            this->inval(NULL);
            break;
        case 'i':
            this->zoomIn();
            break;
        case 'o':
            this->zoomOut();
            break;
        case 'r':
            fRotate = !fRotate;
            this->inval(NULL);
            this->updateTitle();
            return true;
        case 'k':
            fPerspAnim = !fPerspAnim;
            this->inval(NULL);
            this->updateTitle();
            return true;
        case '\\':
            if (fDevManager->supportsDeviceType(kNullGPU_DeviceType)) {
                fDeviceType=  kNullGPU_DeviceType;
                this->inval(NULL);
                this->updateTitle();
            }
            return true;
        case 'p':
            {
                GrContext* grContext = this->getGrContext();
                if (grContext) {
                    size_t cacheBytes = grContext->getGpuTextureCacheBytes();
                    grContext->freeGpuResources();
                    SkDebugf("Purged %d bytes from the GPU resource cache.\n",
                             cacheBytes);
                }
            }
            return true;
        case 's':
            fScale = !fScale;
            this->inval(NULL);
            this->updateTitle();
            return true;
        default:
            break;
    }
    
    if (fAppMenu.handleKeyEquivalent(uni)|| fSlideMenu.handleKeyEquivalent(uni)) {
        this->onUpdateMenu(&fAppMenu);
        this->onUpdateMenu(&fSlideMenu);
        return true;
    }
    return this->INHERITED::onHandleChar(uni);
}

void SampleWindow::setDeviceType(DeviceType type) {
    if (type != fDeviceType && fDevManager->supportsDeviceType(fDeviceType))
        fDeviceType = type;
    this->updateTitle();
    this->inval(NULL);
}

void SampleWindow::toggleSlideshow() {
    fAnimating = !fAnimating;
    this->postAnimatingEvent();
    this->updateTitle();
}

void SampleWindow::toggleRendering() {
    DeviceType origDevType = fDeviceType;
    do {
        fDeviceType = cycle_devicetype(fDeviceType);
    } while (origDevType != fDeviceType &&
             !fDevManager->supportsDeviceType(fDeviceType));
    this->updateTitle();
    this->inval(NULL);
}

void SampleWindow::toggleFPS() {
    fMeasureFPS = !fMeasureFPS;
    this->updateTitle();
    this->inval(NULL);
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
            toggleRendering();
            return true;
        case kUp_SkKey:
            if (USE_ARROWS_FOR_ZOOM) {
                this->changeZoomLevel(1.f);
            } else {
                fNClip = !fNClip;
                this->inval(NULL);
                this->updateTitle();
            }
            return true;
        case kDown_SkKey:
            if (USE_ARROWS_FOR_ZOOM) {
                this->changeZoomLevel(-1.f);
            } else {
                this->setConfig(cycle_configs(this->getBitmap().config()));
                this->updateTitle();
            }
            return true;
        case kOK_SkKey: {
            SkString title;
            if (curr_title(this, &title)) {
                writeTitleToPrefs(title.c_str());
            }
            return true;
        }
        case kBack_SkKey:
            this->showOverview();
            return true;
        default:
            break;
    }
    return this->INHERITED::onHandleKey(key);
}

///////////////////////////////////////////////////////////////////////////////

static const char gGestureClickType[] = "GestureClickType";

bool SampleWindow::onDispatchClick(int x, int y, Click::State state,
        void* owner) {
    if (Click::kMoved_State == state) {
        updatePointer(x, y);
    }
    int w = SkScalarRound(this->width());
    int h = SkScalarRound(this->height());

    // check for the resize-box
    if (w - x < 16 && h - y < 16) {
        return false;   // let the OS handle the click
    } 
    else if (fMagnify) {
        //it's only necessary to update the drawing if there's a click
        this->inval(NULL);
        return false; //prevent dragging while magnify is enabled
    }
    else {
        return this->INHERITED::onDispatchClick(x, y, state, owner);
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
        float x = static_cast<float>(click->fICurr.fX);
        float y = static_cast<float>(click->fICurr.fY);
        
        switch (click->fState) {
            case SkView::Click::kDown_State:
                fGesture.touchBegin(click->fOwner, x, y);
                break;
            case SkView::Click::kMoved_State:
                fGesture.touchMoved(click->fOwner, x, y);
                this->updateMatrix();
                break;
            case SkView::Click::kUp_State:
                fGesture.touchEnd(click->fOwner);
                this->updateMatrix();
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
    
    view->setVisibleP(true);
    view->setClipToBounds(false);
    this->attachChildToFront(view)->unref();
    view->setSize(this->width(), this->height());

    //repopulate the slide menu when a view is loaded
    fSlideMenu.reset();
#ifdef DEBUGGER
    if (!is_debugger(view) && !is_overview(view) && !is_transition(view) && fDebugger) {
        //Force Pipe to be on if using debugger
        fUsePipe = true;
    }
#endif
    (void)SampleView::SetUsePipe(view, fUsePipe);
    if (SampleView::IsSampleView(view))
        ((SampleView*)view)->requestMenu(&fSlideMenu);
    this->onUpdateMenu(&fSlideMenu);
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

static const char* gDeviceTypePrefix[] = {
    "raster: ",
    "picture: ",
    "opengl: ",
    "null-gl: "
};

static const char* trystate_str(SkOSMenu::TriState state,
                                const char trueStr[], const char falseStr[]) {
    if (SkOSMenu::kOnState == state) {
        return trueStr;
    } else if (SkOSMenu::kOffState == state) {
        return falseStr;
    }
    return NULL;
}

void SampleWindow::updateTitle() {
    SkView* view = curr_view(this);

    SkString title;
    if (!curr_title(this, &title)) {
        title.set("<unknown>");
    }

    title.prepend(gDeviceTypePrefix[fDeviceType]);

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
    if (fPerspAnim) {
        title.prepend("<K> ");
    }

    title.prepend(trystate_str(fLCDState, "LCD ", "lcd "));
    title.prepend(trystate_str(fAAState, "AA ", "aa "));
    title.prepend(trystate_str(fFilterState, "H ", "h "));
    title.prepend(fFlipAxis & kFlipAxis_X ? "X " : NULL);
    title.prepend(fFlipAxis & kFlipAxis_Y ? "Y " : NULL);

    if (fZoomLevel) {
        title.prependf("{%.2f} ", SkScalarToFloat(fZoomLevel));
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

    fZoomCenterX = SkScalarHalf(this->width());
    fZoomCenterY = SkScalarHalf(this->height());

#ifdef SK_BUILD_FOR_ANDROID
    // FIXME: The first draw after a size change does not work on Android, so
    // we post an invalidate.
    this->postInvalDelay();
#endif
    this->updateTitle();    // to refresh our config
    fDevManager->windowSizeChanged(this);
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
    
    /**
     * User this method to halt/restart pipe
     */
    void setWriteToPipe(bool writeToPipe) { fWriteToPipe = writeToPipe; }
    virtual void* requestBlock(size_t minRequest, size_t* actual);
    virtual void notifyWritten(size_t bytes);

private:
    SkGPipeReader   fReader;
    void*           fBlock;
    size_t          fBlockSize;
    size_t          fBytesWritten;
    int             fAtomsWritten;
    SkGPipeReader::Status   fStatus;
    bool            fWriteToPipe;

    size_t        fTotalWritten;
};

SimplePC::SimplePC(SkCanvas* target) : fReader(target) {
    fBlock = NULL;
    fBlockSize = fBytesWritten = 0;
    fStatus = SkGPipeReader::kDone_Status;
    fTotalWritten = 0;
    fAtomsWritten = 0;
    fWriteToPipe = true;
}

SimplePC::~SimplePC() {
//    SkASSERT(SkGPipeReader::kDone_Status == fStatus);
    if (fTotalWritten) {
        if (fWriteToPipe) {
            SkDebugf("--- %d bytes %d atoms, status %d\n", fTotalWritten,
                     fAtomsWritten, fStatus);
#ifdef  PIPE_FILE
            //File is open in append mode
            FILE* f = fopen(FILE_PATH, "ab");
            SkASSERT(f != NULL);
            fwrite((const char*)fBlock + fBytesWritten, 1, bytes, f);
            fclose(f);
#endif
#ifdef PIPE_NET
            if (fAtomsWritten > 1 && fTotalWritten > 4) { //ignore done
                gServer.acceptConnections();
                gServer.writePacket(fBlock, fTotalWritten);
            }
#endif
#ifdef  DEBUGGER
            gTempDataStore.reset();
            gTempDataStore.append(fTotalWritten, (const char*)fBlock);
#endif
        }
    }
    sk_free(fBlock);
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
    fStatus = fReader.playback((const char*)fBlock + fBytesWritten, bytes);
    SkASSERT(SkGPipeReader::kError_Status != fStatus);
    fBytesWritten += bytes;
    fTotalWritten += bytes;
    
    fAtomsWritten += 1;
}

#endif

void SampleView::draw(SkCanvas* canvas) {
#ifdef TEST_GPIPE
    if (fUsePipe) {
        SkGPipeWriter writer;
        SimplePC controller(canvas);
        uint32_t flags = SkGPipeWriter::kCrossProcess_Flag;
        canvas = writer.startRecording(&controller, flags);
        //Must draw before controller goes out of scope and sends data
        this->INHERITED::draw(canvas);
        //explicitly end recording to ensure writer is flushed before the memory
        //is freed in the deconstructor of the controller
        writer.endRecording();
        controller.setWriteToPipe(fUsePipe);
    }
    else
        this->INHERITED::draw(canvas);
#else
    this->INHERITED::draw(canvas);
#endif
}
void SampleView::onDraw(SkCanvas* canvas) {
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

SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
//    test();
    return new SampleWindow(hwnd, argc, argv, NULL);
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

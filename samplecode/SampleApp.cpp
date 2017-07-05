/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleApp.h"

#include "OverView.h"
#include "Resources.h"
#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkColorSpace_XYZ.h"
#include "SkCommandLineFlags.h"
#include "SkCommonFlagsPathRenderer.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPaint.h"
#include "SkPaintFilterCanvas.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPM4fPriv.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTemplates.h"
#include "SkTSort.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkWindow.h"
#include "sk_tool_utils.h"
#include "SkScan.h"
#include "SkClipOpPriv.h"
#include "SkThreadedBMPDevice.h"

#include "SkReadBuffer.h"
#include "SkStream.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include "SkCGUtils.h"
#endif

#define PICTURE_MEANS_PIPE  false
#define SERIALIZE_PICTURE   true

#if SK_SUPPORT_GPU
#   include "gl/GrGLInterface.h"
#   include "gl/GrGLUtil.h"
#   include "GrContext.h"
#   include "SkGr.h"
#   if SK_ANGLE
#       include "gl/angle/GLTestContext_angle.h"
#   endif
#else
class GrContext;
#endif

enum OutputColorSpace {
    kLegacy_OutputColorSpace,
    kSRGB_OutputColorSpace,
    kNarrow_OutputColorSpace,
    kMonitor_OutputColorSpace,
};

const struct {
    SkColorType         fColorType;
    OutputColorSpace    fColorSpace;
    const char*         fName;
} gConfig[] = {
    { kN32_SkColorType,      kLegacy_OutputColorSpace,  "L32" },
    { kN32_SkColorType,      kSRGB_OutputColorSpace,    "S32" },
    { kRGBA_F16_SkColorType, kSRGB_OutputColorSpace,    "F16" },
    { kRGBA_F16_SkColorType, kNarrow_OutputColorSpace,  "F16 Narrow" },
    { kRGBA_F16_SkColorType, kMonitor_OutputColorSpace, "F16 Device" },
};

// Should be 3x + 1
#define kMaxFatBitsScale    28

extern SampleView* CreateSamplePictFileView(const char filename[]);

class PictFileFactory : public SkViewFactory {
    SkString fFilename;
public:
    PictFileFactory(const SkString& filename) : fFilename(filename) {}
    SkView* operator() () const override {
        return CreateSamplePictFileView(fFilename.c_str());
    }
};

extern SampleView* CreateSamplePathFinderView(const char filename[]);

class PathFinderFactory : public SkViewFactory {
    SkString fFilename;
public:
    PathFinderFactory(const SkString& filename) : fFilename(filename) {}
    SkView* operator() () const override {
        return CreateSamplePathFinderView(fFilename.c_str());
    }
};

extern SampleView* CreateSampleSVGFileView(const SkString& filename);

class SVGFileFactory : public SkViewFactory {
    SkString fFilename;
public:
    SVGFileFactory(const SkString& filename) : fFilename(filename) {}
    SkView* operator() () const override {
        return CreateSampleSVGFileView(fFilename);
    }
};

#ifdef SAMPLE_PDF_FILE_VIEWER
extern SampleView* CreateSamplePdfFileViewer(const char filename[]);

class PdfFileViewerFactory : public SkViewFactory {
    SkString fFilename;
public:
    PdfFileViewerFactory(const SkString& filename) : fFilename(filename) {}
    SkView* operator() () const override {
        return CreateSamplePdfFileViewer(fFilename.c_str());
    }
};
#endif  // SAMPLE_PDF_FILE_VIEWER

#if SK_ANGLE
//#define DEFAULT_TO_ANGLE 1
#else
#define DEFAULT_TO_GPU 0 // if 1 default rendering is on GPU
#endif

#define ANIMATING_EVENTTYPE "nextSample"
#define ANIMATING_DELAY     250

#ifdef SK_DEBUG
    #define FPS_REPEAT_MULTIPLIER   1
#else
    #define FPS_REPEAT_MULTIPLIER   10
#endif
#define FPS_REPEAT_COUNT    (10 * FPS_REPEAT_MULTIPLIER)

static SampleWindow* gSampleWindow;

static bool gShowGMBounds;

static void post_event_to_sink(SkEvent* evt, SkEventSink* sink) {
    evt->setTargetID(sink->getSinkID())->post();
}

static SkAnimTimer gAnimTimer;

///////////////////////////////////////////////////////////////////////////////

static const char* skip_until(const char* str, const char* skip) {
    if (!str) {
        return nullptr;
    }
    return strstr(str, skip);
}

static const char* skip_past(const char* str, const char* skip) {
    const char* found = skip_until(str, skip);
    if (!found) {
        return nullptr;
    }
    return found + strlen(skip);
}

static const char* gPrefFileName = "sampleapp_prefs.txt";

static bool readTitleFromPrefs(SkString* title) {
    SkFILEStream stream(gPrefFileName);
    if (!stream.isValid()) {
        return false;
    }

    size_t len = stream.getLength();
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
#if SK_SUPPORT_GPU
        fCurContext = nullptr;
        fCurIntf = nullptr;
        fMSAASampleCount = 0;
        fDeepColor = false;
        fActualColorBits = 0;
#endif
        fBackend = kNone_BackEndType;
    }

    ~DefaultDeviceManager() override {
#if SK_SUPPORT_GPU
        SkSafeUnref(fCurContext);
        SkSafeUnref(fCurIntf);
#endif
    }

    void setUpBackend(SampleWindow* win, const BackendOptions& backendOptions) override {
        SkASSERT(kNone_BackEndType == fBackend);

        fBackend = kNone_BackEndType;

#if SK_SUPPORT_GPU
        switch (win->getDeviceType()) {
            case kRaster_DeviceType:    // fallthrough
            case kGPU_DeviceType:
                // all these guys use the native backend
                fBackend = kNativeGL_BackEndType;
                break;
#if SK_ANGLE
            case kANGLE_DeviceType:
                // ANGLE is really the only odd man out
                fBackend = kANGLE_BackEndType;
                break;
#endif // SK_ANGLE
            default:
                SkASSERT(false);
                break;
        }
        AttachmentInfo attachmentInfo;
        bool result = win->attach(fBackend, backendOptions.fMSAASampleCount,
                                  backendOptions.fDeepColor, &attachmentInfo);
        if (!result) {
            SkDebugf("Failed to initialize GL");
            return;
        }
        fMSAASampleCount = backendOptions.fMSAASampleCount;
        fDeepColor = backendOptions.fDeepColor;
        // Assume that we have at least 24-bit output, for backends that don't supply this data
        fActualColorBits = SkTMax(attachmentInfo.fColorBits, 24);

        SkASSERT(nullptr == fCurIntf);
        switch (win->getDeviceType()) {
            case kRaster_DeviceType:    // fallthrough
            case kGPU_DeviceType:
                // all these guys use the native interface
                fCurIntf = GrGLCreateNativeInterface();
                break;
#if SK_ANGLE
            case kANGLE_DeviceType:
                fCurIntf = sk_gpu_test::CreateANGLEGLInterface();
                break;
#endif // SK_ANGLE
            default:
                SkASSERT(false);
                break;
        }

        SkASSERT(nullptr == fCurContext);
        fCurContext = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext) fCurIntf,
                                        backendOptions.fGrContextOptions);

        if (nullptr == fCurContext || nullptr == fCurIntf) {
            // We need some context and interface to see results
            SkSafeUnref(fCurContext);
            SkSafeUnref(fCurIntf);
            fCurContext = nullptr;
            fCurIntf = nullptr;
            SkDebugf("Failed to setup 3D");

            win->release();
        }
#endif // SK_SUPPORT_GPU
        // call windowSizeChanged to create the gpu-backed Surface
        this->windowSizeChanged(win);
    }

    void tearDownBackend(SampleWindow *win) override {
#if SK_SUPPORT_GPU
        if (fCurContext) {
            // in case we have outstanding refs to this guy (lua?)
            fCurContext->abandonContext();
            fCurContext->unref();
            fCurContext = nullptr;
        }

        SkSafeUnref(fCurIntf);
        fCurIntf = nullptr;

        fGpuSurface = nullptr;
#endif
        win->release();
        fBackend = kNone_BackEndType;
    }

    sk_sp<SkSurface> makeSurface(SampleWindow::DeviceType dType, SampleWindow* win) override {
#if SK_SUPPORT_GPU
        if (IsGpuDeviceType(dType) && fCurContext) {
            SkSurfaceProps props(win->getSurfaceProps());
            if (kRGBA_F16_SkColorType == win->info().colorType() || fActualColorBits > 24) {
                // If we're rendering to F16, we need an off-screen surface - the current render
                // target is most likely the wrong format.
                //
                // If we're using a deep (10-bit or higher) surface, we probably need an off-screen
                // surface. 10-bit, in particular, has strange gamma behavior.
                return SkSurface::MakeRenderTarget(fCurContext, SkBudgeted::kNo, win->info(),
                                                   fMSAASampleCount, &props);
            } else {
                return fGpuSurface;
            }
        }
#endif
        return nullptr;
    }

    void publishCanvas(SampleWindow::DeviceType dType,
                       SkCanvas* renderingCanvas, SampleWindow* win) override {
#if SK_SUPPORT_GPU
        if (!IsGpuDeviceType(dType) ||
            kRGBA_F16_SkColorType == win->info().colorType() ||
            fActualColorBits > 24) {
            // We made/have an off-screen surface. Extract the pixels exactly as we rendered them:
            SkImageInfo info = win->info();
            size_t rowBytes = info.minRowBytes();
            size_t size = info.getSafeSize(rowBytes);
            auto data = SkData::MakeUninitialized(size);
            SkASSERT(data);

            if (!renderingCanvas->readPixels(info, data->writable_data(), rowBytes, 0, 0)) {
                SkDEBUGFAIL("Failed to read canvas pixels");
                return;
            }

            // Now, re-interpret those pixels as sRGB, so they won't be color converted when we
            // draw then to FBO0. This ensures that if we rendered in any strange gamut, we'll see
            // the "correct" output (because we generated the pixel values we wanted in the
            // offscreen canvas).
            auto colorSpace = kRGBA_F16_SkColorType == info.colorType()
                ? SkColorSpace::MakeSRGBLinear()
                : SkColorSpace::MakeSRGB();
            auto offscreenImage = SkImage::MakeRasterData(info.makeColorSpace(colorSpace), data,
                                                          rowBytes);

            SkCanvas* gpuCanvas = fGpuSurface->getCanvas();

            // With ten-bit output, we need to manually apply the gamma of the output device
            // (unless we're in non-gamma correct mode, in which case our data is already
            // fake-sRGB, like we're expected to put in the 10-bit buffer):
            bool doGamma = (fActualColorBits == 30) && win->info().colorSpace();

            SkPaint gammaPaint;
            gammaPaint.setBlendMode(SkBlendMode::kSrc);
            if (doGamma) {
                gammaPaint.setColorFilter(SkColorFilter::MakeLinearToSRGBGamma());
            }

            gpuCanvas->drawImage(offscreenImage, 0, 0, &gammaPaint);
        }

        fGpuSurface->prepareForExternalIO();
#endif

        win->present();
    }

    void windowSizeChanged(SampleWindow* win) override {
        win->resetFPS();
#if SK_SUPPORT_GPU
        if (fCurContext) {
            AttachmentInfo attachmentInfo;
            win->attach(fBackend, fMSAASampleCount, fDeepColor, &attachmentInfo);
            fActualColorBits = SkTMax(attachmentInfo.fColorBits, 24);
            fGpuSurface = win->makeGpuBackedSurface(attachmentInfo, fCurIntf, fCurContext);
        }
#endif
    }

    GrContext* getGrContext() override {
#if SK_SUPPORT_GPU
        return fCurContext;
#else
        return nullptr;
#endif
    }

    int numColorSamples() const override {
#if SK_SUPPORT_GPU
        return fMSAASampleCount;
#else
        return 0;
#endif
    }

    int getColorBits() override {
#if SK_SUPPORT_GPU
        return fActualColorBits;
#else
        return 24;
#endif
    }

private:

#if SK_SUPPORT_GPU
    GrContext*              fCurContext;
    const GrGLInterface*    fCurIntf;
    sk_sp<SkSurface>        fGpuSurface;
    int fMSAASampleCount;
    bool fDeepColor;
    int fActualColorBits;
#endif

    SkOSWindow::SkBackEndTypes fBackend;

    typedef SampleWindow::DeviceManager INHERITED;
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

#include "GMSampleView.h"

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
static void SkGMRegistyToSampleRegistry() {
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

//////////////////////////////////////////////////////////////////////////////

enum FlipAxisEnum {
    kFlipAxis_X = (1 << 0),
    kFlipAxis_Y = (1 << 1)
};

#include "SkDrawFilter.h"

struct HintingState {
    SkPaint::Hinting hinting;
    const char* name;
    const char* label;
};
static HintingState gHintingStates[] = {
    {SkPaint::kNo_Hinting, "Mixed", nullptr },
    {SkPaint::kNo_Hinting, "None", "H0 " },
    {SkPaint::kSlight_Hinting, "Slight", "Hs " },
    {SkPaint::kNormal_Hinting, "Normal", "Hn " },
    {SkPaint::kFull_Hinting, "Full", "Hf " },
};

struct PixelGeometryState {
    SkPixelGeometry pixelGeometry;
    const char* name;
    const char* label;
};
static PixelGeometryState gPixelGeometryStates[] = {
    {SkPixelGeometry::kUnknown_SkPixelGeometry, "Mixed", nullptr },
    {SkPixelGeometry::kUnknown_SkPixelGeometry, "Flat",  "{Flat} "  },
    {SkPixelGeometry::kRGB_H_SkPixelGeometry,   "RGB H", "{RGB H} " },
    {SkPixelGeometry::kBGR_H_SkPixelGeometry,   "BGR H", "{BGR H} " },
    {SkPixelGeometry::kRGB_V_SkPixelGeometry,   "RGB_V", "{RGB V} " },
    {SkPixelGeometry::kBGR_V_SkPixelGeometry,   "BGR_V", "{BGR V} " },
};

struct FilterQualityState {
    SkFilterQuality fQuality;
    const char*     fName;
    const char*     fLabel;
};
static FilterQualityState gFilterQualityStates[] = {
    { kNone_SkFilterQuality,   "Mixed",    nullptr    },
    { kNone_SkFilterQuality,   "None",     "F0 "   },
    { kLow_SkFilterQuality,    "Low",      "F1 "   },
    { kMedium_SkFilterQuality, "Medium",   "F2 "   },
    { kHigh_SkFilterQuality,   "High",     "F3 "   },
};

class FlagsFilterCanvas : public SkPaintFilterCanvas {
public:
    FlagsFilterCanvas(SkCanvas* canvas, SkOSMenu::TriState lcd, SkOSMenu::TriState aa,
                      SkOSMenu::TriState subpixel, int hinting, int filterQuality)
        : INHERITED(canvas)
        , fLCDState(lcd)
        , fAAState(aa)
        , fSubpixelState(subpixel)
        , fHintingState(hinting)
        , fFilterQualityIndex(filterQuality) {
        SkASSERT((unsigned)filterQuality < SK_ARRAY_COUNT(gFilterQualityStates));
    }

protected:
    bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type t) const override {
        if (!*paint) {
            return true;
        }

        if (kText_Type == t && SkOSMenu::kMixedState != fLCDState) {
            paint->writable()->setLCDRenderText(SkOSMenu::kOnState == fLCDState);
        }
        if (SkOSMenu::kMixedState != fAAState) {
            paint->writable()->setAntiAlias(SkOSMenu::kOnState == fAAState);
        }
        if (0 != fFilterQualityIndex) {
            paint->writable()->setFilterQuality(gFilterQualityStates[fFilterQualityIndex].fQuality);
        }
        if (SkOSMenu::kMixedState != fSubpixelState) {
            paint->writable()->setSubpixelText(SkOSMenu::kOnState == fSubpixelState);
        }
        if (0 != fHintingState && fHintingState < (int)SK_ARRAY_COUNT(gHintingStates)) {
            paint->writable()->setHinting(gHintingStates[fHintingState].hinting);
        }
        return true;
    }

private:
    SkOSMenu::TriState  fLCDState;
    SkOSMenu::TriState  fAAState;
    SkOSMenu::TriState  fSubpixelState;
    int fHintingState;
    int fFilterQualityIndex;

    typedef SkPaintFilterCanvas INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SampleTFSerializer : public SkTypefaceSerializer {
public:
    sk_sp<SkData> serialize(SkTypeface* tf) override {
        tf->ref();
        return SkData::MakeWithCopy(&tf, sizeof(tf));
    }
};

class SampleTFDeserializer : public SkTypefaceDeserializer {
public:
    sk_sp<SkTypeface> deserialize(const void* data, size_t size) override {
        SkASSERT(sizeof(SkTypeface*) == size);
        SkTypeface* tf;
        memcpy(&tf, data, size);
        return sk_sp<SkTypeface>(tf);   // this was ref'd in SampleTFSerializer
    }
};

///////////////////////////////////////////////////////////////////////////////

enum TilingMode {
    kNo_Tiling,
    kAbs_128x128_Tiling,
    kAbs_256x256_Tiling,
    kRel_4x4_Tiling,
    kRel_1x16_Tiling,
    kRel_16x1_Tiling,

    kLast_TilingMode_Enum
};

struct TilingInfo {
    const char* label;
    SkScalar    w, h;
};

static const struct TilingInfo gTilingInfo[] = {
    { "No tiling", SK_Scalar1        , SK_Scalar1         }, // kNo_Tiling
    { "128x128"  , SkIntToScalar(128), SkIntToScalar(128) }, // kAbs_128x128_Tiling
    { "256x256"  , SkIntToScalar(256), SkIntToScalar(256) }, // kAbs_256x256_Tiling
    { "1/4x1/4"  , SK_Scalar1 / 4    , SK_Scalar1 / 4     }, // kRel_4x4_Tiling
    { "1/1x1/16" , SK_Scalar1        , SK_Scalar1 / 16    }, // kRel_1x16_Tiling
    { "1/16x1/1" , SK_Scalar1 / 16   , SK_Scalar1         }, // kRel_16x1_Tiling
};
static_assert((SK_ARRAY_COUNT(gTilingInfo) == kLast_TilingMode_Enum),
              "Incomplete_tiling_labels");

SkSize SampleWindow::tileSize() const {
    SkASSERT((TilingMode)fTilingMode < kLast_TilingMode_Enum);
    const struct TilingInfo* info = gTilingInfo + fTilingMode;
    return SkSize::Make(info->w > SK_Scalar1 ? info->w : this->width() * info->w,
                        info->h > SK_Scalar1 ? info->h : this->height() * info->h);
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

bool SampleWindow::sendAnimatePulse() {
    SkView* view = curr_view(this);
    if (SampleView::IsSampleView(view)) {
        return ((SampleView*)view)->animate(gAnimTimer);
    }
    return false;
}

void SampleWindow::setZoomCenter(float x, float y) {
    fZoomCenterX = x;
    fZoomCenterY = y;
}

bool SampleWindow::zoomIn() {
    // Arbitrarily decided
    if (fFatBitsScale == kMaxFatBitsScale) return false;
    fFatBitsScale++;
    this->inval(nullptr);
    return true;
}

bool SampleWindow::zoomOut() {
    if (fFatBitsScale == 1) return false;
    fFatBitsScale--;
    this->inval(nullptr);
    return true;
}

void SampleWindow::updatePointer(int x, int y) {
    fMouseX = x;
    fMouseY = y;
    if (fShowZoomer) {
        this->inval(nullptr);
    }
}

static inline SampleWindow::DeviceType cycle_devicetype(SampleWindow::DeviceType ct) {
    static const SampleWindow::DeviceType gCT[] = {
        SampleWindow::kRaster_DeviceType
#if SK_SUPPORT_GPU
        , SampleWindow::kGPU_DeviceType
#if SK_ANGLE
        , SampleWindow::kANGLE_DeviceType
#endif // SK_ANGLE
#endif // SK_SUPPORT_GPU
    };
    static_assert(SK_ARRAY_COUNT(gCT) == SampleWindow::kDeviceTypeCnt, "array_size_mismatch");
    return gCT[ct];
}

static SkString getSampleTitle(const SkViewFactory* sampleFactory) {
    SkView* view = (*sampleFactory)();
    SkString title;
    SampleCode::RequestTitle(view, &title);
    view->unref();
    return title;
}

static bool compareSampleTitle(const SkViewFactory* first, const SkViewFactory* second) {
    return strcmp(getSampleTitle(first).c_str(), getSampleTitle(second).c_str()) < 0;
}

static int find_by_title(const SkViewFactory* const* factories, int count, const char title[]) {
    for (int i = 0; i < count; i++) {
        if (getSampleTitle(factories[i]).equals(title)) {
            return i;
        }
    }
    return -1;
}

static void restrict_samples(SkTDArray<const SkViewFactory*>& factories, const SkString titles[],
                             int count) {
    int newCount = 0;
    for (int i = 0; i < count; ++i) {
        int index = find_by_title(factories.begin(), factories.count(), titles[i].c_str());
        if (index >= 0) {
            SkTSwap(factories.begin()[newCount], factories.begin()[index]);
            newCount += 1;
        }
    }
    if (newCount) {
        factories.setCount(newCount);
    }
}

DEFINE_string(slide, "", "Start on this sample.");
DEFINE_string(pictureDir, "", "Read pictures from here.");
DEFINE_string(picture, "", "Path to single picture.");
DEFINE_string(pathfinder, "", "SKP file with a single path to isolate.");
DEFINE_string(svg, "", "Path to single SVG file.");
DEFINE_string(svgDir, "", "Read SVGs from here.");
DEFINE_string(sequence, "", "Path to file containing the desired samples/gms to show.");
DEFINE_bool(sort, false, "Sort samples by title.");
DEFINE_bool(list, false, "List samples?");
DEFINE_bool(startgpu, false, "Start up with gpu?");
DEFINE_bool(redraw, false, "Force continuous redrawing, for profiling or debugging tools.");
#ifdef SAMPLE_PDF_FILE_VIEWER
DEFINE_string(pdfPath, "", "Path to direcotry of pdf files.");
#endif
#if SK_SUPPORT_GPU
DEFINE_pathrenderer_flag;
DEFINE_int32(msaa, 0, "Request multisampling with this count.");
DEFINE_bool(deepColor, false, "Request deep color (10-bit/channel or more) display buffer.");
#endif

#include "SkTaskGroup.h"

SampleWindow::SampleWindow(void* hwnd, int argc, char** argv, DeviceManager* devManager)
    : INHERITED(hwnd)
    , fDevManager(nullptr) {

    SkCommandLineFlags::Parse(argc, argv);

    fCurrIndex = -1;

    if (!FLAGS_pictureDir.isEmpty()) {
        SkOSFile::Iter iter(FLAGS_pictureDir[0], "skp");
        SkString filename;
        while (iter.next(&filename)) {
            *fSamples.append() = new PictFileFactory(
                SkOSPath::Join(FLAGS_pictureDir[0], filename.c_str()));
        }
    }
    if (!FLAGS_picture.isEmpty()) {
        SkString path(FLAGS_picture[0]);
        fCurrIndex = fSamples.count();
        *fSamples.append() = new PictFileFactory(path);
    }
    if (!FLAGS_pathfinder.isEmpty()) {
        SkString path(FLAGS_pathfinder[0]);
        fCurrIndex = fSamples.count();
        *fSamples.append() = new PathFinderFactory(path);
    }
    if (!FLAGS_svg.isEmpty()) {
        SkString path(FLAGS_svg[0]);
        fCurrIndex = fSamples.count();
        *fSamples.append() = new SVGFileFactory(path);
    }
    if (!FLAGS_svgDir.isEmpty()) {
        SkOSFile::Iter iter(FLAGS_svgDir[0], "svg");
        SkString filename;
        while (iter.next(&filename)) {
            *fSamples.append() = new SVGFileFactory(
                SkOSPath::Join(FLAGS_svgDir[0], filename.c_str()));
        }
    }
#ifdef SAMPLE_PDF_FILE_VIEWER
    if (!FLAGS_pdfPath.isEmpty()) {
        SkOSFile::Iter iter(FLAGS_pdfPath[0], "pdf");
        SkString filename;
        while (iter.next(&filename)) {
            *fSamples.append() = new PdfFileViewerFactory(
                SkOSPath::Join(FLAGS_pictureDir[0], filename.c_str()));
        }
    }
#endif
    SkGMRegistyToSampleRegistry();
    {
        const SkViewRegister* reg = SkViewRegister::Head();
        while (reg) {
            *fSamples.append() = reg->factory();
            reg = reg->next();
        }
    }

    if (!FLAGS_sequence.isEmpty()) {
        // The sequence file just contains a list (separated by CRs) of the samples or GM:gms
        // you want to restrict to. Only these will appear when you cycle through.
        // If none are found, or the file is empty, then it will be ignored, and all samples
        // will be available.
        SkFILEStream stream(FLAGS_sequence[0]);
        if (stream.isValid()) {
            size_t len = stream.getLength();
            SkAutoTMalloc<char> storage(len + 1);
            char* buffer = storage.get();
            stream.read(buffer, len);
            buffer[len] = 0;

            SkTArray<SkString> titles;
            SkStrSplit(buffer, "\n\r", &titles);
            restrict_samples(fSamples, titles.begin(), titles.count());
        }
    }

    if (FLAGS_sort) {
        // Sort samples, so foo.skp and foo.pdf are consecutive and we can quickly spot where
        // skp -> pdf -> png fails.
        SkTQSort(fSamples.begin(), fSamples.end() ? fSamples.end() - 1 : nullptr, compareSampleTitle);
    }

    if (!FLAGS_slide.isEmpty()) {
        fCurrIndex = findByTitle(FLAGS_slide[0]);
        if (fCurrIndex < 0) {
            fprintf(stderr, "Unknown sample \"%s\"\n", FLAGS_slide[0]);
            listTitles();
        }
    }

#if SK_SUPPORT_GPU
    fBackendOptions.fGrContextOptions.fGpuPathRenderers = CollectGpuPathRenderersFromFlags();
    fBackendOptions.fMSAASampleCount = FLAGS_msaa;
    fBackendOptions.fDeepColor = FLAGS_deepColor;
#endif
    fColorConfigIndex = 0;

    if (FLAGS_list) {
        listTitles();
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

    static SkTaskGroup::Enabler enabled(-1);
    gSampleWindow = this;

    fDeviceType = kRaster_DeviceType;
#if SK_SUPPORT_GPU
    if (FLAGS_startgpu) {
        fDeviceType = kGPU_DeviceType;
    }
#endif

#if DEFAULT_TO_GPU
    fDeviceType = kGPU_DeviceType;
#endif
#if SK_ANGLE && DEFAULT_TO_ANGLE
    fDeviceType = kANGLE_DeviceType;
#endif

    fUseClip = false;
    fUsePicture = false;
    fAnimating = false;
    fRotate = false;
    fPerspAnim = false;
    fRequestGrabImage = false;
    fTilingMode = kNo_Tiling;
    fMeasureFPS = false;
    fUseDeferredCanvas = false;
    fLCDState = SkOSMenu::kMixedState;
    fAAState = SkOSMenu::kMixedState;
    fSubpixelState = SkOSMenu::kMixedState;
    fHintingState = 0;
    fPixelGeometryIndex = 0;
    fFilterQualityIndex = 0;
    fFlipAxis = 0;

    fMouseX = fMouseY = 0;
    fFatBitsScale = 8;
    fTypeface = SkTypeface::MakeFromName("Courier", SkFontStyle(SkFontStyle::kBold_Weight,
                                                                SkFontStyle::kNormal_Width,
                                                                SkFontStyle::kUpright_Slant));
    fShowZoomer = false;

    fZoomLevel = 0;
    fZoomScale = SK_Scalar1;
    fOffset = { 0, 0 };

    fMagnify = false;

    fSaveToPdf = false;
    fSaveToSKP = false;

    if (true) {
        fPipeSerializer.setTypefaceSerializer(new SampleTFSerializer);
        fPipeDeserializer.setTypefaceDeserializer(new SampleTFDeserializer);
    }

    int sinkID = this->getSinkID();
    fAppMenu = new SkOSMenu;
    fAppMenu->setTitle("Global Settings");
    int itemID;

    itemID = fAppMenu->appendList("ColorType", "ColorType", sinkID, 0,
                                  gConfig[0].fName,
                                  gConfig[1].fName,
                                  gConfig[2].fName,
                                  gConfig[3].fName,
                                  gConfig[4].fName,
                                  nullptr);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'C');

    itemID = fAppMenu->appendList("Device Type", "Device Type", sinkID, 0,
                                  "Raster",
                                  "OpenGL",
#if SK_ANGLE
                                  "ANGLE",
#endif
                                  nullptr);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'd');
    itemID = fAppMenu->appendTriState("AA", "AA", sinkID, fAAState);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'b');
    itemID = fAppMenu->appendTriState("LCD", "LCD", sinkID, fLCDState);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'l');
    itemID = fAppMenu->appendList("FilterQuality", "FilterQuality", sinkID, fFilterQualityIndex,
                                  gFilterQualityStates[0].fName,
                                  gFilterQualityStates[1].fName,
                                  gFilterQualityStates[2].fName,
                                  gFilterQualityStates[3].fName,
                                  gFilterQualityStates[4].fName,
                                  nullptr);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'n');
    itemID = fAppMenu->appendTriState("Subpixel", "Subpixel", sinkID, fSubpixelState);
    fAppMenu->assignKeyEquivalentToItem(itemID, 's');
    itemID = fAppMenu->appendList("Hinting", "Hinting", sinkID, fHintingState,
                                  gHintingStates[0].name,
                                  gHintingStates[1].name,
                                  gHintingStates[2].name,
                                  gHintingStates[3].name,
                                  gHintingStates[4].name,
                                  nullptr);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'h');

    itemID = fAppMenu->appendList("Pixel Geometry", "Pixel Geometry", sinkID, fPixelGeometryIndex,
                                  gPixelGeometryStates[0].name,
                                  gPixelGeometryStates[1].name,
                                  gPixelGeometryStates[2].name,
                                  gPixelGeometryStates[3].name,
                                  gPixelGeometryStates[4].name,
                                  gPixelGeometryStates[5].name,
                                  nullptr);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'P');

    itemID =fAppMenu->appendList("Tiling", "Tiling", sinkID, fTilingMode,
                                 gTilingInfo[kNo_Tiling].label,
                                 gTilingInfo[kAbs_128x128_Tiling].label,
                                 gTilingInfo[kAbs_256x256_Tiling].label,
                                 gTilingInfo[kRel_4x4_Tiling].label,
                                 gTilingInfo[kRel_1x16_Tiling].label,
                                 gTilingInfo[kRel_16x1_Tiling].label,
                                 nullptr);
    fAppMenu->assignKeyEquivalentToItem(itemID, 't');

    itemID = fAppMenu->appendSwitch("Slide Show", "Slide Show" , sinkID, false);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'a');
    itemID = fAppMenu->appendSwitch("Clip", "Clip" , sinkID, fUseClip);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'c');
    itemID = fAppMenu->appendSwitch("Flip X", "Flip X" , sinkID, false);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'x');
    itemID = fAppMenu->appendSwitch("Flip Y", "Flip Y" , sinkID, false);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'y');
    itemID = fAppMenu->appendSwitch("Zoomer", "Zoomer" , sinkID, fShowZoomer);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'z');
    itemID = fAppMenu->appendSwitch("Magnify", "Magnify" , sinkID, fMagnify);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'm');

    itemID = fAppMenu->appendAction("Save to PDF", sinkID);
    fAppMenu->assignKeyEquivalentToItem(itemID, 'e');

    this->addMenu(fAppMenu);
    fSlideMenu = new SkOSMenu;
    this->addMenu(fSlideMenu);

    this->setVisibleP(true);
    this->setClipToBounds(false);

    this->loadView((*fSamples[fCurrIndex])());

    if (nullptr == devManager) {
        fDevManager = new DefaultDeviceManager();
    } else {
        devManager->ref();
        fDevManager = devManager;
    }
    fDevManager->setUpBackend(this, fBackendOptions);

    // If another constructor set our dimensions, ensure that our
    // onSizeChange gets called.
    if (this->height() && this->width()) {
        this->onSizeChange();
    }

    // can't call this synchronously, since it may require a subclass to
    // to implement, or the caller may need us to have returned from the
    // constructor first. Hence we post an event to ourselves.
//    this->updateTitle();
    post_event_to_sink(new SkEvent(gUpdateWindowTitleEvtName), this);

    gAnimTimer.run();
}

SampleWindow::~SampleWindow() {
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

void SampleWindow::listTitles() {
    int count = fSamples.count();
    SkDebugf("All Slides:\n");
    for (int i = 0; i < count; i++) {
        SkDebugf("    %s\n", getSampleTitle(i).c_str());
    }
}

static SkBitmap capture_bitmap(SkCanvas* canvas) {
    SkBitmap bm;
    if (bm.tryAllocPixels(canvas->imageInfo())) {
        canvas->readPixels(bm, 0, 0);
    }
    return bm;
}

static void drawText(SkCanvas* canvas, SkString str, SkScalar left, SkScalar top, SkPaint& paint) {
    SkColor desiredColor = paint.getColor();
    paint.setColor(SK_ColorWHITE);
    const char* c_str = str.c_str();
    size_t size = str.size();
    SkRect bounds;
    paint.measureText(c_str, size, &bounds);
    bounds.offset(left, top);
    SkScalar inset = SkIntToScalar(-2);
    bounds.inset(inset, inset);
    canvas->drawRect(bounds, paint);
    paint.setColor(desiredColor);
    canvas->drawText(c_str, size, left, top, paint);
}

#define XCLIP_N  8
#define YCLIP_N  8

#include "SkDeferredCanvas.h"
#include "SkDumpCanvas.h"

void SampleWindow::draw(SkCanvas* canvas) {
    std::unique_ptr<SkThreadedBMPDevice> tDev;
    std::unique_ptr<SkCanvas> tCanvas;
    if (fTiles > 0 && fDeviceType == kRaster_DeviceType) {
        tDev.reset(new SkThreadedBMPDevice(this->getBitmap(), fTiles, fThreads));
        tCanvas.reset(new SkCanvas(tDev.get()));
        canvas = tCanvas.get();
    }

    gAnimTimer.updateTime();

    if (fGesture.isActive()) {
        this->updateMatrix();
    }

    if (fMeasureFPS) {
        fMeasureFPS_Time = 0;
    }

    SkSize tile = this->tileSize();

    if (kNo_Tiling == fTilingMode) {
        SkDebugfDumper dumper;
        SkDumpCanvas dump(&dumper);
        SkDeferredCanvas deferred(canvas, SkDeferredCanvas::kEager);
        SkCanvas* c = fUseDeferredCanvas ? &deferred : canvas;
        this->INHERITED::draw(c); // no looping or surfaces needed
    } else {
        const SkScalar w = SkScalarCeilToScalar(tile.width());
        const SkScalar h = SkScalarCeilToScalar(tile.height());
        SkImageInfo info = SkImageInfo::MakeN32Premul(SkScalarTruncToInt(w), SkScalarTruncToInt(h));
        auto surface(canvas->makeSurface(info));
        SkCanvas* tileCanvas = surface->getCanvas();

        for (SkScalar y = 0; y < height(); y += h) {
            for (SkScalar x = 0; x < width(); x += w) {
                SkAutoCanvasRestore acr(tileCanvas, true);
                tileCanvas->translate(-x, -y);
                tileCanvas->clear(0);
                this->INHERITED::draw(tileCanvas);
                surface->draw(canvas, x, y, nullptr);
            }
        }

        // for drawing the borders between tiles
        SkPaint paint;
        paint.setColor(0x60FF00FF);
        paint.setStyle(SkPaint::kStroke_Style);

        for (SkScalar y = 0; y < height(); y += tile.height()) {
            for (SkScalar x = 0; x < width(); x += tile.width()) {
                canvas->drawRect(SkRect::MakeXYWH(x, y, tile.width(), tile.height()), paint);
            }
        }
    }

    if (fShowZoomer && !fSaveToPdf) {
        showZoomer(canvas);
    }
    if (fMagnify && !fSaveToPdf) {
        magnify(canvas);
    }

    if (fMeasureFPS && fMeasureFPS_Time) {
        this->updateTitle();
        this->postInvalDelay();
    }

    if (this->sendAnimatePulse() || FLAGS_redraw) {
        this->inval(nullptr);
    }

    canvas->flush();

    // do this last
    fDevManager->publishCanvas(fDeviceType, canvas, this);
}

static float clipW = 200;
static float clipH = 200;
void SampleWindow::magnify(SkCanvas* canvas) {
    SkRect r;
    int count = canvas->save();

    SkMatrix m = canvas->getTotalMatrix();
    if (!m.invert(&m)) {
        return;
    }
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

static SkPaint& set_color_ref(SkPaint& paint, SkColor c) {
    paint.setColor(c);
    return paint;
}

static void show_lcd_box(SkCanvas* canvas, SkScalar x, SkScalar y, SkColor c,
                         SkScalar sx, SkScalar sy) {
    const SkScalar w = (1 - 1/sx) / 3;
    SkPaint paint;
    SkRect r = SkRect::MakeXYWH(x, y, w, 1 - 1/sy);
    canvas->drawRect(r, set_color_ref(paint, SkColorSetRGB(SkColorGetR(c), 0, 0)));
    r.offset(w, 0);
    canvas->drawRect(r, set_color_ref(paint, SkColorSetRGB(0, SkColorGetG(c), 0)));
    r.offset(w, 0);
    canvas->drawRect(r, set_color_ref(paint, SkColorSetRGB(0, 0, SkColorGetB(c))));
}

static void show_lcd_circle(SkCanvas* canvas, SkScalar x, SkScalar y, SkColor c,
                            SkScalar, SkScalar) {
    const SkRect r = SkRect::MakeXYWH(x, y, 1, 1);
    const SkScalar cx = x + 0.5f;
    const SkScalar cy = y + 0.5f;

    SkPaint paint;
    paint.setAntiAlias(true);

    SkPath path;
    path.addArc(r, 0, 120); path.lineTo(cx, cy);
    canvas->drawPath(path, set_color_ref(paint, SkColorSetRGB(SkColorGetR(c), 0, 0)));

    path.reset(); path.addArc(r, 120, 120); path.lineTo(cx, cy);
    canvas->drawPath(path, set_color_ref(paint, SkColorSetRGB(0, SkColorGetG(c), 0)));

    path.reset(); path.addArc(r, 240, 120); path.lineTo(cx, cy);
    canvas->drawPath(path, set_color_ref(paint, SkColorSetRGB(0, 0, SkColorGetB(c))));
}

typedef void (*ShowLCDProc)(SkCanvas*, SkScalar, SkScalar, SkColor, SkScalar, SkScalar);

/*
 *  Like drawBitmapRect but we manually draw each pixels in RGB
 */
static void show_lcd_grid(SkCanvas* canvas, const SkBitmap& bitmap,
                          const SkIRect& origSrc, const SkRect& dst, ShowLCDProc proc) {
    SkIRect src;
    if (!src.intersect(origSrc, bitmap.bounds())) {
        return;
    }
    const SkScalar sx = dst.width() / src.width();
    const SkScalar sy = dst.height() / src.height();

    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(dst.left(), dst.top());
    canvas->scale(sx, sy);

    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.width(); ++x) {
            proc(canvas, SkIntToScalar(x), SkIntToScalar(y),
                 bitmap.getColor(src.left() + x, src.top() + y), sx, sy);
        }
    }
}

void SampleWindow::showZoomer(SkCanvas* canvas) {
    int count = canvas->save();
    canvas->resetMatrix();
    // Ensure the mouse position is on screen.
    int width = SkScalarRoundToInt(this->width());
    int height = SkScalarRoundToInt(this->height());
    if (fMouseX >= width) fMouseX = width - 1;
    else if (fMouseX < 0) fMouseX = 0;
    if (fMouseY >= height) fMouseY = height - 1;
    else if (fMouseY < 0) fMouseY = 0;

    SkBitmap bitmap = capture_bitmap(canvas);

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
    switch (fFatBitsScale) {
        case kMaxFatBitsScale:
            show_lcd_grid(canvas, bitmap, src, dest, show_lcd_box);
            break;
        case kMaxFatBitsScale - 1:
            show_lcd_grid(canvas, bitmap, src, dest, show_lcd_circle);
            break;
        default:
            canvas->drawBitmapRect(bitmap, src, dest, nullptr);
            break;
    }

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
    paint.setTextSize(18);
    SkScalar lineHeight = paint.getFontMetrics(nullptr);
    SkString string;
    string.appendf("(%i, %i)", fMouseX, fMouseY);
    SkScalar left = dest.fLeft + SkIntToScalar(3);
    SkScalar i = SK_Scalar1;
    drawText(canvas, string, left, lineHeight * i + dest.fTop, paint);
    // Alpha
    i += SK_Scalar1;
    string.reset();
    string.appendf("A: %X", SkColorGetA(color));
    drawText(canvas, string, left, lineHeight * i + dest.fTop, paint);
    // Red
    i += SK_Scalar1;
    string.reset();
    string.appendf("R: %X", SkColorGetR(color));
    paint.setColor(SK_ColorRED);
    drawText(canvas, string, left, lineHeight * i + dest.fTop, paint);
    // Green
    i += SK_Scalar1;
    string.reset();
    string.appendf("G: %X", SkColorGetG(color));
    paint.setColor(0xFF008800);
    drawText(canvas, string, left, lineHeight * i + dest.fTop, paint);
    // Blue
    i += SK_Scalar1;
    string.reset();
    string.appendf("B: %X", SkColorGetB(color));
    paint.setColor(SK_ColorBLUE);
    drawText(canvas, string, left, lineHeight * i + dest.fTop, paint);
    canvas->restoreToCount(count);
}

void SampleWindow::onDraw(SkCanvas* canvas) {
}

#include "SkColorPriv.h"

void SampleWindow::saveToPdf()
{
    fSaveToPdf = true;
    this->inval(nullptr);
}

SkCanvas* SampleWindow::beforeChildren(SkCanvas* canvas) {
    if (fSaveToPdf) {
        SkString name;
        if (!this->getRawTitle(&name)) {
            name.set("unknown_sample");
        }
        name.append(".pdf");
#ifdef SK_BUILD_FOR_ANDROID
        name.prepend("/sdcard/");
#endif
        fPDFDocument = SkDocument::MakePDF(name.c_str());
        canvas = fPDFDocument->beginPage(this->width(), this->height());
    } else if (fSaveToSKP) {
        canvas = fRecorder.beginRecording(9999, 9999, nullptr, 0);
    } else if (fUsePicture) {
        if (PICTURE_MEANS_PIPE) {
            fPipeStream.reset(new SkDynamicMemoryWStream);
            canvas = fPipeSerializer.beginWrite(SkRect::MakeWH(this->width(), this->height()),
                                                fPipeStream.get());
        } else {
            canvas = fRecorder.beginRecording(9999, 9999, nullptr, 0);
        }
    } else {
        canvas = this->INHERITED::beforeChildren(canvas);
    }

    if (fUseClip) {
        canvas->drawColor(0xFFFF88FF);
        canvas->clipPath(fClipPath, kIntersect_SkClipOp, true);
    }

    // Install a flags filter proxy canvas if needed
    if (fLCDState != SkOSMenu::kMixedState ||
        fAAState != SkOSMenu::kMixedState ||
        fSubpixelState != SkOSMenu::kMixedState ||
        fHintingState > 0 ||
        fFilterQualityIndex > 0) {
        canvas = new FlagsFilterCanvas(canvas, fLCDState, fAAState, fSubpixelState, fHintingState,
                                       fFilterQualityIndex);
        fFlagsFilterCanvas.reset(canvas);
    }

    return canvas;
}
#include "SkMultiPictureDraw.h"
void SampleWindow::afterChildren(SkCanvas* orig) {
    fFlagsFilterCanvas.reset(nullptr);

    if (fSaveToPdf) {
        fSaveToPdf = false;
        fPDFDocument->endPage();
        fPDFDocument.reset(nullptr);
        // We took over the draw calls in order to create the PDF, so we need
        // to redraw.
        this->inval(nullptr);
        return;
    }

    if (fRequestGrabImage) {
        fRequestGrabImage = false;

        SkBitmap bmp = capture_bitmap(orig);
        if (!bmp.isNull()) {
            static int gSampleGrabCounter;
            SkString name;
            name.printf("sample_grab_%d.png", gSampleGrabCounter++);
            sk_tool_utils::EncodeImageToFile(name.c_str(), bmp,
                                       SkEncodedImageFormat::kPNG, 100);
        }
        this->inval(nullptr);
        return;
    }

    if (fSaveToSKP) {
        sk_sp<SkPicture> picture(fRecorder.finishRecordingAsPicture());
        SkFILEWStream stream("sample_app.skp");
        picture->serialize(&stream);
        fSaveToSKP = false;
        this->inval(nullptr);
        return;
    }

    if (fUsePicture) {
        if (PICTURE_MEANS_PIPE) {
            fPipeSerializer.endWrite();
            sk_sp<SkData> data(fPipeStream->detachAsData());
            fPipeDeserializer.playback(data->data(), data->size(), orig);
            fPipeStream.reset();
        } else {
            sk_sp<SkPicture> picture(fRecorder.finishRecordingAsPicture());
            if (SERIALIZE_PICTURE) {
                auto data = picture->serialize();
                picture = SkPicture::MakeFromData(data.get(), nullptr);
            }
            orig->drawPicture(picture.get());
        }
    }

    // Do this after presentGL and other finishing, rather than in afterChild
    if (fMeasureFPS) {
        orig->flush();
        fTimer.end();
        fMeasureFPS_Time += fTimer.fWall;
        fCumulativeFPS_Time += fTimer.fWall;
        fCumulativeFPS_Count += FPS_REPEAT_COUNT;
    }
}

void SampleWindow::beforeChild(SkView* child, SkCanvas* canvas) {
    if (fRotate) {
        SkScalar cx = this->width() / 2;
        SkScalar cy = this->height() / 2;
        canvas->rotate(gAnimTimer.scaled(10), cx, cy);
    }

    if (fPerspAnim) {
        SkScalar secs = gAnimTimer.scaled(1);

        static const SkScalar gAnimPeriod = 10 * SK_Scalar1;
        static const SkScalar gAnimMag = SK_Scalar1 / 1000;
        SkScalar t = SkScalarMod(secs, gAnimPeriod);
        if (SkScalarFloorToInt(secs / gAnimPeriod) & 0x1) {
            t = gAnimPeriod - t;
        }
        t = 2 * t - gAnimPeriod;
        t *= gAnimMag / gAnimPeriod;
        SkMatrix m;
        m.reset();
#if 1
        m.setPerspY(t);
#else
        m.setPerspY(SK_Scalar1 / 1000);
        m.setSkewX(8.0f / 25);
        m.dump();
#endif
        canvas->concat(m);
    }

    if (fMeasureFPS) {
        (void)SampleView::SetRepeatDraw(child, FPS_REPEAT_COUNT);
        fTimer.start();
    } else {
        (void)SampleView::SetRepeatDraw(child, 1);
    }
    if (fPerspAnim || fRotate) {
        this->inval(nullptr);
    }
}

void SampleWindow::changeOffset(SkVector delta) {
    fOffset += delta;
    this->updateMatrix();
}

void SampleWindow::changeZoomLevel(float delta) {
    fZoomLevel += delta;
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

    m.postTranslate(fOffset.fX, fOffset.fY);

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
    this->inval(nullptr);
}
bool SampleWindow::previousSample() {
    this->resetFPS();
    fCurrIndex = (fCurrIndex - 1 + fSamples.count()) % fSamples.count();
    this->loadView((*fSamples[fCurrIndex])());
    return true;
}

#include "SkResourceCache.h"
#include "SkGlyphCache.h"
bool SampleWindow::nextSample() {
    this->resetFPS();
    fCurrIndex = (fCurrIndex + 1) % fSamples.count();
    this->loadView((*fSamples[fCurrIndex])());

    if (false) {
        SkResourceCache::TestDumpMemoryStatistics();
        SkGlyphCache::Dump();
        SkDebugf("\n");
    }

    return true;
}

bool SampleWindow::goToSample(int i) {
    this->resetFPS();
    fCurrIndex = (i) % fSamples.count();
    this->loadView((*fSamples[fCurrIndex])());
    return true;
}

SkString SampleWindow::getSampleTitle(int i) {
    return ::getSampleTitle(fSamples[i]);
}

int SampleWindow::sampleCount() {
    return fSamples.count();
}

void SampleWindow::showOverview() {
    this->loadView(create_overview(fSamples.count(), fSamples.begin()));
}

void SampleWindow::postAnimatingEvent() {
    if (fAnimating) {
        (new SkEvent(ANIMATING_EVENTTYPE, this->getSinkID()))->postDelay(ANIMATING_DELAY);
    }
}

static sk_sp<SkColorSpace> getMonitorColorSpace() {
#if defined(SK_BUILD_FOR_MAC)
    CGColorSpaceRef cs = CGDisplayCopyColorSpace(CGMainDisplayID());
    CFDataRef dataRef = CGColorSpaceCopyICCProfile(cs);
    const uint8_t* data = CFDataGetBytePtr(dataRef);
    size_t size = CFDataGetLength(dataRef);

    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeICC(data, size);

    CFRelease(cs);
    CFRelease(dataRef);
    return colorSpace;
#elif defined(SK_BUILD_FOR_WIN)
    DISPLAY_DEVICE dd = { sizeof(DISPLAY_DEVICE) };

    // Chrome's code for this currently just gets the primary monitor's profile. This code iterates
    // over all attached monitors, so it's "better" in that sense. Making intelligent use of this
    // information (via things like MonitorFromWindow or MonitorFromRect to pick the correct
    // profile for a particular window or region of a window), is an exercise left to the reader.
    for (int i = 0; EnumDisplayDevices(NULL, i, &dd, 0); ++i) {
        if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
            // There are other helpful things in dd at this point:
            // dd.DeviceString has a longer name for the adapter
            // dd.StateFlags indicates primary display, mirroring, etc...
            HDC dc = CreateDC(NULL, dd.DeviceName, NULL, NULL);
            if (dc) {
                char icmPath[MAX_PATH + 1];
                DWORD pathLength = MAX_PATH;
                BOOL success = GetICMProfileA(dc, &pathLength, icmPath);
                DeleteDC(dc);
                if (success) {
                    sk_sp<SkData> iccData = SkData::MakeFromFileName(icmPath);
                    return SkColorSpace::MakeICC(iccData->data(), iccData->size());
                }
            }
        }
    }

    return nullptr;
#else
    return nullptr;
#endif
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
    if (evt.isType("set-curr-index")) {
        this->goToSample(evt.getFast32());
        return true;
    }
    if (isInvalEvent(evt)) {
        this->inval(nullptr);
        return true;
    }
    int selected = -1;
    if (SkOSMenu::FindListIndex(evt, "Device Type", &selected)) {
        this->setDeviceType((DeviceType)selected);
        return true;
    }
    if (SkOSMenu::FindListIndex(evt, "ColorType", &selected)) {
        fColorConfigIndex = selected;
        sk_sp<SkColorSpace> colorSpace = nullptr;
        switch (gConfig[selected].fColorSpace) {
            case kSRGB_OutputColorSpace:
                colorSpace = SkColorSpace::MakeSRGB();
                break;
            case kNarrow_OutputColorSpace:
                {
                    // NarrowGamut RGB (an artifically smaller than sRGB gamut)
                    SkColorSpacePrimaries primaries ={
                        0.54f, 0.33f,     // Rx, Ry
                        0.33f, 0.50f,     // Gx, Gy
                        0.25f, 0.20f,     // Bx, By
                        0.3127f, 0.3290f, // Wx, Wy
                    };
                    SkMatrix44 narrowGamutRGBMatrix(SkMatrix44::kUninitialized_Constructor);
                    primaries.toXYZD50(&narrowGamutRGBMatrix);
                    colorSpace = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                       narrowGamutRGBMatrix);
                }
                break;
            case kMonitor_OutputColorSpace:
                colorSpace = getMonitorColorSpace();
                if (!colorSpace) {
                    // Fallback for platforms / machines where we can't get a monitor profile
                    colorSpace = SkColorSpace::MakeSRGB();
                }
                break;
            case kLegacy_OutputColorSpace:
            default:
                // Do nothing
                break;
        }
        if (kRGBA_F16_SkColorType == gConfig[selected].fColorType) {
            SkASSERT(colorSpace);
            SkASSERT(SkColorSpace_Base::Type::kXYZ == as_CSB(colorSpace)->type());
            SkColorSpace_XYZ* csXYZ = static_cast<SkColorSpace_XYZ*>(colorSpace.get());
            colorSpace = csXYZ->makeLinearGamma();
        }
        this->setDeviceColorType(gConfig[selected].fColorType, colorSpace);
        return true;
    }
    if (SkOSMenu::FindSwitchState(evt, "Slide Show", nullptr)) {
        this->toggleSlideshow();
        return true;
    }
    if (SkOSMenu::FindTriState(evt, "AA", &fAAState) ||
        SkOSMenu::FindTriState(evt, "LCD", &fLCDState) ||
        SkOSMenu::FindListIndex(evt, "FilterQuality", &fFilterQualityIndex) ||
        SkOSMenu::FindTriState(evt, "Subpixel", &fSubpixelState) ||
        SkOSMenu::FindListIndex(evt, "Hinting", &fHintingState) ||
        SkOSMenu::FindSwitchState(evt, "Clip", &fUseClip) ||
        SkOSMenu::FindSwitchState(evt, "Zoomer", &fShowZoomer) ||
        SkOSMenu::FindSwitchState(evt, "Magnify", &fMagnify))
    {
        this->inval(nullptr);
        this->updateTitle();
        return true;
    }
    if (SkOSMenu::FindListIndex(evt, "Pixel Geometry", &fPixelGeometryIndex)) {
        this->setPixelGeometry(fPixelGeometryIndex);
        return true;
    }
    if (SkOSMenu::FindListIndex(evt, "Tiling", &fTilingMode)) {
        if (SampleView::IsSampleView(curr_view(this))) {
            ((SampleView*)curr_view(this))->onTileSizeChanged(this->tileSize());
        }
        this->inval(nullptr);
        this->updateTitle();
        return true;
    }
    if (SkOSMenu::FindSwitchState(evt, "Flip X", nullptr)) {
        fFlipAxis ^= kFlipAxis_X;
        this->updateMatrix();
        return true;
    }
    if (SkOSMenu::FindSwitchState(evt, "Flip Y", nullptr)) {
        fFlipAxis ^= kFlipAxis_Y;
        this->updateMatrix();
        return true;
    }
    if (SkOSMenu::FindAction(evt,"Save to PDF")) {
        this->saveToPdf();
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
        query->setFast32(this->getGrContext() != nullptr);
        return true;
    }
    return this->INHERITED::onQuery(query);
}

DECLARE_bool(portableFonts);

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
        this->changeOffset({SkIntToScalar(dx / 32.0f), SkIntToScalar(dy / 32.0f)});
        return true;
    }

    switch (uni) {
        case 27:    // ESC
            gAnimTimer.stop();
            if (this->sendAnimatePulse()) {
                this->inval(nullptr);
            }
            break;
        case '+':
            gSampleWindow->setTiles(gSampleWindow->getTiles() + 1);
            this->inval(nullptr);
            this->updateTitle();
            break;
        case '-':
            gSampleWindow->setTiles(SkTMax(0, gSampleWindow->getTiles() - 1));
            this->inval(nullptr);
            this->updateTitle();
            break;
        case '>':
            gSampleWindow->setThreads(gSampleWindow->getThreads() + 1);
            this->inval(nullptr);
            this->updateTitle();
            break;
        case '<':
            gSampleWindow->setThreads(SkTMax(0, gSampleWindow->getThreads() - 1));
            this->inval(nullptr);
            this->updateTitle();
            break;
        case ' ':
            gAnimTimer.togglePauseResume();
            if (this->sendAnimatePulse()) {
                this->inval(nullptr);
            }
            break;
        case '0':
            this->resetFPS();
            break;
        case 'A':
            if (gSkUseAnalyticAA.load() && !gSkForceAnalyticAA.load()) {
                gSkForceAnalyticAA = true;
            } else {
                gSkUseAnalyticAA = !gSkUseAnalyticAA.load();
                gSkForceAnalyticAA = false;
            }
            this->inval(nullptr);
            this->updateTitle();
            break;
        case 'B':
            post_event_to_sink(new SkEvent("PictFileView::toggleBBox"), curr_view(this));
            // Cannot call updateTitle() synchronously, because the toggleBBox event is still in
            // the queue.
            post_event_to_sink(new SkEvent(gUpdateWindowTitleEvtName), this);
            this->inval(nullptr);
            break;
        case 'D':
            toggleDistanceFieldFonts();
            break;
        case 'E':
            fUseDeferredCanvas = !fUseDeferredCanvas;
            this->inval(nullptr);
            break;
        case 'f':
            // only
            toggleFPS();
            break;
        case 'F':
            FLAGS_portableFonts ^= true;
            this->inval(nullptr);
            break;
        case 'g':
            fRequestGrabImage = true;
            this->inval(nullptr);
            break;
        case 'G':
            gShowGMBounds = !gShowGMBounds;
            post_event_to_sink(GMSampleView::NewShowSizeEvt(gShowGMBounds),
                            curr_view(this));
            this->inval(nullptr);
            break;
        case 'i':
            this->zoomIn();
            break;
        case 'o':
            this->zoomOut();
            break;
        case 'r':
            fRotate = !fRotate;
            this->inval(nullptr);
            this->updateTitle();
            return true;
        case 'k':
            fPerspAnim = !fPerspAnim;
            this->inval(nullptr);
            this->updateTitle();
            return true;
        case 'K':
            fSaveToSKP = true;
            this->inval(nullptr);
            return true;
        case 'M':
            fUsePicture = !fUsePicture;
            this->inval(nullptr);
            this->updateTitle();
            return true;
#if SK_SUPPORT_GPU
        case 'p':
            {
                GrContext* grContext = this->getGrContext();
                if (grContext) {
                    size_t cacheBytes;
                    grContext->getResourceCacheUsage(nullptr, &cacheBytes);
                    grContext->freeGpuResources();
                    SkDebugf("Purged %d bytes from the GPU resource cache.\n", cacheBytes);
                }
            }
            return true;
#endif
        default:
            break;
    }

    if (fAppMenu->handleKeyEquivalent(uni)|| fSlideMenu->handleKeyEquivalent(uni)) {
        this->onUpdateMenu(fAppMenu);
        this->onUpdateMenu(fSlideMenu);
        return true;
    }
    return this->INHERITED::onHandleChar(uni);
}

void SampleWindow::setDeviceType(DeviceType type) {
    if (type == fDeviceType)
        return;

    fDevManager->tearDownBackend(this);
    fDeviceType = type;
    fDevManager->setUpBackend(this, fBackendOptions);

    this->updateTitle();
    this->inval(nullptr);
}

void SampleWindow::setDeviceColorType(SkColorType ct, sk_sp<SkColorSpace> cs) {
    this->setColorType(ct, std::move(cs));

    fDevManager->tearDownBackend(this);
    fDevManager->setUpBackend(this, fBackendOptions);

    this->updateTitle();
    this->inval(nullptr);
}

void SampleWindow::toggleSlideshow() {
    fAnimating = !fAnimating;
    this->postAnimatingEvent();
    this->updateTitle();
}

void SampleWindow::toggleRendering() {
    this->setDeviceType(cycle_devicetype(fDeviceType));
    this->updateTitle();
    this->inval(nullptr);
}

void SampleWindow::toggleFPS() {
    fMeasureFPS = !fMeasureFPS;
    this->updateTitle();
    this->inval(nullptr);
}

void SampleWindow::resetFPS() {
    fCumulativeFPS_Time = 0;
    fCumulativeFPS_Count = 0;
}

void SampleWindow::toggleDistanceFieldFonts() {
    SkSurfaceProps props = this->getSurfaceProps();
    uint32_t flags = props.flags() ^ SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
    this->setSurfaceProps(SkSurfaceProps(flags, props.pixelGeometry()));

    // reset backend
    fDevManager->tearDownBackend(this);
    fDevManager->setUpBackend(this, fBackendOptions);

    this->updateTitle();
    this->inval(nullptr);
}

void SampleWindow::setPixelGeometry(int pixelGeometryIndex) {
    const SkSurfaceProps& oldProps = this->getSurfaceProps();
    SkSurfaceProps newProps(oldProps.flags(), SkSurfaceProps::kLegacyFontHost_InitType);
    if (pixelGeometryIndex > 0) {
        newProps = SkSurfaceProps(oldProps.flags(),
                                  gPixelGeometryStates[pixelGeometryIndex].pixelGeometry);
    }
    this->setSurfaceProps(newProps);

    // reset backend
    fDevManager->tearDownBackend(this);
    fDevManager->setUpBackend(this, fBackendOptions);

    this->updateTitle();
    this->inval(nullptr);
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

    int dx = 0xFF;
    int dy = 0xFF;

    switch (key) {
        case kRight_SkKey:
            if (this->nextSample()) {
                return true;
            }
            break;
        case kLeft_SkKey:
            if (this->previousSample()) {
                return true;
            }
            return true;
        case kUp_SkKey:
            this->changeZoomLevel(1.f / 32.f);
            return true;
        case kDown_SkKey:
            this->changeZoomLevel(-1.f / 32.f);
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

        case k5_SkKey: dx =  0; dy =  0; break;
        case k8_SkKey: dx =  0; dy = -1; break;
        case k6_SkKey: dx =  1; dy =  0; break;
        case k2_SkKey: dx =  0; dy =  1; break;
        case k4_SkKey: dx = -1; dy =  0; break;
        case k7_SkKey: dx = -1; dy = -1; break;
        case k9_SkKey: dx =  1; dy = -1; break;
        case k3_SkKey: dx =  1; dy =  1; break;
        case k1_SkKey: dx = -1; dy =  1; break;

        default:
            break;
    }

    if (0xFF != dx && 0xFF != dy) {
        this->changeOffset({SkIntToScalar(dx / 32.0f), SkIntToScalar(dy / 32.0f)});
        return true;
    }

    return this->INHERITED::onHandleKey(key);
}

///////////////////////////////////////////////////////////////////////////////

static const char gGestureClickType[] = "GestureClickType";

bool SampleWindow::onDispatchClick(int x, int y, Click::State state,
        void* owner, unsigned modi) {
    if (Click::kMoved_State == state) {
        updatePointer(x, y);
    }
    int w = SkScalarRoundToInt(this->width());
    int h = SkScalarRoundToInt(this->height());

    // check for the resize-box
    if (w - x < 16 && h - y < 16) {
        return false;   // let the OS handle the click
    }
    else if (fMagnify) {
        //it's only necessary to update the drawing if there's a click
        this->inval(nullptr);
        return false; //prevent dragging while magnify is enabled
    } else {
        // capture control+option, and trigger debugger
        if ((modi & kControl_SkModifierKey) && (modi & kOption_SkModifierKey)) {
            if (Click::kDown_State == state) {
                SkEvent evt("debug-hit-test");
                evt.setS32("debug-hit-test-x", x);
                evt.setS32("debug-hit-test-y", y);
                curr_view(this)->doEvent(evt);
            }
            return true;
        } else {
            return this->INHERITED::onDispatchClick(x, y, state, owner, modi);
        }
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

SkView::Click* SampleWindow::onFindClickHandler(SkScalar x, SkScalar y,
                                                unsigned modi) {
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
    fSlideMenu->reset();

    this->onUpdateMenu(fSlideMenu);
    this->updateTitle();
}

static const char* gDeviceTypePrefix[] = {
    "raster: ",
#if SK_SUPPORT_GPU
    "opengl: ",
#if SK_ANGLE
    "angle: ",
#endif // SK_ANGLE
#endif // SK_SUPPORT_GPU
};
static_assert(SK_ARRAY_COUNT(gDeviceTypePrefix) == SampleWindow::kDeviceTypeCnt,
              "array_size_mismatch");

static const char* trystate_str(SkOSMenu::TriState state,
                                const char trueStr[], const char falseStr[]) {
    if (SkOSMenu::kOnState == state) {
        return trueStr;
    } else if (SkOSMenu::kOffState == state) {
        return falseStr;
    }
    return nullptr;
}

bool SampleWindow::getRawTitle(SkString* title) {
    return curr_title(this, title);
}

void SampleWindow::updateTitle() {
    SkString title;
    if (!this->getRawTitle(&title)) {
        title.set("<unknown>");
    }

    title.prepend(gDeviceTypePrefix[fDeviceType]);

    if (gSampleWindow->getTiles()) {
        title.prependf("[T%d/%d] ", gSampleWindow->getTiles(), gSampleWindow->getThreads());
    }

    if (gSkUseAnalyticAA) {
        if (gSkForceAnalyticAA) {
            title.prepend("<FAAA> ");
        } else {
            title.prepend("<AAA> ");
        }
    }
    if (fTilingMode != kNo_Tiling) {
        title.prependf("<T: %s> ", gTilingInfo[fTilingMode].label);
    }
    if (fAnimating) {
        title.prepend("<A> ");
    }
    if (fRotate) {
        title.prepend("<R> ");
    }
    if (fPerspAnim) {
        title.prepend("<K> ");
    }
    if (this->getSurfaceProps().flags() & SkSurfaceProps::kUseDeviceIndependentFonts_Flag) {
        title.prepend("<DIF> ");
    }
    if (fUsePicture) {
        title.prepend("<P> ");
    }
    if (fUseDeferredCanvas) {
        title.prepend("<E> ");
    }

    title.prepend(trystate_str(fLCDState, "LCD ", "lcd "));
    title.prepend(trystate_str(fAAState, "AA ", "aa "));
    title.prepend(gFilterQualityStates[fFilterQualityIndex].fLabel);
    title.prepend(trystate_str(fSubpixelState, "S ", "s "));
    title.prepend(fFlipAxis & kFlipAxis_X ? "X " : nullptr);
    title.prepend(fFlipAxis & kFlipAxis_Y ? "Y " : nullptr);
    title.prepend(gHintingStates[fHintingState].label);
    title.prepend(gPixelGeometryStates[fPixelGeometryIndex].label);

    if (fOffset.fX || fOffset.fY) {
        title.prependf("(%.2f, %.2f) ", SkScalarToFloat(fOffset.fX), SkScalarToFloat(fOffset.fY));
    }
    if (fZoomLevel) {
        title.prependf("{%.2f} ", SkScalarToFloat(fZoomLevel));
    }

    if (fMeasureFPS) {
        title.appendf(" %8.4f ms", fMeasureFPS_Time / (float)FPS_REPEAT_COUNT);
        title.appendf(" -> %4.4f ms", fCumulativeFPS_Time / (float)SkTMax(1, fCumulativeFPS_Count));
    }

#if SK_SUPPORT_GPU
    if (IsGpuDeviceType(fDeviceType) &&
        fDevManager &&
        fDevManager->numColorSamples() > 0) {
        title.appendf(" [MSAA: %d]",
                       fDevManager->numColorSamples());
    }
#endif

    title.appendf(" %s", gConfig[fColorConfigIndex].fName);

    if (fDevManager && fDevManager->getColorBits() > 24) {
        title.appendf(" %d bpc", fDevManager->getColorBits());
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

    if (fTilingMode != kNo_Tiling && SampleView::IsSampleView(view)) {
        ((SampleView*)view)->onTileSizeChanged(this->tileSize());
    }
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

// FIXME: this should be in a header
SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv);
SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    if (false) { // avoid bit rot, suppress warning
        test();
    }
    return new SampleWindow(hwnd, argc, argv, nullptr);
}

// FIXME: this should be in a header
void get_preferred_size(int* x, int* y, int* width, int* height);
void get_preferred_size(int* x, int* y, int* width, int* height) {
    *x = 10;
    *y = 50;
    *width = 640;
    *height = 480;
}

#ifdef SK_BUILD_FOR_IOS
#include "SkApplication.h"
IOS_launch_type set_cmd_line_args(int , char *[], const char* resourceDir) {
    SetResourcePath(resourceDir);
    return kApplication__iOSLaunchType;
}
#endif

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
}

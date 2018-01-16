/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Viewer.h"

#include "GMSlide.h"
#include "ImageSlide.h"
#include "Resources.h"
#include "SampleSlide.h"
#include "SkottieSlide.h"
#include "SKPSlide.h"

#include "GrContext.h"
#include "SkCanvas.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkCommonFlagsGpu.h"
#include "SkEventTracingPriv.h"
#include "SkGraphics.h"
#include "SkImagePriv.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPictureRecorder.h"
#include "SkScan.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"
#include "SkThreadedBMPDevice.h"

#include "imgui.h"

#include "ccpr/GrCoverageCountingPathRenderer.h"

#include <stdlib.h>
#include <map>

using namespace sk_app;

static std::map<GpuPathRenderers, std::string> gPathRendererNames;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new Viewer(argc, argv, platformData);
}

static DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of bench name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

static DEFINE_string(slide, "", "Start on this sample.");
static DEFINE_bool(list, false, "List samples?");

#ifdef SK_VULKAN
#    define BACKENDS_STR "\"sw\", \"gl\", and \"vk\""
#else
#    define BACKENDS_STR "\"sw\" and \"gl\""
#endif

#ifdef SK_BUILD_FOR_ANDROID
static DEFINE_string(skps, "/data/local/tmp/skps", "Directory to read skps from.");
static DEFINE_string(jpgs, "/data/local/tmp/resources", "Directory to read jpgs from.");
static DEFINE_string(jsons, "/data/local/tmp/jsons", "Directory to read (Bodymovin) jsons from.");
#else
static DEFINE_string(skps, "skps", "Directory to read skps from.");
static DEFINE_string(jpgs, "jpgs", "Directory to read jpgs from.");
static DEFINE_string(jsons, "jsons", "Directory to read (Bodymovin) jsons from.");
#endif

static DEFINE_string2(backend, b, "sw", "Backend to use. Allowed values are " BACKENDS_STR ".");

static DEFINE_int32(msaa, 0, "Number of subpixel samples. 0 for no HW antialiasing.");

DECLARE_int32(threads)

const char* kBackendTypeStrings[sk_app::Window::kBackendTypeCount] = {
    "OpenGL",
#if SK_ANGLE && defined(SK_BUILD_FOR_WIN)
    "ANGLE",
#endif
#ifdef SK_VULKAN
    "Vulkan",
#endif
    "Raster"
};

static sk_app::Window::BackendType get_backend_type(const char* str) {
#ifdef SK_VULKAN
    if (0 == strcmp(str, "vk")) {
        return sk_app::Window::kVulkan_BackendType;
    } else
#endif
#if SK_ANGLE && defined(SK_BUILD_FOR_WIN)
    if (0 == strcmp(str, "angle")) {
        return sk_app::Window::kANGLE_BackendType;
    } else
#endif
    if (0 == strcmp(str, "gl")) {
        return sk_app::Window::kNativeGL_BackendType;
    } else if (0 == strcmp(str, "sw")) {
        return sk_app::Window::kRaster_BackendType;
    } else {
        SkDebugf("Unknown backend type, %s, defaulting to sw.", str);
        return sk_app::Window::kRaster_BackendType;
    }
}

static SkColorSpacePrimaries gSrgbPrimaries = {
    0.64f, 0.33f,
    0.30f, 0.60f,
    0.15f, 0.06f,
    0.3127f, 0.3290f };

static SkColorSpacePrimaries gAdobePrimaries = {
    0.64f, 0.33f,
    0.21f, 0.71f,
    0.15f, 0.06f,
    0.3127f, 0.3290f };

static SkColorSpacePrimaries gP3Primaries = {
    0.680f, 0.320f,
    0.265f, 0.690f,
    0.150f, 0.060f,
    0.3127f, 0.3290f };

static SkColorSpacePrimaries gRec2020Primaries = {
    0.708f, 0.292f,
    0.170f, 0.797f,
    0.131f, 0.046f,
    0.3127f, 0.3290f };

struct NamedPrimaries {
    const char* fName;
    SkColorSpacePrimaries* fPrimaries;
} gNamedPrimaries[] = {
    { "sRGB", &gSrgbPrimaries },
    { "AdobeRGB", &gAdobePrimaries },
    { "P3", &gP3Primaries },
    { "Rec. 2020", &gRec2020Primaries },
};

static bool primaries_equal(const SkColorSpacePrimaries& a, const SkColorSpacePrimaries& b) {
    return memcmp(&a, &b, sizeof(SkColorSpacePrimaries)) == 0;
}

static Window::BackendType backend_type_for_window(Window::BackendType backendType) {
    // In raster mode, we still use GL for the window.
    // This lets us render the GUI faster (and correct).
    return Window::kRaster_BackendType == backendType ? Window::kNativeGL_BackendType : backendType;
}

const char* kName = "name";
const char* kValue = "value";
const char* kOptions = "options";
const char* kSlideStateName = "Slide";
const char* kBackendStateName = "Backend";
const char* kMSAAStateName = "MSAA";
const char* kPathRendererStateName = "Path renderer";
const char* kSoftkeyStateName = "Softkey";
const char* kSoftkeyHint = "Please select a softkey";
const char* kFpsStateName = "FPS";
const char* kON = "ON";
const char* kOFF = "OFF";
const char* kRefreshStateName = "Refresh";

Viewer::Viewer(int argc, char** argv, void* platformData)
    : fCurrentSlide(-1)
    , fRefresh(false)
    , fSaveToSKP(false)
    , fShowImGuiDebugWindow(false)
    , fShowSlidePicker(false)
    , fShowImGuiTestWindow(false)
    , fShowZoomWindow(false)
    , fLastImage(nullptr)
    , fBackendType(sk_app::Window::kNativeGL_BackendType)
    , fColorMode(ColorMode::kLegacy)
    , fColorSpacePrimaries(gSrgbPrimaries)
    // Our UI can only tweak gamma (currently), so start out gamma-only
    , fColorSpaceTransferFn(g2Dot2_TransferFn)
    , fZoomLevel(0.0f)
    , fGestureDevice(GestureDevice::kNone)
    , fTileCnt(0)
    , fThreadCnt(0)
{
    SkGraphics::Init();

    gPathRendererNames[GpuPathRenderers::kAll] = "All Path Renderers";
    gPathRendererNames[GpuPathRenderers::kDefault] =
            "Default Ganesh Behavior (best path renderer, not including CCPR)";
    gPathRendererNames[GpuPathRenderers::kStencilAndCover] = "NV_path_rendering";
    gPathRendererNames[GpuPathRenderers::kMSAA] = "Sample shading";
    gPathRendererNames[GpuPathRenderers::kSmall] = "Small paths (cached sdf or alpha masks)";
    gPathRendererNames[GpuPathRenderers::kCoverageCounting] = "Coverage counting";
    gPathRendererNames[GpuPathRenderers::kTessellating] = "Tessellating";
    gPathRendererNames[GpuPathRenderers::kNone] = "Software masks";

    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    SkCommandLineFlags::Parse(argc, argv);
#ifdef SK_BUILD_FOR_ANDROID
    SetResourcePath("/data/local/tmp/resources");
#endif

    initializeEventTracingForTools();
    static SkTaskGroup::Enabler kTaskGroupEnabler(FLAGS_threads);

    fBackendType = get_backend_type(FLAGS_backend[0]);
    fWindow = Window::CreateNativeWindow(platformData);

    DisplayParams displayParams;
    displayParams.fMSAASampleCount = FLAGS_msaa;
    SetCtxOptionsFromCommonFlags(&displayParams.fGrContextOptions);
    fWindow->setRequestedDisplayParams(displayParams);

    // Configure timers
    fStatsLayer.setActive(false);
    fAnimateTimer = fStatsLayer.addTimer("Animate", SK_ColorMAGENTA, 0xffff66ff);
    fPaintTimer = fStatsLayer.addTimer("Paint", SK_ColorGREEN);
    fFlushTimer = fStatsLayer.addTimer("Flush", SK_ColorRED, 0xffff6666);

    // register callbacks
    fCommands.attach(fWindow);
    fWindow->pushLayer(this);
    fWindow->pushLayer(&fStatsLayer);
    fWindow->pushLayer(&fImGuiLayer);

    // add key-bindings
    fCommands.addCommand(' ', "GUI", "Toggle Debug GUI", [this]() {
        this->fShowImGuiDebugWindow = !this->fShowImGuiDebugWindow;
        fWindow->inval();
    });
    // Command to jump directly to the slide picker and give it focus
    fCommands.addCommand('/', "GUI", "Jump to slide picker", [this]() {
        this->fShowImGuiDebugWindow = true;
        this->fShowSlidePicker = true;
        fWindow->inval();
    });
    // Alias that to Backspace, to match SampleApp
    fCommands.addCommand(Window::Key::kBack, "Backspace", "GUI", "Jump to slide picker", [this]() {
        this->fShowImGuiDebugWindow = true;
        this->fShowSlidePicker = true;
        fWindow->inval();
    });
    fCommands.addCommand('g', "GUI", "Toggle GUI Demo", [this]() {
        this->fShowImGuiTestWindow = !this->fShowImGuiTestWindow;
        fWindow->inval();
    });
    fCommands.addCommand('z', "GUI", "Toggle zoom window", [this]() {
        this->fShowZoomWindow = !this->fShowZoomWindow;
        fWindow->inval();
    });
    fCommands.addCommand('s', "Overlays", "Toggle stats display", [this]() {
        fStatsLayer.setActive(!fStatsLayer.getActive());
        fWindow->inval();
    });
    fCommands.addCommand('0', "Overlays", "Reset stats", [this]() {
        fStatsLayer.resetMeasurements();
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('c', "Modes", "Cycle color mode", [this]() {
        switch (fColorMode) {
            case ColorMode::kLegacy:
                this->setColorMode(ColorMode::kColorManagedSRGB8888_NonLinearBlending);
                break;
            case ColorMode::kColorManagedSRGB8888_NonLinearBlending:
                this->setColorMode(ColorMode::kColorManagedSRGB8888);
                break;
            case ColorMode::kColorManagedSRGB8888:
                this->setColorMode(ColorMode::kColorManagedLinearF16);
                break;
            case ColorMode::kColorManagedLinearF16:
                this->setColorMode(ColorMode::kLegacy);
                break;
        }
    });
    fCommands.addCommand(Window::Key::kRight, "Right", "Navigation", "Next slide", [this]() {
        this->setCurrentSlide(fCurrentSlide < fSlides.count() - 1 ? fCurrentSlide + 1 : 0);
    });
    fCommands.addCommand(Window::Key::kLeft, "Left", "Navigation", "Previous slide", [this]() {
        this->setCurrentSlide(fCurrentSlide > 0 ? fCurrentSlide - 1 : fSlides.count() - 1);
    });
    fCommands.addCommand(Window::Key::kUp, "Up", "Transform", "Zoom in", [this]() {
        this->changeZoomLevel(1.f / 32.f);
        fWindow->inval();
    });
    fCommands.addCommand(Window::Key::kDown, "Down", "Transform", "Zoom out", [this]() {
        this->changeZoomLevel(-1.f / 32.f);
        fWindow->inval();
    });
    fCommands.addCommand('d', "Modes", "Change rendering backend", [this]() {
        sk_app::Window::BackendType newBackend = (sk_app::Window::BackendType)(
                (fBackendType + 1) % sk_app::Window::kBackendTypeCount);
        // Switching to and from Vulkan is problematic on Linux so disabled for now
#if defined(SK_BUILD_FOR_UNIX) && defined(SK_VULKAN)
        if (newBackend == sk_app::Window::kVulkan_BackendType) {
            newBackend = (sk_app::Window::BackendType)((newBackend + 1) %
                                                       sk_app::Window::kBackendTypeCount);
        } else if (fBackendType == sk_app::Window::kVulkan_BackendType) {
            newBackend = sk_app::Window::kVulkan_BackendType;
        }
#endif
        this->setBackend(newBackend);
    });

    fCommands.addCommand('A', "AA", "Toggle analytic AA", [this]() {
        if (!gSkUseAnalyticAA) {
            gSkUseAnalyticAA = true;
        } else if (!gSkForceAnalyticAA) {
            gSkForceAnalyticAA = true;
        } else {
            gSkUseAnalyticAA = gSkForceAnalyticAA = false;
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('D', "AA", "Toggle delta AA", [this]() {
        if (!gSkUseDeltaAA) {
            gSkUseDeltaAA = true;
        } else if (!gSkForceDeltaAA) {
            gSkForceDeltaAA = true;
        } else {
            gSkUseDeltaAA = gSkForceDeltaAA = false;
        }
        this->updateTitle();
        fWindow->inval();
    });

    fCommands.addCommand('+', "Threaded Backend", "Increase tile count", [this]() {
        fTileCnt++;
        if (fThreadCnt == 0) {
            this->resetExecutor();
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('-', "Threaded Backend", "Decrease tile count", [this]() {
        fTileCnt = SkTMax(0, fTileCnt - 1);
        if (fThreadCnt == 0) {
            this->resetExecutor();
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('>', "Threaded Backend", "Increase thread count", [this]() {
        if (fTileCnt == 0) {
            return;
        }
        fThreadCnt = (fThreadCnt + 1) % fTileCnt;
        this->resetExecutor();
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('<', "Threaded Backend", "Decrease thread count", [this]() {
        if (fTileCnt == 0) {
            return;
        }
        fThreadCnt = (fThreadCnt + fTileCnt - 1) % fTileCnt;
        this->resetExecutor();
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('K', "IO", "Save slide to SKP", [this]() {
        fSaveToSKP = true;
        fWindow->inval();
    });

    // set up slides
    this->initSlides();
    this->setCurrentSlide(this->startupSlide());
    if (FLAGS_list) {
        this->listNames();
    }

    fAnimTimer.run();

    auto gamutImage = GetResourceAsImage("images/gamut.png");
    if (gamutImage) {
        fImGuiGamutPaint.setShader(gamutImage->makeShader());
    }
    fImGuiGamutPaint.setColor(SK_ColorWHITE);
    fImGuiGamutPaint.setFilterQuality(kLow_SkFilterQuality);

    fWindow->attach(backend_type_for_window(fBackendType));
}

void Viewer::initSlides() {
    fAllSlideNames = Json::Value(Json::arrayValue);

    const skiagm::GMRegistry* gms(skiagm::GMRegistry::Head());
    while (gms) {
        std::unique_ptr<skiagm::GM> gm(gms->factory()(nullptr));

        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, gm->getName())) {
            sk_sp<Slide> slide(new GMSlide(gm.release()));
            fSlides.push_back(slide);
        }

        gms = gms->next();
    }

    // reverse array
    for (int i = 0; i < fSlides.count()/2; ++i) {
        sk_sp<Slide> temp = fSlides[i];
        fSlides[i] = fSlides[fSlides.count() - i - 1];
        fSlides[fSlides.count() - i - 1] = temp;
    }

    // samples
    const SkViewRegister* reg = SkViewRegister::Head();
    while (reg) {
        sk_sp<Slide> slide(new SampleSlide(reg->factory()));
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, slide->getName().c_str())) {
            fSlides.push_back(slide);
        }
        reg = reg->next();
    }

    // SKPs
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        if (SkStrEndsWith(FLAGS_skps[i], ".skp")) {
            if (SkCommandLineFlags::ShouldSkip(FLAGS_match, FLAGS_skps[i])) {
                continue;
            }

            SkString path(FLAGS_skps[i]);
            sk_sp<SKPSlide> slide(new SKPSlide(SkOSPath::Basename(path.c_str()), path));
            if (slide) {
                fSlides.push_back(slide);
            }
        } else {
            SkOSFile::Iter it(FLAGS_skps[i], ".skp");
            SkString skpName;
            while (it.next(&skpName)) {
                if (SkCommandLineFlags::ShouldSkip(FLAGS_match, skpName.c_str())) {
                    continue;
                }

                SkString path = SkOSPath::Join(FLAGS_skps[i], skpName.c_str());
                sk_sp<SKPSlide> slide(new SKPSlide(skpName, path));
                if (slide) {
                    fSlides.push_back(slide);
                }
            }
        }
    }

    // JPGs
    for (int i = 0; i < FLAGS_jpgs.count(); i++) {
        SkOSFile::Iter it(FLAGS_jpgs[i], ".jpg");
        SkString jpgName;
        while (it.next(&jpgName)) {
            if (SkCommandLineFlags::ShouldSkip(FLAGS_match, jpgName.c_str())) {
                continue;
            }

            SkString path = SkOSPath::Join(FLAGS_jpgs[i], jpgName.c_str());
            sk_sp<ImageSlide> slide(new ImageSlide(jpgName, path));
            if (slide) {
                fSlides.push_back(slide);
            }
        }
    }

    // JSONs
    for (const auto& json : FLAGS_jsons) {
        fSlides.push_back(sk_make_sp<SkottieSlide2>(json));

        SkOSFile::Iter it(json.c_str(), ".json");
        SkString jsonName;
        while (it.next(&jsonName)) {
            if (SkCommandLineFlags::ShouldSkip(FLAGS_match, jsonName.c_str())) {
                continue;
            }
            fSlides.push_back(sk_make_sp<SkottieSlide>(jsonName, SkOSPath::Join(json.c_str(),
                                                                                jsonName.c_str())));
        }
    }
}


Viewer::~Viewer() {
    fWindow->detach();
    delete fWindow;
}

void Viewer::updateTitle() {
    if (!fWindow) {
        return;
    }
    if (fWindow->sampleCount() < 0) {
        return; // Surface hasn't been created yet.
    }

    SkString title("Viewer: ");
    title.append(fSlides[fCurrentSlide]->getName());

    if (gSkUseDeltaAA) {
        if (gSkForceDeltaAA) {
            title.append(" <FDAA>");
        } else {
            title.append(" <DAA>");
        }
    } else if (gSkUseAnalyticAA) {
        if (gSkForceAnalyticAA) {
            title.append(" <FAAA>");
        } else {
            title.append(" <AAA>");
        }
    }

    if (fTileCnt > 0) {
        title.appendf(" T%d", fTileCnt);
        if (fThreadCnt > 0) {
            title.appendf("/%d", fThreadCnt);
        }
    }

    switch (fColorMode) {
        case ColorMode::kLegacy:
            title.append(" Legacy 8888");
            break;
        case ColorMode::kColorManagedSRGB8888_NonLinearBlending:
            title.append(" ColorManaged 8888 (Nonlinear blending)");
            break;
        case ColorMode::kColorManagedSRGB8888:
            title.append(" ColorManaged 8888");
            break;
        case ColorMode::kColorManagedLinearF16:
            title.append(" ColorManaged F16");
            break;
    }

    if (ColorMode::kLegacy != fColorMode) {
        int curPrimaries = -1;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gNamedPrimaries); ++i) {
            if (primaries_equal(*gNamedPrimaries[i].fPrimaries, fColorSpacePrimaries)) {
                curPrimaries = i;
                break;
            }
        }
        title.appendf(" %s", curPrimaries >= 0 ? gNamedPrimaries[curPrimaries].fName : "Custom");

        if (ColorMode::kColorManagedSRGB8888_NonLinearBlending == fColorMode) {
            title.appendf(" Gamma %f", fColorSpaceTransferFn.fG);
        }
    }

    title.append(" [");
    title.append(kBackendTypeStrings[fBackendType]);
    if (int msaa = fWindow->sampleCount()) {
        title.appendf(" MSAA: %i", msaa);
    }
    title.append("]");

    GpuPathRenderers pr = fWindow->getRequestedDisplayParams().fGrContextOptions.fGpuPathRenderers;
    if (GpuPathRenderers::kDefault != pr) {
        title.appendf(" [Path renderer: %s]", gPathRendererNames[pr].c_str());
    }

    fWindow->setTitle(title.c_str());
}

int Viewer::startupSlide() const {

    if (!FLAGS_slide.isEmpty()) {
        int count = fSlides.count();
        for (int i = 0; i < count; i++) {
            if (fSlides[i]->getName().equals(FLAGS_slide[0])) {
                return i;
            }
        }

        fprintf(stderr, "Unknown slide \"%s\"\n", FLAGS_slide[0]);
        this->listNames();
    }

    return 0;
}

void Viewer::listNames() const {
    SkDebugf("All Slides:\n");
    for (const auto& slide : fSlides) {
        SkDebugf("    %s\n", slide->getName().c_str());
    }
}

void Viewer::setCurrentSlide(int slide) {
    SkASSERT(slide >= 0 && slide < fSlides.count());

    if (slide == fCurrentSlide) {
        return;
    }

    if (fCurrentSlide >= 0) {
        fSlides[fCurrentSlide]->unload();
    }

    fSlides[slide]->load(SkIntToScalar(fWindow->width()),
                         SkIntToScalar(fWindow->height()));
    fCurrentSlide = slide;
    this->setupCurrentSlide();
}

void Viewer::setupCurrentSlide() {
    // prepare dimensions for image slides
    fGesture.resetTouchState();
    fDefaultMatrix.reset();

    const SkISize slideSize = fSlides[fCurrentSlide]->getDimensions();
    const SkRect slideBounds = SkRect::MakeIWH(slideSize.width(), slideSize.height());
    const SkRect windowRect = SkRect::MakeIWH(fWindow->width(), fWindow->height());

    // Start with a matrix that scales the slide to the available screen space
    if (fWindow->scaleContentToFit()) {
        if (windowRect.width() > 0 && windowRect.height() > 0) {
            fDefaultMatrix.setRectToRect(slideBounds, windowRect, SkMatrix::kStart_ScaleToFit);
        }
    }

    // Prevent the user from dragging content so far outside the window they can't find it again
    fGesture.setTransLimit(slideBounds, windowRect, fDefaultMatrix);

    this->updateTitle();
    this->updateUIState();

    fStatsLayer.resetMeasurements();

    fWindow->inval();
}

#define MAX_ZOOM_LEVEL  8
#define MIN_ZOOM_LEVEL  -8

void Viewer::changeZoomLevel(float delta) {
    fZoomLevel += delta;
    fZoomLevel = SkScalarPin(fZoomLevel, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);
}

SkMatrix Viewer::computeMatrix() {
    SkMatrix m;

    SkScalar zoomScale = (fZoomLevel < 0) ? SK_Scalar1 / (SK_Scalar1 - fZoomLevel)
                                          : SK_Scalar1 + fZoomLevel;
    m = fGesture.localM();
    m.preConcat(fGesture.globalM());
    m.preConcat(fDefaultMatrix);
    m.preScale(zoomScale, zoomScale);

    return m;
}

void Viewer::setBackend(sk_app::Window::BackendType backendType) {
    fBackendType = backendType;

    fWindow->detach();

#if defined(SK_BUILD_FOR_WIN)
    // Switching between OpenGL, Vulkan, and ANGLE in the same window is problematic at this point
    // on Windows, so we just delete the window and recreate it.
    DisplayParams params = fWindow->getRequestedDisplayParams();
    delete fWindow;
    fWindow = Window::CreateNativeWindow(nullptr);

    // re-register callbacks
    fCommands.attach(fWindow);
    fWindow->pushLayer(this);
    fWindow->pushLayer(&fStatsLayer);
    fWindow->pushLayer(&fImGuiLayer);

    // Don't allow the window to re-attach. If we're in MSAA mode, the params we grabbed above
    // will still include our correct sample count. But the re-created fWindow will lose that
    // information. On Windows, we need to re-create the window when changing sample count,
    // so we'll incorrectly detect that situation, then re-initialize the window in GL mode,
    // rendering this tear-down step pointless (and causing the Vulkan window context to fail
    // as if we had never changed windows at all).
    fWindow->setRequestedDisplayParams(params, false);
#endif

    fWindow->attach(backend_type_for_window(fBackendType));
}

void Viewer::setColorMode(ColorMode colorMode) {
    fColorMode = colorMode;

    // When we're in color managed mode, we tag our window surface as sRGB. If we've switched into
    // or out of legacy/nonlinear mode, we need to update our window configuration.
    DisplayParams params = fWindow->getRequestedDisplayParams();
    bool wasInLegacy = !SkToBool(params.fColorSpace);
    bool wantLegacy = (ColorMode::kLegacy == fColorMode) ||
                      (ColorMode::kColorManagedSRGB8888_NonLinearBlending == fColorMode);
    if (wasInLegacy != wantLegacy) {
        params.fColorSpace = wantLegacy ? nullptr : SkColorSpace::MakeSRGB();
        fWindow->setRequestedDisplayParams(params);
    }

    this->updateTitle();
    fWindow->inval();
}

void Viewer::drawSlide(SkCanvas* canvas) {
    SkAutoCanvasRestore autorestore(canvas, false);

    // By default, we render directly into the window's surface/canvas
    SkCanvas* slideCanvas = canvas;
    fLastImage.reset();

    // If we're in any of the color managed modes, construct the color space we're going to use
    sk_sp<SkColorSpace> cs = nullptr;
    if (ColorMode::kLegacy != fColorMode) {
        auto transferFn = (ColorMode::kColorManagedLinearF16 == fColorMode)
            ? SkColorSpace::kLinear_RenderTargetGamma : SkColorSpace::kSRGB_RenderTargetGamma;
        SkMatrix44 toXYZ(SkMatrix44::kIdentity_Constructor);
        SkAssertResult(fColorSpacePrimaries.toXYZD50(&toXYZ));
        if (ColorMode::kColorManagedSRGB8888_NonLinearBlending == fColorMode) {
            cs = SkColorSpace::MakeRGB(fColorSpaceTransferFn, toXYZ);
        } else {
            cs = SkColorSpace::MakeRGB(transferFn, toXYZ);
        }
    }

    if (fSaveToSKP) {
        SkPictureRecorder recorder;
        SkCanvas* recorderCanvas = recorder.beginRecording(
                SkRect::Make(fSlides[fCurrentSlide]->getDimensions()));
        // In xform-canvas mode, record the transformed output
        std::unique_ptr<SkCanvas> xformCanvas = nullptr;
        if (ColorMode::kColorManagedSRGB8888_NonLinearBlending == fColorMode) {
            xformCanvas = SkCreateColorSpaceXformCanvas(recorderCanvas, cs);
            recorderCanvas = xformCanvas.get();
        }
        fSlides[fCurrentSlide]->draw(recorderCanvas);
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
        SkFILEWStream stream("sample_app.skp");
        picture->serialize(&stream);
        fSaveToSKP = false;
    }

    // If we're in F16, or we're zooming, or we're in color correct 8888 and the gamut isn't sRGB,
    // we need to render offscreen. We also need to render offscreen if we're in any raster mode,
    // because the window surface is actually GL.
    sk_sp<SkSurface> offscreenSurface = nullptr;
    std::unique_ptr<SkThreadedBMPDevice> threadedDevice;
    std::unique_ptr<SkCanvas> threadedCanvas;
    if (Window::kRaster_BackendType == fBackendType ||
        ColorMode::kColorManagedLinearF16 == fColorMode ||
        fShowZoomWindow ||
        (ColorMode::kColorManagedSRGB8888 == fColorMode &&
         !primaries_equal(fColorSpacePrimaries, gSrgbPrimaries))) {

        SkColorType colorType = (ColorMode::kColorManagedLinearF16 == fColorMode)
            ? kRGBA_F16_SkColorType : kN32_SkColorType;
        // In nonlinear blending mode, we actually use a legacy off-screen canvas, and wrap it
        // with a special canvas (below) that has the color space attached
        sk_sp<SkColorSpace> offscreenColorSpace =
            (ColorMode::kColorManagedSRGB8888_NonLinearBlending == fColorMode) ? nullptr : cs;
        SkImageInfo info = SkImageInfo::Make(fWindow->width(), fWindow->height(), colorType,
                                             kPremul_SkAlphaType, std::move(offscreenColorSpace));
        offscreenSurface = Window::kRaster_BackendType == fBackendType ? SkSurface::MakeRaster(info)
                                                                       : canvas->makeSurface(info);
        SkPixmap offscreenPixmap;
        if (fTileCnt > 0 && offscreenSurface->peekPixels(&offscreenPixmap)) {
            SkBitmap offscreenBitmap;
            offscreenBitmap.installPixels(offscreenPixmap);
            threadedDevice.reset(new SkThreadedBMPDevice(offscreenBitmap, fTileCnt,
                                                         fThreadCnt, fExecutor.get()));
            threadedCanvas.reset(new SkCanvas(threadedDevice.get()));
            slideCanvas = threadedCanvas.get();
        } else {
            slideCanvas = offscreenSurface->getCanvas();
        }
    }

    std::unique_ptr<SkCanvas> xformCanvas = nullptr;
    if (ColorMode::kColorManagedSRGB8888_NonLinearBlending == fColorMode) {
        xformCanvas = SkCreateColorSpaceXformCanvas(slideCanvas, cs);
        slideCanvas = xformCanvas.get();
    }

    int count = slideCanvas->save();
    slideCanvas->clear(SK_ColorWHITE);
    slideCanvas->concat(computeMatrix());
    // Time the painting logic of the slide
    fStatsLayer.beginTiming(fPaintTimer);
    fSlides[fCurrentSlide]->draw(slideCanvas);
    fStatsLayer.endTiming(fPaintTimer);
    slideCanvas->restoreToCount(count);

    // Force a flush so we can time that, too
    fStatsLayer.beginTiming(fFlushTimer);
    slideCanvas->flush();
    fStatsLayer.endTiming(fFlushTimer);

    // If we rendered offscreen, snap an image and push the results to the window's canvas
    if (offscreenSurface) {
        fLastImage = offscreenSurface->makeImageSnapshot();

        // Tag the image with the sRGB gamut, so no further color space conversion happens
        sk_sp<SkColorSpace> srgb = (ColorMode::kColorManagedLinearF16 == fColorMode)
            ? SkColorSpace::MakeSRGBLinear() : SkColorSpace::MakeSRGB();
        auto retaggedImage = SkImageMakeRasterCopyAndAssignColorSpace(fLastImage.get(), srgb.get());
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        canvas->drawImage(retaggedImage, 0, 0, &paint);
    }
}

void Viewer::onBackendCreated() {
    this->updateTitle();
    this->updateUIState();
    this->setupCurrentSlide();
    fStatsLayer.resetMeasurements();
    fWindow->show();
    fWindow->inval();
}

void Viewer::onPaint(SkCanvas* canvas) {
    this->drawSlide(canvas);

    fCommands.drawHelp(canvas);

    this->drawImGui();

    // Update the FPS
    this->updateUIState();
}

bool Viewer::onTouch(intptr_t owner, Window::InputState state, float x, float y) {
    if (GestureDevice::kMouse == fGestureDevice) {
        return false;
    }
    void* castedOwner = reinterpret_cast<void*>(owner);
    switch (state) {
        case Window::kUp_InputState: {
            fGesture.touchEnd(castedOwner);
            break;
        }
        case Window::kDown_InputState: {
            fGesture.touchBegin(castedOwner, x, y);
            break;
        }
        case Window::kMove_InputState: {
            fGesture.touchMoved(castedOwner, x, y);
            break;
        }
    }
    fGestureDevice = fGesture.isBeingTouched() ? GestureDevice::kTouch : GestureDevice::kNone;
    fWindow->inval();
    return true;
}

bool Viewer::onMouse(int x, int y, Window::InputState state, uint32_t modifiers) {
    if (GestureDevice::kTouch == fGestureDevice) {
        return false;
    }

    if (fSlides[fCurrentSlide]->onMouse(x, y, state, modifiers)) {
        fWindow->inval();
        return true;
    }

    switch (state) {
        case Window::kUp_InputState: {
            fGesture.touchEnd(nullptr);
            break;
        }
        case Window::kDown_InputState: {
            fGesture.touchBegin(nullptr, x, y);
            break;
        }
        case Window::kMove_InputState: {
            fGesture.touchMoved(nullptr, x, y);
            break;
        }
    }
    fGestureDevice = fGesture.isBeingTouched() ? GestureDevice::kMouse : GestureDevice::kNone;

    if (state != Window::kMove_InputState || fGesture.isBeingTouched()) {
        fWindow->inval();
    }
    return true;
}

static ImVec2 ImGui_DragPrimary(const char* label, float* x, float* y,
                                const ImVec2& pos, const ImVec2& size) {
    // Transform primaries ([0, 0] - [0.8, 0.9]) to screen coords (including Y-flip)
    ImVec2 center(pos.x + (*x / 0.8f) * size.x, pos.y + (1.0f - (*y / 0.9f)) * size.y);

    // Invisible 10x10 button
    ImGui::SetCursorScreenPos(ImVec2(center.x - 5, center.y - 5));
    ImGui::InvisibleButton(label, ImVec2(10, 10));

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging()) {
        ImGuiIO& io = ImGui::GetIO();
        // Normalized mouse position, relative to our gamut box
        ImVec2 mousePosXY((io.MousePos.x - pos.x) / size.x, (io.MousePos.y - pos.y) / size.y);
        // Clamp to edge of box, convert back to primary scale
        *x = SkTPin(mousePosXY.x, 0.0f, 1.0f) * 0.8f;
        *y = SkTPin(1 - mousePosXY.y, 0.0f, 1.0f) * 0.9f;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("x: %.3f\ny: %.3f", *x, *y);
    }

    // Return screen coordinates for the caller. We could just return center here, but we'd have
    // one frame of lag during drag.
    return ImVec2(pos.x + (*x / 0.8f) * size.x, pos.y + (1.0f - (*y / 0.9f)) * size.y);
}

static void ImGui_Primaries(SkColorSpacePrimaries* primaries, SkPaint* gamutPaint) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // The gamut image covers a (0.8 x 0.9) shaped region, so fit our image/canvas to the available
    // width, and scale the height to maintain aspect ratio.
    float canvasWidth = SkTMax(ImGui::GetContentRegionAvailWidth(), 50.0f);
    ImVec2 size = ImVec2(canvasWidth, canvasWidth * (0.9f / 0.8f));
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Background image. Only draw a subset of the image, to avoid the regions less than zero.
    // Simplifes re-mapping math, clipping behavior, and increases resolution in the useful area.
    // Magic numbers are pixel locations of the origin and upper-right corner.
    drawList->AddImage(gamutPaint, pos, ImVec2(pos.x + size.x, pos.y + size.y),
                       ImVec2(242, 61), ImVec2(1897, 1922));
    ImVec2 endPos = ImGui::GetCursorPos();

    // Primary markers
    ImVec2 r = ImGui_DragPrimary("R", &primaries->fRX, &primaries->fRY, pos, size);
    ImVec2 g = ImGui_DragPrimary("G", &primaries->fGX, &primaries->fGY, pos, size);
    ImVec2 b = ImGui_DragPrimary("B", &primaries->fBX, &primaries->fBY, pos, size);
    ImVec2 w = ImGui_DragPrimary("W", &primaries->fWX, &primaries->fWY, pos, size);

    // Gamut triangle
    drawList->AddCircle(r, 5.0f, 0xFF000040);
    drawList->AddCircle(g, 5.0f, 0xFF004000);
    drawList->AddCircle(b, 5.0f, 0xFF400000);
    drawList->AddCircle(w, 5.0f, 0xFFFFFFFF);
    drawList->AddTriangle(r, g, b, 0xFFFFFFFF);

    // Re-position cursor immediate after the diagram for subsequent controls
    ImGui::SetCursorPos(endPos);
}

void Viewer::drawImGui() {
    // Support drawing the ImGui demo window. Superfluous, but gives a good idea of what's possible
    if (fShowImGuiTestWindow) {
        ImGui::ShowTestWindow(&fShowImGuiTestWindow);
    }

    if (fShowImGuiDebugWindow) {
        // We have some dynamic content that sizes to fill available size. If the scroll bar isn't
        // always visible, we can end up in a layout feedback loop.
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiSetCond_FirstUseEver);
        DisplayParams params = fWindow->getRequestedDisplayParams();
        bool paramsChanged = false;
        if (ImGui::Begin("Tools", &fShowImGuiDebugWindow,
                         ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            if (ImGui::CollapsingHeader("Backend")) {
                int newBackend = static_cast<int>(fBackendType);
                ImGui::RadioButton("Raster", &newBackend, sk_app::Window::kRaster_BackendType);
                ImGui::SameLine();
                ImGui::RadioButton("OpenGL", &newBackend, sk_app::Window::kNativeGL_BackendType);
#if SK_ANGLE && defined(SK_BUILD_FOR_WIN)
                ImGui::SameLine();
                ImGui::RadioButton("ANGLE", &newBackend, sk_app::Window::kANGLE_BackendType);
#endif
#if defined(SK_VULKAN)
                ImGui::SameLine();
                ImGui::RadioButton("Vulkan", &newBackend, sk_app::Window::kVulkan_BackendType);
#endif
                if (newBackend != fBackendType) {
                    fDeferredActions.push_back([=]() {
                        this->setBackend(static_cast<sk_app::Window::BackendType>(newBackend));
                    });
                }

                const GrContext* ctx = fWindow->getGrContext();
                bool* wire = &params.fGrContextOptions.fWireframeMode;
                if (ctx && ImGui::Checkbox("Wireframe Mode", wire)) {
                    paramsChanged = true;
                }

                if (ctx) {
                    int sampleCount = fWindow->sampleCount();
                    ImGui::Text("MSAA: "); ImGui::SameLine();
                    ImGui::RadioButton("0", &sampleCount, 0); ImGui::SameLine();
                    ImGui::RadioButton("4", &sampleCount, 4); ImGui::SameLine();
                    ImGui::RadioButton("8", &sampleCount, 8); ImGui::SameLine();
                    ImGui::RadioButton("16", &sampleCount, 16);

                    if (sampleCount != params.fMSAASampleCount) {
                        params.fMSAASampleCount = sampleCount;
                        paramsChanged = true;
                    }
                }

                if (ImGui::TreeNode("Path Renderers")) {
                    GpuPathRenderers prevPr = params.fGrContextOptions.fGpuPathRenderers;
                    auto prButton = [&](GpuPathRenderers x) {
                        if (ImGui::RadioButton(gPathRendererNames[x].c_str(), prevPr == x)) {
                            if (x != params.fGrContextOptions.fGpuPathRenderers) {
                                params.fGrContextOptions.fGpuPathRenderers = x;
                                paramsChanged = true;
                            }
                        }
                    };

                    if (!ctx) {
                        ImGui::RadioButton("Software", true);
                    } else if (fWindow->sampleCount()) {
                        prButton(GpuPathRenderers::kDefault);
                        prButton(GpuPathRenderers::kAll);
                        if (ctx->caps()->shaderCaps()->pathRenderingSupport()) {
                            prButton(GpuPathRenderers::kStencilAndCover);
                        }
                        if (ctx->caps()->sampleShadingSupport()) {
                            prButton(GpuPathRenderers::kMSAA);
                        }
                        prButton(GpuPathRenderers::kTessellating);
                        prButton(GpuPathRenderers::kNone);
                    } else {
                        prButton(GpuPathRenderers::kDefault);
                        prButton(GpuPathRenderers::kAll);
                        if (GrCoverageCountingPathRenderer::IsSupported(*ctx->caps())) {
                            prButton(GpuPathRenderers::kCoverageCounting);
                        }
                        prButton(GpuPathRenderers::kSmall);
                        prButton(GpuPathRenderers::kTessellating);
                        prButton(GpuPathRenderers::kNone);
                    }
                    ImGui::TreePop();
                }
            }

            if (fShowSlidePicker) {
                ImGui::SetNextTreeNodeOpen(true);
            }

            if (ImGui::CollapsingHeader("Slide")) {
                static ImGuiTextFilter filter;
                static ImVector<const char*> filteredSlideNames;
                static ImVector<int> filteredSlideIndices;

                if (fShowSlidePicker) {
                    ImGui::SetKeyboardFocusHere();
                    fShowSlidePicker = false;
                }

                filter.Draw();
                filteredSlideNames.clear();
                filteredSlideIndices.clear();
                int filteredIndex = 0;
                for (int i = 0; i < fSlides.count(); ++i) {
                    const char* slideName = fSlides[i]->getName().c_str();
                    if (filter.PassFilter(slideName) || i == fCurrentSlide) {
                        if (i == fCurrentSlide) {
                            filteredIndex = filteredSlideIndices.size();
                        }
                        filteredSlideNames.push_back(slideName);
                        filteredSlideIndices.push_back(i);
                    }
                }

                if (ImGui::ListBox("", &filteredIndex, filteredSlideNames.begin(),
                                   filteredSlideNames.size(), 20)) {
                    this->setCurrentSlide(filteredSlideIndices[filteredIndex]);
                }
            }

            if (ImGui::CollapsingHeader("Color Mode")) {
                ColorMode newMode = fColorMode;
                auto cmButton = [&](ColorMode mode, const char* label) {
                    if (ImGui::RadioButton(label, mode == fColorMode)) {
                        newMode = mode;
                    }
                };

                cmButton(ColorMode::kLegacy, "Legacy 8888");
                cmButton(ColorMode::kColorManagedSRGB8888_NonLinearBlending,
                         "Color Managed 8888 (Nonlinear blending)");
                cmButton(ColorMode::kColorManagedSRGB8888, "Color Managed 8888");
                cmButton(ColorMode::kColorManagedLinearF16, "Color Managed F16");

                if (newMode != fColorMode) {
                    // It isn't safe to switch color mode now (in the middle of painting). We might
                    // tear down the back-end, etc... Defer this change until the next onIdle.
                    fDeferredActions.push_back([=]() {
                        this->setColorMode(newMode);
                    });
                }

                // Pick from common gamuts:
                int primariesIdx = 4; // Default: Custom
                for (size_t i = 0; i < SK_ARRAY_COUNT(gNamedPrimaries); ++i) {
                    if (primaries_equal(*gNamedPrimaries[i].fPrimaries, fColorSpacePrimaries)) {
                        primariesIdx = i;
                        break;
                    }
                }

                // When we're in xform canvas mode, we can alter the transfer function, too
                if (ColorMode::kColorManagedSRGB8888_NonLinearBlending == fColorMode) {
                    ImGui::SliderFloat("Gamma", &fColorSpaceTransferFn.fG, 0.5f, 3.5f);
                }

                if (ImGui::Combo("Primaries", &primariesIdx,
                                 "sRGB\0AdobeRGB\0P3\0Rec. 2020\0Custom\0\0")) {
                    if (primariesIdx >= 0 && primariesIdx <= 3) {
                        fColorSpacePrimaries = *gNamedPrimaries[primariesIdx].fPrimaries;
                    }
                }

                // Allow direct editing of gamut
                ImGui_Primaries(&fColorSpacePrimaries, &fImGuiGamutPaint);
            }
        }
        if (paramsChanged) {
            fDeferredActions.push_back([=]() {
                fWindow->setRequestedDisplayParams(params);
                fWindow->inval();
                this->updateTitle();
            });
        }
        ImGui::End();
    }

    if (fShowZoomWindow && fLastImage) {
        if (ImGui::Begin("Zoom", &fShowZoomWindow, ImVec2(200, 200))) {
            static int zoomFactor = 8;
            if (ImGui::Button("<<")) {
                zoomFactor = SkTMax(zoomFactor / 2, 4);
            }
            ImGui::SameLine(); ImGui::Text("%2d", zoomFactor); ImGui::SameLine();
            if (ImGui::Button(">>")) {
                zoomFactor = SkTMin(zoomFactor * 2, 32);
            }

            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 avail = ImGui::GetContentRegionAvail();

            uint32_t pixel = 0;
            SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
            if (fLastImage->readPixels(info, &pixel, info.minRowBytes(), mousePos.x, mousePos.y)) {
                ImGui::SameLine();
                ImGui::Text("(X, Y): %d, %d RGBA: %x %x %x %x",
                            sk_float_round2int(mousePos.x), sk_float_round2int(mousePos.y),
                            SkGetPackedR32(pixel), SkGetPackedG32(pixel),
                            SkGetPackedB32(pixel), SkGetPackedA32(pixel));
            }

            fImGuiLayer.skiaWidget(avail, [=](SkCanvas* c) {
                // Translate so the region of the image that's under the mouse cursor is centered
                // in the zoom canvas:
                c->scale(zoomFactor, zoomFactor);
                c->translate(avail.x * 0.5f / zoomFactor - mousePos.x - 0.5f,
                             avail.y * 0.5f / zoomFactor - mousePos.y - 0.5f);
                c->drawImage(this->fLastImage, 0, 0);

                SkPaint outline;
                outline.setStyle(SkPaint::kStroke_Style);
                c->drawRect(SkRect::MakeXYWH(mousePos.x, mousePos.y, 1, 1), outline);
            });
        }

        ImGui::End();
    }
}

void Viewer::onIdle() {
    for (int i = 0; i < fDeferredActions.count(); ++i) {
        fDeferredActions[i]();
    }
    fDeferredActions.reset();

    fStatsLayer.beginTiming(fAnimateTimer);
    fAnimTimer.updateTime();
    bool animateWantsInval = fSlides[fCurrentSlide]->animate(fAnimTimer);
    fStatsLayer.endTiming(fAnimateTimer);

    ImGuiIO& io = ImGui::GetIO();
    if (animateWantsInval || fStatsLayer.getActive() || fRefresh || io.MetricsActiveWindows) {
        fWindow->inval();
    }
}

void Viewer::updateUIState() {
    if (!fWindow) {
        return;
    }
    if (fWindow->sampleCount() < 0) {
        return; // Surface hasn't been created yet.
    }

    // Slide state
    Json::Value slideState(Json::objectValue);
    slideState[kName] = kSlideStateName;
    slideState[kValue] = fSlides[fCurrentSlide]->getName().c_str();
    if (fAllSlideNames.size() == 0) {
        for(auto slide : fSlides) {
            fAllSlideNames.append(Json::Value(slide->getName().c_str()));
        }
    }
    slideState[kOptions] = fAllSlideNames;

    // Backend state
    Json::Value backendState(Json::objectValue);
    backendState[kName] = kBackendStateName;
    backendState[kValue] = kBackendTypeStrings[fBackendType];
    backendState[kOptions] = Json::Value(Json::arrayValue);
    for (auto str : kBackendTypeStrings) {
        backendState[kOptions].append(Json::Value(str));
    }

    // MSAA state
    Json::Value msaaState(Json::objectValue);
    msaaState[kName] = kMSAAStateName;
    msaaState[kValue] = fWindow->sampleCount();
    msaaState[kOptions] = Json::Value(Json::arrayValue);
    if (sk_app::Window::kRaster_BackendType == fBackendType) {
        msaaState[kOptions].append(Json::Value(0));
    } else {
        for (int msaa : {0, 4, 8, 16}) {
            msaaState[kOptions].append(Json::Value(msaa));
        }
    }

    // Path renderer state
    GpuPathRenderers pr = fWindow->getRequestedDisplayParams().fGrContextOptions.fGpuPathRenderers;
    Json::Value prState(Json::objectValue);
    prState[kName] = kPathRendererStateName;
    prState[kValue] = gPathRendererNames[pr];
    prState[kOptions] = Json::Value(Json::arrayValue);
    const GrContext* ctx = fWindow->getGrContext();
    if (!ctx) {
        prState[kOptions].append("Software");
    } else if (fWindow->sampleCount()) {
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kDefault]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kAll]);
        if (ctx->caps()->shaderCaps()->pathRenderingSupport()) {
            prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kStencilAndCover]);
        }
        if (ctx->caps()->sampleShadingSupport()) {
            prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kMSAA]);
        }
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kTessellating]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kNone]);
    } else {
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kDefault]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kAll]);
        if (GrCoverageCountingPathRenderer::IsSupported(*ctx->caps())) {
            prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kCoverageCounting]);
        }
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kSmall]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kTessellating]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kNone]);
    }

    // Softkey state
    Json::Value softkeyState(Json::objectValue);
    softkeyState[kName] = kSoftkeyStateName;
    softkeyState[kValue] = kSoftkeyHint;
    softkeyState[kOptions] = Json::Value(Json::arrayValue);
    softkeyState[kOptions].append(kSoftkeyHint);
    for (const auto& softkey : fCommands.getCommandsAsSoftkeys()) {
        softkeyState[kOptions].append(Json::Value(softkey.c_str()));
    }

    // FPS state
    Json::Value fpsState(Json::objectValue);
    fpsState[kName] = kFpsStateName;
    double animTime = fStatsLayer.getLastTime(fAnimateTimer);
    double paintTime = fStatsLayer.getLastTime(fPaintTimer);
    double flushTime = fStatsLayer.getLastTime(fFlushTimer);
    fpsState[kValue] = SkStringPrintf("%8.3lf ms\n\nA %8.3lf\nP %8.3lf\nF%8.3lf",
                                      animTime + paintTime + flushTime,
                                      animTime, paintTime, flushTime).c_str();
    fpsState[kOptions] = Json::Value(Json::arrayValue);

    Json::Value state(Json::arrayValue);
    state.append(slideState);
    state.append(backendState);
    state.append(msaaState);
    state.append(prState);
    state.append(softkeyState);
    state.append(fpsState);

    fWindow->setUIState(state.toStyledString().c_str());
}

void Viewer::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    // For those who will add more features to handle the state change in this function:
    // After the change, please call updateUIState no notify the frontend (e.g., Android app).
    // For example, after slide change, updateUIState is called inside setupCurrentSlide;
    // after backend change, updateUIState is called in this function.
    if (stateName.equals(kSlideStateName)) {
        for (int i = 0; i < fSlides.count(); ++i) {
            if (fSlides[i]->getName().equals(stateValue)) {
                this->setCurrentSlide(i);
                return;
            }
        }

        SkDebugf("Slide not found: %s", stateValue.c_str());
    } else if (stateName.equals(kBackendStateName)) {
        for (int i = 0; i < sk_app::Window::kBackendTypeCount; i++) {
            if (stateValue.equals(kBackendTypeStrings[i])) {
                if (fBackendType != i) {
                    fBackendType = (sk_app::Window::BackendType)i;
                    fWindow->detach();
                    fWindow->attach(backend_type_for_window(fBackendType));
                }
                break;
            }
        }
    } else if (stateName.equals(kMSAAStateName)) {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        int sampleCount = atoi(stateValue.c_str());
        if (sampleCount != params.fMSAASampleCount) {
            params.fMSAASampleCount = sampleCount;
            fWindow->setRequestedDisplayParams(params);
            fWindow->inval();
            this->updateTitle();
            this->updateUIState();
        }
    } else if (stateName.equals(kPathRendererStateName)) {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        for (const auto& pair : gPathRendererNames) {
            if (pair.second == stateValue.c_str()) {
                if (params.fGrContextOptions.fGpuPathRenderers != pair.first) {
                    params.fGrContextOptions.fGpuPathRenderers = pair.first;
                    fWindow->setRequestedDisplayParams(params);
                    fWindow->inval();
                    this->updateTitle();
                    this->updateUIState();
                }
                break;
            }
        }
    } else if (stateName.equals(kSoftkeyStateName)) {
        if (!stateValue.equals(kSoftkeyHint)) {
            fCommands.onSoftkey(stateValue);
            this->updateUIState(); // This is still needed to reset the value to kSoftkeyHint
        }
    } else if (stateName.equals(kRefreshStateName)) {
        // This state is actually NOT in the UI state.
        // We use this to allow Android to quickly set bool fRefresh.
        fRefresh = stateValue.equals(kON);
    } else {
        SkDebugf("Unknown stateName: %s", stateName.c_str());
    }
}

bool Viewer::onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers) {
    return fCommands.onKey(key, state, modifiers);
}

bool Viewer::onChar(SkUnichar c, uint32_t modifiers) {
    if (fSlides[fCurrentSlide]->onChar(c)) {
        fWindow->inval();
        return true;
    } else {
        return fCommands.onChar(c, modifiers);
    }
}

/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "BisectSlide.h"
#include "GMSlide.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "ImageSlide.h"
#include "Resources.h"
#include "SKPSlide.h"
#include "SampleSlide.h"
#include "SkCanvas.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkCommonFlags.h"
#include "SkCommonFlagsGpu.h"
#include "SkEventTracingPriv.h"
#include "SkFontMgrPriv.h"
#include "SkGraphics.h"
#include "SkImagePriv.h"
#include "SkJSONWriter.h"
#include "SkMakeUnique.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPaintFilterCanvas.h"
#include "SkPictureRecorder.h"
#include "SkScan.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"
#include "SkTestFontMgr.h"
#include "SkThreadedBMPDevice.h"
#include "SkTo.h"
#include "SvgSlide.h"
#include "Viewer.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include "imgui.h"

#include <stdlib.h>
#include <map>

#if defined(SK_HAS_SKSG)
    #include "SlideDir.h"
#endif

#if defined(SK_ENABLE_SKOTTIE)
    #include "SkottieSlide.h"
#endif

#if !(defined(SK_BUILD_FOR_WIN) && defined(__clang__))
    #include "NIMASlide.h"
#endif

using namespace sk_app;

static std::map<GpuPathRenderers, std::string> gPathRendererNames;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new Viewer(argc, argv, platformData);
}

static DEFINE_string(slide, "", "Start on this sample.");
static DEFINE_bool(list, false, "List samples?");

#ifdef SK_VULKAN
#    define BACKENDS_STR "\"sw\", \"gl\", and \"vk\""
#else
#    define BACKENDS_STR "\"sw\" and \"gl\""
#endif

static DEFINE_string2(backend, b, "sw", "Backend to use. Allowed values are " BACKENDS_STR ".");

static DEFINE_int32(msaa, 1, "Number of subpixel samples. 0 for no HW antialiasing.");

DEFINE_string(bisect, "", "Path to a .skp or .svg file to bisect.");

DECLARE_int32(threads)

DEFINE_string2(file, f, "", "Open a single file for viewing.");

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
    , fZoomWindowFixed(false)
    , fZoomWindowLocation{0.0f, 0.0f}
    , fLastImage(nullptr)
    , fBackendType(sk_app::Window::kNativeGL_BackendType)
    , fColorMode(ColorMode::kLegacy)
    , fColorSpacePrimaries(gSrgbPrimaries)
    // Our UI can only tweak gamma (currently), so start out gamma-only
    , fColorSpaceTransferFn(g2Dot2_TransferFn)
    , fZoomLevel(0.0f)
    , fRotation(0.0f)
    , fOffset{0.0f, 0.0f}
    , fGestureDevice(GestureDevice::kNone)
    , fPerspectiveMode(kPerspective_Off)
    , fTileCnt(0)
    , fThreadCnt(0)
{
    SkGraphics::Init();

    gPathRendererNames[GpuPathRenderers::kAll] = "All Path Renderers";
    gPathRendererNames[GpuPathRenderers::kStencilAndCover] = "NV_path_rendering";
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

    if (!FLAGS_nativeFonts) {
        gSkFontMgr_DefaultFactory = &sk_tool_utils::MakePortableFontMgr;
    }

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
    fCommands.addCommand('Z', "GUI", "Toggle zoom window state", [this]() {
        this->fZoomWindowFixed = !this->fZoomWindowFixed;
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
    fCommands.addCommand('G', "Modes", "Geometry", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        uint32_t flags = params.fSurfaceProps.flags();
        if (!fPixelGeometryOverrides) {
            fPixelGeometryOverrides = true;
            params.fSurfaceProps = SkSurfaceProps(flags, kUnknown_SkPixelGeometry);
        } else {
            switch (params.fSurfaceProps.pixelGeometry()) {
                case kUnknown_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kRGB_H_SkPixelGeometry);
                    break;
                case kRGB_H_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kBGR_H_SkPixelGeometry);
                    break;
                case kBGR_H_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kRGB_V_SkPixelGeometry);
                    break;
                case kRGB_V_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kBGR_V_SkPixelGeometry);
                    break;
                case kBGR_V_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, SkSurfaceProps::kLegacyFontHost_InitType);
                    fPixelGeometryOverrides = false;
                    break;
            }
        }
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('H', "Paint", "Hinting mode", [this]() {
        if (!fPaintOverrides.fHinting) {
            fPaintOverrides.fHinting = true;
            fPaint.setHinting(SkPaint::kNo_Hinting);
        } else {
            switch (fPaint.getHinting()) {
                case SkPaint::kNo_Hinting:
                    fPaint.setHinting(SkPaint::kSlight_Hinting);
                    break;
                case SkPaint::kSlight_Hinting:
                    fPaint.setHinting(SkPaint::kNormal_Hinting);
                    break;
                case SkPaint::kNormal_Hinting:
                    fPaint.setHinting(SkPaint::kFull_Hinting);
                    break;
                case SkPaint::kFull_Hinting:
                    fPaint.setHinting(SkPaint::kNo_Hinting);
                    fPaintOverrides.fHinting = false;
                    break;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('A', "Paint", "Antialias Mode", [this]() {
        if (!(fPaintOverrides.fFlags & SkPaint::kAntiAlias_Flag)) {
            fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::Alias;
            fPaintOverrides.fFlags |= SkPaint::kAntiAlias_Flag;
            fPaint.setAntiAlias(false);
            gSkUseAnalyticAA = gSkForceAnalyticAA = false;
            gSkUseDeltaAA = gSkForceDeltaAA = false;
        } else {
            fPaint.setAntiAlias(true);
            switch (fPaintOverrides.fAntiAlias) {
                case SkPaintFields::AntiAliasState::Alias:
                    fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::Normal;
                    gSkUseAnalyticAA = gSkForceAnalyticAA = false;
                    gSkUseDeltaAA = gSkForceDeltaAA = false;
                    break;
                case SkPaintFields::AntiAliasState::Normal:
                    fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::AnalyticAAEnabled;
                    gSkUseAnalyticAA = true;
                    gSkForceAnalyticAA = false;
                    gSkUseDeltaAA = gSkForceDeltaAA = false;
                    break;
                case SkPaintFields::AntiAliasState::AnalyticAAEnabled:
                    fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::AnalyticAAForced;
                    gSkUseAnalyticAA = gSkForceAnalyticAA = true;
                    gSkUseDeltaAA = gSkForceDeltaAA = false;
                    break;
                case SkPaintFields::AntiAliasState::AnalyticAAForced:
                    fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::DeltaAAEnabled;
                    gSkUseAnalyticAA = gSkForceAnalyticAA = false;
                    gSkUseDeltaAA = true;
                    gSkForceDeltaAA = false;
                    break;
                case SkPaintFields::AntiAliasState::DeltaAAEnabled:
                    fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::DeltaAAForced;
                    gSkUseAnalyticAA = gSkForceAnalyticAA = false;
                    gSkUseDeltaAA = gSkForceDeltaAA = true;
                    break;
                case SkPaintFields::AntiAliasState::DeltaAAForced:
                    fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::Alias;
                    fPaintOverrides.fFlags &= ~SkPaint::kAntiAlias_Flag;
                    gSkUseAnalyticAA = fPaintOverrides.fOriginalSkUseAnalyticAA;
                    gSkForceAnalyticAA = fPaintOverrides.fOriginalSkForceAnalyticAA;
                    gSkUseDeltaAA = fPaintOverrides.fOriginalSkUseDeltaAA;
                    gSkForceDeltaAA = fPaintOverrides.fOriginalSkForceDeltaAA;
                    break;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('D', "Modes", "DFT", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        uint32_t flags = params.fSurfaceProps.flags();
        flags ^= SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
        params.fSurfaceProps = SkSurfaceProps(flags, params.fSurfaceProps.pixelGeometry());
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('L', "Paint", "Subpixel Antialias Mode", [this]() {
        if (!(fPaintOverrides.fFlags & SkPaint::kLCDRenderText_Flag)) {
            fPaintOverrides.fFlags |= SkPaint::kLCDRenderText_Flag;
            fPaint.setLCDRenderText(false);
        } else {
            if (!fPaint.isLCDRenderText()) {
                fPaint.setLCDRenderText(true);
            } else {
                fPaintOverrides.fFlags &= ~SkPaint::kLCDRenderText_Flag;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('S', "Paint", "Subpixel Position Mode", [this]() {
        if (!(fPaintOverrides.fFlags & SkPaint::kSubpixelText_Flag)) {
            fPaintOverrides.fFlags |= SkPaint::kSubpixelText_Flag;
            fPaint.setSubpixelText(false);
        } else {
            if (!fPaint.isSubpixelText()) {
                fPaint.setSubpixelText(true);
            } else {
                fPaintOverrides.fFlags &= ~SkPaint::kSubpixelText_Flag;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('p', "Transform", "Toggle Perspective Mode", [this]() {
        fPerspectiveMode = (kPerspective_Real == fPerspectiveMode) ? kPerspective_Fake
                                                                   : kPerspective_Real;
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('P', "Transform", "Toggle Perspective", [this]() {
        fPerspectiveMode = (kPerspective_Off == fPerspectiveMode) ? kPerspective_Real
                                                                  : kPerspective_Off;
        this->updateTitle();
        fWindow->inval();
    });

    // set up slides
    this->initSlides();
    if (FLAGS_list) {
        this->listNames();
    }

    fPerspectivePoints[0].set(0, 0);
    fPerspectivePoints[1].set(1, 0);
    fPerspectivePoints[2].set(0, 1);
    fPerspectivePoints[3].set(1, 1);
    fAnimTimer.run();

    auto gamutImage = GetResourceAsImage("images/gamut.png");
    if (gamutImage) {
        fImGuiGamutPaint.setShader(gamutImage->makeShader());
    }
    fImGuiGamutPaint.setColor(SK_ColorWHITE);
    fImGuiGamutPaint.setFilterQuality(kLow_SkFilterQuality);

    fWindow->attach(backend_type_for_window(fBackendType));
    this->setCurrentSlide(this->startupSlide());
}

void Viewer::initSlides() {
    using SlideFactory = sk_sp<Slide>(*)(const SkString& name, const SkString& path);
    static const struct {
        const char*                            fExtension;
        const char*                            fDirName;
        const SkCommandLineFlags::StringArray& fFlags;
        const SlideFactory                     fFactory;
    } gExternalSlidesInfo[] = {
        { ".skp", "skp-dir", FLAGS_skps,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SKPSlide>(name, path);}
        },
        { ".jpg", "jpg-dir", FLAGS_jpgs,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<ImageSlide>(name, path);}
        },
#if defined(SK_ENABLE_SKOTTIE)
        { ".json", "skottie-dir", FLAGS_jsons,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SkottieSlide>(name, path);}
        },
#endif
        { ".svg", "svg-dir", FLAGS_svgs,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SvgSlide>(name, path);}
        },
#if !(defined(SK_BUILD_FOR_WIN) && defined(__clang__))
        { ".nima", "nima-dir", FLAGS_nimas,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<NIMASlide>(name, path);}
        },
#endif
    };

    SkTArray<sk_sp<Slide>, true> dirSlides;

    const auto addSlide = [&](const SkString& name,
                              const SkString& path,
                              const SlideFactory& fact) {
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match,  name.c_str())) {
            return;
        }

        if (auto slide = fact(name, path)) {
            dirSlides.push_back(slide);
            fSlides.push_back(std::move(slide));
        }
    };

    if (!FLAGS_file.isEmpty()) {
        // single file mode
        const SkString file(FLAGS_file[0]);

        if (sk_exists(file.c_str(), kRead_SkFILE_Flag)) {
            for (const auto& sinfo : gExternalSlidesInfo) {
                if (file.endsWith(sinfo.fExtension)) {
                    addSlide(SkOSPath::Basename(file.c_str()), file, sinfo.fFactory);
                    return;
                }
            }

            fprintf(stderr, "Unsupported file type \"%s\"\n", file.c_str());
        } else {
            fprintf(stderr, "Cannot read \"%s\"\n", file.c_str());
        }

        return;
    }

    // Bisect slide.
    if (!FLAGS_bisect.isEmpty()) {
        sk_sp<BisectSlide> bisect = BisectSlide::Create(FLAGS_bisect[0]);
        if (bisect && !SkCommandLineFlags::ShouldSkip(FLAGS_match, bisect->getName().c_str())) {
            if (FLAGS_bisect.count() >= 2) {
                for (const char* ch = FLAGS_bisect[1]; *ch; ++ch) {
                    bisect->onChar(*ch);
                }
            }
            fSlides.push_back(std::move(bisect));
        }
    }

    // GMs
    int firstGM = fSlides.count();
    const skiagm::GMRegistry* gms(skiagm::GMRegistry::Head());
    while (gms) {
        std::unique_ptr<skiagm::GM> gm(gms->factory()(nullptr));

        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, gm->getName())) {
            sk_sp<Slide> slide(new GMSlide(gm.release()));
            fSlides.push_back(std::move(slide));
        }

        gms = gms->next();
    }
    // reverse gms
    int numGMs = fSlides.count() - firstGM;
    for (int i = 0; i < numGMs/2; ++i) {
        std::swap(fSlides[firstGM + i], fSlides[fSlides.count() - i - 1]);
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

    for (const auto& info : gExternalSlidesInfo) {
        for (const auto& flag : info.fFlags) {
            if (SkStrEndsWith(flag.c_str(), info.fExtension)) {
                // single file
                addSlide(SkOSPath::Basename(flag.c_str()), flag, info.fFactory);
            } else {
                // directory
                SkOSFile::Iter it(flag.c_str(), info.fExtension);
                SkString name;
                while (it.next(&name)) {
                    addSlide(name, SkOSPath::Join(flag.c_str(), name.c_str()), info.fFactory);
                }
            }
#if defined(SK_HAS_SKSG)
            if (!dirSlides.empty()) {
                fSlides.push_back(
                    sk_make_sp<SlideDir>(SkStringPrintf("%s[%s]", info.fDirName, flag.c_str()),
                                         std::move(dirSlides)));
                dirSlides.reset();
            }
#endif
        }
    }
}


Viewer::~Viewer() {
    fWindow->detach();
    delete fWindow;
}

struct SkPaintTitleUpdater {
    SkPaintTitleUpdater(SkString* title) : fTitle(title), fCount(0) {}
    void append(const char* s) {
        if (fCount == 0) {
            fTitle->append(" {");
        } else {
            fTitle->append(", ");
        }
        fTitle->append(s);
        ++fCount;
    }
    void done() {
        if (fCount > 0) {
            fTitle->append("}");
        }
    }
    SkString* fTitle;
    int fCount;
};

void Viewer::updateTitle() {
    if (!fWindow) {
        return;
    }
    if (fWindow->sampleCount() < 1) {
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

    SkPaintTitleUpdater paintTitle(&title);
    auto paintFlag = [this, &paintTitle](SkPaint::Flags flag, bool (SkPaint::* isFlag)() const,
                                         const char* on, const char* off)
    {
        if (fPaintOverrides.fFlags & flag) {
            paintTitle.append((fPaint.*isFlag)() ? on : off);
        }
    };

    paintFlag(SkPaint::kAntiAlias_Flag, &SkPaint::isAntiAlias, "Antialias", "Alias");
    paintFlag(SkPaint::kDither_Flag, &SkPaint::isDither, "DITHER", "No Dither");
    paintFlag(SkPaint::kFakeBoldText_Flag, &SkPaint::isFakeBoldText, "Fake Bold", "No Fake Bold");
    paintFlag(SkPaint::kLinearText_Flag, &SkPaint::isLinearText, "Linear Text", "Non-Linear Text");
    paintFlag(SkPaint::kSubpixelText_Flag, &SkPaint::isSubpixelText, "Subpixel Text", "Pixel Text");
    paintFlag(SkPaint::kLCDRenderText_Flag, &SkPaint::isLCDRenderText, "LCD", "lcd");
    paintFlag(SkPaint::kEmbeddedBitmapText_Flag, &SkPaint::isEmbeddedBitmapText,
              "Bitmap Text", "No Bitmap Text");
    paintFlag(SkPaint::kAutoHinting_Flag, &SkPaint::isAutohinted,
              "Force Autohint", "No Force Autohint");
    paintFlag(SkPaint::kVerticalText_Flag, &SkPaint::isVerticalText,
              "Vertical Text", "No Vertical Text");

    if (fPaintOverrides.fHinting) {
        switch (fPaint.getHinting()) {
            case SkPaint::kNo_Hinting:
                paintTitle.append("No Hinting");
                break;
            case SkPaint::kSlight_Hinting:
                paintTitle.append("Slight Hinting");
                break;
            case SkPaint::kNormal_Hinting:
                paintTitle.append("Normal Hinting");
                break;
            case SkPaint::kFull_Hinting:
                paintTitle.append("Full Hinting");
                break;
        }
    }
    paintTitle.done();

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

    const DisplayParams& params = fWindow->getRequestedDisplayParams();
    if (fPixelGeometryOverrides) {
        switch (params.fSurfaceProps.pixelGeometry()) {
            case kUnknown_SkPixelGeometry:
                title.append( " Flat");
                break;
            case kRGB_H_SkPixelGeometry:
                title.append( " RGB");
                break;
            case kBGR_H_SkPixelGeometry:
                title.append( " BGR");
                break;
            case kRGB_V_SkPixelGeometry:
                title.append( " RGBV");
                break;
            case kBGR_V_SkPixelGeometry:
                title.append( " BGRV");
                break;
        }
    }

    if (params.fSurfaceProps.isUseDeviceIndependentFonts()) {
        title.append(" DFT");
    }

    title.append(" [");
    title.append(kBackendTypeStrings[fBackendType]);
    int msaa = fWindow->sampleCount();
    if (msaa > 1) {
        title.appendf(" MSAA: %i", msaa);
    }
    title.append("]");

    GpuPathRenderers pr = fWindow->getRequestedDisplayParams().fGrContextOptions.fGpuPathRenderers;
    if (GpuPathRenderers::kAll != pr) {
        title.appendf(" [Path renderer: %s]", gPathRendererNames[pr].c_str());
    }

    if (kPerspective_Real == fPerspectiveMode) {
        title.append(" Perpsective (Real)");
    } else if (kPerspective_Fake == fPerspectiveMode) {
        title.append(" Perspective (Fake)");
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
    if (fCurrentSlide >= 0) {
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
        fGesture.setTransLimit(slideBounds, windowRect, this->computePreTouchMatrix());

        this->updateTitle();
        this->updateUIState();

        fStatsLayer.resetMeasurements();

        fWindow->inval();
    }
}

#define MAX_ZOOM_LEVEL  8
#define MIN_ZOOM_LEVEL  -8

void Viewer::changeZoomLevel(float delta) {
    fZoomLevel += delta;
    fZoomLevel = SkScalarPin(fZoomLevel, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);
    this->preTouchMatrixChanged();
}

void Viewer::preTouchMatrixChanged() {
    // Update the trans limit as the transform changes.
    const SkISize slideSize = fSlides[fCurrentSlide]->getDimensions();
    const SkRect slideBounds = SkRect::MakeIWH(slideSize.width(), slideSize.height());
    const SkRect windowRect = SkRect::MakeIWH(fWindow->width(), fWindow->height());
    fGesture.setTransLimit(slideBounds, windowRect, this->computePreTouchMatrix());
}

SkMatrix Viewer::computePerspectiveMatrix() {
    SkScalar w = fWindow->width(), h = fWindow->height();
    SkPoint orthoPts[4] = { { 0, 0 }, { w, 0 }, { 0, h }, { w, h } };
    SkPoint perspPts[4] = {
        { fPerspectivePoints[0].fX * w, fPerspectivePoints[0].fY * h },
        { fPerspectivePoints[1].fX * w, fPerspectivePoints[1].fY * h },
        { fPerspectivePoints[2].fX * w, fPerspectivePoints[2].fY * h },
        { fPerspectivePoints[3].fX * w, fPerspectivePoints[3].fY * h }
    };
    SkMatrix m;
    m.setPolyToPoly(orthoPts, perspPts, 4);
    return m;
}

SkMatrix Viewer::computePreTouchMatrix() {
    SkMatrix m = fDefaultMatrix;
    SkScalar zoomScale = (fZoomLevel < 0) ? SK_Scalar1 / (SK_Scalar1 - fZoomLevel)
                                          : SK_Scalar1 + fZoomLevel;
    m.preTranslate(fOffset.x(), fOffset.y());
    m.preScale(zoomScale, zoomScale);

    const SkISize slideSize = fSlides[fCurrentSlide]->getDimensions();
    m.preRotate(fRotation, slideSize.width() * 0.5f, slideSize.height() * 0.5f);

    if (kPerspective_Real == fPerspectiveMode) {
        SkMatrix persp = this->computePerspectiveMatrix();
        m.postConcat(persp);
    }

    return m;
}

SkMatrix Viewer::computeMatrix() {
    SkMatrix m = fGesture.localM();
    m.preConcat(fGesture.globalM());
    m.preConcat(this->computePreTouchMatrix());
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

class OveridePaintFilterCanvas : public SkPaintFilterCanvas {
public:
    OveridePaintFilterCanvas(SkCanvas* canvas, SkPaint* paint, Viewer::SkPaintFields* fields)
        : SkPaintFilterCanvas(canvas), fPaint(paint), fPaintOverrides(fields)
    { }
    bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type) const override {
        if (*paint == nullptr) {
            return true;
        }
        if (fPaintOverrides->fTextSize) {
            paint->writable()->setTextSize(fPaint->getTextSize());
        }
        if (fPaintOverrides->fHinting) {
            paint->writable()->setHinting(fPaint->getHinting());
        }

        if (fPaintOverrides->fFlags & SkPaint::kAntiAlias_Flag) {
            paint->writable()->setAntiAlias(fPaint->isAntiAlias());
        }
        if (fPaintOverrides->fFlags & SkPaint::kDither_Flag) {
            paint->writable()->setDither(fPaint->isDither());
        }
        if (fPaintOverrides->fFlags & SkPaint::kFakeBoldText_Flag) {
            paint->writable()->setFakeBoldText(fPaint->isFakeBoldText());
        }
        if (fPaintOverrides->fFlags & SkPaint::kLinearText_Flag) {
            paint->writable()->setLinearText(fPaint->isLinearText());
        }
        if (fPaintOverrides->fFlags & SkPaint::kSubpixelText_Flag) {
            paint->writable()->setSubpixelText(fPaint->isSubpixelText());
        }
        if (fPaintOverrides->fFlags & SkPaint::kLCDRenderText_Flag) {
            paint->writable()->setLCDRenderText(fPaint->isLCDRenderText());
        }
        if (fPaintOverrides->fFlags & SkPaint::kEmbeddedBitmapText_Flag) {
            paint->writable()->setEmbeddedBitmapText(fPaint->isEmbeddedBitmapText());
        }
        if (fPaintOverrides->fFlags & SkPaint::kAutoHinting_Flag) {
            paint->writable()->setAutohinted(fPaint->isAutohinted());
        }
        if (fPaintOverrides->fFlags & SkPaint::kVerticalText_Flag) {
            paint->writable()->setVerticalText(fPaint->isVerticalText());
        }

        return true;
    }
    SkPaint* fPaint;
    Viewer::SkPaintFields* fPaintOverrides;
};

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
    // because the window surface is actually GL, or we're doing fake perspective.
    sk_sp<SkSurface> offscreenSurface = nullptr;
    std::unique_ptr<SkCanvas> threadedCanvas;
    if (Window::kRaster_BackendType == fBackendType ||
        kPerspective_Fake == fPerspectiveMode ||
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
        SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        canvas->getProps(&props);
        offscreenSurface = Window::kRaster_BackendType == fBackendType
                         ? SkSurface::MakeRaster(info, &props)
                         : canvas->makeSurface(info);
        SkPixmap offscreenPixmap;
        if (fTileCnt > 0 && offscreenSurface->peekPixels(&offscreenPixmap)) {
            SkBitmap offscreenBitmap;
            offscreenBitmap.installPixels(offscreenPixmap);
            threadedCanvas =
                skstd::make_unique<SkCanvas>(
                    sk_make_sp<SkThreadedBMPDevice>(
                         offscreenBitmap, fTileCnt, fThreadCnt, fExecutor.get()));
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
    if (kPerspective_Real == fPerspectiveMode) {
        slideCanvas->clipRect(SkRect::MakeWH(fWindow->width(), fWindow->height()));
    }
    // Time the painting logic of the slide
    fStatsLayer.beginTiming(fPaintTimer);
    OveridePaintFilterCanvas filterCanvas(slideCanvas, &fPaint, &fPaintOverrides);
    fSlides[fCurrentSlide]->draw(&filterCanvas);
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
        int prePerspectiveCount = canvas->save();
        if (kPerspective_Fake == fPerspectiveMode) {
            paint.setFilterQuality(kHigh_SkFilterQuality);
            canvas->clear(SK_ColorWHITE);
            canvas->concat(this->computePerspectiveMatrix());
        }
        canvas->drawImage(retaggedImage, 0, 0, &paint);
        canvas->restoreToCount(prePerspectiveCount);
    }
}

void Viewer::onBackendCreated() {
    this->setupCurrentSlide();
    fWindow->show();
}

void Viewer::onPaint(SkCanvas* canvas) {
    this->drawSlide(canvas);

    fCommands.drawHelp(canvas);

    this->drawImGui();
}

SkPoint Viewer::mapEvent(float x, float y) {
    const auto m = this->computeMatrix();
    SkMatrix inv;

    SkAssertResult(m.invert(&inv));

    return inv.mapXY(x, y);
}

bool Viewer::onTouch(intptr_t owner, Window::InputState state, float x, float y) {
    if (GestureDevice::kMouse == fGestureDevice) {
        return false;
    }

    const auto slidePt = this->mapEvent(x, y);
    if (fSlides[fCurrentSlide]->onMouse(slidePt.x(), slidePt.y(), state, 0)) {
        fWindow->inval();
        return true;
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

    const auto slidePt = this->mapEvent(x, y);
    if (fSlides[fCurrentSlide]->onMouse(slidePt.x(), slidePt.y(), state, modifiers)) {
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
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
}

static ImVec2 ImGui_DragPoint(const char* label, SkPoint* p,
                              const ImVec2& pos, const ImVec2& size, bool* dragging) {
    // Transform points ([0, 0] - [1.0, 1.0]) to screen coords
    ImVec2 center(pos.x + p->fX * size.x, pos.y + p->fY * size.y);

    // Invisible 10x10 button
    ImGui::SetCursorScreenPos(ImVec2(center.x - 5, center.y - 5));
    ImGui::InvisibleButton(label, ImVec2(10, 10));

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging()) {
        ImGuiIO& io = ImGui::GetIO();
        // Normalized mouse position, relative to our gamut box
        ImVec2 mousePosXY((io.MousePos.x - pos.x) / size.x, (io.MousePos.y - pos.y) / size.y);
        // Clamp to edge of box
        p->fX = SkTPin(mousePosXY.x, 0.0f, 1.0f);
        p->fY = SkTPin(mousePosXY.y, 0.0f, 1.0f);
        *dragging = true;
    }

    // Return screen coordinates for the caller. We could just return center here, but we'd have
    // one frame of lag during drag.
    return ImVec2(pos.x + p->fX * size.x, pos.y + p->fY * size.y);
}

static bool ImGui_DragLocation(SkPoint* pt) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Fit our image/canvas to the available width, and scale the height to maintain aspect ratio.
    float canvasWidth = SkTMax(ImGui::GetContentRegionAvailWidth(), 50.0f);
    ImVec2 size = ImVec2(canvasWidth, canvasWidth);
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Background rectangle
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 128));

    // Location marker
    bool dragging = false;
    ImVec2 tl = ImGui_DragPoint("SL", pt + 0, pos, size, &dragging);
    drawList->AddCircle(tl, 5.0f, 0xFFFFFFFF);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
    ImGui::Spacing();

    return dragging;
}

static bool ImGui_DragQuad(SkPoint* pts) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Fit our image/canvas to the available width, and scale the height to maintain aspect ratio.
    float canvasWidth = SkTMax(ImGui::GetContentRegionAvailWidth(), 50.0f);
    ImVec2 size = ImVec2(canvasWidth, canvasWidth);
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Background rectangle
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 128));

    // Corner markers
    bool dragging = false;
    ImVec2 tl = ImGui_DragPoint("TL", pts + 0, pos, size, &dragging);
    ImVec2 tr = ImGui_DragPoint("TR", pts + 1, pos, size, &dragging);
    ImVec2 bl = ImGui_DragPoint("BL", pts + 2, pos, size, &dragging);
    ImVec2 br = ImGui_DragPoint("BR", pts + 3, pos, size, &dragging);

    // Draw markers and quad
    drawList->AddCircle(tl, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(tr, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(bl, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(br, 5.0f, 0xFFFFFFFF);
    drawList->AddLine(tl, tr, 0xFFFFFFFF);
    drawList->AddLine(tr, br, 0xFFFFFFFF);
    drawList->AddLine(br, bl, 0xFFFFFFFF);
    drawList->AddLine(bl, tl, 0xFFFFFFFF);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
    ImGui::Spacing();

    return dragging;
}

void Viewer::drawImGui() {
    // Support drawing the ImGui demo window. Superfluous, but gives a good idea of what's possible
    if (fShowImGuiTestWindow) {
        ImGui::ShowDemoWindow(&fShowImGuiTestWindow);
    }

    if (fShowImGuiDebugWindow) {
        // We have some dynamic content that sizes to fill available size. If the scroll bar isn't
        // always visible, we can end up in a layout feedback loop.
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
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
                    ImGui::RadioButton("1", &sampleCount, 1); ImGui::SameLine();
                    ImGui::RadioButton("4", &sampleCount, 4); ImGui::SameLine();
                    ImGui::RadioButton("8", &sampleCount, 8); ImGui::SameLine();
                    ImGui::RadioButton("16", &sampleCount, 16);

                    if (sampleCount != params.fMSAASampleCount) {
                        params.fMSAASampleCount = sampleCount;
                        paramsChanged = true;
                    }
                }

                int pixelGeometryIdx = 0;
                if (fPixelGeometryOverrides) {
                    pixelGeometryIdx = params.fSurfaceProps.pixelGeometry() + 1;
                }
                if (ImGui::Combo("Pixel Geometry", &pixelGeometryIdx,
                                 "Default\0Flat\0RGB\0BGR\0RGBV\0BGRV\0\0"))
                {
                    uint32_t flags = params.fSurfaceProps.flags();
                    if (pixelGeometryIdx == 0) {
                        fPixelGeometryOverrides = false;
                        params.fSurfaceProps = SkSurfaceProps(flags, SkSurfaceProps::kLegacyFontHost_InitType);
                    } else {
                        fPixelGeometryOverrides = true;
                        SkPixelGeometry pixelGeometry = SkTo<SkPixelGeometry>(pixelGeometryIdx - 1);
                        params.fSurfaceProps = SkSurfaceProps(flags, pixelGeometry);
                    }
                    paramsChanged = true;
                }

                bool useDFT = params.fSurfaceProps.isUseDeviceIndependentFonts();
                if (ImGui::Checkbox("DFT", &useDFT)) {
                    uint32_t flags = params.fSurfaceProps.flags();
                    if (useDFT) {
                        flags |= SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
                    } else {
                        flags &= ~SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
                    }
                    SkPixelGeometry pixelGeometry = params.fSurfaceProps.pixelGeometry();
                    params.fSurfaceProps = SkSurfaceProps(flags, pixelGeometry);
                    paramsChanged = true;
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
                    } else if (fWindow->sampleCount() > 1) {
                        prButton(GpuPathRenderers::kAll);
                        if (ctx->contextPriv().caps()->shaderCaps()->pathRenderingSupport()) {
                            prButton(GpuPathRenderers::kStencilAndCover);
                        }
                        prButton(GpuPathRenderers::kTessellating);
                        prButton(GpuPathRenderers::kNone);
                    } else {
                        prButton(GpuPathRenderers::kAll);
                        if (GrCoverageCountingPathRenderer::IsSupported(
                                    *ctx->contextPriv().caps())) {
                            prButton(GpuPathRenderers::kCoverageCounting);
                        }
                        prButton(GpuPathRenderers::kSmall);
                        prButton(GpuPathRenderers::kTessellating);
                        prButton(GpuPathRenderers::kNone);
                    }
                    ImGui::TreePop();
                }
            }

            if (ImGui::CollapsingHeader("Transform")) {
                float zoom = fZoomLevel;
                if (ImGui::SliderFloat("Zoom", &zoom, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL)) {
                    fZoomLevel = zoom;
                    this->preTouchMatrixChanged();
                    paramsChanged = true;
                }
                float deg = fRotation;
                if (ImGui::SliderFloat("Rotate", &deg, -30, 360, "%.3f deg")) {
                    fRotation = deg;
                    this->preTouchMatrixChanged();
                    paramsChanged = true;
                }
                if (ImGui::CollapsingHeader("Subpixel offset", ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
                    if (ImGui_DragLocation(&fOffset)) {
                        this->preTouchMatrixChanged();
                        paramsChanged = true;
                    }
                }
                int perspectiveMode = static_cast<int>(fPerspectiveMode);
                if (ImGui::Combo("Perspective", &perspectiveMode, "Off\0Real\0Fake\0\0")) {
                    fPerspectiveMode = static_cast<PerspectiveMode>(perspectiveMode);
                    this->preTouchMatrixChanged();
                    paramsChanged = true;
                }
                if (perspectiveMode != kPerspective_Off && ImGui_DragQuad(fPerspectivePoints)) {
                    this->preTouchMatrixChanged();
                    paramsChanged = true;
                }
            }

            if (ImGui::CollapsingHeader("Paint")) {
                int hintingIdx = 0;
                if (fPaintOverrides.fHinting) {
                    hintingIdx = fPaint.getHinting() + 1;
                }
                if (ImGui::Combo("Hinting", &hintingIdx,
                                 "Default\0None\0Slight\0Normal\0Full\0\0"))
                {
                    if (hintingIdx == 0) {
                        fPaintOverrides.fHinting = false;
                        fPaint.setHinting(SkPaint::kNo_Hinting);
                    } else {
                        fPaintOverrides.fHinting = true;
                        SkPaint::Hinting hinting = SkTo<SkPaint::Hinting>(hintingIdx - 1);
                        fPaint.setHinting(hinting);
                    }
                    paramsChanged = true;
                }

                int aliasIdx = 0;
                if (fPaintOverrides.fFlags & SkPaint::kAntiAlias_Flag) {
                    aliasIdx = SkTo<int>(fPaintOverrides.fAntiAlias) + 1;
                }
                if (ImGui::Combo("Anti-Alias", &aliasIdx,
                                 "Default\0Alias\0Normal\0AnalyticAAEnabled\0AnalyticAAForced\0"
                                 "DeltaAAEnabled\0DeltaAAForced\0\0"))
                {
                    gSkUseAnalyticAA = fPaintOverrides.fOriginalSkUseAnalyticAA;
                    gSkForceAnalyticAA = fPaintOverrides.fOriginalSkForceAnalyticAA;
                    gSkUseDeltaAA = fPaintOverrides.fOriginalSkUseDeltaAA;
                    gSkForceDeltaAA = fPaintOverrides.fOriginalSkForceDeltaAA;
                    if (aliasIdx == 0) {
                        fPaintOverrides.fAntiAlias = SkPaintFields::AntiAliasState::Alias;
                        fPaintOverrides.fFlags &= ~SkPaint::kAntiAlias_Flag;
                    } else {
                        fPaintOverrides.fFlags |= SkPaint::kAntiAlias_Flag;
                        fPaintOverrides.fAntiAlias =SkTo<SkPaintFields::AntiAliasState>(aliasIdx-1);
                        fPaint.setAntiAlias(aliasIdx > 1);
                        switch (fPaintOverrides.fAntiAlias) {
                            case SkPaintFields::AntiAliasState::Alias:
                                break;
                            case SkPaintFields::AntiAliasState::Normal:
                                break;
                            case SkPaintFields::AntiAliasState::AnalyticAAEnabled:
                                gSkUseAnalyticAA = true;
                                gSkForceAnalyticAA = false;
                                gSkUseDeltaAA = gSkForceDeltaAA = false;
                                break;
                            case SkPaintFields::AntiAliasState::AnalyticAAForced:
                                gSkUseAnalyticAA = gSkForceAnalyticAA = true;
                                gSkUseDeltaAA = gSkForceDeltaAA = false;
                                break;
                            case SkPaintFields::AntiAliasState::DeltaAAEnabled:
                                gSkUseAnalyticAA = gSkForceAnalyticAA = false;
                                gSkUseDeltaAA = true;
                                gSkForceDeltaAA = false;
                                break;
                            case SkPaintFields::AntiAliasState::DeltaAAForced:
                                gSkUseAnalyticAA = gSkForceAnalyticAA = false;
                                gSkUseDeltaAA = gSkForceDeltaAA = true;
                                break;
                        }
                    }
                    paramsChanged = true;
                }

                auto paintFlag = [this, &paramsChanged](const char* label, const char* items,
                                                        SkPaint::Flags flag,
                                                        bool (SkPaint::* isFlag)() const,
                                                        void (SkPaint::* setFlag)(bool) )
                {
                    int itemIndex = 0;
                    if (fPaintOverrides.fFlags & flag) {
                        itemIndex = (fPaint.*isFlag)() ? 2 : 1;
                    }
                    if (ImGui::Combo(label, &itemIndex, items)) {
                        if (itemIndex == 0) {
                            fPaintOverrides.fFlags &= ~flag;
                        } else {
                            fPaintOverrides.fFlags |= flag;
                            (fPaint.*setFlag)(itemIndex == 2);
                        }
                        paramsChanged = true;
                    }
                };

                paintFlag("Dither",
                          "Default\0No Dither\0Dither\0\0",
                          SkPaint::kDither_Flag,
                          &SkPaint::isDither, &SkPaint::setDither);

                paintFlag("Fake Bold Glyphs",
                          "Default\0No Fake Bold\0Fake Bold\0\0",
                          SkPaint::kFakeBoldText_Flag,
                          &SkPaint::isFakeBoldText, &SkPaint::setFakeBoldText);

                paintFlag("Linear Text",
                          "Default\0No Linear Text\0Linear Text\0\0",
                          SkPaint::kLinearText_Flag,
                          &SkPaint::isLinearText, &SkPaint::setLinearText);

                paintFlag("Subpixel Position Glyphs",
                          "Default\0Pixel Text\0Subpixel Text\0\0",
                          SkPaint::kSubpixelText_Flag,
                          &SkPaint::isSubpixelText, &SkPaint::setSubpixelText);

                paintFlag("Subpixel Anti-Alias",
                          "Default\0lcd\0LCD\0\0",
                          SkPaint::kLCDRenderText_Flag,
                          &SkPaint::isLCDRenderText, &SkPaint::setLCDRenderText);

                paintFlag("Embedded Bitmap Text",
                          "Default\0No Embedded Bitmaps\0Embedded Bitmaps\0\0",
                          SkPaint::kEmbeddedBitmapText_Flag,
                          &SkPaint::isEmbeddedBitmapText, &SkPaint::setEmbeddedBitmapText);

                paintFlag("Force Auto-Hinting",
                          "Default\0No Force Auto-Hinting\0Force Auto-Hinting\0\0",
                          SkPaint::kAutoHinting_Flag,
                          &SkPaint::isAutohinted, &SkPaint::setAutohinted);

                paintFlag("Vertical Text",
                          "Default\0No Vertical Text\0Vertical Text\0\0",
                          SkPaint::kVerticalText_Flag,
                          &SkPaint::isVerticalText, &SkPaint::setVerticalText);

                ImGui::Checkbox("Override TextSize", &fPaintOverrides.fTextSize);
                if (fPaintOverrides.fTextSize) {
                    ImGui::DragFloat2("TextRange", fPaintOverrides.fTextSizeRange,
                                      0.001f, -10.0f, 300.0f, "%.6f", 2.0f);
                    float textSize = fPaint.getTextSize();
                    if (ImGui::DragFloat("TextSize", &textSize, 0.001f,
                                         fPaintOverrides.fTextSizeRange[0],
                                         fPaintOverrides.fTextSizeRange[1],
                                         "%.6f", 2.0f))
                    {
                        fPaint.setTextSize(textSize);
                        this->preTouchMatrixChanged();
                        paramsChanged = true;
                    }
                }
            }

            {
                SkMetaData controls;
                if (fSlides[fCurrentSlide]->onGetControls(&controls)) {
                    if (ImGui::CollapsingHeader("Current Slide")) {
                        SkMetaData::Iter iter(controls);
                        const char* name;
                        SkMetaData::Type type;
                        int count;
                        bool found = false;
                        while ((name = iter.next(&type, &count)) != nullptr && found == false) {
                            if (type == SkMetaData::kScalar_Type) {
                                float val[3];
                                SkASSERT(count == 3);
                                controls.findScalars(name, &count, val);
                                if (ImGui::SliderFloat(name, &val[0], val[1], val[2])) {
                                    controls.setScalars(name, 3, val);
                                    fSlides[fCurrentSlide]->onSetControls(controls);
                                    found = paramsChanged = true;
                                }
                            }
                        }
                    }
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
        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Zoom", &fShowZoomWindow)) {
            static int zoomFactor = 8;
            if (ImGui::Button("<<")) {
                zoomFactor = SkTMax(zoomFactor / 2, 4);
            }
            ImGui::SameLine(); ImGui::Text("%2d", zoomFactor); ImGui::SameLine();
            if (ImGui::Button(">>")) {
                zoomFactor = SkTMin(zoomFactor * 2, 32);
            }

            if (!fZoomWindowFixed) {
                ImVec2 mousePos = ImGui::GetMousePos();
                fZoomWindowLocation = SkPoint::Make(mousePos.x, mousePos.y);
            }
            SkScalar x = fZoomWindowLocation.x();
            SkScalar y = fZoomWindowLocation.y();
            int xInt = SkScalarRoundToInt(x);
            int yInt = SkScalarRoundToInt(y);
            ImVec2 avail = ImGui::GetContentRegionAvail();

            uint32_t pixel = 0;
            SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
            if (fLastImage->readPixels(info, &pixel, info.minRowBytes(), xInt, yInt)) {
                ImGui::SameLine();
                ImGui::Text("(X, Y): %d, %d RGBA: %x %x %x %x",
                            xInt, yInt,
                            SkGetPackedR32(pixel), SkGetPackedG32(pixel),
                            SkGetPackedB32(pixel), SkGetPackedA32(pixel));
            }

            fImGuiLayer.skiaWidget(avail, [=](SkCanvas* c) {
                // Translate so the region of the image that's under the mouse cursor is centered
                // in the zoom canvas:
                c->scale(zoomFactor, zoomFactor);
                c->translate(avail.x * 0.5f / zoomFactor - x - 0.5f,
                             avail.y * 0.5f / zoomFactor - y - 0.5f);
                c->drawImage(this->fLastImage, 0, 0);

                SkPaint outline;
                outline.setStyle(SkPaint::kStroke_Style);
                c->drawRect(SkRect::MakeXYWH(x, y, 1, 1), outline);
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

template <typename OptionsFunc>
static void WriteStateObject(SkJSONWriter& writer, const char* name, const char* value,
                             OptionsFunc&& optionsFunc) {
    writer.beginObject();
    {
        writer.appendString(kName , name);
        writer.appendString(kValue, value);

        writer.beginArray(kOptions);
        {
            optionsFunc(writer);
        }
        writer.endArray();
    }
    writer.endObject();
}


void Viewer::updateUIState() {
    if (!fWindow) {
        return;
    }
    if (fWindow->sampleCount() < 1) {
        return; // Surface hasn't been created yet.
    }

    SkDynamicMemoryWStream memStream;
    SkJSONWriter writer(&memStream);
    writer.beginArray();

    // Slide state
    WriteStateObject(writer, kSlideStateName, fSlides[fCurrentSlide]->getName().c_str(),
        [this](SkJSONWriter& writer) {
            for(const auto& slide : fSlides) {
                writer.appendString(slide->getName().c_str());
            }
        });

    // Backend state
    WriteStateObject(writer, kBackendStateName, kBackendTypeStrings[fBackendType],
        [](SkJSONWriter& writer) {
            for (const auto& str : kBackendTypeStrings) {
                writer.appendString(str);
            }
        });

    // MSAA state
    const auto countString = SkStringPrintf("%d", fWindow->sampleCount());
    WriteStateObject(writer, kMSAAStateName, countString.c_str(),
        [this](SkJSONWriter& writer) {
            writer.appendS32(0);

            if (sk_app::Window::kRaster_BackendType == fBackendType) {
                return;
            }

            for (int msaa : {4, 8, 16}) {
                writer.appendS32(msaa);
            }
        });

    // Path renderer state
    GpuPathRenderers pr = fWindow->getRequestedDisplayParams().fGrContextOptions.fGpuPathRenderers;
    WriteStateObject(writer, kPathRendererStateName, gPathRendererNames[pr].c_str(),
        [this](SkJSONWriter& writer) {
            const GrContext* ctx = fWindow->getGrContext();
            if (!ctx) {
                writer.appendString("Software");
            } else {
                const auto* caps = ctx->contextPriv().caps();

                writer.appendString(gPathRendererNames[GpuPathRenderers::kAll].c_str());
                if (fWindow->sampleCount() > 1) {
                    if (caps->shaderCaps()->pathRenderingSupport()) {
                        writer.appendString(
                            gPathRendererNames[GpuPathRenderers::kStencilAndCover].c_str());
                    }
                } else {
                    if(GrCoverageCountingPathRenderer::IsSupported(*caps)) {
                        writer.appendString(
                            gPathRendererNames[GpuPathRenderers::kCoverageCounting].c_str());
                    }
                    writer.appendString(gPathRendererNames[GpuPathRenderers::kSmall].c_str());
                }
                    writer.appendString(
                        gPathRendererNames[GpuPathRenderers::kTessellating].c_str());
                    writer.appendString(gPathRendererNames[GpuPathRenderers::kNone].c_str());
            }
        });

    // Softkey state
    WriteStateObject(writer, kSoftkeyStateName, kSoftkeyHint,
        [this](SkJSONWriter& writer) {
            writer.appendString(kSoftkeyHint);
            for (const auto& softkey : fCommands.getCommandsAsSoftkeys()) {
                writer.appendString(softkey.c_str());
            }
        });

    writer.endArray();
    writer.flush();

    auto data = memStream.detachAsData();

    // TODO: would be cool to avoid this copy
    const SkString cstring(static_cast<const char*>(data->data()), data->size());

    fWindow->setUIState(cstring.c_str());
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

/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkTPin.h"
#include "include/private/SkTo.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMD5.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkScan.h"
#include "src/core/SkTSort.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/image/SkImage_Base.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"
#include "tools/trace/EventTracingPriv.h"
#include "tools/viewer/BisectSlide.h"
#include "tools/viewer/GMSlide.h"
#include "tools/viewer/ImageSlide.h"
#include "tools/viewer/ParticlesSlide.h"
#include "tools/viewer/SKPSlide.h"
#include "tools/viewer/SampleSlide.h"
#include "tools/viewer/SkSLSlide.h"
#include "tools/viewer/SlideDir.h"
#include "tools/viewer/SvgSlide.h"
#include "tools/viewer/Viewer.h"

#include <cstdlib>
#include <map>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"  // For ImGui support of std::string

#ifdef SK_VULKAN
#include "spirv-tools/libspirv.hpp"
#endif

#if defined(SK_ENABLE_SKOTTIE)
    #include "tools/viewer/SkottieSlide.h"
#endif
#if defined(SK_ENABLE_SKRIVE)
    #include "tools/viewer/SkRiveSlide.h"
#endif

namespace SkSL {
extern bool gSkSLOptimizer;
extern bool gSkSLInliner;
}

class CapturingShaderErrorHandler : public GrContextOptions::ShaderErrorHandler {
public:
    void compileError(const char* shader, const char* errors) override {
        fShaders.push_back(SkString(shader));
        fErrors.push_back(SkString(errors));
    }

    void reset() {
        fShaders.reset();
        fErrors.reset();
    }

    SkTArray<SkString> fShaders;
    SkTArray<SkString> fErrors;
};

static CapturingShaderErrorHandler gShaderErrorHandler;

GrContextOptions::ShaderErrorHandler* Viewer::ShaderErrorHandler() { return &gShaderErrorHandler; }

using namespace sk_app;

static std::map<GpuPathRenderers, std::string> gPathRendererNames;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new Viewer(argc, argv, platformData);
}

static DEFINE_string(slide, "", "Start on this sample.");
static DEFINE_bool(list, false, "List samples?");

#ifdef SK_GL
#define GL_BACKEND_STR ", \"gl\""
#else
#define GL_BACKEND_STR
#endif
#ifdef SK_VULKAN
#define VK_BACKEND_STR ", \"vk\""
#else
#define VK_BACKEND_STR
#endif
#ifdef SK_METAL
#define MTL_BACKEND_STR ", \"mtl\""
#else
#define MTL_BACKEND_STR
#endif
#ifdef SK_DIRECT3D
#define D3D_BACKEND_STR ", \"d3d\""
#else
#define D3D_BACKEND_STR
#endif
#ifdef SK_DAWN
#define DAWN_BACKEND_STR ", \"dawn\""
#else
#define DAWN_BACKEND_STR
#endif
#define BACKENDS_STR_EVALUATOR(sw, gl, vk, mtl, d3d, dawn) sw gl vk mtl d3d dawn
#define BACKENDS_STR BACKENDS_STR_EVALUATOR( \
    "\"sw\"", GL_BACKEND_STR, VK_BACKEND_STR, MTL_BACKEND_STR, D3D_BACKEND_STR, DAWN_BACKEND_STR)

static DEFINE_string2(backend, b, "sw", "Backend to use. Allowed values are " BACKENDS_STR ".");

static DEFINE_int(msaa, 1, "Number of subpixel samples. 0 for no HW antialiasing.");
static DEFINE_bool(dmsaa, false, "Use internal MSAA to render to non-MSAA surfaces?");

static DEFINE_string(bisect, "", "Path to a .skp or .svg file to bisect.");

static DEFINE_string2(file, f, "", "Open a single file for viewing.");

static DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching name to always be skipped\n"
               "^ requires the start of the name to match\n"
               "$ requires the end of the name to match\n"
               "^ and $ requires an exact match\n"
               "If a name does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

#if defined(SK_BUILD_FOR_ANDROID)
    static DEFINE_string(jpgs, "/data/local/tmp/resources", "Directory to read jpgs from.");
    static DEFINE_string(skps, "/data/local/tmp/skps", "Directory to read skps from.");
    static DEFINE_string(lotties, "/data/local/tmp/lotties",
                         "Directory to read (Bodymovin) jsons from.");
    static DEFINE_string(rives, "/data/local/tmp/rives",
                         "Directory to read Rive (Flare) files from.");
#else
    static DEFINE_string(jpgs, "jpgs", "Directory to read jpgs from.");
    static DEFINE_string(skps, "skps", "Directory to read skps from.");
    static DEFINE_string(lotties, "lotties", "Directory to read (Bodymovin) jsons from.");
    static DEFINE_string(rives, "rives", "Directory to read Rive (Flare) files from.");
#endif

static DEFINE_string(svgs, "", "Directory to read SVGs from, or a single SVG file.");

static DEFINE_int_2(threads, j, -1,
               "Run threadsafe tests on a threadpool with this many extra threads, "
               "defaulting to one extra thread per core.");

static DEFINE_bool(redraw, false, "Toggle continuous redraw.");

static DEFINE_bool(offscreen, false, "Force rendering to an offscreen surface.");
static DEFINE_bool(skvm, false, "Force skvm blitters for raster.");
static DEFINE_bool(jit, true, "JIT SkVM?");
static DEFINE_bool(dylib, false, "JIT via dylib (much slower compile but easier to debug/profile)");
static DEFINE_bool(stats, false, "Display stats overlay on startup.");
static DEFINE_bool(binaryarchive, false, "Enable MTLBinaryArchive use (if available).");

#ifndef SK_GL
static_assert(false, "viewer requires GL backend for raster.")
#endif

const char* kBackendTypeStrings[sk_app::Window::kBackendTypeCount] = {
    "OpenGL",
#if SK_ANGLE && defined(SK_BUILD_FOR_WIN)
    "ANGLE",
#endif
#ifdef SK_DAWN
    "Dawn",
#endif
#ifdef SK_VULKAN
    "Vulkan",
#endif
#ifdef SK_METAL
    "Metal",
#endif
#ifdef SK_DIRECT3D
    "Direct3D",
#endif
    "Raster"
};

static sk_app::Window::BackendType get_backend_type(const char* str) {
#ifdef SK_DAWN
    if (0 == strcmp(str, "dawn")) {
        return sk_app::Window::kDawn_BackendType;
    } else
#endif
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
#ifdef SK_METAL
    if (0 == strcmp(str, "mtl")) {
        return sk_app::Window::kMetal_BackendType;
    } else
#endif
#ifdef SK_DIRECT3D
    if (0 == strcmp(str, "d3d")) {
        return sk_app::Window::kDirect3D_BackendType;
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

class NullSlide : public Slide {
    SkISize getDimensions() const override {
        return SkISize::Make(640, 480);
    }

    void draw(SkCanvas* canvas) override {
        canvas->clear(0xffff11ff);
    }
};

static const char kName[] = "name";
static const char kValue[] = "value";
static const char kOptions[] = "options";
static const char kSlideStateName[] = "Slide";
static const char kBackendStateName[] = "Backend";
static const char kMSAAStateName[] = "MSAA";
static const char kPathRendererStateName[] = "Path renderer";
static const char kSoftkeyStateName[] = "Softkey";
static const char kSoftkeyHint[] = "Please select a softkey";
static const char kON[] = "ON";
static const char kRefreshStateName[] = "Refresh";

extern bool gUseSkVMBlitter;
extern bool gSkVMAllowJIT;
extern bool gSkVMJITViaDylib;

Viewer::Viewer(int argc, char** argv, void* platformData)
    : fCurrentSlide(-1)
    , fRefresh(false)
    , fSaveToSKP(false)
    , fShowSlideDimensions(false)
    , fShowImGuiDebugWindow(false)
    , fShowSlidePicker(false)
    , fShowImGuiTestWindow(false)
    , fShowZoomWindow(false)
    , fZoomWindowFixed(false)
    , fZoomWindowLocation{0.0f, 0.0f}
    , fLastImage(nullptr)
    , fZoomUI(false)
    , fBackendType(sk_app::Window::kNativeGL_BackendType)
    , fColorMode(ColorMode::kLegacy)
    , fColorSpacePrimaries(gSrgbPrimaries)
    // Our UI can only tweak gamma (currently), so start out gamma-only
    , fColorSpaceTransferFn(SkNamedTransferFn::k2Dot2)
    , fApplyBackingScale(true)
    , fZoomLevel(0.0f)
    , fRotation(0.0f)
    , fOffset{0.5f, 0.5f}
    , fGestureDevice(GestureDevice::kNone)
    , fTiled(false)
    , fDrawTileBoundaries(false)
    , fTileScale{0.25f, 0.25f}
    , fPerspectiveMode(kPerspective_Off)
{
    SkGraphics::Init();

    gPathRendererNames[GpuPathRenderers::kDefault] = "Default Path Renderers";
    gPathRendererNames[GpuPathRenderers::kTessellation] = "Tessellation";
    gPathRendererNames[GpuPathRenderers::kStencilAndCover] = "NV_path_rendering";
    gPathRendererNames[GpuPathRenderers::kSmall] = "Small paths (cached sdf or alpha masks)";
    gPathRendererNames[GpuPathRenderers::kCoverageCounting] = "CCPR";
    gPathRendererNames[GpuPathRenderers::kTriangulating] = "Triangulating";
    gPathRendererNames[GpuPathRenderers::kNone] = "Software masks";

    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    CommandLineFlags::Parse(argc, argv);
#ifdef SK_BUILD_FOR_ANDROID
    SetResourcePath("/data/local/tmp/resources");
#endif

    gUseSkVMBlitter = FLAGS_skvm;
    gSkVMAllowJIT = FLAGS_jit;
    gSkVMJITViaDylib = FLAGS_dylib;

    ToolUtils::SetDefaultFontMgr();

    initializeEventTracingForTools();
    static SkTaskGroup::Enabler kTaskGroupEnabler(FLAGS_threads);

    fBackendType = get_backend_type(FLAGS_backend[0]);
    fWindow = Window::CreateNativeWindow(platformData);

    DisplayParams displayParams;
    displayParams.fMSAASampleCount = FLAGS_msaa;
    displayParams.fEnableBinaryArchive = FLAGS_binaryarchive;
    SetCtxOptionsFromCommonFlags(&displayParams.fGrContextOptions);
    displayParams.fGrContextOptions.fPersistentCache = &fPersistentCache;
    displayParams.fGrContextOptions.fShaderCacheStrategy =
            GrContextOptions::ShaderCacheStrategy::kBackendSource;
    displayParams.fGrContextOptions.fShaderErrorHandler = &gShaderErrorHandler;
    displayParams.fGrContextOptions.fSuppressPrints = true;
    displayParams.fGrContextOptions.fAlwaysAntialias = FLAGS_dmsaa;
    fWindow->setRequestedDisplayParams(displayParams);
    fDisplay = fWindow->getRequestedDisplayParams();
    fRefresh = FLAGS_redraw;

    fImGuiLayer.setScaleFactor(fWindow->scaleFactor());
    fStatsLayer.setDisplayScale((fZoomUI ? 2.0f : 1.0f) * fWindow->scaleFactor());

    // Configure timers
    fStatsLayer.setActive(FLAGS_stats);
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
    fCommands.addCommand(skui::Key::kBack, "Backspace", "GUI", "Jump to slide picker", [this]() {
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
    fCommands.addCommand('v', "Swapchain", "Toggle vsync on/off", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        params.fDisableVsync = !params.fDisableVsync;
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('V', "Swapchain", "Toggle delayed acquire on/off (Metal only)", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        params.fDelayDrawableAcquisition = !params.fDelayDrawableAcquisition;
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('r', "Redraw", "Toggle redraw", [this]() {
        fRefresh = !fRefresh;
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
                this->setColorMode(ColorMode::kColorManaged8888);
                break;
            case ColorMode::kColorManaged8888:
                this->setColorMode(ColorMode::kColorManagedF16);
                break;
            case ColorMode::kColorManagedF16:
                this->setColorMode(ColorMode::kColorManagedF16Norm);
                break;
            case ColorMode::kColorManagedF16Norm:
                this->setColorMode(ColorMode::kLegacy);
                break;
        }
    });
    fCommands.addCommand('w', "Modes", "Toggle wireframe", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        params.fGrContextOptions.fWireframeMode = !params.fGrContextOptions.fWireframeMode;
        fWindow->setRequestedDisplayParams(params);
        fWindow->inval();
    });
    fCommands.addCommand(skui::Key::kRight, "Right", "Navigation", "Next slide", [this]() {
        this->setCurrentSlide(fCurrentSlide < fSlides.count() - 1 ? fCurrentSlide + 1 : 0);
    });
    fCommands.addCommand(skui::Key::kLeft, "Left", "Navigation", "Previous slide", [this]() {
        this->setCurrentSlide(fCurrentSlide > 0 ? fCurrentSlide - 1 : fSlides.count() - 1);
    });
    fCommands.addCommand(skui::Key::kUp, "Up", "Transform", "Zoom in", [this]() {
        this->changeZoomLevel(1.f / 32.f);
        fWindow->inval();
    });
    fCommands.addCommand(skui::Key::kDown, "Down", "Transform", "Zoom out", [this]() {
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
    fCommands.addCommand('K', "IO", "Save slide to SKP", [this]() {
        fSaveToSKP = true;
        fWindow->inval();
    });
    fCommands.addCommand('&', "Overlays", "Show slide dimensios", [this]() {
        fShowSlideDimensions = !fShowSlideDimensions;
        fWindow->inval();
    });
    fCommands.addCommand('G', "Modes", "Geometry", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        uint32_t flags = params.fSurfaceProps.flags();
        SkPixelGeometry defaultPixelGeometry = fDisplay.fSurfaceProps.pixelGeometry();
        if (!fDisplayOverrides.fSurfaceProps.fPixelGeometry) {
            fDisplayOverrides.fSurfaceProps.fPixelGeometry = true;
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
                    params.fSurfaceProps = SkSurfaceProps(flags, defaultPixelGeometry);
                    fDisplayOverrides.fSurfaceProps.fPixelGeometry = false;
                    break;
            }
        }
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('H', "Font", "Hinting mode", [this]() {
        if (!fFontOverrides.fHinting) {
            fFontOverrides.fHinting = true;
            fFont.setHinting(SkFontHinting::kNone);
        } else {
            switch (fFont.getHinting()) {
                case SkFontHinting::kNone:
                    fFont.setHinting(SkFontHinting::kSlight);
                    break;
                case SkFontHinting::kSlight:
                    fFont.setHinting(SkFontHinting::kNormal);
                    break;
                case SkFontHinting::kNormal:
                    fFont.setHinting(SkFontHinting::kFull);
                    break;
                case SkFontHinting::kFull:
                    fFont.setHinting(SkFontHinting::kNone);
                    fFontOverrides.fHinting = false;
                    break;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('A', "Paint", "Antialias Mode", [this]() {
        if (!fPaintOverrides.fAntiAlias) {
            fPaintOverrides.fAntiAliasState = SkPaintFields::AntiAliasState::Alias;
            fPaintOverrides.fAntiAlias = true;
            fPaint.setAntiAlias(false);
            gSkUseAnalyticAA = gSkForceAnalyticAA = false;
        } else {
            fPaint.setAntiAlias(true);
            switch (fPaintOverrides.fAntiAliasState) {
                case SkPaintFields::AntiAliasState::Alias:
                    fPaintOverrides.fAntiAliasState = SkPaintFields::AntiAliasState::Normal;
                    gSkUseAnalyticAA = gSkForceAnalyticAA = false;
                    break;
                case SkPaintFields::AntiAliasState::Normal:
                    fPaintOverrides.fAntiAliasState = SkPaintFields::AntiAliasState::AnalyticAAEnabled;
                    gSkUseAnalyticAA = true;
                    gSkForceAnalyticAA = false;
                    break;
                case SkPaintFields::AntiAliasState::AnalyticAAEnabled:
                    fPaintOverrides.fAntiAliasState = SkPaintFields::AntiAliasState::AnalyticAAForced;
                    gSkUseAnalyticAA = gSkForceAnalyticAA = true;
                    break;
                case SkPaintFields::AntiAliasState::AnalyticAAForced:
                    fPaintOverrides.fAntiAliasState = SkPaintFields::AntiAliasState::Alias;
                    fPaintOverrides.fAntiAlias = false;
                    gSkUseAnalyticAA = fPaintOverrides.fOriginalSkUseAnalyticAA;
                    gSkForceAnalyticAA = fPaintOverrides.fOriginalSkForceAnalyticAA;
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
    fCommands.addCommand('L', "Font", "Subpixel Antialias Mode", [this]() {
        if (!fFontOverrides.fEdging) {
            fFontOverrides.fEdging = true;
            fFont.setEdging(SkFont::Edging::kAlias);
        } else {
            switch (fFont.getEdging()) {
                case SkFont::Edging::kAlias:
                    fFont.setEdging(SkFont::Edging::kAntiAlias);
                    break;
                case SkFont::Edging::kAntiAlias:
                    fFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                    break;
                case SkFont::Edging::kSubpixelAntiAlias:
                    fFont.setEdging(SkFont::Edging::kAlias);
                    fFontOverrides.fEdging = false;
                    break;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('S', "Font", "Subpixel Position Mode", [this]() {
        if (!fFontOverrides.fSubpixel) {
            fFontOverrides.fSubpixel = true;
            fFont.setSubpixel(false);
        } else {
            if (!fFont.isSubpixel()) {
                fFont.setSubpixel(true);
            } else {
                fFontOverrides.fSubpixel = false;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('B', "Font", "Baseline Snapping", [this]() {
        if (!fFontOverrides.fBaselineSnap) {
            fFontOverrides.fBaselineSnap = true;
            fFont.setBaselineSnap(false);
        } else {
            if (!fFont.isBaselineSnap()) {
                fFont.setBaselineSnap(true);
            } else {
                fFontOverrides.fBaselineSnap = false;
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
    fCommands.addCommand('a', "Transform", "Toggle Animation", [this]() {
        fAnimTimer.togglePauseResume();
    });
    fCommands.addCommand('u', "GUI", "Zoom UI", [this]() {
        fZoomUI = !fZoomUI;
        fStatsLayer.setDisplayScale((fZoomUI ? 2.0f : 1.0f) * fWindow->scaleFactor());
        fWindow->inval();
    });
    fCommands.addCommand('$', "ViaSerialize", "Toggle ViaSerialize", [this]() {
        fDrawViaSerialize = !fDrawViaSerialize;
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('!', "SkVM", "Toggle SkVM blitter", [this]() {
        gUseSkVMBlitter = !gUseSkVMBlitter;
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('@', "SkVM", "Toggle SkVM JIT", [this]() {
        gSkVMAllowJIT = !gSkVMAllowJIT;
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
        fImGuiGamutPaint.setShader(gamutImage->makeShader(SkSamplingOptions(SkFilterMode::kLinear)));
    }
    fImGuiGamutPaint.setColor(SK_ColorWHITE);

    fWindow->attach(backend_type_for_window(fBackendType));
    this->setCurrentSlide(this->startupSlide());
}

void Viewer::initSlides() {
    using SlideFactory = sk_sp<Slide>(*)(const SkString& name, const SkString& path);
    static const struct {
        const char*                            fExtension;
        const char*                            fDirName;
        const CommandLineFlags::StringArray&   fFlags;
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
        { ".json", "skottie-dir", FLAGS_lotties,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SkottieSlide>(name, path);}
        },
#endif
    #if defined(SK_ENABLE_SKRIVE)
            { ".flr", "skrive-dir", FLAGS_rives,
                [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                    return sk_make_sp<SkRiveSlide>(name, path);}
            },
    #endif
#if defined(SK_XML)
        { ".svg", "svg-dir", FLAGS_svgs,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SvgSlide>(name, path);}
        },
#endif
    };

    SkTArray<sk_sp<Slide>> dirSlides;

    const auto addSlide =
            [&](const SkString& name, const SkString& path, const SlideFactory& fact) {
                if (CommandLineFlags::ShouldSkip(FLAGS_match, name.c_str())) {
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
        if (bisect && !CommandLineFlags::ShouldSkip(FLAGS_match, bisect->getName().c_str())) {
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
    for (skiagm::GMFactory gmFactory : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm = gmFactory();
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, gm->getName())) {
            sk_sp<Slide> slide(new GMSlide(std::move(gm)));
            fSlides.push_back(std::move(slide));
        }
    }
    // reverse gms
    int numGMs = fSlides.count() - firstGM;
    for (int i = 0; i < numGMs/2; ++i) {
        std::swap(fSlides[firstGM + i], fSlides[fSlides.count() - i - 1]);
    }

    // samples
    for (const SampleFactory factory : SampleRegistry::Range()) {
        sk_sp<Slide> slide(new SampleSlide(factory));
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, slide->getName().c_str())) {
            fSlides.push_back(slide);
        }
    }

    // Particle demo
    {
        // TODO: Convert this to a sample
        sk_sp<Slide> slide(new ParticlesSlide());
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, slide->getName().c_str())) {
            fSlides.push_back(std::move(slide));
        }
    }

    // Runtime shader editor
    {
        sk_sp<Slide> slide(new SkSLSlide());
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, slide->getName().c_str())) {
            fSlides.push_back(std::move(slide));
        }
    }

    for (const auto& info : gExternalSlidesInfo) {
        for (const auto& flag : info.fFlags) {
            if (SkStrEndsWith(flag.c_str(), info.fExtension)) {
                // single file
                addSlide(SkOSPath::Basename(flag.c_str()), flag, info.fFactory);
            } else {
                // directory
                SkString name;
                SkTArray<SkString> sortedFilenames;
                SkOSFile::Iter it(flag.c_str(), info.fExtension);
                while (it.next(&name)) {
                    sortedFilenames.push_back(name);
                }
                if (sortedFilenames.count()) {
                    SkTQSort(sortedFilenames.begin(), sortedFilenames.end(),
                             [](const SkString& a, const SkString& b) {
                                 return strcmp(a.c_str(), b.c_str()) < 0;
                             });
                }
                for (const SkString& filename : sortedFilenames) {
                    addSlide(filename, SkOSPath::Join(flag.c_str(), filename.c_str()),
                             info.fFactory);
                }
            }
            if (!dirSlides.empty()) {
                fSlides.push_back(
                    sk_make_sp<SlideDir>(SkStringPrintf("%s[%s]", info.fDirName, flag.c_str()),
                                         std::move(dirSlides)));
                dirSlides.reset();  // NOLINT(bugprone-use-after-move)
            }
        }
    }

    if (!fSlides.count()) {
        sk_sp<Slide> slide(new NullSlide());
        fSlides.push_back(std::move(slide));
    }
}


Viewer::~Viewer() {
    for(auto& slide : fSlides) {
        slide->gpuTeardown();
    }

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

    if (gSkUseAnalyticAA) {
        if (gSkForceAnalyticAA) {
            title.append(" <FAAA>");
        } else {
            title.append(" <AAA>");
        }
    }
    if (fDrawViaSerialize) {
        title.append(" <serialize>");
    }
    if (gUseSkVMBlitter) {
        title.append(" <SkVMBlitter>");
    }
    if (!gSkVMAllowJIT) {
        title.append(" <SkVM interpreter>");
    }

    SkPaintTitleUpdater paintTitle(&title);
    auto paintFlag = [this, &paintTitle](bool SkPaintFields::* flag,
                                         bool (SkPaint::* isFlag)() const,
                                         const char* on, const char* off)
    {
        if (fPaintOverrides.*flag) {
            paintTitle.append((fPaint.*isFlag)() ? on : off);
        }
    };

    auto fontFlag = [this, &paintTitle](bool SkFontFields::* flag, bool (SkFont::* isFlag)() const,
                                        const char* on, const char* off)
    {
        if (fFontOverrides.*flag) {
            paintTitle.append((fFont.*isFlag)() ? on : off);
        }
    };

    paintFlag(&SkPaintFields::fAntiAlias, &SkPaint::isAntiAlias, "Antialias", "Alias");
    paintFlag(&SkPaintFields::fDither, &SkPaint::isDither, "DITHER", "No Dither");

    fontFlag(&SkFontFields::fForceAutoHinting, &SkFont::isForceAutoHinting,
             "Force Autohint", "No Force Autohint");
    fontFlag(&SkFontFields::fEmbolden, &SkFont::isEmbolden, "Fake Bold", "No Fake Bold");
    fontFlag(&SkFontFields::fBaselineSnap, &SkFont::isBaselineSnap, "BaseSnap", "No BaseSnap");
    fontFlag(&SkFontFields::fLinearMetrics, &SkFont::isLinearMetrics,
             "Linear Metrics", "Non-Linear Metrics");
    fontFlag(&SkFontFields::fEmbeddedBitmaps, &SkFont::isEmbeddedBitmaps,
             "Bitmap Text", "No Bitmap Text");
    fontFlag(&SkFontFields::fSubpixel, &SkFont::isSubpixel, "Subpixel Text", "Pixel Text");

    if (fFontOverrides.fEdging) {
        switch (fFont.getEdging()) {
            case SkFont::Edging::kAlias:
                paintTitle.append("Alias Text");
                break;
            case SkFont::Edging::kAntiAlias:
                paintTitle.append("Antialias Text");
                break;
            case SkFont::Edging::kSubpixelAntiAlias:
                paintTitle.append("Subpixel Antialias Text");
                break;
        }
    }

    if (fFontOverrides.fHinting) {
        switch (fFont.getHinting()) {
            case SkFontHinting::kNone:
                paintTitle.append("No Hinting");
                break;
            case SkFontHinting::kSlight:
                paintTitle.append("Slight Hinting");
                break;
            case SkFontHinting::kNormal:
                paintTitle.append("Normal Hinting");
                break;
            case SkFontHinting::kFull:
                paintTitle.append("Full Hinting");
                break;
        }
    }
    paintTitle.done();

    switch (fColorMode) {
        case ColorMode::kLegacy:
            title.append(" Legacy 8888");
            break;
        case ColorMode::kColorManaged8888:
            title.append(" ColorManaged 8888");
            break;
        case ColorMode::kColorManagedF16:
            title.append(" ColorManaged F16");
            break;
        case ColorMode::kColorManagedF16Norm:
            title.append(" ColorManaged F16 Norm");
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
        title.appendf(" %s Gamma %f",
                      curPrimaries >= 0 ? gNamedPrimaries[curPrimaries].fName : "Custom",
                      fColorSpaceTransferFn.g);
    }

    const DisplayParams& params = fWindow->getRequestedDisplayParams();
    if (fDisplayOverrides.fSurfaceProps.fPixelGeometry) {
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
    if (GpuPathRenderers::kDefault != pr) {
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

    SkScalar scaleFactor = 1.0;
    if (fApplyBackingScale) {
        scaleFactor = fWindow->scaleFactor();
    }
    fSlides[slide]->load(SkIntToScalar(fWindow->width()) / scaleFactor,
                         SkIntToScalar(fWindow->height()) / scaleFactor);
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
                fDefaultMatrix = SkMatrix::RectToRect(slideBounds, windowRect,
                                                      SkMatrix::kStart_ScaleToFit);
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

#define MAX_ZOOM_LEVEL  8.0f
#define MIN_ZOOM_LEVEL  -8.0f

void Viewer::changeZoomLevel(float delta) {
    fZoomLevel += delta;
    fZoomLevel = SkTPin(fZoomLevel, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);
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

    SkScalar zoomScale = exp(fZoomLevel);
    if (fApplyBackingScale) {
        zoomScale *= fWindow->scaleFactor();
    }
    m.preTranslate((fOffset.x() - 0.5f) * 2.0f, (fOffset.y() - 0.5f) * 2.0f);
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
    fPersistentCache.reset();
    fCachedShaders.reset();
    fBackendType = backendType;

    // The active context is going away in 'detach'
    for(auto& slide : fSlides) {
        slide->gpuTeardown();
    }

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
    this->updateTitle();
    fWindow->inval();
}

class OveridePaintFilterCanvas : public SkPaintFilterCanvas {
public:
    OveridePaintFilterCanvas(SkCanvas* canvas, SkPaint* paint, Viewer::SkPaintFields* pfields,
            SkFont* font, Viewer::SkFontFields* ffields)
        : SkPaintFilterCanvas(canvas), fPaint(paint), fPaintOverrides(pfields), fFont(font), fFontOverrides(ffields)
    { }
    const SkTextBlob* filterTextBlob(const SkPaint& paint, const SkTextBlob* blob,
                                     sk_sp<SkTextBlob>* cache) {
        bool blobWillChange = false;
        for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
            SkTCopyOnFirstWrite<SkFont> filteredFont(it.font());
            bool shouldDraw = this->filterFont(&filteredFont);
            if (it.font() != *filteredFont || !shouldDraw) {
                blobWillChange = true;
                break;
            }
        }
        if (!blobWillChange) {
            return blob;
        }

        SkTextBlobBuilder builder;
        for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
            SkTCopyOnFirstWrite<SkFont> filteredFont(it.font());
            bool shouldDraw = this->filterFont(&filteredFont);
            if (!shouldDraw) {
                continue;
            }

            SkFont font = *filteredFont;

            const SkTextBlobBuilder::RunBuffer& runBuffer
                = it.positioning() == SkTextBlobRunIterator::kDefault_Positioning
                    ? builder.allocRunText(font, it.glyphCount(), it.offset().x(),it.offset().y(),
                                           it.textSize())
                : it.positioning() == SkTextBlobRunIterator::kHorizontal_Positioning
                    ? builder.allocRunTextPosH(font, it.glyphCount(), it.offset().y(),
                                               it.textSize())
                : it.positioning() == SkTextBlobRunIterator::kFull_Positioning
                    ? builder.allocRunTextPos(font, it.glyphCount(), it.textSize())
                : it.positioning() == SkTextBlobRunIterator::kRSXform_Positioning
                    ? builder.allocRunTextRSXform(font, it.glyphCount(), it.textSize())
                : (SkASSERT_RELEASE(false), SkTextBlobBuilder::RunBuffer());
            uint32_t glyphCount = it.glyphCount();
            if (it.glyphs()) {
                size_t glyphSize = sizeof(decltype(*it.glyphs()));
                memcpy(runBuffer.glyphs, it.glyphs(), glyphCount * glyphSize);
            }
            if (it.pos()) {
                size_t posSize = sizeof(decltype(*it.pos()));
                unsigned posPerGlyph = it.scalarsPerGlyph();
                memcpy(runBuffer.pos, it.pos(), glyphCount * posPerGlyph * posSize);
            }
            if (it.text()) {
                size_t textSize = sizeof(decltype(*it.text()));
                uint32_t textCount = it.textSize();
                memcpy(runBuffer.utf8text, it.text(), textCount * textSize);
            }
            if (it.clusters()) {
                size_t clusterSize = sizeof(decltype(*it.clusters()));
                memcpy(runBuffer.clusters, it.clusters(), glyphCount * clusterSize);
            }
        }
        *cache = builder.make();
        return cache->get();
    }
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        sk_sp<SkTextBlob> cache;
        this->SkPaintFilterCanvas::onDrawTextBlob(
            this->filterTextBlob(paint, blob, &cache), x, y, paint);
    }
    bool filterFont(SkTCopyOnFirstWrite<SkFont>* font) const {
        if (fFontOverrides->fSize) {
            font->writable()->setSize(fFont->getSize());
        }
        if (fFontOverrides->fScaleX) {
            font->writable()->setScaleX(fFont->getScaleX());
        }
        if (fFontOverrides->fSkewX) {
            font->writable()->setSkewX(fFont->getSkewX());
        }
        if (fFontOverrides->fHinting) {
            font->writable()->setHinting(fFont->getHinting());
        }
        if (fFontOverrides->fEdging) {
            font->writable()->setEdging(fFont->getEdging());
        }
        if (fFontOverrides->fEmbolden) {
            font->writable()->setEmbolden(fFont->isEmbolden());
        }
        if (fFontOverrides->fBaselineSnap) {
            font->writable()->setBaselineSnap(fFont->isBaselineSnap());
        }
        if (fFontOverrides->fLinearMetrics) {
            font->writable()->setLinearMetrics(fFont->isLinearMetrics());
        }
        if (fFontOverrides->fSubpixel) {
            font->writable()->setSubpixel(fFont->isSubpixel());
        }
        if (fFontOverrides->fEmbeddedBitmaps) {
            font->writable()->setEmbeddedBitmaps(fFont->isEmbeddedBitmaps());
        }
        if (fFontOverrides->fForceAutoHinting) {
            font->writable()->setForceAutoHinting(fFont->isForceAutoHinting());
        }

        return true;
    }
    bool onFilter(SkPaint& paint) const override {
        if (fPaintOverrides->fAntiAlias) {
            paint.setAntiAlias(fPaint->isAntiAlias());
        }
        if (fPaintOverrides->fDither) {
            paint.setDither(fPaint->isDither());
        }
        if (fPaintOverrides->fStyle) {
            paint.setStyle(fPaint->getStyle());
        }
        if (fPaintOverrides->fWidth) {
            paint.setStrokeWidth(fPaint->getStrokeWidth());
        }
        if (fPaintOverrides->fMiterLimit) {
            paint.setStrokeMiter(fPaint->getStrokeMiter());
        }
        if (fPaintOverrides->fCapType) {
            paint.setStrokeCap(fPaint->getStrokeCap());
        }
        if (fPaintOverrides->fJoinType) {
            paint.setStrokeJoin(fPaint->getStrokeJoin());
        }
        return true;
    }
    SkPaint* fPaint;
    Viewer::SkPaintFields* fPaintOverrides;
    SkFont* fFont;
    Viewer::SkFontFields* fFontOverrides;
};

void Viewer::drawSlide(SkSurface* surface) {
    if (fCurrentSlide < 0) {
        return;
    }

    SkAutoCanvasRestore autorestore(surface->getCanvas(), false);

    // By default, we render directly into the window's surface/canvas
    SkSurface* slideSurface = surface;
    SkCanvas* slideCanvas = surface->getCanvas();
    fLastImage.reset();

    // If we're in any of the color managed modes, construct the color space we're going to use
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (ColorMode::kLegacy != fColorMode) {
        skcms_Matrix3x3 toXYZ;
        SkAssertResult(fColorSpacePrimaries.toXYZD50(&toXYZ));
        colorSpace = SkColorSpace::MakeRGB(fColorSpaceTransferFn, toXYZ);
    }

    if (fSaveToSKP) {
        SkPictureRecorder recorder;
        SkCanvas* recorderCanvas = recorder.beginRecording(
                SkRect::Make(fSlides[fCurrentSlide]->getDimensions()));
        fSlides[fCurrentSlide]->draw(recorderCanvas);
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
        SkFILEWStream stream("sample_app.skp");
        picture->serialize(&stream);
        fSaveToSKP = false;
    }

    // Grab some things we'll need to make surfaces (for tiling or general offscreen rendering)
    SkColorType colorType;
    switch (fColorMode) {
        case ColorMode::kLegacy:
        case ColorMode::kColorManaged8888:
            colorType = kN32_SkColorType;
            break;
        case ColorMode::kColorManagedF16:
            colorType = kRGBA_F16_SkColorType;
            break;
        case ColorMode::kColorManagedF16Norm:
            colorType = kRGBA_F16Norm_SkColorType;
            break;
    }

    auto make_surface = [=](int w, int h) {
        SkSurfaceProps props(fWindow->getRequestedDisplayParams().fSurfaceProps);
        slideCanvas->getProps(&props);

        SkImageInfo info = SkImageInfo::Make(w, h, colorType, kPremul_SkAlphaType, colorSpace);
        return Window::kRaster_BackendType == this->fBackendType
                ? SkSurface::MakeRaster(info, &props)
                : slideCanvas->makeSurface(info, &props);
    };

    // We need to render offscreen if we're...
    // ... in fake perspective or zooming (so we have a snapped copy of the results)
    // ... in any raster mode, because the window surface is actually GL
    // ... in any color managed mode, because we always make the window surface with no color space
    // ... or if the user explicitly requested offscreen rendering
    sk_sp<SkSurface> offscreenSurface = nullptr;
    if (kPerspective_Fake == fPerspectiveMode ||
        fShowZoomWindow ||
        Window::kRaster_BackendType == fBackendType ||
        colorSpace != nullptr ||
        FLAGS_offscreen) {

        offscreenSurface = make_surface(fWindow->width(), fWindow->height());
        slideSurface = offscreenSurface.get();
        slideCanvas = offscreenSurface->getCanvas();
    }

    SkPictureRecorder recorder;
    SkCanvas* recorderRestoreCanvas = nullptr;
    if (fDrawViaSerialize) {
        recorderRestoreCanvas = slideCanvas;
        slideCanvas = recorder.beginRecording(
                SkRect::Make(fSlides[fCurrentSlide]->getDimensions()));
    }

    int count = slideCanvas->save();
    slideCanvas->clear(SK_ColorWHITE);
    // Time the painting logic of the slide
    fStatsLayer.beginTiming(fPaintTimer);
    if (fTiled) {
        int tileW = SkScalarCeilToInt(fWindow->width() * fTileScale.width());
        int tileH = SkScalarCeilToInt(fWindow->height() * fTileScale.height());
        for (int y = 0; y < fWindow->height(); y += tileH) {
            for (int x = 0; x < fWindow->width(); x += tileW) {
                SkAutoCanvasRestore acr(slideCanvas, true);
                slideCanvas->clipRect(SkRect::MakeXYWH(x, y, tileW, tileH));
                fSlides[fCurrentSlide]->draw(slideCanvas);
            }
        }

        // Draw borders between tiles
        if (fDrawTileBoundaries) {
            SkPaint border;
            border.setColor(0x60FF00FF);
            border.setStyle(SkPaint::kStroke_Style);
            for (int y = 0; y < fWindow->height(); y += tileH) {
                for (int x = 0; x < fWindow->width(); x += tileW) {
                    slideCanvas->drawRect(SkRect::MakeXYWH(x, y, tileW, tileH), border);
                }
            }
        }
    } else {
        slideCanvas->concat(this->computeMatrix());
        if (kPerspective_Real == fPerspectiveMode) {
            slideCanvas->clipRect(SkRect::MakeWH(fWindow->width(), fWindow->height()));
        }
        OveridePaintFilterCanvas filterCanvas(slideCanvas, &fPaint, &fPaintOverrides, &fFont, &fFontOverrides);
        fSlides[fCurrentSlide]->draw(&filterCanvas);
    }
    fStatsLayer.endTiming(fPaintTimer);
    slideCanvas->restoreToCount(count);

    if (recorderRestoreCanvas) {
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
        auto data = picture->serialize();
        slideCanvas = recorderRestoreCanvas;
        slideCanvas->drawPicture(SkPicture::MakeFromData(data.get()));
    }

    // Force a flush so we can time that, too
    fStatsLayer.beginTiming(fFlushTimer);
    slideSurface->flushAndSubmit();
    fStatsLayer.endTiming(fFlushTimer);

    // If we rendered offscreen, snap an image and push the results to the window's canvas
    if (offscreenSurface) {
        fLastImage = offscreenSurface->makeImageSnapshot();

        SkCanvas* canvas = surface->getCanvas();
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        SkSamplingOptions sampling;
        int prePerspectiveCount = canvas->save();
        if (kPerspective_Fake == fPerspectiveMode) {
            sampling = SkSamplingOptions({1.0f/3, 1.0f/3});
            canvas->clear(SK_ColorWHITE);
            canvas->concat(this->computePerspectiveMatrix());
        }
        canvas->drawImage(fLastImage, 0, 0, sampling, &paint);
        canvas->restoreToCount(prePerspectiveCount);
    }

    if (fShowSlideDimensions) {
        SkRect r = SkRect::Make(fSlides[fCurrentSlide]->getDimensions());
        SkPaint paint;
        paint.setColor(0x40FFFF00);
        surface->getCanvas()->drawRect(r, paint);
    }
}

void Viewer::onBackendCreated() {
    this->setupCurrentSlide();
    fWindow->show();
}

void Viewer::onPaint(SkSurface* surface) {
    this->drawSlide(surface);

    fCommands.drawHelp(surface->getCanvas());

    this->drawImGui();

    fLastImage.reset();

    if (auto direct = fWindow->directContext()) {
        // Clean out cache items that haven't been used in more than 10 seconds.
        direct->performDeferredCleanup(std::chrono::seconds(10));
    }
}

void Viewer::onResize(int width, int height) {
    if (fCurrentSlide >= 0) {
        SkScalar scaleFactor = 1.0;
        if (fApplyBackingScale) {
            scaleFactor = fWindow->scaleFactor();
        }
        fSlides[fCurrentSlide]->resize(width / scaleFactor, height / scaleFactor);
    }
}

SkPoint Viewer::mapEvent(float x, float y) {
    const auto m = this->computeMatrix();
    SkMatrix inv;

    SkAssertResult(m.invert(&inv));

    return inv.mapXY(x, y);
}

bool Viewer::onTouch(intptr_t owner, skui::InputState state, float x, float y) {
    if (GestureDevice::kMouse == fGestureDevice) {
        return false;
    }

    const auto slidePt = this->mapEvent(x, y);
    if (fSlides[fCurrentSlide]->onMouse(slidePt.x(), slidePt.y(), state, skui::ModifierKey::kNone)) {
        fWindow->inval();
        return true;
    }

    void* castedOwner = reinterpret_cast<void*>(owner);
    switch (state) {
        case skui::InputState::kUp: {
            fGesture.touchEnd(castedOwner);
#if defined(SK_BUILD_FOR_IOS)
            // TODO: move IOS swipe detection higher up into the platform code
            SkPoint dir;
            if (fGesture.isFling(&dir)) {
                // swiping left or right
                if (SkTAbs(dir.fX) > SkTAbs(dir.fY)) {
                    if (dir.fX < 0) {
                        this->setCurrentSlide(fCurrentSlide < fSlides.count() - 1 ?
                                              fCurrentSlide + 1 : 0);
                    } else {
                        this->setCurrentSlide(fCurrentSlide > 0 ?
                                              fCurrentSlide - 1 : fSlides.count() - 1);
                    }
                }
                fGesture.reset();
            }
#endif
            break;
        }
        case skui::InputState::kDown: {
            fGesture.touchBegin(castedOwner, x, y);
            break;
        }
        case skui::InputState::kMove: {
            fGesture.touchMoved(castedOwner, x, y);
            break;
        }
        default: {
            // kLeft and kRight are only for swipes
            SkASSERT(false);
            break;
        }
    }
    fGestureDevice = fGesture.isBeingTouched() ? GestureDevice::kTouch : GestureDevice::kNone;
    fWindow->inval();
    return true;
}

bool Viewer::onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) {
    if (GestureDevice::kTouch == fGestureDevice) {
        return false;
    }

    const auto slidePt = this->mapEvent(x, y);
    if (fSlides[fCurrentSlide]->onMouse(slidePt.x(), slidePt.y(), state, modifiers)) {
        fWindow->inval();
        return true;
    }

    switch (state) {
        case skui::InputState::kUp: {
            fGesture.touchEnd(nullptr);
            break;
        }
        case skui::InputState::kDown: {
            fGesture.touchBegin(nullptr, x, y);
            break;
        }
        case skui::InputState::kMove: {
            fGesture.touchMoved(nullptr, x, y);
            break;
        }
        default: {
            SkASSERT(false); // shouldn't see kRight or kLeft here
            break;
        }
    }
    fGestureDevice = fGesture.isBeingTouched() ? GestureDevice::kMouse : GestureDevice::kNone;

    if (state != skui::InputState::kMove || fGesture.isBeingTouched()) {
        fWindow->inval();
    }
    return true;
}

bool Viewer::onFling(skui::InputState state) {
    if (skui::InputState::kRight == state) {
        this->setCurrentSlide(fCurrentSlide > 0 ? fCurrentSlide - 1 : fSlides.count() - 1);
        return true;
    } else if (skui::InputState::kLeft == state) {
        this->setCurrentSlide(fCurrentSlide < fSlides.count() - 1 ? fCurrentSlide + 1 : 0);
        return true;
    }
    return false;
}

bool Viewer::onPinch(skui::InputState state, float scale, float x, float y) {
    switch (state) {
        case skui::InputState::kDown:
            fGesture.startZoom();
            return true;
            break;
        case skui::InputState::kMove:
            fGesture.updateZoom(scale, x, y, x, y);
            return true;
            break;
        case skui::InputState::kUp:
            fGesture.endZoom();
            return true;
            break;
        default:
            SkASSERT(false);
            break;
    }

    return false;
}

static void ImGui_Primaries(SkColorSpacePrimaries* primaries, SkPaint* gamutPaint) {
    // The gamut image covers a (0.8 x 0.9) shaped region
    ImGui::DragCanvas dc(primaries, { 0.0f, 0.9f }, { 0.8f, 0.0f });

    // Background image. Only draw a subset of the image, to avoid the regions less than zero.
    // Simplifes re-mapping math, clipping behavior, and increases resolution in the useful area.
    // Magic numbers are pixel locations of the origin and upper-right corner.
    dc.fDrawList->AddImage(gamutPaint, dc.fPos,
                           ImVec2(dc.fPos.x + dc.fSize.x, dc.fPos.y + dc.fSize.y),
                           ImVec2(242, 61), ImVec2(1897, 1922));

    dc.dragPoint((SkPoint*)(&primaries->fRX), true, 0xFF000040);
    dc.dragPoint((SkPoint*)(&primaries->fGX), true, 0xFF004000);
    dc.dragPoint((SkPoint*)(&primaries->fBX), true, 0xFF400000);
    dc.dragPoint((SkPoint*)(&primaries->fWX), true);
    dc.fDrawList->AddPolyline(dc.fScreenPoints.begin(), 3, 0xFFFFFFFF, true, 1.5f);
}

static bool ImGui_DragLocation(SkPoint* pt) {
    ImGui::DragCanvas dc(pt);
    dc.fillColor(IM_COL32(0, 0, 0, 128));
    dc.dragPoint(pt);
    return dc.fDragging;
}

static bool ImGui_DragQuad(SkPoint* pts) {
    ImGui::DragCanvas dc(pts);
    dc.fillColor(IM_COL32(0, 0, 0, 128));

    for (int i = 0; i < 4; ++i) {
        dc.dragPoint(pts + i);
    }

    dc.fDrawList->AddLine(dc.fScreenPoints[0], dc.fScreenPoints[1], 0xFFFFFFFF);
    dc.fDrawList->AddLine(dc.fScreenPoints[1], dc.fScreenPoints[3], 0xFFFFFFFF);
    dc.fDrawList->AddLine(dc.fScreenPoints[3], dc.fScreenPoints[2], 0xFFFFFFFF);
    dc.fDrawList->AddLine(dc.fScreenPoints[2], dc.fScreenPoints[0], 0xFFFFFFFF);

    return dc.fDragging;
}

static SkSL::String build_sksl_highlight_shader() {
    return SkSL::String("out half4 sk_FragColor;\n"
                        "void main() { sk_FragColor = half4(1, 0, 1, 0.5); }");
}

static SkSL::String build_metal_highlight_shader(const SkSL::String& inShader) {
    // Metal fragment shaders need a lot of non-trivial boilerplate that we don't want to recompute
    // here. So keep all shader code, but right before `return *_out;`, swap out the sk_FragColor.
    size_t pos = inShader.rfind("return *_out;\n");
    if (pos == std::string::npos) {
        return inShader;
    }

    SkSL::String replacementShader = inShader;
    replacementShader.insert(pos, "_out->sk_FragColor = float4(1.0, 0.0, 1.0, 0.5); ");
    return replacementShader;
}

static SkSL::String build_glsl_highlight_shader(const GrShaderCaps& shaderCaps) {
    const char* versionDecl = shaderCaps.versionDeclString();
    SkSL::String highlight = versionDecl ? versionDecl : "";
    if (shaderCaps.usesPrecisionModifiers()) {
        highlight.append("precision mediump float;\n");
    }
    highlight.appendf("out vec4 sk_FragColor;\n"
                      "void main() { sk_FragColor = vec4(1, 0, 1, 0.5); }");
    return highlight;
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
        auto ctx = fWindow->directContext();

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
#if defined(SK_DAWN)
                ImGui::SameLine();
                ImGui::RadioButton("Dawn", &newBackend, sk_app::Window::kDawn_BackendType);
#endif
#if defined(SK_VULKAN) && !defined(SK_BUILD_FOR_MAC)
                ImGui::SameLine();
                ImGui::RadioButton("Vulkan", &newBackend, sk_app::Window::kVulkan_BackendType);
#endif
#if defined(SK_METAL)
                ImGui::SameLine();
                ImGui::RadioButton("Metal", &newBackend, sk_app::Window::kMetal_BackendType);
#endif
#if defined(SK_DIRECT3D)
                ImGui::SameLine();
                ImGui::RadioButton("Direct3D", &newBackend, sk_app::Window::kDirect3D_BackendType);
#endif
                if (newBackend != fBackendType) {
                    fDeferredActions.push_back([=]() {
                        this->setBackend(static_cast<sk_app::Window::BackendType>(newBackend));
                    });
                }

                bool* wire = &params.fGrContextOptions.fWireframeMode;
                if (ctx && ImGui::Checkbox("Wireframe Mode", wire)) {
                    paramsChanged = true;
                }

                if (ctx) {
                    // Determine the context's max sample count for MSAA radio buttons.
                    int sampleCount = fWindow->sampleCount();
                    int maxMSAA = (fBackendType != sk_app::Window::kRaster_BackendType) ?
                            ctx->maxSurfaceSampleCountForColorType(kRGBA_8888_SkColorType) :
                            1;

                    // Only display the MSAA radio buttons when there are options above 1x MSAA.
                    if (maxMSAA >= 4) {
                        ImGui::Text("MSAA: ");

                        for (int curMSAA = 1; curMSAA <= maxMSAA; curMSAA *= 2) {
                            // 2x MSAA works, but doesn't offer much of a visual improvement, so we
                            // don't show it in the list.
                            if (curMSAA == 2) {
                                continue;
                            }
                            ImGui::SameLine();
                            ImGui::RadioButton(SkStringPrintf("%d", curMSAA).c_str(),
                                               &sampleCount, curMSAA);
                        }
                    }

                    if (sampleCount != params.fMSAASampleCount) {
                        params.fMSAASampleCount = sampleCount;
                        paramsChanged = true;
                    }
                }

                int pixelGeometryIdx = 0;
                if (fDisplayOverrides.fSurfaceProps.fPixelGeometry) {
                    pixelGeometryIdx = params.fSurfaceProps.pixelGeometry() + 1;
                }
                if (ImGui::Combo("Pixel Geometry", &pixelGeometryIdx,
                                 "Default\0Flat\0RGB\0BGR\0RGBV\0BGRV\0\0"))
                {
                    uint32_t flags = params.fSurfaceProps.flags();
                    if (pixelGeometryIdx == 0) {
                        fDisplayOverrides.fSurfaceProps.fPixelGeometry = false;
                        SkPixelGeometry pixelGeometry = fDisplay.fSurfaceProps.pixelGeometry();
                        params.fSurfaceProps = SkSurfaceProps(flags, pixelGeometry);
                    } else {
                        fDisplayOverrides.fSurfaceProps.fPixelGeometry = true;
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
                    } else {
                        const auto* caps = ctx->priv().caps();
                        prButton(GpuPathRenderers::kDefault);
                        if (fWindow->sampleCount() > 1 || caps->mixedSamplesSupport()) {
                            if (GrTessellationPathRenderer::IsSupported(*caps)) {
                                prButton(GpuPathRenderers::kTessellation);
                            }
                            if (caps->shaderCaps()->pathRenderingSupport()) {
                                prButton(GpuPathRenderers::kStencilAndCover);
                            }
                        }
                        if (1 == fWindow->sampleCount()) {
                            if (GrCoverageCountingPathRenderer::IsSupported(*caps)) {
                                prButton(GpuPathRenderers::kCoverageCounting);
                            }
                            prButton(GpuPathRenderers::kSmall);
                        }
                        prButton(GpuPathRenderers::kTriangulating);
                        prButton(GpuPathRenderers::kNone);
                    }
                    ImGui::TreePop();
                }
            }

            if (ImGui::CollapsingHeader("Tiling")) {
                ImGui::Checkbox("Enable", &fTiled);
                ImGui::Checkbox("Draw Boundaries", &fDrawTileBoundaries);
                ImGui::SliderFloat("Horizontal", &fTileScale.fWidth, 0.1f, 1.0f);
                ImGui::SliderFloat("Vertical", &fTileScale.fHeight, 0.1f, 1.0f);
            }

            if (ImGui::CollapsingHeader("Transform")) {
                if (ImGui::Checkbox("Apply Backing Scale", &fApplyBackingScale)) {
                    this->preTouchMatrixChanged();
                    this->onResize(fWindow->width(), fWindow->height());
                    paramsChanged = true;
                }

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
                } else if (fOffset != SkVector{0.5f, 0.5f}) {
                    this->preTouchMatrixChanged();
                    paramsChanged = true;
                    fOffset = {0.5f, 0.5f};
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
                int aliasIdx = 0;
                if (fPaintOverrides.fAntiAlias) {
                    aliasIdx = SkTo<int>(fPaintOverrides.fAntiAliasState) + 1;
                }
                if (ImGui::Combo("Anti-Alias", &aliasIdx,
                                 "Default\0Alias\0Normal\0AnalyticAAEnabled\0AnalyticAAForced\0\0"))
                {
                    gSkUseAnalyticAA = fPaintOverrides.fOriginalSkUseAnalyticAA;
                    gSkForceAnalyticAA = fPaintOverrides.fOriginalSkForceAnalyticAA;
                    if (aliasIdx == 0) {
                        fPaintOverrides.fAntiAliasState = SkPaintFields::AntiAliasState::Alias;
                        fPaintOverrides.fAntiAlias = false;
                    } else {
                        fPaintOverrides.fAntiAlias = true;
                        fPaintOverrides.fAntiAliasState = SkTo<SkPaintFields::AntiAliasState>(aliasIdx-1);
                        fPaint.setAntiAlias(aliasIdx > 1);
                        switch (fPaintOverrides.fAntiAliasState) {
                            case SkPaintFields::AntiAliasState::Alias:
                                break;
                            case SkPaintFields::AntiAliasState::Normal:
                                break;
                            case SkPaintFields::AntiAliasState::AnalyticAAEnabled:
                                gSkUseAnalyticAA = true;
                                gSkForceAnalyticAA = false;
                                break;
                            case SkPaintFields::AntiAliasState::AnalyticAAForced:
                                gSkUseAnalyticAA = gSkForceAnalyticAA = true;
                                break;
                        }
                    }
                    paramsChanged = true;
                }

                auto paintFlag = [this, &paramsChanged](const char* label, const char* items,
                                                        bool SkPaintFields::* flag,
                                                        bool (SkPaint::* isFlag)() const,
                                                        void (SkPaint::* setFlag)(bool) )
                {
                    int itemIndex = 0;
                    if (fPaintOverrides.*flag) {
                        itemIndex = (fPaint.*isFlag)() ? 2 : 1;
                    }
                    if (ImGui::Combo(label, &itemIndex, items)) {
                        if (itemIndex == 0) {
                            fPaintOverrides.*flag = false;
                        } else {
                            fPaintOverrides.*flag = true;
                            (fPaint.*setFlag)(itemIndex == 2);
                        }
                        paramsChanged = true;
                    }
                };

                paintFlag("Dither",
                          "Default\0No Dither\0Dither\0\0",
                          &SkPaintFields::fDither,
                          &SkPaint::isDither, &SkPaint::setDither);

                int styleIdx = 0;
                if (fPaintOverrides.fStyle) {
                    styleIdx = SkTo<int>(fPaint.getStyle()) + 1;
                }
                if (ImGui::Combo("Style", &styleIdx,
                                 "Default\0Fill\0Stroke\0Stroke and Fill\0\0"))
                {
                    if (styleIdx == 0) {
                        fPaintOverrides.fStyle = false;
                        fPaint.setStyle(SkPaint::kFill_Style);
                    } else {
                        fPaint.setStyle(SkTo<SkPaint::Style>(styleIdx - 1));
                        fPaintOverrides.fStyle = true;
                    }
                    paramsChanged = true;
                }

                ImGui::Checkbox("Override Stroke Width", &fPaintOverrides.fWidth);
                if (fPaintOverrides.fWidth) {
                    float width = fPaint.getStrokeWidth();
                    if (ImGui::SliderFloat("Stroke Width", &width, 0, 20)) {
                        fPaint.setStrokeWidth(width);
                        paramsChanged = true;
                    }
                }

                ImGui::Checkbox("Override Miter Limit", &fPaintOverrides.fMiterLimit);
                if (fPaintOverrides.fMiterLimit) {
                    float miterLimit = fPaint.getStrokeMiter();
                    if (ImGui::SliderFloat("Miter Limit", &miterLimit, 0, 20)) {
                        fPaint.setStrokeMiter(miterLimit);
                        paramsChanged = true;
                    }
                }

                int capIdx = 0;
                if (fPaintOverrides.fCapType) {
                    capIdx = SkTo<int>(fPaint.getStrokeCap()) + 1;
                }
                if (ImGui::Combo("Cap Type", &capIdx,
                                 "Default\0Butt\0Round\0Square\0\0"))
                {
                    if (capIdx == 0) {
                        fPaintOverrides.fCapType = false;
                        fPaint.setStrokeCap(SkPaint::kDefault_Cap);
                    } else {
                        fPaint.setStrokeCap(SkTo<SkPaint::Cap>(capIdx - 1));
                        fPaintOverrides.fCapType = true;
                    }
                    paramsChanged = true;
                }

                int joinIdx = 0;
                if (fPaintOverrides.fJoinType) {
                    joinIdx = SkTo<int>(fPaint.getStrokeJoin()) + 1;
                }
                if (ImGui::Combo("Join Type", &joinIdx,
                                 "Default\0Miter\0Round\0Bevel\0\0"))
                {
                    if (joinIdx == 0) {
                        fPaintOverrides.fJoinType = false;
                        fPaint.setStrokeJoin(SkPaint::kDefault_Join);
                    } else {
                        fPaint.setStrokeJoin(SkTo<SkPaint::Join>(joinIdx - 1));
                        fPaintOverrides.fJoinType = true;
                    }
                    paramsChanged = true;
                }
            }

            if (ImGui::CollapsingHeader("Font")) {
                int hintingIdx = 0;
                if (fFontOverrides.fHinting) {
                    hintingIdx = SkTo<int>(fFont.getHinting()) + 1;
                }
                if (ImGui::Combo("Hinting", &hintingIdx,
                                 "Default\0None\0Slight\0Normal\0Full\0\0"))
                {
                    if (hintingIdx == 0) {
                        fFontOverrides.fHinting = false;
                        fFont.setHinting(SkFontHinting::kNone);
                    } else {
                        fFont.setHinting(SkTo<SkFontHinting>(hintingIdx - 1));
                        fFontOverrides.fHinting = true;
                    }
                    paramsChanged = true;
                }

                auto fontFlag = [this, &paramsChanged](const char* label, const char* items,
                                                       bool SkFontFields::* flag,
                                                       bool (SkFont::* isFlag)() const,
                                                       void (SkFont::* setFlag)(bool) )
                {
                    int itemIndex = 0;
                    if (fFontOverrides.*flag) {
                        itemIndex = (fFont.*isFlag)() ? 2 : 1;
                    }
                    if (ImGui::Combo(label, &itemIndex, items)) {
                        if (itemIndex == 0) {
                            fFontOverrides.*flag = false;
                        } else {
                            fFontOverrides.*flag = true;
                            (fFont.*setFlag)(itemIndex == 2);
                        }
                        paramsChanged = true;
                    }
                };

                fontFlag("Fake Bold Glyphs",
                         "Default\0No Fake Bold\0Fake Bold\0\0",
                         &SkFontFields::fEmbolden,
                         &SkFont::isEmbolden, &SkFont::setEmbolden);

                fontFlag("Baseline Snapping",
                         "Default\0No Baseline Snapping\0Baseline Snapping\0\0",
                         &SkFontFields::fBaselineSnap,
                         &SkFont::isBaselineSnap, &SkFont::setBaselineSnap);

                fontFlag("Linear Text",
                         "Default\0No Linear Text\0Linear Text\0\0",
                         &SkFontFields::fLinearMetrics,
                         &SkFont::isLinearMetrics, &SkFont::setLinearMetrics);

                fontFlag("Subpixel Position Glyphs",
                         "Default\0Pixel Text\0Subpixel Text\0\0",
                         &SkFontFields::fSubpixel,
                         &SkFont::isSubpixel, &SkFont::setSubpixel);

                fontFlag("Embedded Bitmap Text",
                         "Default\0No Embedded Bitmaps\0Embedded Bitmaps\0\0",
                         &SkFontFields::fEmbeddedBitmaps,
                         &SkFont::isEmbeddedBitmaps, &SkFont::setEmbeddedBitmaps);

                fontFlag("Force Auto-Hinting",
                         "Default\0No Force Auto-Hinting\0Force Auto-Hinting\0\0",
                         &SkFontFields::fForceAutoHinting,
                         &SkFont::isForceAutoHinting, &SkFont::setForceAutoHinting);

                int edgingIdx = 0;
                if (fFontOverrides.fEdging) {
                    edgingIdx = SkTo<int>(fFont.getEdging()) + 1;
                }
                if (ImGui::Combo("Edging", &edgingIdx,
                                 "Default\0Alias\0Antialias\0Subpixel Antialias\0\0"))
                {
                    if (edgingIdx == 0) {
                        fFontOverrides.fEdging = false;
                        fFont.setEdging(SkFont::Edging::kAlias);
                    } else {
                        fFont.setEdging(SkTo<SkFont::Edging>(edgingIdx-1));
                        fFontOverrides.fEdging = true;
                    }
                    paramsChanged = true;
                }

                ImGui::Checkbox("Override Size", &fFontOverrides.fSize);
                if (fFontOverrides.fSize) {
                    ImGui::DragFloat2("TextRange", fFontOverrides.fSizeRange,
                                      0.001f, -10.0f, 300.0f, "%.6f", 2.0f);
                    float textSize = fFont.getSize();
                    if (ImGui::DragFloat("TextSize", &textSize, 0.001f,
                                         fFontOverrides.fSizeRange[0],
                                         fFontOverrides.fSizeRange[1],
                                         "%.6f", 2.0f))
                    {
                        fFont.setSize(textSize);
                        paramsChanged = true;
                    }
                }

                ImGui::Checkbox("Override ScaleX", &fFontOverrides.fScaleX);
                if (fFontOverrides.fScaleX) {
                    float scaleX = fFont.getScaleX();
                    if (ImGui::SliderFloat("ScaleX", &scaleX, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL)) {
                        fFont.setScaleX(scaleX);
                        paramsChanged = true;
                    }
                }

                ImGui::Checkbox("Override SkewX", &fFontOverrides.fSkewX);
                if (fFontOverrides.fSkewX) {
                    float skewX = fFont.getSkewX();
                    if (ImGui::SliderFloat("SkewX", &skewX, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL)) {
                        fFont.setSkewX(skewX);
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
                        while ((name = iter.next(&type, &count)) != nullptr) {
                            if (type == SkMetaData::kScalar_Type) {
                                float val[3];
                                SkASSERT(count == 3);
                                controls.findScalars(name, &count, val);
                                if (ImGui::SliderFloat(name, &val[0], val[1], val[2])) {
                                    controls.setScalars(name, 3, val);
                                }
                            } else if (type == SkMetaData::kBool_Type) {
                                bool val;
                                SkASSERT(count == 1);
                                controls.findBool(name, &val);
                                if (ImGui::Checkbox(name, &val)) {
                                    controls.setBool(name, val);
                                }
                            }
                        }
                        fSlides[fCurrentSlide]->onSetControls(controls);
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
                cmButton(ColorMode::kColorManaged8888, "Color Managed 8888");
                cmButton(ColorMode::kColorManagedF16, "Color Managed F16");
                cmButton(ColorMode::kColorManagedF16Norm, "Color Managed F16 Norm");

                if (newMode != fColorMode) {
                    this->setColorMode(newMode);
                }

                // Pick from common gamuts:
                int primariesIdx = 4; // Default: Custom
                for (size_t i = 0; i < SK_ARRAY_COUNT(gNamedPrimaries); ++i) {
                    if (primaries_equal(*gNamedPrimaries[i].fPrimaries, fColorSpacePrimaries)) {
                        primariesIdx = i;
                        break;
                    }
                }

                // Let user adjust the gamma
                ImGui::SliderFloat("Gamma", &fColorSpaceTransferFn.g, 0.5f, 3.5f);

                if (ImGui::Combo("Primaries", &primariesIdx,
                                 "sRGB\0AdobeRGB\0P3\0Rec. 2020\0Custom\0\0")) {
                    if (primariesIdx >= 0 && primariesIdx <= 3) {
                        fColorSpacePrimaries = *gNamedPrimaries[primariesIdx].fPrimaries;
                    }
                }

                // Allow direct editing of gamut
                ImGui_Primaries(&fColorSpacePrimaries, &fImGuiGamutPaint);
            }

            if (ImGui::CollapsingHeader("Animation")) {
                bool isPaused = AnimTimer::kPaused_State == fAnimTimer.state();
                if (ImGui::Checkbox("Pause", &isPaused)) {
                    fAnimTimer.togglePauseResume();
                }

                float speed = fAnimTimer.getSpeed();
                if (ImGui::DragFloat("Speed", &speed, 0.1f)) {
                    fAnimTimer.setSpeed(speed);
                }
            }

            if (ImGui::CollapsingHeader("Shaders")) {
                bool sksl = params.fGrContextOptions.fShaderCacheStrategy ==
                            GrContextOptions::ShaderCacheStrategy::kSkSL;

                int optLevel =                           sksl ? kShaderOptLevel_Source :
                                           SkSL::gSkSLInliner ? kShaderOptLevel_Inline :
                                         SkSL::gSkSLOptimizer ? kShaderOptLevel_Optimize :
                                                                kShaderOptLevel_Compile;

#if defined(SK_VULKAN)
                const bool isVulkan = fBackendType == sk_app::Window::kVulkan_BackendType;
#else
                const bool isVulkan = false;
#endif

                // To re-load shaders from the currently active programs, we flush all
                // caches on one frame, then set a flag to poll the cache on the next frame.
                static bool gLoadPending = false;
                if (gLoadPending) {
                    auto collectShaders = [this](sk_sp<const SkData> key, sk_sp<SkData> data,
                                                 const SkString& description, int hitCount) {
                        CachedShader& entry(fCachedShaders.push_back());
                        entry.fKey = key;
                        SkMD5 hash;
                        hash.write(key->bytes(), key->size());
                        SkMD5::Digest digest = hash.finish();
                        for (int i = 0; i < 16; ++i) {
                            entry.fKeyString.appendf("%02x", digest.data[i]);
                        }
                        entry.fKeyDescription = description;

                        SkReadBuffer reader(data->data(), data->size());
                        entry.fShaderType = GrPersistentCacheUtils::GetType(&reader);
                        GrPersistentCacheUtils::UnpackCachedShaders(&reader, entry.fShader,
                                                                    entry.fInputs,
                                                                    kGrShaderTypeCount);
                    };
                    fCachedShaders.reset();
                    fPersistentCache.foreach(collectShaders);
                    gLoadPending = false;

#if defined(SK_VULKAN)
                    if (isVulkan && !sksl) {
                        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
                        for (auto& entry : fCachedShaders) {
                            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                                const SkSL::String& spirv(entry.fShader[i]);
                                std::string disasm;
                                tools.Disassemble((const uint32_t*)spirv.c_str(), spirv.size() / 4,
                                                  &disasm);
                                entry.fShader[i].assign(disasm);
                            }
                        }
                    }
#endif
                }

                // Defer actually doing the View/Apply logic so that we can trigger an Apply when we
                // start or finish hovering on a tree node in the list below:
                bool doView      = ImGui::Button("View"); ImGui::SameLine();
                bool doApply     = ImGui::Button("Apply Changes"); ImGui::SameLine();
                bool doDump      = ImGui::Button("Dump SkSL to resources/sksl/");

                int newOptLevel = optLevel;
                ImGui::RadioButton("SkSL", &newOptLevel, kShaderOptLevel_Source);
                ImGui::SameLine();
                ImGui::RadioButton("Compile", &newOptLevel, kShaderOptLevel_Compile);
                ImGui::SameLine();
                ImGui::RadioButton("Optimize", &newOptLevel, kShaderOptLevel_Optimize);
                ImGui::SameLine();
                ImGui::RadioButton("Inline", &newOptLevel, kShaderOptLevel_Inline);

                // If we are changing the compile mode, we want to reset the cache and redo
                // everything.
                if (doDump || newOptLevel != optLevel) {
                    sksl = doDump || (newOptLevel == kShaderOptLevel_Source);
                    SkSL::gSkSLOptimizer           = (newOptLevel >= kShaderOptLevel_Optimize);
                    SkSL::gSkSLInliner             = (newOptLevel >= kShaderOptLevel_Inline);

                    params.fGrContextOptions.fShaderCacheStrategy =
                            sksl ? GrContextOptions::ShaderCacheStrategy::kSkSL
                                 : GrContextOptions::ShaderCacheStrategy::kBackendSource;
                    paramsChanged = true;
                    doView = true;

                    fDeferredActions.push_back([=]() {
                        // Reset the cache.
                        fPersistentCache.reset();
                        // Dump the cache once we have drawn a frame with it.
                        if (doDump) {
                            fDeferredActions.push_back([this]() {
                                this->dumpShadersToResources();
                            });
                        }
                    });
                }

                ImGui::BeginChild("##ScrollingRegion");
                for (auto& entry : fCachedShaders) {
                    bool inTreeNode = ImGui::TreeNode(entry.fKeyString.c_str());
                    bool hovered = ImGui::IsItemHovered();
                    if (hovered != entry.fHovered) {
                        // Force an Apply to patch the highlight shader in/out
                        entry.fHovered = hovered;
                        doApply = true;
                    }
                    if (inTreeNode) {
                        auto stringBox = [](const char* label, std::string* str) {
                            // Full width, and not too much space for each shader
                            int lines = std::count(str->begin(), str->end(), '\n') + 2;
                            ImVec2 boxSize(-1.0f, ImGui::GetTextLineHeight() * std::min(lines, 30));
                            ImGui::InputTextMultiline(label, str, boxSize);
                        };
                        if (ImGui::TreeNode("Key")) {
                            ImGui::TextWrapped("%s", entry.fKeyDescription.c_str());
                            ImGui::TreePop();
                        }
                        stringBox("##VP", &entry.fShader[kVertex_GrShaderType]);
                        stringBox("##FP", &entry.fShader[kFragment_GrShaderType]);
                        ImGui::TreePop();
                    }
                }
                ImGui::EndChild();

                if (doView) {
                    fPersistentCache.reset();
                    ctx->priv().getGpu()->resetShaderCacheForTesting();
                    gLoadPending = true;
                }

                // We don't support updating SPIRV shaders. We could re-assemble them (with edits),
                // but I'm not sure anyone wants to do that.
                if (isVulkan && !sksl) {
                    doApply = false;
                }
                if (doApply) {
                    fPersistentCache.reset();
                    ctx->priv().getGpu()->resetShaderCacheForTesting();
                    for (auto& entry : fCachedShaders) {
                        SkSL::String backup = entry.fShader[kFragment_GrShaderType];
                        if (entry.fHovered) {
                            // The hovered item (if any) gets a special shader to make it
                            // identifiable.
                            SkSL::String& fragShader = entry.fShader[kFragment_GrShaderType];
                            switch (entry.fShaderType) {
                                case SkSetFourByteTag('S', 'K', 'S', 'L'): {
                                    fragShader = build_sksl_highlight_shader();
                                    break;
                                }
                                case SkSetFourByteTag('G', 'L', 'S', 'L'): {
                                    fragShader = build_glsl_highlight_shader(
                                        *ctx->priv().caps()->shaderCaps());
                                    break;
                                }
                                case SkSetFourByteTag('M', 'S', 'L', ' '): {
                                    fragShader = build_metal_highlight_shader(fragShader);
                                    break;
                                }
                            }
                        }

                        auto data = GrPersistentCacheUtils::PackCachedShaders(entry.fShaderType,
                                                                              entry.fShader,
                                                                              entry.fInputs,
                                                                              kGrShaderTypeCount);
                        fPersistentCache.store(*entry.fKey, *data, entry.fKeyDescription);

                        entry.fShader[kFragment_GrShaderType] = backup;
                    }
                }
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

    if (gShaderErrorHandler.fErrors.count()) {
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Shader Errors", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
        for (int i = 0; i < gShaderErrorHandler.fErrors.count(); ++i) {
            ImGui::TextWrapped("%s", gShaderErrorHandler.fErrors[i].c_str());
            SkSL::String sksl(gShaderErrorHandler.fShaders[i].c_str());
            GrShaderUtils::VisitLineByLine(sksl, [](int lineNumber, const char* lineText) {
                ImGui::TextWrapped("%4i\t%s\n", lineNumber, lineText);
            });
        }
        ImGui::End();
        gShaderErrorHandler.reset();
    }

    if (fShowZoomWindow && fLastImage) {
        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Zoom", &fShowZoomWindow)) {
            static int zoomFactor = 8;
            if (ImGui::Button("<<")) {
                zoomFactor = std::max(zoomFactor / 2, 4);
            }
            ImGui::SameLine(); ImGui::Text("%2d", zoomFactor); ImGui::SameLine();
            if (ImGui::Button(">>")) {
                zoomFactor = std::min(zoomFactor * 2, 32);
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
            auto dContext = fWindow->directContext();
            if (fLastImage->readPixels(dContext, info, &pixel, info.minRowBytes(), xInt, yInt)) {
                ImGui::SameLine();
                ImGui::Text("(X, Y): %d, %d RGBA: %X %X %X %X",
                            xInt, yInt,
                            SkGetPackedR32(pixel), SkGetPackedG32(pixel),
                            SkGetPackedB32(pixel), SkGetPackedA32(pixel));
            }

            fImGuiLayer.skiaWidget(avail, [=, lastImage = fLastImage](SkCanvas* c) {
                // Translate so the region of the image that's under the mouse cursor is centered
                // in the zoom canvas:
                c->scale(zoomFactor, zoomFactor);
                c->translate(avail.x * 0.5f / zoomFactor - x - 0.5f,
                             avail.y * 0.5f / zoomFactor - y - 0.5f);
                c->drawImage(lastImage, 0, 0);

                SkPaint outline;
                outline.setStyle(SkPaint::kStroke_Style);
                c->drawRect(SkRect::MakeXYWH(x, y, 1, 1), outline);
            });
        }

        ImGui::End();
    }
}

void Viewer::dumpShadersToResources() {
    // Sort the list of cached shaders so we can maintain some minimal level of consistency.
    // It doesn't really matter, but it will keep files from switching places unpredictably.
    std::vector<const CachedShader*> shaders;
    shaders.reserve(fCachedShaders.size());
    for (const CachedShader& shader : fCachedShaders) {
        shaders.push_back(&shader);
    }

    std::sort(shaders.begin(), shaders.end(), [](const CachedShader* a, const CachedShader* b) {
        return std::tie(a->fShader[kFragment_GrShaderType], a->fShader[kVertex_GrShaderType]) <
               std::tie(b->fShader[kFragment_GrShaderType], b->fShader[kVertex_GrShaderType]);
    });

    // Make the resources/sksl/SlideName/ directory.
    SkString directory = SkStringPrintf("%ssksl/%s",
                                        GetResourcePath().c_str(),
                                        fSlides[fCurrentSlide]->getName().c_str());
    if (!sk_mkdir(directory.c_str())) {
        SkDEBUGFAILF("Unable to create directory '%s'", directory.c_str());
        return;
    }

    int index = 0;
    for (const auto& entry : shaders) {
        SkString vertPath = SkStringPrintf("%s/Vertex_%02d.vert", directory.c_str(), index);
        FILE* vertFile = sk_fopen(vertPath.c_str(), kWrite_SkFILE_Flag);
        if (vertFile) {
            const SkSL::String& vertText = entry->fShader[kVertex_GrShaderType];
            SkAssertResult(sk_fwrite(vertText.c_str(), vertText.size(), vertFile));
            sk_fclose(vertFile);
        } else {
            SkDEBUGFAILF("Unable to write shader to path '%s'", vertPath.c_str());
        }

        SkString fragPath = SkStringPrintf("%s/Fragment_%02d.frag", directory.c_str(), index);
        FILE* fragFile = sk_fopen(fragPath.c_str(), kWrite_SkFILE_Flag);
        if (fragFile) {
            const SkSL::String& fragText = entry->fShader[kFragment_GrShaderType];
            SkAssertResult(sk_fwrite(fragText.c_str(), fragText.size(), fragFile));
            sk_fclose(fragFile);
        } else {
            SkDEBUGFAILF("Unable to write shader to path '%s'", fragPath.c_str());
        }

        ++index;
    }
}

void Viewer::onIdle() {
    SkTArray<std::function<void()>> actionsToRun;
    actionsToRun.swap(fDeferredActions);

    for (const auto& fn : actionsToRun) {
        fn();
    }

    fStatsLayer.beginTiming(fAnimateTimer);
    fAnimTimer.updateTime();
    bool animateWantsInval = fSlides[fCurrentSlide]->animate(fAnimTimer.nanos());
    fStatsLayer.endTiming(fAnimateTimer);

    ImGuiIO& io = ImGui::GetIO();
    // ImGui always has at least one "active" window, which is the default "Debug" window. It may
    // not be visible, though. So we need to redraw if there is at least one visible window, or
    // more than one active window. Newly created windows are active but not visible for one frame
    // while they determine their layout and sizing.
    if (animateWantsInval || fStatsLayer.getActive() || fRefresh ||
        io.MetricsActiveWindows > 1 || io.MetricsRenderWindows > 0) {
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
            auto ctx = fWindow->directContext();
            if (!ctx) {
                writer.appendString("Software");
            } else {
                const auto* caps = ctx->priv().caps();
                writer.appendString(gPathRendererNames[GpuPathRenderers::kDefault].c_str());
                if (fWindow->sampleCount() > 1 || caps->mixedSamplesSupport()) {
                    if (GrTessellationPathRenderer::IsSupported(*caps)) {
                        writer.appendString(
                                gPathRendererNames[GpuPathRenderers::kTessellation].c_str());
                    }
                    if (caps->shaderCaps()->pathRenderingSupport()) {
                        writer.appendString(
                                gPathRendererNames[GpuPathRenderers::kStencilAndCover].c_str());
                    }
                }
                if (1 == fWindow->sampleCount()) {
                    if(GrCoverageCountingPathRenderer::IsSupported(*caps)) {
                        writer.appendString(
                            gPathRendererNames[GpuPathRenderers::kCoverageCounting].c_str());
                    }
                    writer.appendString(gPathRendererNames[GpuPathRenderers::kSmall].c_str());
                }
                writer.appendString(gPathRendererNames[GpuPathRenderers::kTriangulating].c_str());
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
                    for(auto& slide : fSlides) {
                        slide->gpuTeardown();
                    }
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

bool Viewer::onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers) {
    return fCommands.onKey(key, state, modifiers);
}

bool Viewer::onChar(SkUnichar c, skui::ModifierKey modifiers) {
    if (fSlides[fCurrentSlide]->onChar(c)) {
        fWindow->inval();
        return true;
    } else {
        return fCommands.onChar(c, modifiers);
    }
}

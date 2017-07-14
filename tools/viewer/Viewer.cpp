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
#include "SKPSlide.h"

#include "GrContext.h"
#include "SkATrace.h"
#include "SkCanvas.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkCommonFlagsPathRenderer.h"
#include "SkDashPathEffect.h"
#include "SkGraphics.h"
#include "SkImagePriv.h"
#include "SkMetaData.h"
#include "SkOnce.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkSwizzle.h"
#include "SkTaskGroup.h"
#include "SkTime.h"
#include "SkVertices.h"

#include "imgui.h"

#include "ccpr/GrCoverageCountingPathRenderer.h"

#include <stdlib.h>
#include <map>

using namespace sk_app;

using GpuPathRenderers = GrContextOptions::GpuPathRenderers;
static std::map<GpuPathRenderers, std::string> gPathRendererNames;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new Viewer(argc, argv, platformData);
}

static void on_backend_created_func(void* userData) {
    Viewer* vv = reinterpret_cast<Viewer*>(userData);

    return vv->onBackendCreated();
}

static void on_paint_handler(SkCanvas* canvas, void* userData) {
    Viewer* vv = reinterpret_cast<Viewer*>(userData);

    return vv->onPaint(canvas);
}

static bool on_touch_handler(intptr_t owner, Window::InputState state, float x, float y, void* userData)
{
    Viewer* viewer = reinterpret_cast<Viewer*>(userData);

    return viewer->onTouch(owner, state, x, y);
}

static void on_ui_state_changed_handler(const SkString& stateName, const SkString& stateValue, void* userData) {
    Viewer* viewer = reinterpret_cast<Viewer*>(userData);

    return viewer->onUIStateChanged(stateName, stateValue);
}

static bool on_mouse_handler(int x, int y, Window::InputState state, uint32_t modifiers,
                             void* userData) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = static_cast<float>(x);
    io.MousePos.y = static_cast<float>(y);
    if (Window::kDown_InputState == state) {
        io.MouseDown[0] = true;
    } else if (Window::kUp_InputState == state) {
        io.MouseDown[0] = false;
    }
    if (io.WantCaptureMouse) {
        return true;
    } else {
        Viewer* viewer = reinterpret_cast<Viewer*>(userData);
        return viewer->onMouse(x, y, state, modifiers);
    }
}

static bool on_mouse_wheel_handler(float delta, uint32_t modifiers, void* userData) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel += delta;
    return true;
}

static bool on_key_handler(Window::Key key, Window::InputState state, uint32_t modifiers,
                           void* userData) {
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[static_cast<int>(key)] = (Window::kDown_InputState == state);

    if (io.WantCaptureKeyboard) {
        return true;
    } else {
        Viewer* viewer = reinterpret_cast<Viewer*>(userData);
        return viewer->onKey(key, state, modifiers);
    }
}

static bool on_char_handler(SkUnichar c, uint32_t modifiers, void* userData) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) {
        if (c > 0 && c < 0x10000) {
            io.AddInputCharacter(c);
        }
        return true;
    } else {
        Viewer* viewer = reinterpret_cast<Viewer*>(userData);
        return viewer->onChar(c, modifiers);
    }
}

static DEFINE_bool2(fullscreen, f, true, "Run fullscreen.");

static DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of bench name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching bench to always be skipped\n"
               "^ requires the start of the bench to match\n"
               "$ requires the end of the bench to match\n"
               "^ and $ requires an exact match\n"
               "If a bench does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

DEFINE_string(slide, "", "Start on this sample.");
DEFINE_bool(list, false, "List samples?");

#ifdef SK_VULKAN
#    define BACKENDS_STR "\"sw\", \"gl\", and \"vk\""
#else
#    define BACKENDS_STR "\"sw\" and \"gl\""
#endif

#ifdef SK_BUILD_FOR_ANDROID
static DEFINE_string(skps, "/data/local/tmp/skps", "Directory to read skps from.");
static DEFINE_string(jpgs, "/data/local/tmp/resources", "Directory to read jpgs from.");
#else
static DEFINE_string(skps, "skps", "Directory to read skps from.");
static DEFINE_string(jpgs, "jpgs", "Directory to read jpgs from.");
#endif

static DEFINE_string2(backend, b, "sw", "Backend to use. Allowed values are " BACKENDS_STR ".");

static DEFINE_bool(atrace, false, "Enable support for using ATrace. ATrace is only supported on Android.");

DEFINE_int32(msaa, 0, "Number of subpixel samples. 0 for no HW antialiasing.");
DEFINE_pathrenderer_flag;

DEFINE_bool(instancedRendering, false, "Enable instanced rendering on GPU backends.");

const char *kBackendTypeStrings[sk_app::Window::kBackendTypeCount] = {
    "OpenGL",
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

const char* kName = "name";
const char* kValue = "value";
const char* kOptions = "options";
const char* kSlideStateName = "Slide";
const char* kBackendStateName = "Backend";
const char* kMSAAStateName = "MSAA";
const char* kPathRendererStateName = "Path renderer";
const char* kInstancedRenderingStateName = "Instanced rendering";
const char* kSoftkeyStateName = "Softkey";
const char* kSoftkeyHint = "Please select a softkey";
const char* kFpsStateName = "FPS";
const char* kON = "ON";
const char* kOFF = "OFF";
const char* kRefreshStateName = "Refresh";

Viewer::Viewer(int argc, char** argv, void* platformData)
    : fCurrentMeasurement(0)
    , fDisplayStats(false)
    , fRefresh(false)
    , fShowImGuiDebugWindow(false)
    , fShowImGuiTestWindow(false)
    , fShowZoomWindow(false)
    , fLastImage(nullptr)
    , fBackendType(sk_app::Window::kNativeGL_BackendType)
    , fColorMode(ColorMode::kLegacy)
    , fColorSpacePrimaries(gSrgbPrimaries)
    , fZoomLevel(0.0f)
    , fGestureDevice(GestureDevice::kNone)
{
    static SkTaskGroup::Enabler kTaskGroupEnabler;
    SkGraphics::Init();

    static SkOnce initPathRendererNames;
    initPathRendererNames([]() {
        gPathRendererNames[GpuPathRenderers::kAll] = "Default Ganesh Behavior (best path renderer)";
        gPathRendererNames[GpuPathRenderers::kStencilAndCover] = "NV_path_rendering";
        gPathRendererNames[GpuPathRenderers::kMSAA] = "Sample shading";
        gPathRendererNames[GpuPathRenderers::kSmall] = "Small paths (cached sdf or alpha masks)";
        gPathRendererNames[GpuPathRenderers::kCoverageCounting] = "Coverage counting";
        gPathRendererNames[GpuPathRenderers::kTessellating] = "Tessellating";
        gPathRendererNames[GpuPathRenderers::kDefault] = "Original Ganesh path renderer";
        gPathRendererNames[GpuPathRenderers::kNone] = "Software masks";
    });

    memset(fPaintTimes, 0, sizeof(fPaintTimes));
    memset(fFlushTimes, 0, sizeof(fFlushTimes));
    memset(fAnimateTimes, 0, sizeof(fAnimateTimes));

    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    SkCommandLineFlags::Parse(argc, argv);
#ifdef SK_BUILD_FOR_ANDROID
    SetResourcePath("/data/local/tmp/resources");
#endif

    if (FLAGS_atrace) {
        SkAssertResult(SkEventTracer::SetInstance(new SkATrace()));
    }

    fBackendType = get_backend_type(FLAGS_backend[0]);
    fWindow = Window::CreateNativeWindow(platformData);

    DisplayParams displayParams;
    displayParams.fMSAASampleCount = FLAGS_msaa;
    displayParams.fGrContextOptions.fEnableInstancedRendering = FLAGS_instancedRendering;
    displayParams.fGrContextOptions.fGpuPathRenderers = CollectGpuPathRenderersFromFlags();
    fWindow->setRequestedDisplayParams(displayParams);

    // register callbacks
    fCommands.attach(fWindow);
    fWindow->registerBackendCreatedFunc(on_backend_created_func, this);
    fWindow->registerPaintFunc(on_paint_handler, this);
    fWindow->registerTouchFunc(on_touch_handler, this);
    fWindow->registerUIStateChangedFunc(on_ui_state_changed_handler, this);
    fWindow->registerMouseFunc(on_mouse_handler, this);
    fWindow->registerMouseWheelFunc(on_mouse_wheel_handler, this);
    fWindow->registerKeyFunc(on_key_handler, this);
    fWindow->registerCharFunc(on_char_handler, this);

    // add key-bindings
    fCommands.addCommand(' ', "GUI", "Toggle Debug GUI", [this]() {
        this->fShowImGuiDebugWindow = !this->fShowImGuiDebugWindow;
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
        this->fDisplayStats = !this->fDisplayStats;
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
        int previousSlide = fCurrentSlide;
        fCurrentSlide++;
        if (fCurrentSlide >= fSlides.count()) {
            fCurrentSlide = 0;
        }
        this->setupCurrentSlide(previousSlide);
    });
    fCommands.addCommand(Window::Key::kLeft, "Left", "Navigation", "Previous slide", [this]() {
        int previousSlide = fCurrentSlide;
        fCurrentSlide--;
        if (fCurrentSlide < 0) {
            fCurrentSlide = fSlides.count() - 1;
        }
        this->setupCurrentSlide(previousSlide);
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
        sk_app::Window::BackendType newBackend = fBackendType;
#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
        if (sk_app::Window::kRaster_BackendType == fBackendType) {
            newBackend = sk_app::Window::kNativeGL_BackendType;
#ifdef SK_VULKAN
        } else if (sk_app::Window::kNativeGL_BackendType == fBackendType) {
            newBackend = sk_app::Window::kVulkan_BackendType;
#endif
        } else {
            newBackend = sk_app::Window::kRaster_BackendType;
        }
#elif defined(SK_BUILD_FOR_UNIX)
        // Switching to and from Vulkan is problematic on Linux so disabled for now
        if (sk_app::Window::kRaster_BackendType == fBackendType) {
            newBackend = sk_app::Window::kNativeGL_BackendType;
        } else if (sk_app::Window::kNativeGL_BackendType == fBackendType) {
            newBackend = sk_app::Window::kRaster_BackendType;
        }
#endif

        this->setBackend(newBackend);
    });

    // set up slides
    this->initSlides();
    this->setStartupSlide();
    if (FLAGS_list) {
        this->listNames();
    }

    fAnimTimer.run();

    // ImGui initialization:
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(fWindow->width());
    io.DisplaySize.y = static_cast<float>(fWindow->height());

    // Keymap...
    io.KeyMap[ImGuiKey_Tab] = (int)Window::Key::kTab;
    io.KeyMap[ImGuiKey_LeftArrow] = (int)Window::Key::kLeft;
    io.KeyMap[ImGuiKey_RightArrow] = (int)Window::Key::kRight;
    io.KeyMap[ImGuiKey_UpArrow] = (int)Window::Key::kUp;
    io.KeyMap[ImGuiKey_DownArrow] = (int)Window::Key::kDown;
    io.KeyMap[ImGuiKey_PageUp] = (int)Window::Key::kPageUp;
    io.KeyMap[ImGuiKey_PageDown] = (int)Window::Key::kPageDown;
    io.KeyMap[ImGuiKey_Home] = (int)Window::Key::kHome;
    io.KeyMap[ImGuiKey_End] = (int)Window::Key::kEnd;
    io.KeyMap[ImGuiKey_Delete] = (int)Window::Key::kDelete;
    io.KeyMap[ImGuiKey_Backspace] = (int)Window::Key::kBack;
    io.KeyMap[ImGuiKey_Enter] = (int)Window::Key::kOK;
    io.KeyMap[ImGuiKey_Escape] = (int)Window::Key::kEscape;
    io.KeyMap[ImGuiKey_A] = (int)Window::Key::kA;
    io.KeyMap[ImGuiKey_C] = (int)Window::Key::kC;
    io.KeyMap[ImGuiKey_V] = (int)Window::Key::kV;
    io.KeyMap[ImGuiKey_X] = (int)Window::Key::kX;
    io.KeyMap[ImGuiKey_Y] = (int)Window::Key::kY;
    io.KeyMap[ImGuiKey_Z] = (int)Window::Key::kZ;

    int w, h;
    unsigned char* pixels;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &w, &h);
    SkImageInfo info = SkImageInfo::MakeA8(w, h);
    SkPixmap pmap(info, pixels, info.minRowBytes());
    SkMatrix localMatrix = SkMatrix::MakeScale(1.0f / w, 1.0f / h);
    auto fontImage = SkImage::MakeFromRaster(pmap, nullptr, nullptr);
    auto fontShader = fontImage->makeShader(&localMatrix);
    fImGuiFontPaint.setShader(fontShader);
    fImGuiFontPaint.setColor(SK_ColorWHITE);
    fImGuiFontPaint.setFilterQuality(kLow_SkFilterQuality);
    io.Fonts->TexID = &fImGuiFontPaint;

    auto gamutImage = GetResourceAsImage("gamut.png");
    if (gamutImage) {
        fImGuiGamutPaint.setShader(gamutImage->makeShader());
    }
    fImGuiGamutPaint.setColor(SK_ColorWHITE);
    fImGuiGamutPaint.setFilterQuality(kLow_SkFilterQuality);

    fWindow->attach(fBackendType);
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
    }

    title.append(" [");
    title.append(kBackendTypeStrings[fBackendType]);
    if (int msaa = fWindow->sampleCount()) {
        title.appendf(" MSAA: %i", msaa);
    }
    title.append("]");

    GpuPathRenderers pr = fWindow->getRequestedDisplayParams().fGrContextOptions.fGpuPathRenderers;
    if (GpuPathRenderers::kAll != pr) {
        title.appendf(" [Path renderer: %s]", gPathRendererNames[pr].c_str());
    }

    fWindow->setTitle(title.c_str());
}

void Viewer::setStartupSlide() {

    if (!FLAGS_slide.isEmpty()) {
        int count = fSlides.count();
        for (int i = 0; i < count; i++) {
            if (fSlides[i]->getName().equals(FLAGS_slide[0])) {
                fCurrentSlide = i;
                return;
            }
        }

        fprintf(stderr, "Unknown slide \"%s\"\n", FLAGS_slide[0]);
        this->listNames();
    }

    fCurrentSlide = 0;
}

void Viewer::listNames() {
    int count = fSlides.count();
    SkDebugf("All Slides:\n");
    for (int i = 0; i < count; i++) {
        SkDebugf("    %s\n", fSlides[i]->getName().c_str());
    }
}

void Viewer::setupCurrentSlide(int previousSlide) {
    if (fCurrentSlide == previousSlide) {
        return; // no change; do nothing
    }
    // prepare dimensions for image slides
    fSlides[fCurrentSlide]->load(SkIntToScalar(fWindow->width()), SkIntToScalar(fWindow->height()));

    fGesture.reset();
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
    if (previousSlide >= 0) {
        fSlides[previousSlide]->unload();
    }
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

#if defined(SK_BUILD_FOR_WIN) && defined(SK_VULKAN)
    // Switching from OpenGL to Vulkan (or vice-versa on some systems) in the same window is
    // problematic at this point on Windows, so we just delete the window and recreate it.
    if (sk_app::Window::kVulkan_BackendType == fBackendType ||
            sk_app::Window::kNativeGL_BackendType == fBackendType) {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        delete fWindow;
        fWindow = Window::CreateNativeWindow(nullptr);

        // re-register callbacks
        fCommands.attach(fWindow);
        fWindow->registerBackendCreatedFunc(on_backend_created_func, this);
        fWindow->registerPaintFunc(on_paint_handler, this);
        fWindow->registerTouchFunc(on_touch_handler, this);
        fWindow->registerUIStateChangedFunc(on_ui_state_changed_handler, this);
        fWindow->registerMouseFunc(on_mouse_handler, this);
        fWindow->registerMouseWheelFunc(on_mouse_wheel_handler, this);
        fWindow->registerKeyFunc(on_key_handler, this);
        fWindow->registerCharFunc(on_char_handler, this);
        // Don't allow the window to re-attach. If we're in MSAA mode, the params we grabbed above
        // will still include our correct sample count. But the re-created fWindow will lose that
        // information. On Windows, we need to re-create the window when changing sample count,
        // so we'll incorrectly detect that situation, then re-initialize the window in GL mode,
        // rendering this tear-down step pointless (and causing the Vulkan window context to fail
        // as if we had never changed windows at all).
        fWindow->setRequestedDisplayParams(params, false);
    }
#endif

    fWindow->attach(fBackendType);
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
        SkMatrix44 toXYZ;
        SkAssertResult(fColorSpacePrimaries.toXYZD50(&toXYZ));
        cs = SkColorSpace::MakeRGB(transferFn, toXYZ);
    }

    // If we're in F16, or we're zooming, or we're in color correct 8888 and the gamut isn't sRGB,
    // we need to render offscreen
    sk_sp<SkSurface> offscreenSurface = nullptr;
    if (ColorMode::kColorManagedLinearF16 == fColorMode ||
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
        offscreenSurface = canvas->makeSurface(info);
        slideCanvas = offscreenSurface->getCanvas();
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
    double startTime = SkTime::GetMSecs();
    fSlides[fCurrentSlide]->draw(slideCanvas);
    fPaintTimes[fCurrentMeasurement] = SkTime::GetMSecs() - startTime;
    slideCanvas->restoreToCount(count);

    // Force a flush so we can time that, too
    startTime = SkTime::GetMSecs();
    slideCanvas->flush();
    fFlushTimes[fCurrentMeasurement] = SkTime::GetMSecs() - startTime;

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
    this->setupCurrentSlide(-1);
    fWindow->show();
    fWindow->inval();
}

void Viewer::onPaint(SkCanvas* canvas) {
    // Update ImGui input
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;
    io.DisplaySize.x = static_cast<float>(fWindow->width());
    io.DisplaySize.y = static_cast<float>(fWindow->height());

    io.KeyAlt = io.KeysDown[static_cast<int>(Window::Key::kOption)];
    io.KeyCtrl = io.KeysDown[static_cast<int>(Window::Key::kCtrl)];
    io.KeyShift = io.KeysDown[static_cast<int>(Window::Key::kShift)];

    ImGui::NewFrame();

    drawSlide(canvas);

    // Advance our timing bookkeeping
    fCurrentMeasurement = (fCurrentMeasurement + 1) & (kMeasurementCount - 1);
    SkASSERT(fCurrentMeasurement < kMeasurementCount);

    // Draw any overlays or UI that we don't want timed
    if (fDisplayStats) {
        drawStats(canvas);
    }
    fCommands.drawHelp(canvas);

    drawImGui(canvas);

    // Update the FPS
    updateUIState();
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

bool Viewer::onMouse(float x, float y, Window::InputState state, uint32_t modifiers) {
    if (GestureDevice::kTouch == fGestureDevice) {
        return false;
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
    fWindow->inval();
    return true;
}

void Viewer::drawStats(SkCanvas* canvas) {
    static const float kPixelPerMS = 2.0f;
    static const int kDisplayWidth = 130;
    static const int kDisplayHeight = 100;
    static const int kDisplayPadding = 10;
    static const int kGraphPadding = 3;
    static const SkScalar kBaseMS = 1000.f / 60.f;  // ms/frame to hit 60 fps

    SkISize canvasSize = canvas->getBaseLayerSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(kDisplayHeight));
    SkPaint paint;
    canvas->save();

    canvas->clipRect(rect);
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(rect, paint);
    // draw the 16ms line
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawLine(rect.fLeft, rect.fBottom - kBaseMS*kPixelPerMS,
                     rect.fRight, rect.fBottom - kBaseMS*kPixelPerMS, paint);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);

    int x = SkScalarTruncToInt(rect.fLeft) + kGraphPadding;
    const int xStep = 2;
    int i = fCurrentMeasurement;
    do {
        // Round to nearest values
        int animateHeight = (int)(fAnimateTimes[i] * kPixelPerMS + 0.5);
        int paintHeight = (int)(fPaintTimes[i] * kPixelPerMS + 0.5);
        int flushHeight = (int)(fFlushTimes[i] * kPixelPerMS + 0.5);
        int startY = SkScalarTruncToInt(rect.fBottom);
        int endY = startY - flushHeight;
        paint.setColor(SK_ColorRED);
        canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                         SkIntToScalar(x), SkIntToScalar(endY), paint);
        startY = endY;
        endY = startY - paintHeight;
        paint.setColor(SK_ColorGREEN);
        canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                         SkIntToScalar(x), SkIntToScalar(endY), paint);
        startY = endY;
        endY = startY - animateHeight;
        paint.setColor(SK_ColorMAGENTA);
        canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                         SkIntToScalar(x), SkIntToScalar(endY), paint);
        i++;
        i &= (kMeasurementCount - 1);  // fast mod
        x += xStep;
    } while (i != fCurrentMeasurement);

    canvas->restore();
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

void Viewer::drawImGui(SkCanvas* canvas) {
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
                bool* inst = &params.fGrContextOptions.fEnableInstancedRendering;
                if (ctx && ImGui::Checkbox("Instanced Rendering", inst)) {
                    paramsChanged = true;
                }
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
                        prButton(GpuPathRenderers::kAll);
                        if (ctx->caps()->shaderCaps()->pathRenderingSupport()) {
                            prButton(GpuPathRenderers::kStencilAndCover);
                        }
                        if (ctx->caps()->sampleShadingSupport()) {
                            prButton(GpuPathRenderers::kMSAA);
                        }
                        prButton(GpuPathRenderers::kTessellating);
                        prButton(GpuPathRenderers::kDefault);
                        prButton(GpuPathRenderers::kNone);
                    } else {
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

            if (ImGui::CollapsingHeader("Slide")) {
                static ImGuiTextFilter filter;
                filter.Draw();
                int previousSlide = fCurrentSlide;
                fCurrentSlide = 0;
                for (auto slide : fSlides) {
                    if (filter.PassFilter(slide->getName().c_str())) {
                        ImGui::BulletText("%s", slide->getName().c_str());
                        if (ImGui::IsItemClicked()) {
                            setupCurrentSlide(previousSlide);
                            break;
                        }
                    }
                    ++fCurrentSlide;
                }
                if (fCurrentSlide >= fSlides.count()) {
                    fCurrentSlide = previousSlide;
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

    SkPaint zoomImagePaint;
    if (fShowZoomWindow && fLastImage) {
        if (ImGui::Begin("Zoom", &fShowZoomWindow, ImVec2(200, 200))) {
            static int zoomFactor = 4;
            ImGui::SliderInt("Scale", &zoomFactor, 1, 16);

            zoomImagePaint.setShader(fLastImage->makeShader());
            zoomImagePaint.setColor(SK_ColorWHITE);

            // Zoom by shrinking the corner UVs towards the mouse cursor
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 avail = ImGui::GetContentRegionAvail();

            ImVec2 zoomHalfExtents = ImVec2((avail.x * 0.5f) / zoomFactor,
                                            (avail.y * 0.5f) / zoomFactor);
            ImGui::Image(&zoomImagePaint, avail,
                         ImVec2(mousePos.x - zoomHalfExtents.x, mousePos.y - zoomHalfExtents.y),
                         ImVec2(mousePos.x + zoomHalfExtents.x, mousePos.y + zoomHalfExtents.y));
        }

        ImGui::End();
    }

    // This causes ImGui to rebuild vertex/index data based on all immediate-mode commands
    // (widgets, etc...) that have been issued
    ImGui::Render();

    // Then we fetch the most recent data, and convert it so we can render with Skia
    const ImDrawData* drawData = ImGui::GetDrawData();
    SkTDArray<SkPoint> pos;
    SkTDArray<SkPoint> uv;
    SkTDArray<SkColor> color;

    for (int i = 0; i < drawData->CmdListsCount; ++i) {
        const ImDrawList* drawList = drawData->CmdLists[i];

        // De-interleave all vertex data (sigh), convert to Skia types
        pos.rewind(); uv.rewind(); color.rewind();
        for (int i = 0; i < drawList->VtxBuffer.size(); ++i) {
            const ImDrawVert& vert = drawList->VtxBuffer[i];
            pos.push(SkPoint::Make(vert.pos.x, vert.pos.y));
            uv.push(SkPoint::Make(vert.uv.x, vert.uv.y));
            color.push(vert.col);
        }
        // ImGui colors are RGBA
        SkSwapRB(color.begin(), color.begin(), color.count());

        int indexOffset = 0;

        // Draw everything with canvas.drawVertices...
        for (int j = 0; j < drawList->CmdBuffer.size(); ++j) {
            const ImDrawCmd* drawCmd = &drawList->CmdBuffer[j];

            // TODO: Find min/max index for each draw, so we know how many vertices (sigh)
            if (drawCmd->UserCallback) {
                drawCmd->UserCallback(drawList, drawCmd);
            } else {
                SkPaint* paint = static_cast<SkPaint*>(drawCmd->TextureId);
                SkASSERT(paint);

                canvas->save();
                canvas->clipRect(SkRect::MakeLTRB(drawCmd->ClipRect.x, drawCmd->ClipRect.y,
                                                  drawCmd->ClipRect.z, drawCmd->ClipRect.w));
                canvas->drawVertices(SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                                          drawList->VtxBuffer.size(), pos.begin(),
                                                          uv.begin(), color.begin(),
                                                          drawCmd->ElemCount,
                                                          drawList->IdxBuffer.begin() + indexOffset),
                                     SkBlendMode::kModulate, *paint);
                indexOffset += drawCmd->ElemCount;
                canvas->restore();
            }
        }
    }
}

void Viewer::onIdle() {
    for (int i = 0; i < fDeferredActions.count(); ++i) {
        fDeferredActions[i]();
    }
    fDeferredActions.reset();

    double startTime = SkTime::GetMSecs();
    fAnimTimer.updateTime();
    bool animateWantsInval = fSlides[fCurrentSlide]->animate(fAnimTimer);
    fAnimateTimes[fCurrentMeasurement] = SkTime::GetMSecs() - startTime;

    ImGuiIO& io = ImGui::GetIO();
    if (animateWantsInval || fDisplayStats || fRefresh || io.MetricsActiveWindows) {
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
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kAll]);
        if (ctx->caps()->shaderCaps()->pathRenderingSupport()) {
            prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kStencilAndCover]);
        }
        if (ctx->caps()->sampleShadingSupport()) {
            prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kMSAA]);
        }
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kTessellating]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kDefault]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kNone]);
    } else {
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kAll]);
        if (GrCoverageCountingPathRenderer::IsSupported(*ctx->caps())) {
            prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kCoverageCounting]);
        }
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kSmall]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kTessellating]);
        prState[kOptions].append(gPathRendererNames[GpuPathRenderers::kNone]);
    }

    // Instanced rendering state
    Json::Value instState(Json::objectValue);
    instState[kName] = kInstancedRenderingStateName;
    if (ctx) {
        if (fWindow->getRequestedDisplayParams().fGrContextOptions.fEnableInstancedRendering) {
            instState[kValue] = kON;
        } else {
            instState[kValue] = kOFF;
        }
        instState[kOptions] = Json::Value(Json::arrayValue);
        instState[kOptions].append(kOFF);
        instState[kOptions].append(kON);
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
    int idx = (fCurrentMeasurement + (kMeasurementCount - 1)) & (kMeasurementCount - 1);
    fpsState[kValue] = SkStringPrintf("%8.3lf ms\n\nA %8.3lf\nP %8.3lf\nF%8.3lf",
                                      fAnimateTimes[idx] + fPaintTimes[idx] + fFlushTimes[idx],
                                      fAnimateTimes[idx],
                                      fPaintTimes[idx],
                                      fFlushTimes[idx]).c_str();
    fpsState[kOptions] = Json::Value(Json::arrayValue);

    Json::Value state(Json::arrayValue);
    state.append(slideState);
    state.append(backendState);
    state.append(msaaState);
    state.append(prState);
    state.append(instState);
    state.append(softkeyState);
    state.append(fpsState);

    fWindow->setUIState(state);
}

void Viewer::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    // For those who will add more features to handle the state change in this function:
    // After the change, please call updateUIState no notify the frontend (e.g., Android app).
    // For example, after slide change, updateUIState is called inside setupCurrentSlide;
    // after backend change, updateUIState is called in this function.
    if (stateName.equals(kSlideStateName)) {
        int previousSlide = fCurrentSlide;
        fCurrentSlide = 0;
        for(auto slide : fSlides) {
            if (slide->getName().equals(stateValue)) {
                this->setupCurrentSlide(previousSlide);
                break;
            }
            fCurrentSlide++;
        }
        if (fCurrentSlide >= fSlides.count()) {
            fCurrentSlide = previousSlide;
            SkDebugf("Slide not found: %s", stateValue.c_str());
        }
    } else if (stateName.equals(kBackendStateName)) {
        for (int i = 0; i < sk_app::Window::kBackendTypeCount; i++) {
            if (stateValue.equals(kBackendTypeStrings[i])) {
                if (fBackendType != i) {
                    fBackendType = (sk_app::Window::BackendType)i;
                    fWindow->detach();
                    fWindow->attach(fBackendType);
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
    } else if (stateName.equals(kInstancedRenderingStateName)) {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        bool value = !strcmp(stateValue.c_str(), kON);
        if (params.fGrContextOptions.fEnableInstancedRendering != value) {
            params.fGrContextOptions.fEnableInstancedRendering = value;
            fWindow->setRequestedDisplayParams(params);
            fWindow->inval();
            this->updateTitle();
            this->updateUIState();
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
    }

    return fCommands.onChar(c, modifiers);
}

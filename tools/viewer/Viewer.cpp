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

#include "SkATrace.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkDashPathEffect.h"
#include "SkGraphics.h"
#include "SkImagePriv.h"
#include "SkMetaData.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkSwizzle.h"
#include "SkTaskGroup.h"
#include "SkTime.h"

#include "imgui.h"

using namespace sk_app;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new Viewer(argc, argv, platformData);
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
    return true;
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
static DEFINE_string(skps, "/data/local/tmp/skia", "Directory to read skps from.");
static DEFINE_string(jpgs, "/data/local/tmp/skia", "Directory to read jpgs from.");
#else
static DEFINE_string(skps, "skps", "Directory to read skps from.");
static DEFINE_string(jpgs, "jpgs", "Directory to read jpgs from.");
#endif

static DEFINE_string2(backend, b, "sw", "Backend to use. Allowed values are " BACKENDS_STR ".");

static DEFINE_bool(atrace, false, "Enable support for using ATrace. ATrace is only supported on Android.");

const char *kBackendTypeStrings[sk_app::Window::kBackendTypeCount] = {
    " [OpenGL]",
#ifdef SK_VULKAN
    " [Vulkan]",
#endif
    " [Raster]"
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

const char* kName = "name";
const char* kValue = "value";
const char* kOptions = "options";
const char* kSlideStateName = "Slide";
const char* kBackendStateName = "Backend";
const char* kSoftkeyStateName = "Softkey";
const char* kSoftkeyHint = "Please select a softkey";
const char* kFpsStateName = "FPS";
const char* kON = "ON";
const char* kOFF = "OFF";
const char* kRefreshStateName = "Refresh";

Viewer::Viewer(int argc, char** argv, void* platformData)
    : fCurrentMeasurement(0)
    , fSetupFirstFrame(false)
    , fDisplayStats(false)
    , fRefresh(false)
    , fShowImGuiDebugWindow(false)
    , fShowImGuiTestWindow(false)
    , fShowZoomWindow(false)
    , fLastImage(nullptr)
    , fBackendType(sk_app::Window::kNativeGL_BackendType)
    , fColorType(kN32_SkColorType)
    , fColorSpace(nullptr)
    , fZoomCenterX(0.0f)
    , fZoomCenterY(0.0f)
    , fZoomLevel(0.0f)
    , fZoomScale(SK_Scalar1)
{
    static SkTaskGroup::Enabler kTaskGroupEnabler;
    SkGraphics::Init();
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
    SetResourcePath("/data/local/tmp/skia");
#endif

    if (FLAGS_atrace) {
        SkEventTracer::SetInstance(new SkATrace());
    }

    fBackendType = get_backend_type(FLAGS_backend[0]);
    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->attach(fBackendType, DisplayParams());

    // register callbacks
    fCommands.attach(fWindow);
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
        if (!fColorSpace) {
            // Legacy -> sRGB
            this->setColorMode(kN32_SkColorType, SkColorSpace::MakeSRGB());
        } else if (kN32_SkColorType == fColorType) {
            // sRGB -> F16 sRGB
            this->setColorMode(kRGBA_F16_SkColorType, SkColorSpace::MakeSRGBLinear());
        } else {
            // F16 sRGB -> Legacy
            this->setColorMode(kN32_SkColorType, nullptr);
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
#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
        if (sk_app::Window::kRaster_BackendType == fBackendType) {
            fBackendType = sk_app::Window::kNativeGL_BackendType;
#ifdef SK_VULKAN
        } else if (sk_app::Window::kNativeGL_BackendType == fBackendType) {
            fBackendType = sk_app::Window::kVulkan_BackendType;
#endif
        } else {
            fBackendType = sk_app::Window::kRaster_BackendType;
        }
#elif defined(SK_BUILD_FOR_UNIX)
        // Switching to and from Vulkan is problematic on Linux so disabled for now
        if (sk_app::Window::kRaster_BackendType == fBackendType) {
            fBackendType = sk_app::Window::kNativeGL_BackendType;
        } else if (sk_app::Window::kNativeGL_BackendType == fBackendType) {
            fBackendType = sk_app::Window::kRaster_BackendType;
        }
#endif
        fWindow->detach();

#if defined(SK_BUILD_FOR_WIN) && defined(SK_VULKAN)
        // Switching from OpenGL to Vulkan in the same window is problematic at this point on
        // Windows, so we just delete the window and recreate it.
        if (sk_app::Window::kVulkan_BackendType == fBackendType) {
            delete fWindow;
            fWindow = Window::CreateNativeWindow(nullptr);

            // re-register callbacks
            fCommands.attach(fWindow);
            fWindow->registerPaintFunc(on_paint_handler, this);
            fWindow->registerTouchFunc(on_touch_handler, this);
            fWindow->registerUIStateChangedFunc(on_ui_state_changed_handler, this);
            fWindow->registerMouseFunc(on_mouse_handler, this);
            fWindow->registerMouseWheelFunc(on_mouse_wheel_handler, this);
            fWindow->registerKeyFunc(on_key_handler, this);
            fWindow->registerCharFunc(on_char_handler, this);
        }
#endif
        fWindow->attach(fBackendType, DisplayParams());

        this->updateTitle();
        fWindow->inval();
        fWindow->show();
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
    auto fontShader = fontImage->makeShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                                            &localMatrix);
    fImGuiFontPaint.setShader(fontShader);
    fImGuiFontPaint.setColor(SK_ColorWHITE);
    fImGuiFontPaint.setFilterQuality(kLow_SkFilterQuality);
    io.Fonts->TexID = &fImGuiFontPaint;

    fWindow->show();
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
    SkString title("Viewer: ");
    title.append(fSlides[fCurrentSlide]->getName());

    title.appendf(" %s", sk_tool_utils::colortype_name(fColorType));

    // TODO: Find a short string to describe the gamut of the color space?
    if (fColorSpace) {
        title.append(" ColorManaged");
    }

    title.append(kBackendTypeStrings[fBackendType]);
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
    fDefaultMatrixInv.reset();

    if (fWindow->supportsContentRect() && fWindow->scaleContentToFit()) {
        const SkRect contentRect = fWindow->getContentRect();
        const SkISize slideSize = fSlides[fCurrentSlide]->getDimensions();
        const SkRect slideBounds = SkRect::MakeIWH(slideSize.width(), slideSize.height());
        if (contentRect.width() > 0 && contentRect.height() > 0) {
            fDefaultMatrix.setRectToRect(slideBounds, contentRect, SkMatrix::kStart_ScaleToFit);
            SkAssertResult(fDefaultMatrix.invert(&fDefaultMatrixInv));
        }
    }

    if (fWindow->supportsContentRect()) {
        const SkISize slideSize = fSlides[fCurrentSlide]->getDimensions();
        SkRect windowRect = fWindow->getContentRect();
        fDefaultMatrixInv.mapRect(&windowRect);
        fGesture.setTransLimit(SkRect::MakeWH(SkIntToScalar(slideSize.width()), 
                                              SkIntToScalar(slideSize.height())),
                               windowRect);
    }

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
    if (fZoomLevel > 0) {
        fZoomLevel = SkMinScalar(fZoomLevel, MAX_ZOOM_LEVEL);
        fZoomScale = fZoomLevel + SK_Scalar1;
    } else if (fZoomLevel < 0) {
        fZoomLevel = SkMaxScalar(fZoomLevel, MIN_ZOOM_LEVEL);
        fZoomScale = SK_Scalar1 / (SK_Scalar1 - fZoomLevel);
    } else {
        fZoomScale = SK_Scalar1;
    }
}

SkMatrix Viewer::computeMatrix() {
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

    m.preConcat(fGesture.localM());
    m.preConcat(fGesture.globalM());

    return m;
}

void Viewer::setColorMode(SkColorType colorType, sk_sp<SkColorSpace> colorSpace) {
    fColorType = colorType;
    fColorSpace = std::move(colorSpace);

    // When we're in color managed mode, we tag our window surface as sRGB. If we've switched into
    // or out of legacy mode, we need to update our window configuration.
    DisplayParams params = fWindow->getDisplayParams();
    if (SkToBool(fColorSpace) != SkToBool(params.fColorSpace)) {
        params.fColorSpace = fColorSpace ? SkColorSpace::MakeSRGB() : nullptr;
        fWindow->setDisplayParams(params);
    }

    this->updateTitle();
    fWindow->inval();
}

void Viewer::drawSlide(SkCanvas* canvas) {
    int count = canvas->save();
    if (fWindow->supportsContentRect()) {
        SkRect contentRect = fWindow->getContentRect();
        canvas->clipRect(contentRect);
        canvas->translate(contentRect.fLeft, contentRect.fTop);
    }

    // By default, we render directly into the window's surface/canvas
    SkCanvas* slideCanvas = canvas;
    fLastImage.reset();

    // If we're in F16, or the gamut isn't sRGB, or we're zooming, we need to render offscreen
    sk_sp<SkSurface> offscreenSurface = nullptr;
    if (kRGBA_F16_SkColorType == fColorType || fShowZoomWindow ||
        (fColorSpace && fColorSpace != SkColorSpace::MakeSRGB())) {
        SkImageInfo info = SkImageInfo::Make(fWindow->width(), fWindow->height(), fColorType,
                                             kPremul_SkAlphaType, fColorSpace);
        offscreenSurface = canvas->makeSurface(info);
        slideCanvas = offscreenSurface->getCanvas();
    }

    slideCanvas->clear(SK_ColorWHITE);
    slideCanvas->concat(fDefaultMatrix);
    slideCanvas->concat(computeMatrix());

    // Time the painting logic of the slide
    double startTime = SkTime::GetMSecs();
    fSlides[fCurrentSlide]->draw(slideCanvas);
    fPaintTimes[fCurrentMeasurement] = SkTime::GetMSecs() - startTime;

    // Force a flush so we can time that, too
    startTime = SkTime::GetMSecs();
    slideCanvas->flush();
    fFlushTimes[fCurrentMeasurement] = SkTime::GetMSecs() - startTime;

    // If we rendered offscreen, snap an image and push the results to the window's canvas
    if (offscreenSurface) {
        fLastImage = offscreenSurface->makeImageSnapshot();

        // Tag the image with the sRGB gamut, so no further color space conversion happens
        sk_sp<SkColorSpace> cs = (kRGBA_F16_SkColorType == fColorType)
            ? SkColorSpace::MakeSRGBLinear() : SkColorSpace::MakeSRGB();
        auto retaggedImage = SkImageMakeRasterCopyAndAssignColorSpace(fLastImage.get(), cs.get());
        canvas->drawImage(retaggedImage, 0, 0);
    }

    canvas->restoreToCount(count);
}

void Viewer::onPaint(SkCanvas* canvas) {
    // We have to wait until the first draw to make sure the window size is set correctly
    if (!fSetupFirstFrame) {
        // set up first frame
        setupCurrentSlide(-1);
        fSetupFirstFrame = true;
    }

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
    void* castedOwner = reinterpret_cast<void*>(owner);
    SkPoint touchPoint = fDefaultMatrixInv.mapXY(x, y);
    switch (state) {
        case Window::kUp_InputState: {
            fGesture.touchEnd(castedOwner);
            break;
        }
        case Window::kDown_InputState: {
            fGesture.touchBegin(castedOwner, touchPoint.fX, touchPoint.fY);
            break;
        }
        case Window::kMove_InputState: {
            fGesture.touchMoved(castedOwner, touchPoint.fX, touchPoint.fY);
            break;
        }
    }
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

    SkISize canvasSize = canvas->getDeviceSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(kDisplayHeight));
    SkPaint paint;
    canvas->save();

    if (fWindow->supportsContentRect()) {
        SkRect contentRect = fWindow->getContentRect();
        canvas->clipRect(contentRect);
        canvas->translate(contentRect.fLeft, contentRect.fTop);
    }

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

void Viewer::drawImGui(SkCanvas* canvas) {
    // Support drawing the ImGui demo window. Superfluous, but gives a good idea of what's possible
    if (fShowImGuiTestWindow) {
        ImGui::ShowTestWindow(&fShowImGuiTestWindow);
    }

    if (fShowImGuiDebugWindow) {
        if (ImGui::Begin("Debug", &fShowImGuiDebugWindow)) {
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
        }

        ImGui::End();
    }

    SkPaint zoomImagePaint;
    if (fShowZoomWindow && fLastImage) {
        if (ImGui::Begin("Zoom", &fShowZoomWindow, ImVec2(200, 200))) {
            static int zoomFactor = 4;
            ImGui::SliderInt("Scale", &zoomFactor, 1, 16);

            zoomImagePaint.setShader(fLastImage->makeShader(SkShader::kClamp_TileMode,
                                                            SkShader::kClamp_TileMode));
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
                canvas->drawVertices(SkCanvas::kTriangles_VertexMode, drawList->VtxBuffer.size(),
                                     pos.begin(), uv.begin(), color.begin(),
                                     drawList->IdxBuffer.begin() + indexOffset, drawCmd->ElemCount,
                                     *paint);
                indexOffset += drawCmd->ElemCount;
                canvas->restore();
            }
        }
    }
}

void Viewer::onIdle() {
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
                setupCurrentSlide(previousSlide);
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
                    fWindow->attach(fBackendType, DisplayParams());
                    fWindow->inval();
                    updateTitle();
                    updateUIState();
                }
                break;
            }
        }
    } else if (stateName.equals(kSoftkeyStateName)) {
        if (!stateValue.equals(kSoftkeyHint)) {
            fCommands.onSoftkey(stateValue);
            updateUIState(); // This is still needed to reset the value to kSoftkeyHint
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

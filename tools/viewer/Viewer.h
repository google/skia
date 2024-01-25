/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Viewer_DEFINED
#define Viewer_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "modules/skcms/skcms.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "tools/gpu/MemoryCache.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/CommandSet.h"
#include "tools/sk_app/Window.h"
#include "tools/viewer/AnimTimer.h"
#include "tools/viewer/ImGuiLayer.h"
#include "tools/viewer/StatsLayer.h"
#include "tools/viewer/TouchGesture.h"
#include "tools/window/DisplayParams.h"

#include <cstdint>
#include <atomic>
#include <functional>
#include <string>

class SkImage;
class SkSurface;
class Slide;
namespace skui {
enum class InputState;
enum class Key;
enum class ModifierKey;
}  // namespace skui

class Viewer : public sk_app::Application, sk_app::Window::Layer {
public:
    Viewer(int argc, char** argv, void* platformData);
    ~Viewer() override;

    void onIdle() override;

    void onBackendCreated() override;
    void onPaint(SkSurface*) override;
    void onResize(int width, int height) override;
    bool onTouch(intptr_t owner, skui::InputState state, float x, float y) override;
    bool onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) override;
    bool onMouseWheel(float delta, int x, int y, skui::ModifierKey) override;
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue) override;
    bool onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers) override;
    bool onChar(SkUnichar c, skui::ModifierKey modifiers) override;
    bool onPinch(skui::InputState state, float scale, float x, float y) override;
    bool onFling(skui::InputState state) override;

    static GrContextOptions::ShaderErrorHandler* ShaderErrorHandler();

    struct SkFontFields {
        bool overridesSomething() const {
            return fTypeface ||
                   fSize ||
                   fScaleX ||
                   fSkewX ||
                   fHinting ||
                   fEdging ||
                   fSubpixel ||
                   fForceAutoHinting ||
                   fEmbeddedBitmaps ||
                   fLinearMetrics ||
                   fEmbolden ||
                   fBaselineSnap;
        }

        bool fTypeface = false;
        bool fSize = false;
        SkScalar fSizeRange[2] = { 0, 20 };
        bool fScaleX = false;
        bool fSkewX = false;
        bool fHinting = false;
        bool fEdging = false;
        bool fSubpixel = false;
        bool fForceAutoHinting = false;
        bool fEmbeddedBitmaps = false;
        bool fLinearMetrics = false;
        bool fEmbolden = false;
        bool fBaselineSnap = false;
    };
    struct SkPaintFields {
        bool overridesSomething() const {
            return fPathEffect ||
                   fShader ||
                   fMaskFilter ||
                   fColorFilter ||
                   fImageFilter ||
                   fColor ||
                   fStrokeWidth ||
                   fMiterLimit ||
                   fBlendMode ||
                   fAntiAlias ||
                   fDither ||
                   fForceRuntimeBlend ||
                   fCapType ||
                   fJoinType ||
                   fStyle;
        }

        bool fPathEffect = false;
        bool fShader = false;
        bool fMaskFilter = false;
        bool fColorFilter = false;
        bool fImageFilter = false;

        bool fColor = false;
        bool fStrokeWidth = false;
        bool fMiterLimit = false;
        bool fBlendMode = false;

        bool fAntiAlias = false;
        bool fDither = false;
        bool fForceRuntimeBlend = false;

        bool fCapType = false;
        bool fJoinType = false;
        bool fStyle = false;
    };
    struct SkSurfacePropsFields {
        bool fFlags = false;
        bool fPixelGeometry = false;
    };
    struct DisplayFields {
        bool fColorType = false;
        bool fColorSpace = false;
        bool fMSAASampleCount = false;
        bool fGrContextOptions = false;
        SkSurfacePropsFields fSurfaceProps;
        bool fDisableVsync = false;
    };
private:
    enum class ColorMode {
        kLegacy,                // 8888, no color management
        kColorManaged8888,      // 8888 with color management
        kColorManagedF16,       // F16 with color management
        kColorManagedF16Norm,   // Normalized F16 with color management
    };

    void initSlides();
    void updateTitle();
    void setBackend(sk_app::Window::BackendType);
    void setColorMode(ColorMode);
    int startupSlide() const;
    void setCurrentSlide(int);
    void setupCurrentSlide();
    SkISize currentSlideSize() const;
    void listNames() const;
    void dumpShadersToResources();

    void updateUIState();

    void drawSlide(SkSurface* surface);
    void drawImGui();

    void changeZoomLevel(float delta);
    void preTouchMatrixChanged();
    SkMatrix computePreTouchMatrix();
    SkMatrix computePerspectiveMatrix();
    SkMatrix computeMatrix();
    SkPoint mapEvent(float x, float y);

    sk_app::Window*        fWindow;

    StatsLayer             fStatsLayer;
    StatsLayer::Timer      fPaintTimer;
    StatsLayer::Timer      fFlushTimer;
    StatsLayer::Timer      fAnimateTimer;

    AnimTimer              fAnimTimer;
    skia_private::TArray<sk_sp<Slide>> fSlides;
    int                    fCurrentSlide;

    bool                   fRefresh; // whether to continuously refresh for measuring render time

    bool                   fSaveToSKP;
    bool                   fShowSlideDimensions;

    ImGuiLayer             fImGuiLayer;
    SkPaint                fImGuiGamutPaint;
    bool                   fShowImGuiDebugWindow;
    bool                   fShowSlidePicker;
    bool                   fShowImGuiTestWindow;
    bool                   fShowHistogramWindow;

    bool                   fShowZoomWindow;
    bool                   fZoomWindowFixed;
    SkPoint                fZoomWindowLocation;
    sk_sp<SkImage>         fLastImage;
    bool                   fZoomUI;

    sk_app::Window::BackendType fBackendType;

    // Color properties for slide rendering
    ColorMode              fColorMode;
    SkColorSpacePrimaries  fColorSpacePrimaries;
    skcms_TransferFunction fColorSpaceTransferFn;

    // transform data
    bool                   fApplyBackingScale;
    SkScalar               fZoomLevel;
    SkScalar               fRotation;
    SkVector               fOffset;

    sk_app::CommandSet     fCommands;

    enum class GestureDevice {
        kNone,
        kTouch,
        kMouse,
    };

    TouchGesture           fGesture;
    GestureDevice          fGestureDevice;

    // identity unless the window initially scales the content to fit the screen.
    SkMatrix               fDefaultMatrix;

    bool                   fTiled;
    bool                   fDrawTileBoundaries;
    SkSize                 fTileScale;
    bool                   fDrawViaSerialize = false;

    enum PerspectiveMode {
        kPerspective_Off,
        kPerspective_Real,
        kPerspective_Fake,
    };
    PerspectiveMode        fPerspectiveMode;
    SkPoint                fPerspectivePoints[4];

    skia_private::TArray<std::function<void()>> fDeferredActions;

    // fPaint contains override values, fPaintOverrides controls if overrides are applied.
    SkPaint fPaint;
    SkPaintFields fPaintOverrides;

    // fFont contains override values, fFontOverrides controls if overrides are applied.
    SkFont fFont;
    SkFontFields fFontOverrides;

    // fDisplay contains default values (fWindow.fRequestedDisplayParams contains the overrides),
    // fDisplayOverrides controls if overrides are applied.
    skwindow::DisplayParams fDisplay;
    DisplayFields fDisplayOverrides;

    struct CachedShader {
        bool                fHovered = false;

        sk_sp<const SkData> fKey;
        SkString            fKeyString;
        SkString            fKeyDescription;

        SkFourByteTag            fShaderType;
        std::string              fShader[kGrShaderTypeCount];
        SkSL::Program::Interface fInterfaces[kGrShaderTypeCount];
    };

    sk_gpu_test::MemoryCache fPersistentCache;
    skia_private::TArray<CachedShader>   fCachedShaders;

    enum ShaderOptLevel : int {
        kShaderOptLevel_Source,
        kShaderOptLevel_Compile,
        kShaderOptLevel_Optimize,
        kShaderOptLevel_Inline,
    };
    ShaderOptLevel fOptLevel = kShaderOptLevel_Source;
};

#endif

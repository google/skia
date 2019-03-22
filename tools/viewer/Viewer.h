/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Viewer_DEFINED
#define Viewer_DEFINED

#include "AnimTimer.h"
#include "ImGuiLayer.h"
#include "SkExecutor.h"
#include "SkFont.h"
#include "SkScan.h"
#include "Slide.h"
#include "StatsLayer.h"
#include "TouchGesture.h"
#include "gm.h"
#include "sk_app/Application.h"
#include "sk_app/CommandSet.h"
#include "sk_app/Window.h"

class SkCanvas;

class Viewer : public sk_app::Application, sk_app::Window::Layer {
public:
    Viewer(int argc, char** argv, void* platformData);
    ~Viewer() override;

    void onIdle() override;

    void onBackendCreated() override;
    void onPaint(SkSurface*) override;
    void onResize(int width, int height) override;
    bool onTouch(intptr_t owner, sk_app::Window::InputState state, float x, float y) override;
    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override;
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue) override;
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onChar(SkUnichar c, uint32_t modifiers) override;

    struct SkFontFields {
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
    };
    struct SkPaintFields {
        bool fPathEffect = false;
        bool fShader = false;
        bool fMaskFilter = false;
        bool fColorFilter = false;
        bool fDrawLooper = false;
        bool fImageFilter = false;

        bool fColor = false;
        bool fWidth = false;
        bool fMiterLimit = false;
        bool fBlendMode = false;

        bool fAntiAlias = false;
        bool fDither = false;
        enum class AntiAliasState {
            Alias,
            Normal,
            AnalyticAAEnabled,
            AnalyticAAForced,
            DeltaAAEnabled,
            DeltaAAForced,
        } fAntiAliasState = AntiAliasState::Alias;
        const bool fOriginalSkUseAnalyticAA = gSkUseAnalyticAA;
        const bool fOriginalSkForceAnalyticAA = gSkForceAnalyticAA;
        const bool fOriginalSkUseDeltaAA = gSkUseDeltaAA;
        const bool fOriginalSkForceDeltaAA = gSkForceDeltaAA;

        bool fCapType = false;
        bool fJoinType = false;
        bool fStyle = false;
        bool fFilterQuality = false;
    };
private:
    enum class ColorMode {
        kLegacy,            // 8888, no color management
        kColorManaged8888,  // 8888 with color management
        kColorManagedF16,   // F16 with color management
    };

    void initSlides();
    void updateTitle();
    void setBackend(sk_app::Window::BackendType);
    void setColorMode(ColorMode);
    int startupSlide() const;
    void setCurrentSlide(int);
    void setupCurrentSlide();
    void listNames() const;

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
    SkTArray<sk_sp<Slide>> fSlides;
    int                    fCurrentSlide;

    bool                   fRefresh; // whether to continuously refresh for measuring render time

    bool                   fSaveToSKP;
    bool                   fShowSlideDimensions;

    ImGuiLayer             fImGuiLayer;
    SkPaint                fImGuiGamutPaint;
    bool                   fShowImGuiDebugWindow;
    bool                   fShowSlidePicker;
    bool                   fShowImGuiTestWindow;

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

    enum PerspectiveMode {
        kPerspective_Off,
        kPerspective_Real,
        kPerspective_Fake,
    };
    PerspectiveMode        fPerspectiveMode;
    SkPoint                fPerspectivePoints[4];

    SkTArray<std::function<void(void)>> fDeferredActions;

    SkPaint fPaint;
    SkPaintFields fPaintOverrides;
    SkFont fFont;
    SkFontFields fFontOverrides;
    bool fPixelGeometryOverrides = false;
};


#endif

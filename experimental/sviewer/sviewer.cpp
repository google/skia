// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/effects/SkGradientShader.h"

#include "tools/ModifierKey.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

#include <memory>

namespace {
static void draw(SkCanvas* c, SkISize s) {
    static constexpr SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(
                SkPoint::Make(s.width() * 0.5f, s.height() * 0.5f), 180.0f,
                colors, nullptr, 2, SkTileMode::kClamp, 0, nullptr));
    SkASSERT(paint.getShader());
    c->drawPaint(paint);
}

struct SViewerLayer : public sk_app::Window::Layer {
    sk_app::Window* fParent = nullptr;
    SkISize fSize = {100, 100};
    double fNow = -DBL_MAX;
    void inval() { if (fParent) { fParent->inval(); } }
    void onPaint(SkSurface* surface) override { draw(surface->getCanvas(), fSize); }
    void onResize(int width, int height) override {
        if (fSize != SkISize{width, height}) {
            fSize = SkISize{width, height};
            this->inval();
        }
    }
    void onAttach(sk_app::Window* w) override { fParent = w; }
    bool onMouseWheel(float delta, ModifierKey) override { return false; }
    bool onMouse(int x, int y, InputState, ModifierKey) override { return false; }
    bool onChar(SkUnichar, ModifierKey) override { return false; }
    bool onKey(sk_app::Window::Key, InputState, ModifierKey) override { return false; }
    bool onAnimate(double now) { fNow = now; return true; }
};

static constexpr sk_app::Window::BackendType kBackendType =
    sk_app::Window::kRaster_BackendType;
//  sk_app::Window::kNativeGL_BackendType;
//  sk_app::Window::kMetal_BackendType;

struct SViewerApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    SViewerLayer fLayer;
    SViewerApplication(std::unique_ptr<sk_app::Window> win) : fWindow(std::move(win)) {}
    bool init() {
        SkASSERT(fWindow);
        fWindow->attach(kBackendType);
        fWindow->pushLayer(&fLayer);
        fWindow->setTitle("SViewer");
        fLayer.onResize(fWindow->width(), fWindow->height());
        fWindow->show();
        fWindow->inval();
        return true;
    }
    ~SViewerApplication() override { fWindow->detach(); }
    void onIdle() override { if (fLayer.onAnimate(SkTime::GetNSecs())) { fWindow->inval(); } }
};
}  // namespace

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    std::unique_ptr<sk_app::Window> win(sk_app::Window::CreateNativeWindow(dat));
    if (!win) {
        SK_ABORT("CreateNativeWindow failed.");
    }
    std::unique_ptr<SViewerApplication> app(new SViewerApplication(std::move(win)));
    (void)app->init();
    return app.release();
}

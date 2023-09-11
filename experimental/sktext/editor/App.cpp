// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "src/base/SkTime.h"

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

#include "experimental/sktext/editor/Editor.h"

#include "third_party/icu/SkLoadICU.h"

#include <cfloat>
#include <fstream>
#include <memory>

using namespace skia::text;
namespace skia {
namespace editor {

struct EditorApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    std::unique_ptr<Editor> fLayer;
    double fNextTime = -DBL_MAX;

    EditorApplication(std::unique_ptr<sk_app::Window> win) : fWindow(std::move(win)) {}

    bool init(const char* path) {
        fWindow->attach(sk_app::Window::kRaster_BackendType);

        fLayer = Editor::MakeDemo(fWindow->width(), fWindow->height());

        fWindow->pushLayer(fLayer.get());
        fWindow->setTitle("Editor");

        fLayer->onResize(fWindow->width(), fWindow->height());
        fWindow->show();
        return true;
    }
    ~EditorApplication() override { fWindow->detach(); }

    void onIdle() override {
        double now = SkTime::GetNSecs();
        if (now >= fNextTime) {
            constexpr double kHalfPeriodNanoSeconds = 0.5 * 1e9;
            fNextTime = now + kHalfPeriodNanoSeconds;
            fLayer->blink();
            fWindow->inval();
        }
    }
};
} // namespace text
} // namespace skia

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    if (!SkLoadICU()) {
        SK_ABORT("SkLoadICU failed.");
    }
    std::unique_ptr<sk_app::Window> win(sk_app::Window::CreateNativeWindow(dat));
    if (!win) {
        SK_ABORT("CreateNativeWindow failed.");
    }
    std::unique_ptr<skia::editor::EditorApplication> app(new skia::editor::EditorApplication(std::move(win)));
    (void)app->init(argc > 1 ? argv[1] : nullptr);
    return app.release();
}

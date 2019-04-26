// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

namespace {
struct TestApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    TestApplication(std::unique_ptr<sk_app::Window> w) : fWindow(std::move(w)) {}
    ~TestApplication() override { fWindow->detach(); }
    void onIdle() override {}
};
}  // namespace

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    std::unique_ptr<sk_app::Window> window{sk_app::Window::CreateNativeWindow(dat)};
    window->attach(sk_app::Window::kRaster_BackendType);
    window->show();
    return new TestApplication(std::move(window));
}

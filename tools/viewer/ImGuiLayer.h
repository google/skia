/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef ImGuiLayer_DEFINED
#define ImGuiLayer_DEFINED

#include "SkPaint.h"
#include "SkTArray.h"
#include "sk_app/Window.h"
#include "imgui.h"

class ImGuiLayer : public sk_app::Window::Layer {
public:
    ImGuiLayer();

    typedef std::function<void(SkCanvas*)> SkiaWidgetFunc;
    void skiaWidget(const ImVec2& size, SkiaWidgetFunc func);

    void onAttach(sk_app::Window* window) override;
    void onPrePaint() override;
    void onPaint(SkCanvas* canvas) override;
    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onMouseWheel(float delta, uint32_t modifiers) override;
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onChar(SkUnichar c, uint32_t modifiers) override;

private:
    sk_app::Window* fWindow;
    SkPaint fFontPaint;
    SkTArray<SkiaWidgetFunc> fSkiaWidgetFuncs;
};

#endif

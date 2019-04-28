// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// [Work In Progress] Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "tools/sk_app/Application.h"
#include "tools/sk_app/CommandSet.h"
#include "tools/sk_app/Window.h"

#include "editor.h"

#include <fstream>
#include <iostream>
#include <memory>

namespace {

struct Timer {
    double fTime;
    const char* fDesc;
    Timer(const char* desc = "") : fTime(SkTime::GetNSecs()), fDesc(desc) {}
    ~Timer() { SkDebugf("%s: %d ms\n", fDesc, (int)((SkTime::GetNSecs() - fTime) * 1e-6)); }
};

static std::vector<editor::TextLine> read_file(const char* path) {
    std::vector<editor::TextLine> ret;
    if (path) {
        std::ifstream stream(path);
        for (std::string line; std::getline(stream, line);) {
            ret.push_back(editor::TextLine(line));
        }
    }
    return ret;
}

struct EditorLayer : public sk_app::Window::Layer {
    sk_app::Window* fParent = nullptr;
    editor::Editor fEditor;
    int fPos = 0;
    int fWidth = 0;
    int fHeight = 0;

    EditorLayer(std::vector<editor::TextLine> lines) {
        fEditor.setText(std::move(lines));
    }

    void onPaint(SkSurface* surface) override {
        Timer timer("painting");
        SkCanvas* c = surface->getCanvas();
        SkAutoCanvasRestore acr(c, true);
        c->clipRect({0, 0, (float)fWidth, (float)fHeight});
        c->translate(0, -(float)fPos);
        fEditor.paint(c);
    }

    void onResize(int width, int height) override {
        if (SkISize{fWidth, fHeight} != SkISize{width, height}) {
            fHeight = height;
            if (width != fWidth) {
                fWidth = width;
                Timer timer("shaping");
                fEditor.setWidth(fWidth);
            }
            if (fParent) {
                fParent->inval();
            }
        }
    }

    void onAttach(sk_app::Window* w) override { fParent = w; }

    bool onMouseWheel(float delta, uint32_t modifiers) override {
        int change = (int)(delta * fEditor.font().getSpacing());
        int maxPos = std::max(0, fEditor.getHeight() - fHeight);
        int newpos = std::max(0, std::min(fPos + change, maxPos));
        if (newpos != fPos) {
            fPos = newpos;
            if (fParent) { fParent->inval(); }
            return true;
        }
        return false;
    }

    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override {
        if (sk_app::Window::kDown_InputState == state) {
            y += fPos;
            y -= fEditor.getMargin();
            if (y >= 0) {
                const std::vector<editor::TextLine>& lines = fEditor.lines();
                for (unsigned i = 0; i < lines.size(); ++i) {
                    if (y < lines[i].fHeight) {
                        fEditor.select(i);
                        SkDebugf("  %d %d\n", x - fEditor.getMargin(), y);
                        fParent->inval();
                        break;
                    }
                    y -= lines[i].fHeight;
                }
            }
            return true;
        }
        return false;
    }
};

struct EditorApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    std::unique_ptr<EditorLayer> fLayer;
    sk_app::CommandSet fCommandSet;

    EditorApplication(const char* path, void* platformData)
        : fWindow(sk_app::Window::CreateNativeWindow(platformData))
    {
        fWindow->attach(sk_app::Window::kRaster_BackendType);
        fLayer.reset(new EditorLayer(read_file(path)));
        fWindow->pushLayer(fLayer.get());
        fCommandSet.attach(fWindow.get());
        fWindow->show();
        fLayer->onResize(fWindow->width(), fWindow->height());
    }
    ~EditorApplication() override { fWindow->detach(); }

    void onIdle() override {}
};

}  // namespace

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    const char* path = argc > 1 ? argv[1] : nullptr;
    return new EditorApplication(path, dat);
}

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// [Work In Progress] Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

#include "editor.h"

#include <memory>

static const char* key_name(sk_app::Window::Key k) {
    switch (k) {
        #define M(X) case sk_app::Window::Key::k ## X: return #X
        M(NONE); M(LeftSoftKey); M(RightSoftKey); M(Home); M(Back); M(Send); M(End); M(0); M(1);
        M(2); M(3); M(4); M(5); M(6); M(7); M(8); M(9); M(Star); M(Hash); M(Up); M(Down); M(Left);
        M(Right); M(Tab); M(PageUp); M(PageDown); M(Delete); M(Escape); M(Shift); M(Ctrl);
        M(Option); M(A); M(C); M(V); M(X); M(Y); M(Z); M(OK); M(VolUp); M(VolDown); M(Power);
        M(Camera);
        #undef M
        default: return "?";
    }
}

static SkString modifiers_desc(uint32_t m) {
    SkString s;
    #define M(X) if (m & sk_app::Window::k ## X ##_ModifierKey) { s.append(" {" #X "}"); }
    M(Shift) M(Control) M(Option) M(Command) M(FirstPress)
    #undef M
    return s;
}

namespace {

struct Timer {
    double fTime;
    const char* fDesc;
    Timer(const char* desc = "") : fTime(SkTime::GetNSecs()), fDesc(desc) {}
    ~Timer() { SkDebugf("%s: %5d μs\n", fDesc, (int)((SkTime::GetNSecs() - fTime) * 1e-3)); }
};


struct EditorLayer : public sk_app::Window::Layer {
    sk_app::Window* fParent = nullptr;
    editor::Editor fEditor;
    int fPos = 0;  // window pixel position in file
    int fWidth = 0;  // window width
    int fHeight = 0;  // window height

    void loadFile(const char* path) {
        if (sk_sp<SkData> data = SkData::MakeFromFileName(path)) {
            fEditor.setText((const char*)data->data(), data->size());
        }
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


    void scroll(int delta) {
        int maxPos = std::max(0, fEditor.getHeight() - fHeight / 2);
        int newpos = std::max(0, std::min(fPos + delta, maxPos));
        if (newpos != fPos) {
            fPos = newpos;
            if (fParent) { fParent->inval(); }
        }
    }

    bool onMouseWheel(float delta, uint32_t modifiers) override {
        this->scroll(-(int)(delta * fEditor.font().getSpacing()));
        return true;
    }

    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override {
        if (sk_app::Window::kDown_InputState == state) {
            y += fPos;
            y -= fEditor.getMargin();
            if (y >= 0) {
                for (size_t i = 0; i < fEditor.lineCount(); ++i) {
                    int height = fEditor.lineHeight(i);
                    if (y < height) {
                        fEditor.select(i);
                        SkDebugf("select:  line:%d x:%d y:%d\n", i, x - fEditor.getMargin(), y);
                        fParent->inval();
                        break;
                    }
                    y -= height;
                }
            }
            return true;
        }
        return false;
    }

    bool onChar(SkUnichar c, uint32_t modifiers) override {
        SkString m = modifiers_desc(modifiers);
        SkDebugf("char: %c (0x%08X)%s\n", (char)(c & 0xFF), (unsigned)c, m.c_str());
        return true;
    }
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers) override {
        if (state == sk_app::Window::kDown_InputState) {
            if (key == sk_app::Window::Key::kPageDown) {
                this->scroll(fHeight * 4 / 5);
                return true;
            }
            if (key == sk_app::Window::Key::kPageUp) {
                this->scroll(-fHeight * 4 / 5);
                return true;
            }
            SkString m = modifiers_desc(modifiers);
            SkDebugf("key: %s%s\n", key_name(key), m.c_str());
        }
        return true;
    }

};

struct EditorApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    EditorLayer fLayer;

    EditorApplication(const char* path, void* platformData)
        : fWindow(sk_app::Window::CreateNativeWindow(platformData))
    {
        //sk_app::Window::BackendType backendType = sk_app::Window::kRaster_BackendType;
        sk_app::Window::BackendType backendType = sk_app::Window::kNativeGL_BackendType;
        fWindow->attach(backendType);
        fLayer.loadFile(path);
        fWindow->pushLayer(&fLayer);
        fWindow->show();
        fLayer.onResize(fWindow->width(), fWindow->height());
    }
    ~EditorApplication() override { fWindow->detach(); }

    void onIdle() override {}
};
}  // namespace

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    return new EditorApplication(argc > 1 ? argv[1] : nullptr, dat);
}

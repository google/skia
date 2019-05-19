// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// [Work In Progress] Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

#include "experimental/editor/editor.h"

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

static void debug_on_char(SkUnichar c, uint32_t modifiers) {
    SkString m = modifiers_desc(modifiers);
    if ((unsigned)c < 0x100) {
        SkDebugf("char: %c (0x%02X)%s\n", (char)(c & 0xFF), (unsigned)c, m.c_str());
    } else {
        SkDebugf("char: 0x%08X%s\n", (unsigned)c, m.c_str());
    }
}

static void debug_on_key(sk_app::Window::Key key, sk_app::Window::InputState, uint32_t modifiers) {
    SkDebugf("key: %s%s\n", key_name(key), modifiers_desc(modifiers).c_str());
}

static editor::Editor::Movement convert(sk_app::Window::Key key) {
    switch (key) {
        case sk_app::Window::Key::kLeft:  return editor::Editor::Movement::kLeft;
        case sk_app::Window::Key::kRight: return editor::Editor::Movement::kRight;
        case sk_app::Window::Key::kUp:    return editor::Editor::Movement::kUp;
        case sk_app::Window::Key::kDown:  return editor::Editor::Movement::kDown;
        case sk_app::Window::Key::kHome:  return editor::Editor::Movement::kHome;
        case sk_app::Window::Key::kEnd:   return editor::Editor::Movement::kEnd;
        default: return editor::Editor::Movement::kNowhere;
    }
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
    editor::Editor::TextPosition fTextPos{0, 0};
    editor::Editor::TextPosition fMarkPos;
    int fPos = 0;  // window pixel position in file
    int fWidth = 0;  // window width
    int fHeight = 0;  // window height
    bool fShiftDown = false;

    EditorLayer() {
        fEditor.setFont(SkFont(nullptr, 24));
    }

    void loadFile(const char* path) {
        if (sk_sp<SkData> data = SkData::MakeFromFileName(path)) {
            fEditor.setText((const char*)data->data(), data->size());
        }
    }

    void onPaint(SkSurface* surface) override {
        Timer timer("painting");
        SkCanvas* canvas = surface->getCanvas();
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect({0, 0, (float)fWidth, (float)fHeight});
        canvas->translate(0, -(float)fPos);
        editor::Editor::PaintOpts options;
        options.fCursor = fTextPos;
        options.fBackgroundColor = SkColor4f{0.8f, 0.8f, 0.8f, 1};
        if (fMarkPos != editor::Editor::TextPosition()) {
            options.fSelectionBegin = fMarkPos;
            options.fSelectionEnd = fTextPos;
        }
        fEditor.paint(canvas, options);
    }

    void onResize(int width, int height) override {
        if (SkISize{fWidth, fHeight} != SkISize{width, height}) {
            fHeight = height;
            if (width != fWidth) {
                fWidth = width;
                Timer timer("shaping");
                fEditor.setWidth(fWidth);
            }
            this->inval();
        }
    }

    void onAttach(sk_app::Window* w) override { fParent = w; }

    bool scroll(int delta) {
        int maxPos = std::max(0, fEditor.getHeight() - fHeight / 2);
        int newpos = std::max(0, std::min(fPos + delta, maxPos));
        if (newpos != fPos) {
            fPos = newpos;
            this->inval();
        }
        return true;
    }

    void inval() { if (fParent) { fParent->inval(); } }

    bool onMouseWheel(float delta, uint32_t modifiers) override {
        this->scroll(-(int)(delta * fEditor.font().getSpacing()));
        return true;
    }

    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override {
        if (sk_app::Window::kDown_InputState == state) {
            y += fPos;
            editor::Editor::TextPosition pos = fEditor.getPosition(SkIPoint{x, y});
            SkDebugf("select:  line:%d column:%d \n", pos.fParagraphIndex, pos.fTextByteIndex);
            if (pos != editor::Editor::TextPosition()) {
                fTextPos = pos;
                this->inval();
            }
        }
        return true;
    }

    bool onChar(SkUnichar c, uint32_t modifiers) override {
        if (0 == (modifiers & (sk_app::Window::kControl_ModifierKey |
                               sk_app::Window::kOption_ModifierKey |
                               sk_app::Window::kCommand_ModifierKey))) {
            if ((unsigned) c < 0x7F && (unsigned)c >= 0x20) {
                char ch = (char)c;
                fEditor.insert(fTextPos, &ch, 1);
                SkDebugf("insert: %X'%c'\n", (unsigned)c, ch);
                return this->moveCursor(editor::Editor::Movement::kRight);
            }
        }
        debug_on_char(c, modifiers);
        return false;
    }
    bool moveCursor(editor::Editor::Movement m, bool shift = false) {
        if (shift != fShiftDown) {
            fMarkPos = shift ? fTextPos :  editor::Editor::TextPosition();
            fShiftDown = shift;
        }
        fTextPos = fEditor.move(m, fTextPos);
        this->inval();
        return true;
    }
    bool onKey(sk_app::Window::Key key,
               sk_app::Window::InputState state,
               uint32_t modifiers) override {
        if (state == sk_app::Window::kDown_InputState) {
            switch (key) {
                case sk_app::Window::Key::kPageDown:
                    return this->scroll(fHeight * 4 / 5);
                case sk_app::Window::Key::kPageUp:
                    return this->scroll(-fHeight * 4 / 5);
                case sk_app::Window::Key::kLeft:
                case sk_app::Window::Key::kRight:
                case sk_app::Window::Key::kUp:
                case sk_app::Window::Key::kDown:
                case sk_app::Window::Key::kHome:
                case sk_app::Window::Key::kEnd:
                    return this->moveCursor(convert(key),
                                            modifiers & sk_app::Window::kShift_ModifierKey);
                case sk_app::Window::Key::kDelete:
                    if (fMarkPos != editor::Editor::TextPosition()) {
                        fTextPos = fEditor.remove(fMarkPos, fTextPos);
                        fMarkPos = editor::Editor::TextPosition();
                    } else {
                        fTextPos = fEditor.remove(fTextPos,
                                                  fEditor.move(editor::Editor::Movement::kRight,
                                                               fTextPos));
                    }
                    this->inval();
                    return true;
                case sk_app::Window::Key::kBack:
                    if (fMarkPos != editor::Editor::TextPosition()) {
                        fTextPos = fEditor.remove(fMarkPos, fTextPos);
                        fMarkPos = editor::Editor::TextPosition();
                    } else {
                        fTextPos = fEditor.remove(fTextPos,
                                                  fEditor.move(editor::Editor::Movement::kLeft,
                                                               fTextPos));
                    }
                    this->inval();
                    return true;
                default:
                    break;
            }
            debug_on_key(key, state, modifiers);
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
        fLayer.inval();
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

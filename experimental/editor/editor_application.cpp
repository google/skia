// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// [Work In Progress] Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

#include "tools/ModifierKey.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

#include "experimental/editor/editor.h"

#include "third_party/icu/SkLoadICU.h"

#include <fstream>
#include <memory>

#ifdef SK_EDITOR_DEBUG_OUT
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

static SkString modifiers_desc(ModifierKey m) {
    SkString s;
    #define M(X) if (m & ModifierKey::k ## X ##) { s.append(" {" #X "}"); }
    M(Shift) M(Control) M(Option) M(Command) M(FirstPress)
    #undef M
    return s;
}

static void debug_on_char(SkUnichar c, ModifierKey modifiers) {
    SkString m = modifiers_desc(modifiers);
    if ((unsigned)c < 0x100) {
        SkDebugf("char: %c (0x%02X)%s\n", (char)(c & 0xFF), (unsigned)c, m.c_str());
    } else {
        SkDebugf("char: 0x%08X%s\n", (unsigned)c, m.c_str());
    }
}

static void debug_on_key(sk_app::Window::Key key, InputState, ModifierKey modi) {
    SkDebugf("key: %s%s\n", key_name(key), modifiers_desc(modi).c_str());
}
#endif  // SK_EDITOR_DEBUG_OUT

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
    SkString fPath;
    sk_app::Window* fParent = nullptr;
    std::vector<char> fClipboard;
    editor::Editor fEditor;
    editor::Editor::TextPosition fTextPos{0, 0};
    editor::Editor::TextPosition fMarkPos;
    int fPos = 0;  // window pixel position in file
    int fWidth = 0;  // window width
    int fHeight = 0;  // window height
    int fMargin = 10;
    bool fShiftDown = false;
    bool fBlink = false;

    void loadFile(const char* path) {
        if (sk_sp<SkData> data = SkData::MakeFromFileName(path)) {
            fPath = path;
            fEditor.insert(editor::Editor::TextPosition{0, 0},
                           (const char*)data->data(), data->size());
        } else {
            fPath  = "output.txt";
        }
    }

    void onPaint(SkSurface* surface) override {
        SkCanvas* canvas = surface->getCanvas();
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect({0, 0, (float)fWidth, (float)fHeight});
        canvas->translate(fMargin, (float)(fMargin - fPos));
        editor::Editor::PaintOpts options;
        options.fCursor = fTextPos;
        options.fCursorColor = {1, 0, 0, fBlink ? 0.0f : 1.0f};
        options.fBackgroundColor = SkColor4f{0.8f, 0.8f, 0.8f, 1};
        options.fCursorColor = {1, 0, 0, fBlink ? 0.0f : 1.0f};
        if (fMarkPos != editor::Editor::TextPosition()) {
            options.fSelectionBegin = fMarkPos;
            options.fSelectionEnd = fTextPos;
        }
        #ifdef SK_EDITOR_DEBUG_OUT
        {
            Timer timer("shaping");
            fEditor.paint(nullptr, options);
        }
        Timer timer("painting");
        #endif  // SK_EDITOR_DEBUG_OUT
        fEditor.paint(canvas, options);
    }

    void onResize(int width, int height) override {
        if (SkISize{fWidth, fHeight} != SkISize{width, height}) {
            fHeight = height;
            if (width != fWidth) {
                fWidth = width;
                fEditor.setWidth(fWidth - 2 * fMargin);
            }
            this->inval();
        }
    }

    void onAttach(sk_app::Window* w) override { fParent = w; }

    bool scroll(int delta) {
        int maxPos = std::max(0, fEditor.getHeight() + 2 * fMargin - fHeight / 2);
        int newpos = std::max(0, std::min(fPos + delta, maxPos));
        if (newpos != fPos) {
            fPos = newpos;
            this->inval();
        }
        return true;
    }

    void inval() { if (fParent) { fParent->inval(); } }

    bool onMouseWheel(float delta, ModifierKey modifiers) override {
        this->scroll(-(int)(delta * fEditor.font().getSpacing()));
        return true;
    }

    bool onMouse(int x, int y, InputState state, ModifierKey modifiers) override {
        if (InputState::kDown == state) {
            y += fPos - fMargin;
            x -= fMargin;
            editor::Editor::TextPosition pos = fEditor.getPosition(SkIPoint{x, y});
            #ifdef SK_EDITOR_DEBUG_OUT
            SkDebugf("select:  line:%d column:%d \n", pos.fParagraphIndex, pos.fTextByteIndex);
            #endif  // SK_EDITOR_DEBUG_OUT
            if (pos != editor::Editor::TextPosition()) {
                fTextPos = pos;
                this->inval();
            }
        }
        return true;
    }

    bool onChar(SkUnichar c, ModifierKey modifiers) override {
        if (!ModifierKeyIsSet(modifiers & (ModifierKey::kControl |
                                           ModifierKey::kOption  |
                                           ModifierKey::kCommand))) {
            if (((unsigned)c < 0x7F && (unsigned)c >= 0x20) || c == '\n') {
                char ch = (char)c;
                fEditor.insert(fTextPos, &ch, 1);
                #ifdef SK_EDITOR_DEBUG_OUT
                SkDebugf("insert: %X'%c'\n", (unsigned)c, ch);
                #endif  // SK_EDITOR_DEBUG_OUT
                return this->moveCursor(editor::Editor::Movement::kRight);
            }
        }
        if (modifiers == ModifierKey::kControl) {
            switch (c) {
                case 'p':
                    for (editor::StringView str : fEditor.text()) {
                        SkDebugf(">>  '%.*s'\n", str.size, str.data);
                    }
                    return true;
                case 's':
                    {
                        std::ofstream out(fPath.c_str());
                        for (editor::StringView str : fEditor.text()) {
                            out.write(str.data, str.size) << '\n';
                        }
                    }
                    return true;
                case 'c':
                    if (fMarkPos != editor::Editor::TextPosition()) {
                        fClipboard.resize(fEditor.copy(fMarkPos, fTextPos, nullptr));
                        fEditor.copy(fMarkPos, fTextPos, fClipboard.data());
                        return true;
                    }
                case 'x':
                    if (fMarkPos != editor::Editor::TextPosition()) {
                        fClipboard.resize(fEditor.copy(fMarkPos, fTextPos, nullptr));
                        fEditor.copy(fMarkPos, fTextPos, fClipboard.data());
                        fTextPos = fEditor.remove(fMarkPos, fTextPos);
                        this->inval();
                        return true;
                    }
                case 'v':
                    if (fClipboard.size()) {
                        fEditor.insert(fTextPos, fClipboard.data(), fClipboard.size());
                        this->inval();
                        return true;
                    }
            }
        }
        #ifdef SK_EDITOR_DEBUG_OUT
        debug_on_char(c, modifiers);
        #endif  // SK_EDITOR_DEBUG_OUT
        return false;
    }
    bool moveCursor(editor::Editor::Movement m, bool shift = false) {
        if (shift != fShiftDown) {
            fMarkPos = shift ? fTextPos :  editor::Editor::TextPosition();
            fShiftDown = shift;
        }
        fTextPos = fEditor.move(m, fTextPos);

        // scroll if needed.
        SkIRect cursor = fEditor.getLocation(fTextPos).roundOut();
        if (fPos < cursor.bottom() - fHeight + 2 * fMargin) {
            fPos = cursor.bottom() - fHeight + 2 * fMargin;
        } else if (cursor.top() < fPos) {
            fPos = cursor.top();
        }
        this->inval();
        return true;
    }

    bool onKey(sk_app::Window::Key key,
               InputState state,
               ModifierKey modifiers) override {
        if (state != InputState::kDown) {
            return false;  // ignore keyup
        }
        // ignore other modifiers.
        ModifierKey ctrlAltCmd = modifiers & (ModifierKey::kControl |
                                              ModifierKey::kOption  |
                                              ModifierKey::kCommand);
        if (!ModifierKeyIsSet(ctrlAltCmd)) {
            // no modifiers
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
                                            (int)(modifiers & ModifierKey::kShift));
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
                case sk_app::Window::Key::kOK:
                    return this->onChar('\n', modifiers);
                default:
                    break;
            }
        } else if (ModifierKeyIsSet(ctrlAltCmd & (ModifierKey::kControl | ModifierKey::kCommand))) {
            switch (key) {
                case sk_app::Window::Key::kLeft:
                    return this->moveCursor(editor::Editor::Movement::kWordLeft,
                                            (int)(modifiers & ModifierKey::kShift));
                case sk_app::Window::Key::kRight:
                    return this->moveCursor(editor::Editor::Movement::kWordRight,
                                            (int)(modifiers & ModifierKey::kShift));
                default:
                    break;
            }
        }
        #ifdef SK_EDITOR_DEBUG_OUT
        debug_on_key(key, state, modifiers);
        #endif  // SK_EDITOR_DEBUG_OUT
        return false;
    }
};

static constexpr float kFontSize = 18;
// static constexpr char kTypefaceName[] = "monospace";
static constexpr char kTypefaceName[] = "sans-serif";
static constexpr SkFontStyle::Weight kFontWeight = SkFontStyle::kNormal_Weight;
static constexpr SkFontStyle::Width  kFontWidth  = SkFontStyle::kNormal_Width;
static constexpr SkFontStyle::Slant  kFontSlant  = SkFontStyle::kUpright_Slant;

//static constexpr sk_app::Window::BackendType kBackendType = sk_app::Window::kRaster_BackendType;
static constexpr sk_app::Window::BackendType kBackendType = sk_app::Window::kNativeGL_BackendType;

struct EditorApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    EditorLayer fLayer;
    double fNextTime = -DBL_MAX;

    EditorApplication(void* platformData)
        : fWindow(sk_app::Window::CreateNativeWindow(platformData)) {}

    bool init(const char* path) {
        fWindow->attach(kBackendType);
        fLayer.inval();

        fLayer.fEditor.setFont(SkFont(SkTypeface::MakeFromName(kTypefaceName,
                               SkFontStyle(kFontWeight, kFontWidth, kFontSlant)), kFontSize));
        fLayer.loadFile(path);

        fWindow->pushLayer(&fLayer);
        fWindow->setTitle(SkStringPrintf("Editor: \"%s\"", fLayer.fPath.c_str()).c_str());
        fWindow->show();
        fLayer.onResize(fWindow->width(), fWindow->height());
        return true;
    }
    ~EditorApplication() override { fWindow->detach(); }

    void onIdle() override {
        double now = SkTime::GetNSecs();
        if (now >= fNextTime) {
            constexpr double kHalfPeriodNanoSeconds = 0.5 * 1e9;
            fNextTime = now + kHalfPeriodNanoSeconds;
            fLayer.fBlink = !fLayer.fBlink;
            fWindow->inval();
        }
    }
};
}  // namespace

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    if (!SkLoadICU()) {
        SK_ABORT("SkLoadICU failed.");
    }
    EditorApplication* app = new EditorApplication(dat);
    SkASSERT(app->init(argc > 1 ? argv[1] : nullptr));
    return app;
}

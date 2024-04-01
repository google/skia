// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkSurface.h"
#include "src/base/SkTime.h"

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

#include "modules/skplaintexteditor/include/editor.h"

#include "third_party/icu/SkLoadICU.h"

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#if defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
#include "include/ports/SkTypeface_win.h"
#endif

#include <cfloat>
#include <fstream>
#include <memory>

using SkPlainTextEditor::Editor;
using SkPlainTextEditor::StringView;

#ifdef SK_EDITOR_DEBUG_OUT
static const char* key_name(skui::Key k) {
    switch (k) {
        #define M(X) case skui::Key::k ## X: return #X
        M(NONE); M(LeftSoftKey); M(RightSoftKey); M(Home); M(Back); M(Send); M(End); M(0); M(1);
        M(2); M(3); M(4); M(5); M(6); M(7); M(8); M(9); M(Star); M(Hash); M(Up); M(Down); M(Left);
        M(Right); M(Tab); M(PageUp); M(PageDown); M(Delete); M(Escape); M(Shift); M(Ctrl);
        M(Option); M(A); M(C); M(V); M(X); M(Y); M(Z); M(OK); M(VolUp); M(VolDown); M(Power);
        M(Camera);
        #undef M
        default: return "?";
    }
}

static SkString modifiers_desc(skui::ModifierKey m) {
    SkString s;
    #define M(X) if (m & skui::ModifierKey::k ## X ##) { s.append(" {" #X "}"); }
    M(Shift) M(Control) M(Option) M(Command) M(FirstPress)
    #undef M
    return s;
}

static void debug_on_char(SkUnichar c, skui::ModifierKey modifiers) {
    SkString m = modifiers_desc(modifiers);
    if ((unsigned)c < 0x100) {
        SkDebugf("char: %c (0x%02X)%s\n", (char)(c & 0xFF), (unsigned)c, m.c_str());
    } else {
        SkDebugf("char: 0x%08X%s\n", (unsigned)c, m.c_str());
    }
}

static void debug_on_key(skui::Key key, skui::InputState, skui::ModifierKey modi) {
    SkDebugf("key: %s%s\n", key_name(key), modifiers_desc(modi).c_str());
}
#endif  // SK_EDITOR_DEBUG_OUT

static Editor::Movement convert(skui::Key key) {
    switch (key) {
        case skui::Key::kLeft:  return Editor::Movement::kLeft;
        case skui::Key::kRight: return Editor::Movement::kRight;
        case skui::Key::kUp:    return Editor::Movement::kUp;
        case skui::Key::kDown:  return Editor::Movement::kDown;
        case skui::Key::kHome:  return Editor::Movement::kHome;
        case skui::Key::kEnd:   return Editor::Movement::kEnd;
        default: return Editor::Movement::kNowhere;
    }
}
namespace {

struct Timer {
    double fTime;
    const char* fDesc;
    Timer(const char* desc = "") : fTime(SkTime::GetNSecs()), fDesc(desc) {}
    ~Timer() { SkDebugf("%s: %5d μs\n", fDesc, (int)((SkTime::GetNSecs() - fTime) * 1e-3)); }
};

static constexpr float kFontSize = 18;
static const char* kTypefaces[3] = {"sans-serif", "serif", "monospace"};
static constexpr size_t kTypefaceCount = std::size(kTypefaces);

static constexpr SkFontStyle::Weight kFontWeight = SkFontStyle::kNormal_Weight;
static constexpr SkFontStyle::Width  kFontWidth  = SkFontStyle::kNormal_Width;
static constexpr SkFontStyle::Slant  kFontSlant  = SkFontStyle::kUpright_Slant;

// Note: initialization is not thread safe
sk_sp<SkFontMgr> fontMgr() {
    static bool init = false;
    static sk_sp<SkFontMgr> fontMgr = nullptr;
    if (!init) {
#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
        fontMgr = SkFontMgr_New_FontConfig(nullptr);
#elif defined(SK_FONTMGR_CORETEXT_AVAILABLE)
        fontMgr = SkFontMgr_New_CoreText(nullptr);
#elif defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
        fontMgr = SkFontMgr_New_DirectWrite();
#endif
        init = true;
    }
    return fontMgr;
}

struct EditorLayer : public sk_app::Window::Layer {
    EditorLayer() {
        fEditor.setFontMgr(fontMgr());
    }

    SkString fPath;
    sk_app::Window* fParent = nullptr;
    // TODO(halcanary): implement a cross-platform clipboard interface.
    std::vector<char> fClipboard;
    Editor fEditor;
    Editor::TextPosition fTextPos{0, 0};
    Editor::TextPosition fMarkPos;
    int fPos = 0;  // window pixel position in file
    int fWidth = 0;  // window width
    int fHeight = 0;  // window height
    int fMargin = 10;
    size_t fTypefaceIndex = 0;
    float fFontSize = kFontSize;
    bool fShiftDown = false;
    bool fBlink = false;
    bool fMouseDown = false;

    void setFont() {
        fEditor.setFont(SkFont(fontMgr()->matchFamilyStyle(kTypefaces[fTypefaceIndex],
                               SkFontStyle(kFontWeight, kFontWidth, kFontSlant)), fFontSize));
    }


    void loadFile(const char* path) {
        if (sk_sp<SkData> data = SkData::MakeFromFileName(path)) {
            fPath = path;
            fEditor.insert(Editor::TextPosition{0, 0},
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
        Editor::PaintOpts options;
        options.fCursor = fTextPos;
        options.fCursorColor = {1, 0, 0, fBlink ? 0.0f : 1.0f};
        options.fBackgroundColor = SkColor4f{0.8f, 0.8f, 0.8f, 1};
        options.fCursorColor = {1, 0, 0, fBlink ? 0.0f : 1.0f};
        if (fMarkPos != Editor::TextPosition()) {
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

    bool onMouseWheel(float delta, int, int, skui::ModifierKey) override {
        this->scroll(-(int)(delta * fEditor.font().getSpacing()));
        return true;
    }

    bool onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) override {
        bool mouseDown = skui::InputState::kDown == state;
        if (mouseDown) {
            fMouseDown = true;
        } else if (skui::InputState::kUp == state) {
            fMouseDown = false;
        }
        bool shiftOrDrag = sknonstd::Any(modifiers & skui::ModifierKey::kShift) || !mouseDown;
        if (fMouseDown) {
            return this->move(fEditor.getPosition({x - fMargin, y + fPos - fMargin}), shiftOrDrag);
        }
        return false;
    }

    bool onChar(SkUnichar c, skui::ModifierKey modi) override {
        using sknonstd::Any;
        modi &= ~skui::ModifierKey::kFirstPress;
        if (!Any(modi & (skui::ModifierKey::kControl |
                         skui::ModifierKey::kOption  |
                         skui::ModifierKey::kCommand))) {
            if (((unsigned)c < 0x7F && (unsigned)c >= 0x20) || c == '\n') {
                char ch = (char)c;
                fEditor.insert(fTextPos, &ch, 1);
                #ifdef SK_EDITOR_DEBUG_OUT
                SkDebugf("insert: %X'%c'\n", (unsigned)c, ch);
                #endif  // SK_EDITOR_DEBUG_OUT
                return this->moveCursor(Editor::Movement::kRight);
            }
        }
        static constexpr skui::ModifierKey kCommandOrControl = skui::ModifierKey::kCommand |
                                                               skui::ModifierKey::kControl;
        if (Any(modi & kCommandOrControl) && !Any(modi & ~kCommandOrControl)) {
            switch (c) {
                case 'p':
                    for (StringView str : fEditor.text()) {
                        SkDebugf(">>  '%.*s'\n", (int)str.size, str.data);
                    }
                    return true;
                case 's':
                    {
                        std::ofstream out(fPath.c_str());
                        size_t count = fEditor.lineCount();
                        for (size_t i = 0; i < count; ++i) {
                            if (i != 0) {
                                out << '\n';
                            }
                            StringView str = fEditor.line(i);
                            out.write(str.data, str.size);
                        }
                    }
                    return true;
                case 'c':
                    if (fMarkPos != Editor::TextPosition()) {
                        fClipboard.resize(fEditor.copy(fMarkPos, fTextPos, nullptr));
                        fEditor.copy(fMarkPos, fTextPos, fClipboard.data());
                        return true;
                    }
                    return false;
                case 'x':
                    if (fMarkPos != Editor::TextPosition()) {
                        fClipboard.resize(fEditor.copy(fMarkPos, fTextPos, nullptr));
                        fEditor.copy(fMarkPos, fTextPos, fClipboard.data());
                        (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
                        this->inval();
                        return true;
                    }
                    return false;
                case 'v':
                    if (fClipboard.size()) {
                        fEditor.insert(fTextPos, fClipboard.data(), fClipboard.size());
                        this->inval();
                        return true;
                    }
                    return false;
                case '0':
                    fTypefaceIndex = (fTypefaceIndex + 1) % kTypefaceCount;
                    this->setFont();
                    return true;
                case '=':
                case '+':
                    fFontSize = fFontSize + 1;
                    this->setFont();
                    return true;
                case '-':
                case '_':
                    if (fFontSize > 1) {
                        fFontSize = fFontSize - 1;
                        this->setFont();
                    }
            }
        }
        #ifdef SK_EDITOR_DEBUG_OUT
        debug_on_char(c, modifiers);
        #endif  // SK_EDITOR_DEBUG_OUT
        return false;
    }

    bool moveCursor(Editor::Movement m, bool shift = false) {
        return this->move(fEditor.move(m, fTextPos), shift);
    }

    bool move(Editor::TextPosition pos, bool shift) {
        if (pos == fTextPos || pos == Editor::TextPosition()) {
            if (!shift) {
                fMarkPos = Editor::TextPosition();
            }
            return false;
        }
        if (shift != fShiftDown) {
            fMarkPos = shift ? fTextPos : Editor::TextPosition();
            fShiftDown = shift;
        }
        fTextPos = pos;

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

    bool onKey(skui::Key key,
               skui::InputState state,
               skui::ModifierKey modifiers) override {
        if (state != skui::InputState::kDown) {
            return false;  // ignore keyup
        }
        // ignore other modifiers.
        using sknonstd::Any;
        skui::ModifierKey ctrlAltCmd = modifiers & (skui::ModifierKey::kControl |
                                              skui::ModifierKey::kOption  |
                                              skui::ModifierKey::kCommand);
        bool shift = Any(modifiers & (skui::ModifierKey::kShift));
        if (!Any(ctrlAltCmd)) {
            // no modifiers
            switch (key) {
                case skui::Key::kPageDown:
                    return this->scroll(fHeight * 4 / 5);
                case skui::Key::kPageUp:
                    return this->scroll(-fHeight * 4 / 5);
                case skui::Key::kLeft:
                case skui::Key::kRight:
                case skui::Key::kUp:
                case skui::Key::kDown:
                case skui::Key::kHome:
                case skui::Key::kEnd:
                    return this->moveCursor(convert(key), shift);
                case skui::Key::kDelete:
                    if (fMarkPos != Editor::TextPosition()) {
                        (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
                    } else {
                        auto pos = fEditor.move(Editor::Movement::kRight, fTextPos);
                        (void)this->move(fEditor.remove(fTextPos, pos), false);
                    }
                    this->inval();
                    return true;
                case skui::Key::kBack:
                    if (fMarkPos != Editor::TextPosition()) {
                        (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
                    } else {
                        auto pos = fEditor.move(Editor::Movement::kLeft, fTextPos);
                        (void)this->move(fEditor.remove(fTextPos, pos), false);
                    }
                    this->inval();
                    return true;
                case skui::Key::kOK:
                    return this->onChar('\n', modifiers);
                default:
                    break;
            }
        } else if (sknonstd::Any(ctrlAltCmd & (skui::ModifierKey::kControl |
                                               skui::ModifierKey::kCommand))) {
            switch (key) {
                case skui::Key::kLeft:
                    return this->moveCursor(Editor::Movement::kWordLeft, shift);
                case skui::Key::kRight:
                    return this->moveCursor(Editor::Movement::kWordRight, shift);
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

#ifdef SK_VULKAN
static constexpr sk_app::Window::BackendType kBackendType = sk_app::Window::kVulkan_BackendType;
#elif SK_METAL
static constexpr sk_app::Window::BackendType kBackendType = sk_app::Window::kMetal_BackendType;
#elif SK_GL
static constexpr sk_app::Window::BackendType kBackendType = sk_app::Window::kNativeGL_BackendType;
#else
static constexpr sk_app::Window::BackendType kBackendType = sk_app::Window::kRaster_BackendType;
#endif

struct EditorApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    EditorLayer fLayer;
    double fNextTime = -DBL_MAX;

    EditorApplication(std::unique_ptr<sk_app::Window> win) : fWindow(std::move(win)) {}

    bool init(const char* path) {
        fWindow->attach(kBackendType);

        fLayer.loadFile(path);
        fLayer.setFont();

        fWindow->pushLayer(&fLayer);
        fWindow->setTitle(SkStringPrintf("Editor: \"%s\"", fLayer.fPath.c_str()).c_str());
        fLayer.onResize(fWindow->width(), fWindow->height());
        fLayer.fEditor.paint(nullptr, Editor::PaintOpts());

        fWindow->show();
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
    std::unique_ptr<sk_app::Window> win(sk_app::Window::CreateNativeWindow(dat));
    if (!win) {
        SK_ABORT("CreateNativeWindow failed.");
    }
    std::unique_ptr<EditorApplication> app(new EditorApplication(std::move(win)));
    (void)app->init(argc > 1 ? argv[1] : nullptr);
    return app.release();
}

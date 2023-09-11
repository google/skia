// Copyright 2021 Google LLC.
#ifndef Editor_DEFINED
#define Editor_DEFINED
#include <sstream>
#include "experimental/sktext/editor/Cursor.h"
#include "experimental/sktext/editor/Defaults.h"
#include "experimental/sktext/editor/Mouse.h"
#include "experimental/sktext/editor/Selection.h"
#include "experimental/sktext/editor/Texts.h"
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Paint.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "src/base/SkTime.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

namespace skia {
namespace editor {

using namespace skia::text;

class Editor : public sk_app::Window::Layer {
public:
    static std::unique_ptr<Editor> Make(std::u16string text, SkSize size);
    static std::unique_ptr<Editor> MakeDemo(SkScalar width, SkScalar height);

    Editor(std::u16string text, SkSize size);
    ~Editor() override = default;

    void paint(SkCanvas* canvas);
    void blink() { fCursor->blink(); }
    void onResize(int width, int height) override;
private:

    void update();

    void onAttach(sk_app::Window* w) override { fParent = w; }
    void onPaint(SkSurface* surface) override;

    bool onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) override;
    bool onKey(skui::Key, skui::InputState, skui::ModifierKey) override;
    bool onChar(SkUnichar c, skui::ModifierKey modifier) override;
    void invalidate() { if (fParent) { fParent->inval(); } }

    bool moveCursor(skui::Key key);
    bool insertCodepoint(SkUnichar unichar);
    bool deleteElement(skui::Key key);

    std::unique_ptr<EditableText> fEditableText;
    std::unique_ptr<DynamicText> fStatus;

    std::unique_ptr<Cursor> fCursor;
    std::unique_ptr<Mouse> fMouse;

    sk_app::Window* fParent;
    int fWidth;
    int fHeight;

    PositionType fDefaultPositionType;
    bool fInsertMode;
};

} // namespace editor
} // namespace skia
#endif // Editor_DEFINED

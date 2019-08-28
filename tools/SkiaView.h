// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkiaView_DEFINED
#define SkiaView_DEFINED

enum class ModifierKey { kNone = 0, kShift = 1 << 0, kControl = 1 << 1, kOption = 1 << 2,
                         kCommand = 1 << 3, kFirstPress = 1 << 4, };
enum class InputState { kDown, kUp, kMove, };

using Key = int;

class SkSurface;

class SkiaView {
public:
    SkiaView() {}
    virtual ~SkiaView() {}
    virtual bool onChar(int c, ModifierKey modifiers) { return false; }
    virtual bool onKey(Key key, InputState state, ModifierKey modifiers) { return false; }
    virtual bool onMouse(int x, int y, InputState state, ModifierKey modifiers) { return false; }
    virtual bool onMouseWheel(float delta, ModifierKey modifiers) { return false; }
    virtual bool onTouch(int owner, InputState state, float x, float y) { return false; }
    virtual bool onAnimate() { return false; }
    virtual void onPaint(SkSurface*) {}
    virtual void onSizeChage(int width, int height) {}

private:
    SkiaView(const SkiaView&) = delete;
    SkiaView& operator=(const SkiaView&) = delete;
};
#endif  // SkiaView_DEFINED

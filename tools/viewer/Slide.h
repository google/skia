/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Slide_DEFINED
#define Slide_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "tools/Registry.h"
#include "tools/sk_app/Window.h"

class SkCanvas;
class SkMetaData;
class Slide;

using SlideFactory = Slide* (*)();
using SlideRegistry = sk_tools::Registry<SlideFactory>;

#define DEF_SLIDE(code) \
    static Slide*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SlideRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

class Slide : public SkRefCnt {
public:
    /**
     * A slide may have a content dimensions that is independent of the current window size. An
     * empty size indicates that the Slide's dimensions are equal to the window's dimensions.
     */
    virtual SkISize getDimensions() const { return SkISize::MakeEmpty(); }

    virtual void gpuTeardown() { }
    virtual void draw(SkCanvas* canvas) = 0;
    virtual bool animate(double nanos) { return false; }
    virtual void load(SkScalar winWidth, SkScalar winHeight) {}
    virtual void resize(SkScalar winWidth, SkScalar winHeight) {}
    virtual void unload() {}

    virtual bool onChar(SkUnichar c) { return false; }
    virtual bool onMouse(SkScalar x, SkScalar y, skui::InputState state,
                         skui::ModifierKey modifiers) { return false; }

    virtual bool onGetControls(SkMetaData*) { return false; }
    virtual void onSetControls(const SkMetaData&) {}

    const SkString& getName() { return fName; }

protected:
    SkString    fName;
};

#endif

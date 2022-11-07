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
#include "tools/sk_app/Window.h"

class SkCanvas;
class SkMetaData;

namespace skgpu::graphite {
class Context;
}

class Slide : public SkRefCnt {
public:
    /**
     * A slide may have a content dimensions that is independent of the current window size. An
     * empty size indicates that the Slide's dimensions are equal to the window's dimensions.
     */
    virtual SkISize getDimensions() const { return SkISize::MakeEmpty(); }

    virtual void gpuTeardown() { }
    virtual void draw(SkCanvas*) { SK_ABORT("Not implemented."); }
    virtual void draw(skgpu::graphite::Context*, SkCanvas* canvas) { this->draw(canvas); }
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

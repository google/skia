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
class AnimTimer;
class SkMetaData;

class Slide : public SkRefCnt {
public:
    virtual ~Slide() {}

    virtual SkISize getDimensions() const = 0;

    virtual void draw(SkCanvas* canvas) = 0;
    virtual bool animate(const AnimTimer&) { return false; }
    virtual void load(SkScalar winWidth, SkScalar winHeight) {}
    virtual void resize(SkScalar winWidth, SkScalar winHeight) {}
    virtual void unload() {}

    virtual bool onChar(SkUnichar c) { return false; }
    virtual bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                         ModifierKey modifiers) { return false; }

    virtual bool onGetControls(SkMetaData*) { return false; }
    virtual void onSetControls(const SkMetaData&) {}

    SkString getName() { return fName; }


protected:
    SkString    fName;
};


#endif

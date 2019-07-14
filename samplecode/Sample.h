/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleCode_DEFINED
#define SampleCode_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkMacros.h"
#include "src/utils/SkMetaData.h"
#include "tools/ClickState.h"
#include "tools/ModifierKey.h"
#include "tools/Registry.h"

class SkCanvas;
class Sample;

using SampleFactory = Sample* (*)();
using SampleRegistry = sk_tools::Registry<SampleFactory>;

#define DEF_SAMPLE(code) \
    static Sample*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SampleRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

///////////////////////////////////////////////////////////////////////////////

class Sample {
public:
    virtual ~Sample() = default;

    SkScalar    width() const { return fSize.fWidth; }
    SkScalar    height() const { return fSize.fHeight; }
    SkSize      size() const { return fSize; }
    void        setSize(SkScalar width, SkScalar height);
    void        setSize(const SkPoint& size) { this->setSize(size.fX, size.fY); }
    void        setWidth(SkScalar width) { this->setSize(width, fSize.fHeight); }
    void        setHeight(SkScalar height) { this->setSize(fSize.fWidth, height); }

    /** Call this to have the view draw into the specified canvas. */
    virtual void draw(SkCanvas* canvas);

    virtual bool onChar(SkUnichar) { return false; }

    virtual bool onMouse(SkPoint, ClickState, ModifierKey) { return false; }

    void setBGColor(SkColor color) { fBGColor = color; }
    bool animate(double nanos) { return this->onAnimate(nanos); }

    virtual SkString name() = 0;

protected:
    Sample() {}

    /** Override to be notified of size changes. Overriders must call the super class. */
    virtual void onSizeChange();

    virtual void onDrawBackground(SkCanvas*);
    virtual void onDrawContent(SkCanvas*) = 0;
    virtual bool onAnimate(double /*nanos*/) { return false; }
    virtual void onOnceBeforeDraw() {}

private:
    SkColor fBGColor = SK_ColorWHITE;
    SkSize fSize = {0, 0};
    bool fHaveCalledOnceBeforeDraw = false;

    Sample(const Sample&) = delete;
    Sample& operator=(const Sample&) = delete;
};

#endif

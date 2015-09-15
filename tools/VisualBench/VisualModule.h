/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef VisualModule_DEFINED
#define VisualModule_DEFINED

#include "SkRefCnt.h"

class SkCanvas;

/*
 * VisualModule is the base class for all of the various types of activities VisualBench supports.
 *
 * The common theme tying these all together is they need to display an image to the screen.  Later,
 * on we some modules will also be interactive
 */
class VisualModule : public SkRefCnt {
public:
    virtual ~VisualModule() {}

    virtual void draw(SkCanvas* canvas)=0;

    virtual bool onHandleChar(SkUnichar unichar) = 0;

private:
    typedef SkRefCnt INHERITED;
};

#endif

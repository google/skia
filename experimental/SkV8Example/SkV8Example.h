/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_DEFINED
#define SkV8Example_DEFINED

#include <v8.h>

#include "SkWindow.h"


using namespace v8;


class SkCanvas;


class SkV8ExampleWindow : public SkOSWindow {
public:
   SkV8ExampleWindow(void* hwnd,
                     Isolate* isolate,
                     Handle<Context> context,
                     Handle<Script> script);


protected:
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE;



#ifdef SK_BUILD_FOR_WIN
    virtual void onHandleInval(const SkIRect&) SK_OVERRIDE;
#endif

private:
    typedef SkOSWindow INHERITED;
    Isolate* fIsolate;
    Persistent<Context> fContext;
    Persistent<Script> fScript;
    SkScalar fRotationAngle;
};

#endif

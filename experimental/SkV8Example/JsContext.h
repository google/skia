/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_JsContext_DEFINED
#define SkV8Example_JsContext_DEFINED

#include <v8.h>

#include "SkPaint.h"
#include "DrawingMethods.h"

class SkCanvas;
class Global;

// Provides the canvas context implementation in JS, and the OnDraw() method in
// C++ that's used to bridge from C++ to JS. Should be used in JS as:
//
//  function onDraw(context) {
//    context.fillStyle="#FF0000";
//    context.fillRect(x, y, w, h);
//  }
class JsContext : public DrawingMethods {
public:
    JsContext(Global* global)
            : INHERITED(global)
            , fCanvas(NULL)
    {
    }
    virtual ~JsContext() {}

    // Parse the script.
    bool initialize();

    // Call this with the SkCanvas you want onDraw to draw on.
    void onDraw(SkCanvas* canvas);

    virtual SkCanvas* getCanvas() { return fCanvas; };

private:

    // Wrap the 'this' pointer into an Object. Can be retrieved via Unwrap.
    v8::Handle<v8::Object> wrap();

    // A handle to the onDraw function defined in the script.
    v8::Persistent<v8::Function> fOnDraw;

    // The template for what a canvas context object looks like. The canvas
    // context object is what's passed into the JS onDraw() function.
    static v8::Persistent<v8::ObjectTemplate> gContextTemplate;

    // Only valid when inside OnDraw().
    SkCanvas* fCanvas;

    typedef DrawingMethods INHERITED;
};

#endif

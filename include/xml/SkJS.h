
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"
#include "SkWindow.h"

extern "C" {
    typedef long JSWord;
    typedef JSWord jsword;
    typedef jsword  jsval;
    typedef struct JSRuntime JSRuntime;
    typedef struct JSContext JSContext;
    typedef struct JSObject JSObject;
}

class SkString;

class SkJS : public SkOSWindow {
public:
    SkJS(void* hwnd);
    ~SkJS();
    SkBool EvaluateScript(const char* script, jsval* rVal);
    SkBool ValueToString(jsval value, SkString* string);
#ifdef SK_DEBUG
    static void Test(void* hwnd);
#endif
protected:
    void InitializeDisplayables(const SkBitmap& , JSContext *, JSObject *, JSObject *);
    void DisposeDisplayables();
    JSRuntime *fRuntime;
    JSContext *fContext;
    JSObject *fGlobal;
};


/*
 * Copyright 2011 Skia
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOSWindow_Android_DEFINED
#define SkOSWindow_Android_DEFINED

#include "SkWindow.h"

class SkIRect;

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void*) {}
    ~SkOSWindow() {}
    bool attachGL() { return true; }
    void detachGL() {}
    void presentGL() {}

    virtual void onPDFSaved(const char title[], const char desc[],
        const char path[]);

protected:
    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    virtual void onSetTitle(const char title[]);

private:
    typedef SkWindow INHERITED;
};

#endif


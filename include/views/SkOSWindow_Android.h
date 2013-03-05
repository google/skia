
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

    enum SkBackEndTypes {
        kNone_BackEndType,
        kNativeGL_BackEndType,
    };

    struct AttachmentInfo {
        int fSampleCount;
        int fStencilBits;
    };

    bool attach(SkBackEndTypes /* attachType */, int /* msaaSampleCount */, AttachmentInfo* info) {
        // These are the values requested in SkiaSampleView.java
        info->fSampleCount = 0;
        info->fStencilBits = 8;
        return true;
    }
    void detach() {}
    void present() {}

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

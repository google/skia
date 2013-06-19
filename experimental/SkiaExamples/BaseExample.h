/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef BaseExample_DEFINED
#define BaseExample_DEFINED

#include "SkWindow.h"

class GrContext;
struct GrGLInterface;
class GrRenderTarget;
class SkCanvas;

class BaseExample : public SkOSWindow {
public:
    enum DeviceType {
        kRaster_DeviceType,
        kGPU_DeviceType,
    };
    BaseExample(void* hWnd, int argc, char** argv);

    // Changes the device type of the object.
    bool setupBackend(DeviceType type);
    void tearDownBackend();

    DeviceType getDeviceType() const { return fType; }

protected:
    // Your class should override this method to do its thing.
    virtual void draw(SkCanvas* canvas) SK_OVERRIDE;

    virtual void onSizeChange() SK_OVERRIDE;

#ifdef SK_BUILD_FOR_WIN
    virtual void onHandleInval(const SkIRect&) SK_OVERRIDE;
#endif

    SkCanvas* createCanvas() SK_OVERRIDE;

private:
    void setupRenderTarget();

    DeviceType fType;

    GrContext* fContext;
    GrRenderTarget* fRenderTarget;
    AttachmentInfo fAttachmentInfo;
    const GrGLInterface* fInterface;
    typedef SkOSWindow INHERITED;
};
#endif

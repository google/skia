/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkExample_DEFINED
#define SkExample_DEFINED

#include "SkWindow.h"
#include "SkTRegistry.h"

class GrContext;
struct GrGLInterface;
class GrRenderTarget;
class SkCanvas;
class SkExampleWindow;

class SkExample : SkNoncopyable {
public:
    SkExample(SkExampleWindow* window) : fWindow(window) {}

    virtual ~SkExample() {}

    // Your class should override this method to do its thing.
    virtual void draw(SkCanvas* canvas) = 0;

    SkString getName() { return fName; };
    // Use this public registry to tell the world about your sample.
    typedef SkTRegistry<SkExample*(*)(SkExampleWindow*)> Registry;

protected:
    SkExampleWindow* fWindow;
    SkString fName;
};

class SkExampleWindow : public SkOSWindow {
public:
    enum DeviceType {
        kRaster_DeviceType,
        kGPU_DeviceType,
    };
    SkExampleWindow(void* hwnd);

    // Changes the device type of the object.
    bool setupBackend(DeviceType type);
    void tearDownBackend();

    DeviceType getDeviceType() const { return fType; }

protected:
    virtual void draw(SkCanvas* canvas) SK_OVERRIDE;

    virtual void onSizeChange() SK_OVERRIDE;

#ifdef SK_BUILD_FOR_WIN
    virtual void onHandleInval(const SkIRect&) SK_OVERRIDE;
#endif

    SkCanvas* createCanvas() SK_OVERRIDE;

private:
    bool findNextMatch();  // Set example to the first one that matches FLAGS_match.
    void setupRenderTarget();
    bool onHandleChar(SkUnichar unichar) SK_OVERRIDE;

    DeviceType fType;

    SkExample* fCurrExample;
    const SkExample::Registry* fRegistry;
    GrContext* fContext;
    GrRenderTarget* fRenderTarget;
    AttachmentInfo fAttachmentInfo;
    const GrGLInterface* fInterface;

    typedef SkOSWindow INHERITED;
};

#endif

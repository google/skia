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

#include "SkSurface.h"
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
    virtual ~SkExampleWindow() SK_OVERRIDE;

    // Changes the device type of the object.
    bool setUpBackend();

    DeviceType getDeviceType() const { return fType; }

protected:
    SkSurface* createSurface() SK_OVERRIDE {
        if (kGPU_DeviceType == fType) {
            SkSurfaceProps props(INHERITED::getSurfaceProps());
            return SkSurface::NewRenderTargetDirect(fRenderTarget, &props);
        }
        static const SkImageInfo info = SkImageInfo::MakeN32Premul(
                SkScalarRoundToInt(this->width()), SkScalarRoundToInt(this->height()));
        return fSurface = SkSurface::NewRaster(info);
   }

    void draw(SkCanvas* canvas) SK_OVERRIDE;
    void drawContents(SkCanvas* canvas);

    void onSizeChange() SK_OVERRIDE;

private:
    bool findNextMatch();  // Set example to the first one that matches FLAGS_match.
    void setTitle();
    void setUpRenderTarget();
    bool onHandleChar(SkUnichar unichar) SK_OVERRIDE;
    void tearDownBackend();

    // draw contents
    SkScalar fRotationAngle;

    // support framework
    DeviceType fType;
    SkSurface* fSurface;
    GrContext* fContext;
    GrRenderTarget* fRenderTarget;
    AttachmentInfo fAttachmentInfo;
    const GrGLInterface* fInterface;

    typedef SkOSWindow INHERITED;
};

#endif

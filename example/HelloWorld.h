/*
 * Copyright 2015 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef HelloWorld_DEFINED
#define HelloWorld_DEFINED

#include "SkSurface.h"
#include "SkWindow.h"

class GrContext;
struct GrGLInterface;
class GrRenderTarget;
class SkCanvas;

class HelloWorldWindow : public SkOSWindow {
public:
    enum DeviceType {
        kRaster_DeviceType,
        kGPU_DeviceType,
    };
    HelloWorldWindow(void* hwnd);
    virtual ~HelloWorldWindow() override;

    // Changes the device type of the object.
    bool setUpBackend();

    DeviceType getDeviceType() const { return fType; }

protected:
    sk_sp<SkSurface> makeSurface() override {
        SkSurfaceProps props(INHERITED::getSurfaceProps());
        if (kGPU_DeviceType == fType) {
            return fGpuSurface;
        }
        const SkImageInfo info = SkImageInfo::MakeN32Premul(SkScalarRoundToInt(this->width()),
                                                            SkScalarRoundToInt(this->height()));
        fRasterSurface = SkSurface::MakeRaster(info, &props);
        return fRasterSurface;
    }

    void draw(SkCanvas* canvas) override;
    void drawContents(SkCanvas* canvas);

    void onSizeChange() override;

private:
    bool findNextMatch();  // Set example to the first one that matches FLAGS_match.
    void setTitle();
    void setUpGpuBackedSurface();
    bool onHandleChar(SkUnichar unichar) override;
    void tearDownBackend();

    // draw contents
    SkScalar fRotationAngle;

    // support framework
    DeviceType fType;
    sk_sp<SkSurface> fRasterSurface;
    GrContext* fContext;
    sk_sp<SkSurface> fGpuSurface;
    AttachmentInfo fAttachmentInfo;
    const GrGLInterface* fInterface;

    typedef SkOSWindow INHERITED;
};

#endif


/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkiOSDeviceManager_DEFINED
#define SkiOSDeviceManager_DEFINED
#include "SampleApp.h"
#include "SkCanvas.h"
#include "GrContext.h"
#include "GrGLInterface.h"
#include "SkGpuDevice.h"
#include "SkCGUtils.h"
#include "GrContext.h"
class SkiOSDeviceManager : public SampleWindow::DeviceManager {
public:
    SkiOSDeviceManager();
    virtual ~SkiOSDeviceManager();
    
    virtual void init(SampleWindow* win);
    
    virtual bool supportsDeviceType(SampleWindow::DeviceType dType);
    virtual bool prepareCanvas(SampleWindow::DeviceType dType,
                               SkCanvas* canvas,
                               SampleWindow* win);
    virtual void publishCanvas(SampleWindow::DeviceType dType,
                               SkCanvas* canvas,
                               SampleWindow* win);
    
    virtual void windowSizeChanged(SampleWindow* win) {}
    
    bool isUsingGL() { return usingGL; }
    
    virtual GrContext* getGrContext() { return fGrContext; }
private:
    bool usingGL;
    GrContext* fGrContext;
    GrRenderTarget* fGrRenderTarget;
};

#endif
/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFooDevice_DEFINED
#define SkFooDevice_DEFINED

#include "src/core/SkDevice.h"

class SkFooDevice : public SkBaseDevice {
public:
    SkFooDevice(const SkImageInfo& ii, const SkSurfaceProps& props)
        : INHERITED(ii, props) {
    }

    virtual GrSurfaceProxyView readSurfaceView() = 0;
    GrRenderTargetProxy* targetProxy() override {
        return this->readSurfaceView().asRenderTargetProxy();
    }
    virtual GrRenderTarget* accessRenderTarget() = 0;

protected:

private:
    using INHERITED = SkBaseDevice;
};

#endif

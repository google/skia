/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeviceImageFilterProxy_DEFINED
#define SkDeviceImageFilterProxy_DEFINED

#include "SkDevice.h"
#include "SkImageFilter.h"
#include "SkSurfaceProps.h"

class SkDeviceImageFilterProxy : public SkImageFilter::Proxy {
public:
    SkDeviceImageFilterProxy(SkBaseDevice* device, const SkSurfaceProps& props)
        : fDevice(device)
        , fProps(props.flags(),
                 SkBaseDevice::CreateInfo::AdjustGeometry(SkImageInfo(),
                                                          SkBaseDevice::kImageFilter_Usage,
                                                          props.pixelGeometry()))
    {}

    SkBaseDevice* createDevice(int w, int h) SK_OVERRIDE {
        SkBaseDevice::CreateInfo cinfo(SkImageInfo::MakeN32Premul(w, h),
                                       SkBaseDevice::kImageFilter_Usage,
                                       kUnknown_SkPixelGeometry);
        return fDevice->onCreateCompatibleDevice(cinfo);
    }
    bool canHandleImageFilter(const SkImageFilter* filter) SK_OVERRIDE {
        return fDevice->canHandleImageFilter(filter);
    }
    virtual bool filterImage(const SkImageFilter* filter, const SkBitmap& src,
                             const SkImageFilter::Context& ctx,
                             SkBitmap* result, SkIPoint* offset) SK_OVERRIDE {
        return fDevice->filterImage(filter, src, ctx, result, offset);
    }

    const SkSurfaceProps* surfaceProps() const SK_OVERRIDE {
        return &fProps;
    }

private:
    SkBaseDevice*  fDevice;
    const SkSurfaceProps fProps;
};

#endif

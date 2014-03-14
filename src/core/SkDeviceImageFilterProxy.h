/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeviceImageFilterProxy_DEFINED
#define SkDeviceImageFilterProxy_DEFINED

#include "SkImageFilter.h"

class SkDeviceImageFilterProxy : public SkImageFilter::Proxy {
public:
    SkDeviceImageFilterProxy(SkBaseDevice* device) : fDevice(device) {}

    virtual SkBaseDevice* createDevice(int w, int h) SK_OVERRIDE {
        return fDevice->createCompatibleDevice(SkImageInfo::MakeN32Premul(w, h));
    }
    virtual bool canHandleImageFilter(const SkImageFilter* filter) SK_OVERRIDE {
        return fDevice->canHandleImageFilter(filter);
    }
    virtual bool filterImage(const SkImageFilter* filter, const SkBitmap& src,
                             const SkImageFilter::Context& ctx,
                             SkBitmap* result, SkIPoint* offset) SK_OVERRIDE {
        return fDevice->filterImage(filter, src, ctx, result, offset);
    }

private:
    SkBaseDevice* fDevice;
};

#endif

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
    SkDeviceImageFilterProxy(SkDevice* device) : fDevice(device) {}

    virtual SkDevice* createDevice(int w, int h) SK_OVERRIDE {
        return fDevice->createCompatibleDevice(SkBitmap::kARGB_8888_Config,
                                               w, h, false);
    }
    virtual bool canHandleImageFilter(SkImageFilter* filter) SK_OVERRIDE {
        return fDevice->canHandleImageFilter(filter);
    }
    virtual bool filterImage(SkImageFilter* filter, const SkBitmap& src,
                             const SkMatrix& ctm,
                             SkBitmap* result, SkIPoint* offset) SK_OVERRIDE {
        return fDevice->filterImage(filter, src, ctm, result, offset);
    }

private:
    SkDevice* fDevice;
};

#endif

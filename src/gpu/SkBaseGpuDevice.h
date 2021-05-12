/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBaseGpuDevice_DEFINED
#define SkBaseGpuDevice_DEFINED

// NOTE: when not defined, SkGpuDevice extends SkBaseDevice directly and manages its clip stack
// using GrClipStack. When false, SkGpuDevice continues to extend SkClipStackDevice and uses
// SkClipStack and GrClipStackClip to manage the clip stack.
#if !defined(SK_DISABLE_NEW_GR_CLIP_STACK)
    // For staging purposes, disable this for Android Framework
    #if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
        #define SK_DISABLE_NEW_GR_CLIP_STACK
    #endif
#endif

#if !defined(SK_DISABLE_NEW_GR_CLIP_STACK)
    #include "src/core/SkDevice.h"
    #define BASE_DEVICE   SkBaseDevice
#else
    #include "src/core/SkClipStackDevice.h"
    #define BASE_DEVICE   SkClipStackDevice
#endif

class SkBaseGpuDevice : public BASE_DEVICE {
public:
    SkBaseGpuDevice(const SkImageInfo& ii, const SkSurfaceProps& props)
        : INHERITED(ii, props) {
    }

    // TODO: SkGpuDevice/SkGpuDevice_nga shared stuff goes here

protected:

private:
    using INHERITED = BASE_DEVICE;
};

#undef BASE_DEVICE

#endif

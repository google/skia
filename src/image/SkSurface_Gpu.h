/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Gpu_DEFINED
#define SkSurface_Gpu_DEFINED

#include "SkSurface_Base.h"

#if SK_SUPPORT_GPU

class SkGpuDevice;

class SkSurface_Gpu : public SkSurface_Base {
public:
    SkSurface_Gpu(sk_sp<SkGpuDevice>);
    ~SkSurface_Gpu() override;

    GrBackendObject onGetTextureHandle(BackendHandleAccess) override;
    bool onGetRenderTargetHandle(GrBackendObject*, BackendHandleAccess) override;
    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot() override;
    void onCopyOnWrite(ContentChangeMode) override;
    void onDiscard() override;
    void onPrepareForExternalIO() override;

    SkGpuDevice* getDevice() { return fDevice.get(); }

    static bool Valid(const SkImageInfo&);
    static bool Valid(GrContext*, GrPixelConfig, SkColorSpace*);

private:
    sk_sp<SkGpuDevice> fDevice;

    typedef SkSurface_Base INHERITED;
};

#endif // SK_SUPPORT_GPU

#endif // SkSurface_Gpu_DEFINED

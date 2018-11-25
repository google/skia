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

    // This is an internal-only factory
    static sk_sp<SkSurface> MakeWrappedRenderTarget(GrContext*, sk_sp<GrRenderTargetContext>);

    GrBackendObject onGetTextureHandle(BackendHandleAccess) override;
    bool onGetRenderTargetHandle(GrBackendObject*, BackendHandleAccess) override;
    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot() override;
    void onCopyOnWrite(ContentChangeMode) override;
    void onDiscard() override;
    GrSemaphoresSubmitted onFlush(int numSemaphores,
                                  GrBackendSemaphore signalSemaphores[]) override;
    bool onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores) override;
    bool onCharacterize(SkSurfaceCharacterization*) const override;
    bool isCompatible(const SkSurfaceCharacterization&) const;
    bool onDraw(const SkDeferredDisplayList*) override;

    SkGpuDevice* getDevice() { return fDevice.get(); }

    static bool Valid(const SkImageInfo&);
    static bool Valid(GrContext*, GrPixelConfig, SkColorSpace*);

private:
    sk_sp<SkGpuDevice> fDevice;

    typedef SkSurface_Base INHERITED;
};

#endif // SK_SUPPORT_GPU

#endif // SkSurface_Gpu_DEFINED

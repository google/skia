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
    SK_DECLARE_INST_COUNT(SkSurface_Gpu)

    SkSurface_Gpu(SkGpuDevice*);
    virtual ~SkSurface_Gpu();

    SkCanvas* onNewCanvas() override;
    SkSurface* onNewSurface(const SkImageInfo&) override;
    SkImage* onNewImageSnapshot(Budgeted) override;
    void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) override;
    void onCopyOnWrite(ContentChangeMode) override;
    void onDiscard() override;

    SkGpuDevice* getDevice() { return fDevice; }

private:
    SkGpuDevice* fDevice;

    typedef SkSurface_Base INHERITED;
};

#endif // SK_SUPPORT_GPU

#endif // SkSurface_Gpu_DEFINED

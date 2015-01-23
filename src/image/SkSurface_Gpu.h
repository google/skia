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

    SkCanvas* onNewCanvas() SK_OVERRIDE;
    SkSurface* onNewSurface(const SkImageInfo&) SK_OVERRIDE;
    SkImage* onNewImageSnapshot(Budgeted) SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;
    void onCopyOnWrite(ContentChangeMode) SK_OVERRIDE;
    void onDiscard() SK_OVERRIDE;

    SkGpuDevice* getDevice() { return fDevice; }

private:
    SkGpuDevice* fDevice;

    typedef SkSurface_Base INHERITED;
};

#endif // SK_SUPPORT_GPU

#endif // SkSurface_Gpu_DEFINED

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSecondaryCBDrawContext_DEFINED
#define GrVkSecondaryCBDrawContext_DEFINED

#include "SkTypes.h"
#include "SkRefCnt.h"

class GrContext;
struct GrVkDrawableInfo;
class SkCanvas;
class SkDeferredDisplayList;
class SkGpuDevice;
struct SkImageInfo;
class SkSurfaceCharacterization;
class SkSurfaceProps;

class SK_API GrVkSecondaryCBDrawContext : public SkRefCnt {
public:
    static sk_sp<GrVkSecondaryCBDrawContext> Make(GrContext*, const SkImageInfo&,
                                                  const GrVkDrawableInfo&,
                                                  const SkSurfaceProps* props);

    ~GrVkSecondaryCBDrawContext() override;

    SkCanvas* getCanvas();

    void flush();

    // This call will release all resources held by the draw context. The client must call release
    // resources before deleting the drawing context. However, the resources also includes any
    // Vulkan resources that were created and used for draws. Therefore the client must call
    // releaseResources after submitting the secondary command buffer, and waiting for it to finish
    // on the GPU. If it is called earlier than we may delete some vulkan objects before they are
    // processed by the GPU.
    void releaseResources();

    // TODO: Fill out these calls to support DDL
    bool characterize(SkSurfaceCharacterization* characterization) const;
    bool draw(SkDeferredDisplayList* deferredDisplayList);

private:
    explicit GrVkSecondaryCBDrawContext(sk_sp<SkGpuDevice>);

    sk_sp<SkGpuDevice>        fDevice;
    std::unique_ptr<SkCanvas> fCachedCanvas;

    typedef SkRefCnt INHERITED;
};

#endif

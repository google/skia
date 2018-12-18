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

    // TODO: We will need the client to tell us when it is safe to clean up our resources that we
    // drew into the secondary command buffer.
    void cleanup();

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

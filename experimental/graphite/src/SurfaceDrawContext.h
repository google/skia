/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_SurfaceDrawContext_DEFINED
#define skgpu_SurfaceDrawContext_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class SDCTask;

class SurfaceDrawContext final : public SkRefCnt {
public:
    static sk_sp<SurfaceDrawContext> Make(const SkImageInfo&);

    ~SurfaceDrawContext() override;

    const SkImageInfo& imageInfo() { return fImageInfo; }

private:
    SurfaceDrawContext(const SkImageInfo&, sk_sp<SDCTask>);

    SkImageInfo    fImageInfo;
    sk_sp<SDCTask> fTask;
};

} // namespace skgpu

#endif // skgpu_SurfaceDrawContext_DEFINED


/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuYUVA_DEFINED
#define SkImage_GpuYUVA_DEFINED

#include "SkImage_Gpu.h"

class SkImage_GpuYUVA : public SkImage_Gpu {
public:
    SkImage_GpuYUVA(sk_sp<GrContext>, uint32_t uniqueID, sk_sp<GrTextureProxy> finalProxy,
                    sk_sp<GrTextureProxy> yuvaProxies[],
                    sk_sp<SkColorSpace>, SkBudgeted);
    ~SkImage_GpuYUVA() override;

private:
    sk_sp<GrTextureProxy> fProxies[4];

    typedef SkImage_Gpu INHERITED;
};

#endif

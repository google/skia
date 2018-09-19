/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_GpuYUVA.h"

#include "GrContext.h"

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context,
                                 uint32_t uniqueID,
                                 sk_sp<GrTextureProxy> finalProxy,
                                 sk_sp<GrTextureProxy> proxies[],
                                 sk_sp<SkColorSpace> colorSpace,
                                 SkBudgeted budgeted)
        : INHERITED(std::move(context), uniqueID, kPremul_SkAlphaType,
                    std::move(finalProxy), std::move(colorSpace), budgeted) {

    for (int i = 0; i < 4; ++i) {
        fProxies[i] = proxies[i];
    }
}

SkImage_GpuYUVA::~SkImage_GpuYUVA() {}
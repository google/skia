/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "MtlTestGpu.h"
#include "mtl/GrMtlGpu.h"

#ifdef SK_METAL

MtlTestGpu::MtlTestGpu(GrGpu* gpu) : fMtlGpu(static_cast<GrMtlGpu*>(gpu)) {};

bool MtlTestGpu::readPixels(GrSurface* surf, int left, int top, int width, int height,
                            GrColorType ct, void* buffer, size_t rowBytes) {
    return fMtlGpu->readPixels(surf, left, top, width, height, ct, buffer, rowBytes);
}

sk_sp<GrTexture> MtlTestGpu::wrapBackendTexture(GrBackendTexture tex, GrWrapOwnership ownership) {
    return fMtlGpu->wrapBackendTexture(tex, ownership);
}

sk_sp<GrTexture> MtlTestGpu::wrapRenderableBackendTexture(GrBackendTexture tex, int sampleCnt,
                                                          GrWrapOwnership ownership) {
    return fMtlGpu->wrapRenderableBackendTexture(tex, sampleCnt, ownership);
}

GrBackendTexture MtlTestGpu::createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                             GrPixelConfig config, bool isRT,
                                                             GrMipMapped mipMapped) {
    return fMtlGpu->createTestingOnlyBackendTexture(pixels, w, h, config, isRT, mipMapped);
}
#endif

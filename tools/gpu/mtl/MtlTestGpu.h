/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Allows cpp unit tests to call GrMtlGpu functions. Only exposes functions as needed for testing.

#ifndef MtlTestGpu_DEFINED
#define MtlTestGpu_DEFINED

#ifdef SK_METAL

#include "GrGpu.h"

class GrMtlGpu;

class MtlTestGpu {
public:
    MtlTestGpu(GrGpu* gpu);

    bool readPixels(GrSurface*, int left, int top, int width, int height, GrColorType, void* buffer,
                    size_t rowBytes);

    sk_sp<GrTexture> wrapBackendTexture(GrBackendTexture, GrWrapOwnership);

    sk_sp<GrTexture> wrapRenderableBackendTexture(GrBackendTexture, int sampleCnt, GrWrapOwnership);

    GrBackendTexture createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                     GrPixelConfig, bool isRT, GrMipMapped);
private:
    GrMtlGpu* fMtlGpu;
};

#endif // SK_METAL
#endif

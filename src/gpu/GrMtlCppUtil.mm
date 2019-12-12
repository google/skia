// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/gpu/GrMtlCppUtil.h"

#import <Metal/Metal.h>

GrMTLPixelFormat GrGetMTLPixelFormatFromMtlTextureInfo(const GrMtlTextureInfo& info) {
    return (GrMTLPixelFormat)((__bridge id<MTLTexture>)(info.fTexture.get())).pixelFormat;
}

#if GR_TEST_UTILS
const char* GrMtlFormatToStr(GrMTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
#define M(X) case MTLPixelFormat ## X: return #X;
        M(Invalid)
        M(RGBA8Unorm)
        M(R8Unorm)
        M(A8Unorm)
        M(BGRA8Unorm)
#ifdef SK_BUILD_FOR_IOS
        M(B5G6R5Unorm)
#endif
        M(RGBA16Float)
        M(R16Float)
        M(RG8Unorm)
        M(RGB10A2Unorm)
#ifdef SK_BUILD_FOR_IOS
        M(ABGR4Unorm)
#endif
        M(RGBA8Unorm_sRGB)
        M(R16Unorm)
        M(RG16Unorm)
#ifdef SK_BUILD_FOR_IOS
        M(ETC2_RGB8)
#endif
        M(RGBA16Unorm)
        M(RG16Float)
#undef M
        default: return "Unknown";
    }
}
#endif

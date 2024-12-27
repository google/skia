/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphiteTypesUtils_DEFINED
#define skgpu_graphite_MtlGraphiteTypesUtils_DEFINED

#include "include/core/SkTypes.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAPI.h"

#import <CoreFoundation/CoreFoundation.h>

namespace skgpu::graphite {

struct SK_API MtlTextureInfo;

namespace TextureInfos {
SK_API TextureInfo MakeMetal(const MtlTextureInfo&);
SK_API TextureInfo MakeMetal(CFTypeRef mtlTexture);

SK_API bool GetMtlTextureInfo(const TextureInfo&, MtlTextureInfo*);
}  // namespace TextureInfos

namespace BackendTextures {
// The BackendTexture will not call retain or release on the passed in CFTypeRef. Thus the
// client must keep the CFTypeRef valid until they are no longer using the BackendTexture.
SK_API BackendTexture MakeMetal(SkISize dimensions, CFTypeRef mtlTexture);

SK_API CFTypeRef GetMtlTexture(const BackendTexture&);
}  // namespace BackendTextures

namespace BackendSemaphores {
// TODO(b/286088355) Determine creator's responsibility for setting refcnt.
SK_API BackendSemaphore MakeMetal(CFTypeRef mtlEvent, uint64_t value);

SK_API CFTypeRef GetMtlEvent(const BackendSemaphore&);
SK_API uint64_t GetMtlValue(const BackendSemaphore&);
}  // namespace BackendSemaphores

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MtlGraphiteTypesUtils_DEFINED

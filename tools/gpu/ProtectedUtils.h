/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProtectedUtils_DEFINED
#define ProtectedUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"

class GrDirectContext;
class SkImage;
class SkSurface;
struct SkISize;

#ifdef SK_GRAPHITE
namespace skgpu {
    enum class Protected : bool;
}
namespace skgpu::graphite {
    class Recorder;
}
#endif

namespace ProtectedUtils {

/*
 * These factories create Surfaces and Images with an explicitly specified protected status.
 * If the Surface/Image cannot be created with the specified protected status nullptr will
 * be returned.
 */

sk_sp<SkSurface> CreateProtectedSkSurface(GrDirectContext*,
                                          SkISize size,
                                          bool textureable,
                                          bool isProtected);

void CheckImageBEProtection(SkImage*, bool expectingProtected);

sk_sp<SkImage> CreateProtectedSkImage(GrDirectContext*,
                                      SkISize size,
                                      SkColor4f color,
                                      bool isProtected);

#ifdef SK_GRAPHITE
sk_sp<SkSurface> CreateProtectedSkSurface(skgpu::graphite::Recorder*,
                                          SkISize size,
                                          skgpu::Protected);

sk_sp<SkImage> CreateProtectedSkImage(skgpu::graphite::Recorder*,
                                      SkISize size,
                                      SkColor4f color,
                                      skgpu::Protected);
#endif

}  // namespace ProtectedUtils

#endif  // ProtectedUtils_DEFINED

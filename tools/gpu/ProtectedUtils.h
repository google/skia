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

namespace ProtectedUtils {

sk_sp<SkSurface> CreateProtectedSkSurface(GrDirectContext*,
                                          SkISize size,
                                          bool textureable = true,
                                          bool isProtected = true);

void CheckImageBEProtection(SkImage*, bool expectingProtected);

sk_sp<SkImage> CreateProtectedSkImage(GrDirectContext*,
                                      SkISize size,
                                      SkColor4f color,
                                      bool isProtected);

}  // namespace ProtectedUtils

#endif  // ProtectedUtils_DEFINED

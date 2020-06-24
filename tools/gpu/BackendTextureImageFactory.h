/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

class GrContext;
class SkImage;
class SkPixmap;

namespace sk_gpu_test {
sk_sp<SkImage> MakeBackendTextureImage(GrContext*, const SkPixmap&, GrRenderable, GrSurfaceOrigin);
}

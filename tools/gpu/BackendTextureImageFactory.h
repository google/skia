/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

class GrDirectContext;
class SkImage;
class SkPixmap;

namespace sk_gpu_test {
/**
 * Creates a backend texture with pixmap contents and wraps it in a SkImage that safely deletes
 * the texture when it goes away. Unlike using makeTextureImage() on a non-GPU image, this will
 * fail rather than fallback if the pixmaps's color type doesn't map to a supported texture format.
 * For testing purposes the texture can be made renderable to exercise different code paths for
 * renderable textures/formats.
 */
sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext*, const SkPixmap&,
                                       GrRenderable, GrSurfaceOrigin);
}  // namespace sk_gpu_test

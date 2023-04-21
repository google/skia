/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageGeneratorPriv_DEFINED
#define SkImageGeneratorPriv_DEFINED

#include "include/core/SkRefCnt.h" // IWYU pragma: keep

#include <memory>

class SkColorSpace;
class SkImageGenerator;
class SkMatrix;
class SkPaint;
class SkPicture;
class SkSurfaceProps;
namespace SkImages { enum class BitDepth; }
struct SkISize;

namespace SkImageGenerators {
/** Return a new image generator backed by the specified picture.  If the size is empty or
 *  the picture is NULL, this returns NULL.
 *  The optional matrix and paint arguments are passed to drawPicture() at rasterization
 *  time.
 */
std::unique_ptr<SkImageGenerator> MakeFromPicture(const SkISize&,
                                                  sk_sp<SkPicture>,
                                                  const SkMatrix*,
                                                  const SkPaint*,
                                                  SkImages::BitDepth,
                                                  sk_sp <SkColorSpace>,
                                                  SkSurfaceProps props);

std::unique_ptr<SkImageGenerator> MakeFromPicture(const SkISize&,
                                                  sk_sp<SkPicture>,
                                                  const SkMatrix*,
                                                  const SkPaint*,
                                                  SkImages::BitDepth,
                                                  sk_sp<SkColorSpace>);
}

#endif

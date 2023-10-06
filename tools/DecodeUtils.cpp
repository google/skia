/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/DecodeUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "src/image/SkImageGeneratorPriv.h"

#include <memory>
#include <utility>

namespace ToolUtils {

bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst) {
    std::unique_ptr<SkImageGenerator> gen(SkImageGenerators::MakeFromEncoded(std::move(data)));
    return gen && dst->tryAllocPixels(gen->getInfo()) &&
           gen->getPixels(
                   gen->getInfo().makeColorSpace(nullptr), dst->getPixels(), dst->rowBytes());
}

}  // namespace ToolUtils

/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "src/image/SkImageGeneratorPriv.h"

namespace SkImageGenerators {

std::unique_ptr<SkImageGenerator> MakeFromEncodedImpl(sk_sp<SkData>, std::optional<SkAlphaType>) {
    return nullptr;
}

}  // namespace SkImageGenerators

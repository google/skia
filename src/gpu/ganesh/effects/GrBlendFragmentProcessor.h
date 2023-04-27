/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlendFragmentProcessor_DEFINED
#define GrBlendFragmentProcessor_DEFINED

#include "src/gpu/ganesh/GrFragmentProcessor.h"

#include <memory>
#include <utility>

enum class SkBlendMode;

namespace GrBlendFragmentProcessor {

/**
 * Blends src and dst inputs according to the blend mode. If either input is null, fInputColor is
 * used instead.
 * - When `shareBlendLogic` is false, the blend function logic is written directly into the code.
 * - When `shareBlendLogic` is true, most Porter-Duff blends share the same code, and a uniform
 *   is used to pick the blend type. This can reduce our overall shader count.
 */
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                          std::unique_ptr<GrFragmentProcessor> dst,
                                          SkBlendMode mode,
                                          bool shareBlendLogic = true);
/**
 * Blends src and dst inputs according to the blend mode. If either input is null, fInputColor is
 * used instead. Hard-wires a single blend mode into the code (slightly reducing complexity).
 */
template <SkBlendMode mode>
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                          std::unique_ptr<GrFragmentProcessor> dst) {
    return Make(std::move(src), std::move(dst), mode, /*shareBlendLogic=*/false);
}


}  // namespace GrBlendFragmentProcessor

#endif

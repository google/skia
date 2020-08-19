/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlendFragmentProcessor_DEFINED
#define GrBlendFragmentProcessor_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"

class GrFragmentProcessor;

namespace GrBlendFragmentProcessor {

// TODO(skbug.com/10457): Standardize on a single blend behavior
enum class BlendBehavior {
    // Picks "ComposeOne" or "ComposeTwo" automatically depending on presence of src/dst FPs.
    kDefault = 0,

    // half(1) is passed as the input color to child FPs. No alpha channel trickery.
    kComposeOneBehavior,

    // fInputColor.rgb1 is passed as the input color to child FPs. Alpha is manually blended.
    kComposeTwoBehavior,

    // half(1) is passed to src; fInputColor is passed to dst. No alpha channel trickery.
    kSkModeBehavior,

    kLastBlendBehavior = kSkModeBehavior,
};

/** Blends src and dst inputs according to the blend mode.
 *  If either input is null, fInputColor is used instead.
 */
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                          std::unique_ptr<GrFragmentProcessor> dst,
                                          SkBlendMode mode,
                                          BlendBehavior behavior = BlendBehavior::kDefault);

}  // namespace GrBlendFragmentProcessor

#endif

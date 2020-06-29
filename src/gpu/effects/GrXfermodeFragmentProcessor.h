/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXfermodeFragmentProcessor_DEFINED
#define GrXfermodeFragmentProcessor_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"

class GrFragmentProcessor;

namespace GrXfermodeFragmentProcessor {

/** Takes the input color, which is assumed to be unpremultiplied, passes it as an opaque color
 *  to both src and dst. The outputs of a src and dst are blended using mode and the original
 *  input's alpha is applied to the blended color to produce a premul output. Null can be passed for
 *  `src` or `dst`, in which case the input FP will be substituted. */
std::unique_ptr<GrFragmentProcessor> Make(
        std::unique_ptr<GrFragmentProcessor> inputFP,
        std::unique_ptr<GrFragmentProcessor> src,
        std::unique_ptr<GrFragmentProcessor> dst,
        SkBlendMode mode);

};

#endif

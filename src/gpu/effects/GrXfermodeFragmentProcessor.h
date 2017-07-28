/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXfermodeFragmentProcessor_DEFINED
#define GrXfermodeFragmentProcessor_DEFINED

#include "GrTypes.h"
#include "SkRefCnt.h"
#include "SkBlendMode.h"

class GrFragmentProcessor;

namespace GrXfermodeFragmentProcessor {
    /** The color input to the returned processor is treated as the src and the passed in processor
        is the dst. */
    gr_fp<GrFragmentProcessor> MakeFromDstProcessor(gr_fp<GrFragmentProcessor> dst,
                                                    SkBlendMode mode);

    /** The color input to the returned processor is treated as the dst and the passed in processor
        is the src. */
    gr_fp<GrFragmentProcessor> MakeFromSrcProcessor(gr_fp<GrFragmentProcessor> src,
                                                    SkBlendMode mode);

    /** Takes the input color, which is assumed to be unpremultiplied, passes it as an opaque color
        to both src and dst. The outputs of a src and dst are blended using mode and the original
        input's alpha is applied to the blended color to produce a premul output. */
    gr_fp<GrFragmentProcessor> MakeFromTwoProcessors(gr_fp<GrFragmentProcessor> src,
                                                     gr_fp<GrFragmentProcessor> dst,
                                                     SkBlendMode mode);
};

#endif

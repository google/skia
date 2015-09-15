/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXfermodeFragmentProcessor_DEFINED
#define GrXfermodeFragmentProcessor_DEFINED

#include "SkXfermode.h"

class GrFragmentProcessor;

namespace GrXfermodeFragmentProcessor {
    /** The color input to the returned processor is treated as the src and the passed in processor
        is the dst. */
    const GrFragmentProcessor* CreateFromDstProcessor(const GrFragmentProcessor* dst,
                                                      SkXfermode::Mode mode);

    /** The color input to the returned processor is treated as the dst and the passed in processor
        is the src. */
    const GrFragmentProcessor* CreateFromSrcProcessor(const GrFragmentProcessor* src,
                                                      SkXfermode::Mode mode);

    const GrFragmentProcessor* CreateFromTwoProcessors(const GrFragmentProcessor* src,
                                                       const GrFragmentProcessor* dst,
                                                       SkXfermode::Mode mode);
};

#endif

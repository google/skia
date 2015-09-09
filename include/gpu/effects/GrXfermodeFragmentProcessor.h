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
    const GrFragmentProcessor* CreateFromTwoProcessors(const GrFragmentProcessor* src,
                                                       const GrFragmentProcessor* dst,
                                                       SkXfermode::Mode mode);
};

#endif

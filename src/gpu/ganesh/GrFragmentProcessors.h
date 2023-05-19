/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessors_DEFINED
#define GrFragmentProcessors_DEFINED

#include <memory>

class GrFragmentProcessor;
class SkMaskFilter;
struct GrFPArgs;
class SkMatrix;

namespace GrFragmentProcessors {
std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter*,
                                          const GrFPArgs&,
                                          const SkMatrix& ctm);

bool IsSupported(const SkMaskFilter*);
}

#endif

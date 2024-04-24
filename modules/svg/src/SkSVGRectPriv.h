/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRectPriv_DEFINED
#define SkSVGRectPriv_DEFINED

#include <tuple>

#include "src/base/SkTLazy.h"

class SkSVGLength;
class SkSVGLengthContext;

std::tuple<float, float> ResolveOptionalRadii(const SkTLazy<SkSVGLength>& rx,
                                              const SkTLazy<SkSVGLength>& ry,
                                              const SkSVGLengthContext&);

#endif // SkSVGRectPriv_DEFINED

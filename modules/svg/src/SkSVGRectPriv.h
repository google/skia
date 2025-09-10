/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRectPriv_DEFINED
#define SkSVGRectPriv_DEFINED

#include "include/core/SkTypes.h"

#include <optional>
#include <tuple>

class SkSVGLength;
class SkSVGLengthContext;

std::tuple<float, float> ResolveOptionalRadii(const std::optional<SkSVGLength>& rx,
                                              const std::optional<SkSVGLength>& ry,
                                              const SkSVGLengthContext&);

#endif // SkSVGRectPriv_DEFINED

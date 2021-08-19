/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AAConvexPathRenderer_DEFINED
#define AAConvexPathRenderer_DEFINED

#include "src/gpu/v1/PathRenderer.h"

namespace skgpu::v1 {

class AAConvexPathRenderer final : public PathRenderer {
public:
    AAConvexPathRenderer() = default;

    const char* name() const override { return "AAConvex"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;
};

} // namespace skgpu::v1

#endif // AAConvexPathRenderer_DEFINED

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AAConvexPathRenderer_DEFINED
#define AAConvexPathRenderer_DEFINED

#include "src/gpu/ganesh/PathRenderer.h"

namespace skgpu::ganesh {

class AAConvexPathRenderer final : public PathRenderer {
public:
    AAConvexPathRenderer() = default;

    const char* name() const override { return "AAConvex"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;
};

}  // namespace skgpu::ganesh

#endif // AAConvexPathRenderer_DEFINED

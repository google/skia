/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AAHairLinePathRenderer_DEFINED
#define AAHairLinePathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"

namespace skgpu::v1 {

class AAHairLinePathRenderer final : public GrPathRenderer {
public:
    AAHairLinePathRenderer() = default;

    const char* name() const override { return "AAHairline"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    using INHERITED = GrPathRenderer;
};

} // namespace skgpu::v1

#endif // AAHairLinePathRenderer_DEFINED

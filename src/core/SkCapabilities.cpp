/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCapabilities.h"

#include "src/sksl/SkSLUtil.h"

sk_sp<const SkCapabilities> SkCapabilities::RasterBackend() {
    static SkCapabilities* sCaps = []() {
        SkCapabilities* caps = new SkCapabilities;
        caps->fSkSLVersion = SkSL::Version::k100;
        return caps;
    }();

    return sk_ref_sp(sCaps);
}

void SkCapabilities::initSkCaps(const SkSL::ShaderCaps* shaderCaps) {
    this->fSkSLVersion = shaderCaps->supportedSkSLVerion();
}

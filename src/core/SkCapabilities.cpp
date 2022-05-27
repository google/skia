/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCapabilities.h"

#ifdef SK_ENABLE_SKSL
#include "src/sksl/SkSLUtil.h"
#endif

sk_sp<const SkCapabilities> SkCapabilities::RasterBackend() {
    static SkCapabilities* sCaps = [](){
        SkCapabilities* caps = new SkCapabilities;
#ifdef SK_ENABLE_SKSL
        caps->fSkSLVersion = SkSL::Version::k100;
#endif
        return caps;
    }();

    return sk_ref_sp(sCaps);
}

#ifdef SK_ENABLE_SKSL
void SkCapabilities::initSkCaps(const SkSL::ShaderCaps* shaderCaps) {
    this->fSkSLVersion = shaderCaps->supportedSkSLVerion();
}
#endif

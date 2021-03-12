/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
    return false;
}

std::unique_ptr<GrCoverageCountingPathRenderer> GrCoverageCountingPathRenderer::CreateIfSupported(
        const GrCaps&) {
    return nullptr;
}

std::unique_ptr<GrFragmentProcessor> GrCoverageCountingPathRenderer::makeClipProcessor(
        std::unique_ptr<GrFragmentProcessor> inputFP, uint32_t opsTaskID,
        const SkPath& deviceSpacePath, const SkIRect& accessRect, const GrCaps& caps) {
    return nullptr;
}

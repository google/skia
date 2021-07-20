/*
* Copyright 2021 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/mtl/GrMtlFramebuffer.h"

#include "src/gpu/mtl/GrMtlAttachment.h"

sk_sp<const GrMtlFramebuffer> GrMtlFramebuffer::Make(
        GrMtlAttachment* colorAttachment,
        GrMtlAttachment* resolveAttachment,
        GrMtlAttachment* stencilAttachment) {
    // At the very least we need a colorAttachment
    SkASSERT(colorAttachment);

    auto fb = new GrMtlFramebuffer(sk_ref_sp(colorAttachment), sk_ref_sp(resolveAttachment),
                                   sk_ref_sp(stencilAttachment));
    return sk_sp<const GrMtlFramebuffer>(fb);
}

GrMtlFramebuffer::GrMtlFramebuffer(sk_sp<GrMtlAttachment> colorAttachment,
                                   sk_sp<GrMtlAttachment> resolveAttachment,
                                   sk_sp<GrMtlAttachment> stencilAttachment)
        : fColorAttachment(std::move(colorAttachment))
        , fResolveAttachment(std::move(resolveAttachment))
        , fStencilAttachment(std::move(stencilAttachment)) {
}

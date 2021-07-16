/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLFramebuffer.h"

#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/gl/GrGLAttachment.h"
#include "src/gpu/gl/GrGLCaps.h"

GrGLFramebuffer::GrGLFramebuffer(GrGLuint fboID,
                                 sk_sp<GrGLAttachment> colorAttachment,
                                 sk_sp<GrGLAttachment> stencilAttachment)
        : fFBOID(fboID)
        , fColorAttachment(std::move(colorAttachment))
        , fStencilAttachment(std::move(stencilAttachment))
        , fUniqueID(GenID()) {
    SkASSERT(fColorAttachment);
}

SkISize GrGLFramebuffer::dimensions() const { return fColorAttachment->dimensions(); }

int GrGLFramebuffer::numColorSamples() const { return fColorAttachment->numSamples(); }

int GrGLFramebuffer::numStencilBits() const {
    SkASSERT(fStencilAttachment);
    return GrBackendFormatStencilBits(fStencilAttachment->backendFormat());
}


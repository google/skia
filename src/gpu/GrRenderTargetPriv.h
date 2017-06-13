/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetPriv_DEFINED
#define GrRenderTargetPriv_DEFINED

#include "GrRenderTarget.h"
#include "GrGpu.h"

class GrStencilSettings;

/** Class that adds methods to GrRenderTarget that are only intended for use internal to Skia.
    This class is purely a privileged window into GrRenderTarget. It should never have additional
    data members or virtual methods. */
class GrRenderTargetPriv {
public:
    /**
     * GrStencilAttachment is not part of the public API.
     */
    GrStencilAttachment* getStencilAttachment() const { return fRenderTarget->fStencilAttachment; }

    /**
     * Attaches the GrStencilAttachment onto the render target. If stencil is a nullptr then the
     * currently attached GrStencilAttachment will be removed if one was previously attached. This
     * function returns false if there were any failure in attaching the GrStencilAttachment.
     */
    bool attachStencilAttachment(GrStencilAttachment* stencil);

    int numStencilBits() const;

    // Finds a render target's multisample specs. The pipeline is only needed in case the info isn't
    // cached and we need to flush the draw state in order to query it. The pipeline is not expected
    // to affect the multisample information itself.
    const GrGpu::MultisampleSpecs& getMultisampleSpecs(const GrPipeline&) const;

    GrRenderTargetFlags flags() const { return fRenderTarget->fFlags; }

private:
    explicit GrRenderTargetPriv(GrRenderTarget* renderTarget) : fRenderTarget(renderTarget) {}
    GrRenderTargetPriv(const GrRenderTargetPriv&) {} // unimpl
    GrRenderTargetPriv& operator=(const GrRenderTargetPriv&); // unimpl

    // No taking addresses of this type.
    const GrRenderTargetPriv* operator&() const;
    GrRenderTargetPriv* operator&();

    GrRenderTarget* fRenderTarget;

    friend class GrRenderTarget; // to construct/copy this type.
};

inline GrRenderTargetPriv GrRenderTarget::renderTargetPriv() { return GrRenderTargetPriv(this); }

inline const GrRenderTargetPriv GrRenderTarget::renderTargetPriv () const {
    return GrRenderTargetPriv(const_cast<GrRenderTarget*>(this));
}

#endif

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetPriv_DEFINED
#define GrRenderTargetPriv_DEFINED

#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/GrGpu.h"

class GrStencilSettings;

/** Class that adds methods to GrRenderTarget that are only intended for use internal to Skia.
    This class is purely a privileged window into GrRenderTarget. It should never have additional
    data members or virtual methods. */
class GrRenderTargetPriv {
public:
    /**
     * GrStencilAttachment is not part of the public API.
     */
    GrStencilAttachment* getStencilAttachment() const {
        return fRenderTarget->fStencilAttachment.get();
    }

    /**
     * Attaches the GrStencilAttachment onto the render target. If stencil is a nullptr then the
     * currently attached GrStencilAttachment will be removed if one was previously attached. This
     * function returns false if there were any failure in attaching the GrStencilAttachment.
     */
    void attachStencilAttachment(sk_sp<GrStencilAttachment> stencil);

    int numStencilBits() const;

    /**
     * Returns a unique key that identifies this render target's sample pattern. (Must be
     * multisampled.)
     */
    int getSamplePatternKey() const;

    /**
     * Retrieves the per-pixel HW sample locations for this render target, and, as a by-product, the
     * actual number of samples in use. (This may differ from fSampleCnt.) Sample locations are
     * returned as 0..1 offsets relative to the top-left corner of the pixel.
     */
    const SkTArray<SkPoint>& getSampleLocations() const {
        int samplePatternKey = this->getSamplePatternKey();
        return fRenderTarget->getGpu()->retrieveSampleLocations(samplePatternKey);
    }

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

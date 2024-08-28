/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTarget_DEFINED
#define GrRenderTarget_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrSurface.h"

#include <string_view>

class GrGpu;
struct SkISize;
struct SkPoint;

/**
 * GrRenderTarget represents a 2D buffer of pixels that can be rendered to.
 * A context's render target is set by setRenderTarget(). Render targets are
 * created by a createTexture with the kRenderTarget_SurfaceFlag flag.
 * Additionally, GrContext provides methods for creating GrRenderTargets
 * that wrap externally created render targets.
 */
class GrRenderTarget : virtual public GrSurface {
public:
    // Make manual MSAA resolve publicly accessible from GrRenderTarget.
    using GrSurface::setRequiresManualMSAAResolve;
    using GrSurface::requiresManualMSAAResolve;

    virtual bool alwaysClearStencil() const { return false; }

    // GrSurface overrides
    GrRenderTarget* asRenderTarget() override { return this; }
    const GrRenderTarget* asRenderTarget() const  override { return this; }

    /**
     * Returns the number of samples/pixel in the color buffer (One if non-MSAA).
     */
    int numSamples() const { return fSampleCnt; }

    virtual GrBackendRenderTarget getBackendRenderTarget() const = 0;

    GrAttachment* getStencilAttachment(bool useMSAASurface) const {
        return (useMSAASurface) ? fMSAAStencilAttachment.get() : fStencilAttachment.get();
    }

    GrAttachment* getStencilAttachment() const {
        return getStencilAttachment(this->numSamples() > 1);
    }

    // Checked when this object is asked to attach a stencil buffer.
    virtual bool canAttemptStencilAttachment(bool useMSAASurface) const = 0;

    void attachStencilAttachment(sk_sp<GrAttachment> stencil, bool useMSAASurface);

    int numStencilBits(bool useMSAASurface) const;

    /**
     * Returns a unique key that identifies this render target's sample pattern. (Must be
     * multisampled.)
     */
    int getSamplePatternKey();

    /**
     * Retrieves the per-pixel HW sample locations for this render target, and, as a by-product, the
     * actual number of samples in use. (This may differ from fSampleCnt.) Sample locations are
     * returned as 0..1 offsets relative to the top-left corner of the pixel.
     */
    const skia_private::TArray<SkPoint>& getSampleLocations();

protected:
    GrRenderTarget(GrGpu*,
                   const SkISize&,
                   int sampleCount,
                   GrProtected,
                   std::string_view label,
                   sk_sp<GrAttachment> stenicl = nullptr);
    ~GrRenderTarget() override;

    // override of GrResource
    void onAbandon() override;
    void onRelease() override;

private:
    // Allows the backends to perform any additional work that is required for attaching a
    // GrAttachment. When this is called, the GrAttachment has already been put onto
    // the GrRenderTarget. This function must return false if any failures occur when completing the
    // stencil attachment.
    virtual bool completeStencilAttachment(GrAttachment* stencil, bool useMSAASurface) = 0;

    sk_sp<GrAttachment> fStencilAttachment;
    sk_sp<GrAttachment> fMSAAStencilAttachment;
    int fSampleCnt;

    using INHERITED = GrSurface;
};

#endif

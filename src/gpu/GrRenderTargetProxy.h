/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxy_DEFINED
#define GrRenderTargetProxy_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSwizzle.h"

class GrResourceProvider;
class GrRenderTargetProxyPriv;

// This class delays the acquisition of RenderTargets until they are actually
// required
// Beware: the uniqueID of the RenderTargetProxy will usually be different than
// the uniqueID of the RenderTarget it represents!
class GrRenderTargetProxy : virtual public GrSurfaceProxy {
public:
    GrRenderTargetProxy* asRenderTargetProxy() override { return this; }
    const GrRenderTargetProxy* asRenderTargetProxy() const override { return this; }

    // Actually instantiate the backing rendertarget, if necessary.
    bool instantiate(GrResourceProvider*) override;

    GrFSAAType fsaaType() const {
        if (fSampleCnt <= 1) {
            SkASSERT(!this->hasMixedSamples());
            return GrFSAAType::kNone;
        }
        return this->hasMixedSamples() ? GrFSAAType::kMixedSamples : GrFSAAType::kUnifiedMSAA;
    }

    /*
     * When instantiated does this proxy require a stencil buffer?
     */
    void setNeedsStencil() { fNeedsStencil = true; }
    bool needsStencil() const { return fNeedsStencil; }

    /**
     * Returns the number of samples/pixel in the stencil buffer (One if non-MSAA).
     */
    int numStencilSamples() const { return fSampleCnt; }

    /**
     * Returns the number of samples/pixel in the color buffer (One if non-MSAA or mixed sampled).
     */
    int numColorSamples() const {
        return GrFSAAType::kMixedSamples == this->fsaaType() ? 1 : fSampleCnt;
    }

    int maxWindowRectangles(const GrCaps& caps) const;

    const GrSwizzle& outputSwizzle() const { return fOutputSwizzle; }

    bool wrapsVkSecondaryCB() const { return fWrapsVkSecondaryCB == WrapsVkSecondaryCB::kYes; }

    // TODO: move this to a priv class!
    bool refsWrappedObjects() const;

    // Provides access to special purpose functions.
    GrRenderTargetProxyPriv rtPriv();
    const GrRenderTargetProxyPriv rtPriv() const;

protected:
    friend class GrProxyProvider;  // for ctors
    friend class GrRenderTargetProxyPriv;

    // Deferred version
    GrRenderTargetProxy(const GrCaps&, const GrBackendFormat&, const GrSurfaceDesc&,
                        GrSurfaceOrigin, const GrSwizzle& textureSwizzle,
                        const GrSwizzle& outputSwizzle, SkBackingFit, SkBudgeted,
                        GrInternalSurfaceFlags);

    enum class WrapsVkSecondaryCB : bool { kNo = false, kYes = true };

    // Lazy-callback version
    // There are two main use cases for lazily-instantiated proxies:
    //   basic knowledge - width, height, config, samples, origin are known
    //   minimal knowledge - only config is known.
    //
    // The basic knowledge version is used for DDL where we know the type of proxy we are going to
    // use, but we don't have access to the GPU yet to instantiate it.
    //
    // The minimal knowledge version is used for CCPR where we are generating an atlas but we do not
    // know the final size until flush time.
    GrRenderTargetProxy(LazyInstantiateCallback&&, LazyInstantiationType lazyType,
                        const GrBackendFormat&, const GrSurfaceDesc&, GrSurfaceOrigin,
                        const GrSwizzle& textureSwizzle, const GrSwizzle& outputSwizzle,
                        SkBackingFit, SkBudgeted, GrInternalSurfaceFlags,
                        WrapsVkSecondaryCB wrapsVkSecondaryCB);

    // Wrapped version
    GrRenderTargetProxy(sk_sp<GrSurface>, GrSurfaceOrigin, const GrSwizzle& textureSwizzle,
                        const GrSwizzle& outputSwizzle,
                        WrapsVkSecondaryCB wrapsVkSecondaryCB = WrapsVkSecondaryCB::kNo);

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

private:
    void setHasMixedSamples() {
        fSurfaceFlags |= GrInternalSurfaceFlags::kMixedSampled;
    }
    bool hasMixedSamples() const { return fSurfaceFlags & GrInternalSurfaceFlags::kMixedSampled; }

    void setGLRTFBOIDIs0() {
        fSurfaceFlags |= GrInternalSurfaceFlags::kGLRTFBOIDIs0;
    }
    bool glRTFBOIDIs0() const {
        return fSurfaceFlags & GrInternalSurfaceFlags::kGLRTFBOIDIs0;
    }


    size_t onUninstantiatedGpuMemorySize() const override;
    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override;)

    // WARNING: Be careful when adding or removing fields here. ASAN is likely to trigger warnings
    // when instantiating GrTextureRenderTargetProxy. The std::function in GrSurfaceProxy makes
    // each class in the diamond require 16 byte alignment. Clang appears to layout the fields for
    // each class to achieve the necessary alignment. However, ASAN checks the alignment of 'this'
    // in the constructors, and always looks for the full 16 byte alignment, even if the fields in
    // that particular class don't require it. Changing the size of this object can move the start
    // address of other types, leading to this problem.

    int                fSampleCnt;
    GrSwizzle          fOutputSwizzle;
    bool               fNeedsStencil;
    WrapsVkSecondaryCB fWrapsVkSecondaryCB;
    // This is to fix issue in large comment above. Without the padding we end 6 bytes into a 16
    // byte range, so the GrTextureProxy ends up starting 8 byte aligned by not 16. We add the
    // padding here to get us right up to the 16 byte alignment (technically any padding of 3-10
    // bytes would work since it always goes up to 8 byte alignment, but we use 10 to more explicit
    // about what we're doing).
    char               fDummyPadding[10];

    // For wrapped render targets the actual GrRenderTarget is stored in the GrIORefProxy class.
    // For deferred proxies that pointer is filled in when we need to instantiate the
    // deferred resource.

    typedef GrSurfaceProxy INHERITED;
};

#endif

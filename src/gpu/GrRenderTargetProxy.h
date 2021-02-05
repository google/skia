/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxy_DEFINED
#define GrRenderTargetProxy_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrNativeRect.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSwizzle.h"

class GrResourceProvider;

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

    bool canUseMixedSamples(const GrCaps& caps) const {
        return caps.mixedSamplesSupport() && !this->glRTFBOIDIs0() &&
               caps.internalMultisampleCount(this->backendFormat()) > 1 &&
               this->canChangeStencilAttachment();
    }

    /*
     * Indicate that a draw to this proxy requires stencil, and how many stencil samples it needs.
     * The number of stencil samples on this proxy will be equal to the largest sample count passed
     * to this method.
     */
    void setNeedsStencil(int8_t numStencilSamples) {
        SkASSERT(numStencilSamples >= fSampleCnt);
        fNumStencilSamples = std::max(numStencilSamples, fNumStencilSamples);
    }

    /**
     * Returns the number of stencil samples this proxy will use, or 0 if it does not use stencil.
     */
    int numStencilSamples() const { return fNumStencilSamples; }

    /**
     * Returns the number of samples/pixel in the color buffer (One if non-MSAA).
     */
    int numSamples() const { return fSampleCnt; }

    int maxWindowRectangles(const GrCaps& caps) const;

    bool glRTFBOIDIs0() const { return fSurfaceFlags & GrInternalSurfaceFlags::kGLRTFBOIDIs0; }

    bool wrapsVkSecondaryCB() const { return fWrapsVkSecondaryCB == WrapsVkSecondaryCB::kYes; }

    bool supportsVkInputAttachment() const {
        return fSurfaceFlags & GrInternalSurfaceFlags::kVkRTSupportsInputAttachment;
    }

    void markMSAADirty(SkIRect dirtyRect) {
        SkASSERT(SkIRect::MakeSize(this->backingStoreDimensions()).contains(dirtyRect));
        SkASSERT(this->requiresManualMSAAResolve());
        fMSAADirtyRect.join(dirtyRect);
    }
    void markMSAAResolved() {
        SkASSERT(this->requiresManualMSAAResolve());
        fMSAADirtyRect.setEmpty();
    }
    bool isMSAADirty() const {
        SkASSERT(fMSAADirtyRect.isEmpty() || this->requiresManualMSAAResolve());
        return this->requiresManualMSAAResolve() && !fMSAADirtyRect.isEmpty();
    }
    const SkIRect& msaaDirtyRect() const {
        SkASSERT(this->requiresManualMSAAResolve());
        return fMSAADirtyRect;
    }

    // TODO: move this to a priv class!
    bool refsWrappedObjects() const;

protected:
    friend class GrProxyProvider;  // for ctors
    friend class GrRenderTargetProxyPriv;

    // Deferred version
    GrRenderTargetProxy(const GrCaps&,
                        const GrBackendFormat&,
                        SkISize,
                        int sampleCount,
                        SkBackingFit,
                        SkBudgeted,
                        GrProtected,
                        GrInternalSurfaceFlags,
                        UseAllocator);

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
    GrRenderTargetProxy(LazyInstantiateCallback&&,
                        const GrBackendFormat&,
                        SkISize,
                        int sampleCount,
                        SkBackingFit,
                        SkBudgeted,
                        GrProtected,
                        GrInternalSurfaceFlags,
                        UseAllocator,
                        WrapsVkSecondaryCB);

    // Wrapped version
    GrRenderTargetProxy(sk_sp<GrSurface>,
                        UseAllocator,
                        WrapsVkSecondaryCB = WrapsVkSecondaryCB::kNo);

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

private:
    bool canChangeStencilAttachment() const;

    size_t onUninstantiatedGpuMemorySize() const override;
    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override;)

            LazySurfaceDesc callbackDesc() const override;

    // WARNING: Be careful when adding or removing fields here. ASAN is likely to trigger warnings
    // when instantiating GrTextureRenderTargetProxy. The std::function in GrSurfaceProxy makes
    // each class in the diamond require 16 byte alignment. Clang appears to layout the fields for
    // each class to achieve the necessary alignment. However, ASAN checks the alignment of 'this'
    // in the constructors, and always looks for the full 16 byte alignment, even if the fields in
    // that particular class don't require it. Changing the size of this object can move the start
    // address of other types, leading to this problem.
    int8_t             fSampleCnt;
    int8_t             fNumStencilSamples = 0;
    WrapsVkSecondaryCB fWrapsVkSecondaryCB;
    SkIRect            fMSAADirtyRect = SkIRect::MakeEmpty();
    // This is to fix issue in large comment above. Without the padding we can end up with the
    // GrTextureProxy starting 8 byte aligned by not 16. This happens when the RT ends at bytes 1-8.
    // Note: with the virtual inheritance an 8 byte pointer is at the start of GrRenderTargetProxy.
    //
    // In the current world we end the RT proxy at 12 bytes. Technically any padding between 0-4
    // will work, but we use 4 to be more explicit about getting it to 16 byte alignment.
    char               fDummyPadding[4];

    using INHERITED = GrSurfaceProxy;
};

#endif

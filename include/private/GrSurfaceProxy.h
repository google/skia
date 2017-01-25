/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxy_DEFINED
#define GrSurfaceProxy_DEFINED

#include "GrGpuResource.h"
#include "GrSurface.h"

#include "SkRect.h"

class GrCaps;
class GrRenderTargetOpList;
class GrRenderTargetProxy;
class GrSurfaceContext;
class GrTextureOpList;
class GrTextureProvider;
class GrTextureProxy;

// This class replicates the functionality GrIORef<GrSurface> but tracks the
// utilitization for later resource allocation (for the deferred case) and
// forwards on the utilization in the wrapped case
class GrIORefProxy : public SkNoncopyable {
public:
    void ref() const {
        this->validate();

        ++fRefCnt;
        if (fTarget) {
            fTarget->ref();
        }
    }

    void unref() const {
        this->validate();

        if (fTarget) {
            fTarget->unref();
        }

        if (!(--fRefCnt)) {
            delete this;
            return;
        }

        this->validate();
    }

    void validate() const {
#ifdef SK_DEBUG    
        SkASSERT(fRefCnt >= 1);
        SkASSERT(fPendingReads >= 0);
        SkASSERT(fPendingWrites >= 0);
        SkASSERT(fRefCnt + fPendingReads + fPendingWrites >= 1);

        if (fTarget) {
            SkASSERT(!fPendingReads && !fPendingWrites);
            // The backing GrSurface can have more refs than the proxy if the proxy
            // started off wrapping an external resource (that came in with refs).
            // The GrSurface should never have fewer refs than the proxy however.
            SkASSERT(fTarget->fRefCnt >= fRefCnt);
        }
#endif
    }

    int32_t getProxyRefCnt_TestOnly() const;
    int32_t getBackingRefCnt_TestOnly() const;
    int32_t getPendingReadCnt_TestOnly() const;
    int32_t getPendingWriteCnt_TestOnly() const;

protected:
    GrIORefProxy() : fTarget(nullptr), fRefCnt(1), fPendingReads(0), fPendingWrites(0) {}
    GrIORefProxy(sk_sp<GrSurface> surface) : fRefCnt(1), fPendingReads(0), fPendingWrites(0) {
        // Since we're manually forwarding on refs & unrefs we don't want sk_sp doing
        // anything extra.
        fTarget = surface.release();
    }
    virtual ~GrIORefProxy() {
        // We don't unref 'fTarget' here since the 'unref' method will already
        // have forwarded on the unref call that got use here.
    }

    // This GrIORefProxy was deferred before but has just been instantiated. To
    // make all the reffing & unreffing work out we now need to transfer any deferred
    // refs & unrefs to the new GrSurface
    void transferRefs() {
        SkASSERT(fTarget);

        fTarget->fRefCnt += (fRefCnt-1); // don't xfer the proxy's creation ref
        fTarget->fPendingReads += fPendingReads;
        fTarget->fPendingWrites += fPendingWrites;

        fPendingReads = 0;
        fPendingWrites = 0;
    }

    // For deferred proxies this will be null. For wrapped proxies it will point to the
    // wrapped resource.
    GrSurface* fTarget;

private:
    // This class is used to manage conversion of refs to pending reads/writes.
    friend class GrGpuResourceRef;
    template <typename, GrIOType> friend class GrPendingIOResource;

    void addPendingRead() const {
        this->validate();

        if (fTarget) {
            fTarget->addPendingRead();
            return;
        }

        ++fPendingReads;
    }

    void completedRead() const {
        this->validate();

        if (fTarget) {
            fTarget->completedRead();
            return;
        }
    
        SkFAIL("How was the read completed if the Proxy hasn't been instantiated?");
    }

    void addPendingWrite() const {
        this->validate();

        if (fTarget) {
            fTarget->addPendingWrite();
            return;
        }

        ++fPendingWrites;
    }

    void completedWrite() const {
        this->validate();

        if (fTarget) {
            fTarget->completedWrite();
            return;
        }
    
        SkFAIL("How was the write completed if the Proxy hasn't been instantiated?");
    }

    mutable int32_t fRefCnt;
    mutable int32_t fPendingReads;
    mutable int32_t fPendingWrites;
};

class GrSurfaceProxy : public GrIORefProxy {
public:
    static sk_sp<GrSurfaceProxy> MakeWrapped(sk_sp<GrSurface>);

    static sk_sp<GrSurfaceProxy> MakeDeferred(const GrCaps&, const GrSurfaceDesc&,
                                              SkBackingFit, SkBudgeted);

    // TODO: need to refine ownership semantics of 'srcData' if we're in completely
    // deferred mode
    static sk_sp<GrSurfaceProxy> MakeDeferred(const GrCaps&, GrTextureProvider*,
                                              const GrSurfaceDesc&, SkBudgeted,
                                              const void* srcData, size_t rowBytes);

    const GrSurfaceDesc& desc() const { return fDesc; }

    GrSurfaceOrigin origin() const {
        SkASSERT(kTopLeft_GrSurfaceOrigin == fDesc.fOrigin ||
                 kBottomLeft_GrSurfaceOrigin == fDesc.fOrigin);
        return fDesc.fOrigin;
    }
    int width() const { return fDesc.fWidth; }
    int height() const { return fDesc.fHeight; }
    GrPixelConfig config() const { return fDesc.fConfig; }

    class UniqueID {
    public:
        // wrapped
        explicit UniqueID(const GrGpuResource::UniqueID& id) : fID(id.asUInt()) { }
        // deferred
        UniqueID() : fID(GrGpuResource::CreateUniqueID()) { }

        uint32_t asUInt() const { return fID; }

        bool operator==(const UniqueID& other) const {
            return fID == other.fID;
        }
        bool operator!=(const UniqueID& other) const {
            return !(*this == other);
        }

        bool isInvalid() const { return SK_InvalidUniqueID == fID; }

    private:
        const uint32_t fID;
    };

    /*
     * The contract for the uniqueID is:
     *   for wrapped resources:
     *      the uniqueID will match that of the wrapped resource
     *
     *   for deferred resources:
     *      the uniqueID will be different from the real resource, when it is allocated
     *      the proxy's uniqueID will not change across the instantiate call
     *
     *    the uniqueIDs of the proxies and the resources draw from the same pool
     *
     * What this boils down to is that the uniqueID of a proxy can be used to consistently
     * track/identify a proxy but should never be used to distinguish between
     * resources and proxies - beware!
     */
    UniqueID uniqueID() const { return fUniqueID; }

    GrSurface* instantiate(GrTextureProvider* texProvider);

    /**
     * Helper that gets the width and height of the surface as a bounding rectangle.
     */
    SkRect getBoundsRect() const { return SkRect::MakeIWH(this->width(), this->height()); }

    int worstCaseWidth(const GrCaps& caps) const;
    int worstCaseHeight(const GrCaps& caps) const;

    /**
     * @return the texture proxy associated with the surface proxy, may be NULL.
     */
    virtual GrTextureProxy* asTextureProxy() { return nullptr; }
    virtual const GrTextureProxy* asTextureProxy() const { return nullptr; }

    /**
     * @return the render target proxy associated with the surface proxy, may be NULL.
     */
    virtual GrRenderTargetProxy* asRenderTargetProxy() { return nullptr; }
    virtual const GrRenderTargetProxy* asRenderTargetProxy() const { return nullptr; }

    /**
     * Does the resource count against the resource budget?
     */
    SkBudgeted isBudgeted() const { return fBudgeted; }

    void setLastOpList(GrOpList* opList);
    GrOpList* getLastOpList() { return fLastOpList; }

    GrRenderTargetOpList* getLastRenderTargetOpList();
    GrTextureOpList* getLastTextureOpList();

    /**
     * Retrieves the amount of GPU memory that will be or currently is used by this resource 
     * in bytes. It is approximate since we aren't aware of additional padding or copies made
     * by the driver.
     *
     * @return the amount of GPU memory used in bytes
     */
    size_t gpuMemorySize() const {
        if (kInvalidGpuMemorySize == fGpuMemorySize) {
            fGpuMemorySize = this->onGpuMemorySize();
            SkASSERT(kInvalidGpuMemorySize != fGpuMemorySize);
        }
        return fGpuMemorySize;
    }

    // Helper function that creates a temporary SurfaceContext to perform the copy
    static sk_sp<GrSurfaceProxy> Copy(GrContext*, GrSurfaceProxy* src,
                                      SkIRect srcRect, SkBudgeted);

    // Copy the entire 'src'
    static sk_sp<GrSurfaceProxy> Copy(GrContext* context, GrSurfaceProxy* src,
                                      SkBudgeted budgeted) {
        return Copy(context, src, SkIRect::MakeWH(src->width(), src->height()), budgeted);
    }

    // Test-only entry point - should decrease in use as proxies propagate
    static sk_sp<GrSurfaceContext> TestCopy(GrContext* context, const GrSurfaceDesc& dstDesc,
                                            GrSurfaceProxy* srcProxy);

    bool isWrapped_ForTesting() const;

    SkDEBUGCODE(void validate(GrContext*) const;)

protected:
    // Deferred version
    GrSurfaceProxy(const GrSurfaceDesc& desc, SkBackingFit fit, SkBudgeted budgeted)
        : fDesc(desc)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fGpuMemorySize(kInvalidGpuMemorySize)
        , fLastOpList(nullptr) {
        // Note: this ctor pulls a new uniqueID from the same pool at the GrGpuResources
    }

    // Wrapped version
    GrSurfaceProxy(sk_sp<GrSurface> surface, SkBackingFit fit);

    virtual ~GrSurfaceProxy();

    // For wrapped resources, 'fDesc' will always be filled in from the wrapped resource.
    const GrSurfaceDesc  fDesc;
    const SkBackingFit   fFit;      // always exact for wrapped resources
    const SkBudgeted     fBudgeted; // set from the backing resource for wrapped resources
    const UniqueID       fUniqueID; // set from the backing resource for wrapped resources

    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);
    SkDEBUGCODE(size_t getRawGpuMemorySize_debugOnly() const { return fGpuMemorySize; })

private:
    virtual size_t onGpuMemorySize() const = 0;

    // This entry is lazily evaluated so, when the proxy wraps a resource, the resource
    // will be called but, when the proxy is deferred, it will compute the answer itself.
    // If the proxy computes its own answer that answer is checked (in debug mode) in
    // the instantiation method.
    mutable size_t      fGpuMemorySize;

    // The last opList that wrote to or is currently going to write to this surface
    // The opList can be closed (e.g., no render target context is currently bound
    // to this renderTarget).
    // This back-pointer is required so that we can add a dependancy between
    // the opList used to create the current contents of this surface
    // and the opList of a destination surface to which this one is being drawn or copied.
    GrOpList* fLastOpList;


    typedef GrIORefProxy INHERITED;
};

#endif

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrCaps.h"
#include "GrContextPriv.h"
#include "GrContextThreadSafeProxyPriv.h"

/**
 * The DDL Context is the one in effect during DDL Recording. It isn't backed by a GrGPU and
 * cannot allocate any GPU resources.
 */
class SK_API GrDDLContext : public GrContext {
public:
    GrDDLContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(proxy->priv().backend(), proxy->priv().contextUniqueID()) {
        fCaps = proxy->priv().refCaps();
        fThreadSafeProxy = std::move(proxy);
    }

    ~GrDDLContext() override {
        // The GrDDLContext doesn't actually own the fRestrictedAtlasManager so don't delete it
    }

    void abandonContext() override {
        SkASSERT(0); // abandoning in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::abandonContext();
    }

    void releaseResourcesAndAbandonContext() override {
        SkASSERT(0); // abandoning in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::releaseResourcesAndAbandonContext();
    }

    void freeGpuResources() override {
        SkASSERT(0); // freeing resources in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::freeGpuResources();
    }

protected:
    bool init(const GrContextOptions& options) override {
        SkASSERT(fCaps);  // should've been set in ctor
        SkASSERT(fThreadSafeProxy); // should've been set in the ctor

        if (!INHERITED::initCommon(options)) {
            return false;
        }

        return true;
    }

    GrAtlasManager* onGetAtlasManager() override {
        SkASSERT(0);   // the DDL Recorders should never invoke this
        return nullptr;
    }

private:
    typedef GrContext INHERITED;
};

sk_sp<GrContext> GrContextPriv::MakeDDL(const sk_sp<GrContextThreadSafeProxy>& proxy) {
    sk_sp<GrContext> context(new GrDDLContext(proxy));

    // Note: we aren't creating a Gpu here. This causes the resource provider & cache to
    // also not be created
    if (!context->init(proxy->priv().contextOptions())) {
        return nullptr;
    }
    return context;
}

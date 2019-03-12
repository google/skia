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
#include "GrSkSLFPFactoryCache.h"

/**
 * The DDL Context is the one in effect during DDL Recording. It isn't backed by a GrGPU and
 * cannot allocate any GPU resources.
 */
class SK_API GrDDLContext : public GrContext {
public:
    GrDDLContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(proxy->backend(), proxy->priv().options(), proxy->priv().contextID()) {
        fThreadSafeProxy = std::move(proxy);
    }

    ~GrDDLContext() override { }

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
    // TODO: Here we're pretending this isn't derived from GrContext. Switch this to be derived from
    // GrRecordingContext!
    GrContext* asDirectContext() override { return nullptr; }

    bool init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> FPFactoryCache) override {
        SkASSERT(caps && FPFactoryCache);
        SkASSERT(fThreadSafeProxy); // should've been set in the ctor

        if (!INHERITED::init(std::move(caps), std::move(FPFactoryCache))) {
            return false;
        }

        // DDL contexts/drawing managers always sort the oplists. This, in turn, implies that
        // explicit resource allocation is always on (regardless of how Skia is compiled).
        this->setupDrawingManager(true, true);

        SkASSERT(this->caps());

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

    if (!context->init(proxy->priv().refCaps(), proxy->priv().fpFactoryCache())) {
        return nullptr;
    }
    return context;
}

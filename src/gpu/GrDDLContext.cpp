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
class SK_API GrDDLContext : public GrRecordingContext {
public:
    GrDDLContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(proxy->backend(), proxy->priv().options(), proxy->priv().contextID()) {
        fThreadSafeProxy = std::move(proxy);
    }

    ~GrDDLContext() override { }

    static sk_sp<GrRecordingContext> Make(const sk_sp<GrContextThreadSafeProxy> proxy) {
        sk_sp<GrDDLContext> context(new GrDDLContext(proxy));

        if (!context->initDDL(proxy->priv().refCaps(),
                              proxy,
                              proxy->priv().fpFactoryCache())) {
            return nullptr;
        }

        return context;
    }

protected:
    // TODO: Here we're pretending this isn't derived from GrContext. Switch this to be derived from
    // GrRecordingContext!
    GrContext* asDirectContext() override { return nullptr; }

    bool init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> cache) override {
        SkASSERT(caps && FPFactoryCache);
        SkASSERT(fThreadSafeProxy); // should've been set in the ctor

        if (!INHERITED::init(std::move(caps), std::move(cache))) {
            return false;
        }

        SkASSERT(this->caps());
        // Create drawingManager and proxyManager in here

        return true;
    }

        return true;
    }

private:
    typedef GrRecordingContext INHERITED;
};

sk_sp<GrContext> GrContextPriv::MakeDDL(const sk_sp<GrContextThreadSafeProxy>& proxy) {
    sk_sp<GrContext> context(new GrDDLContext(proxy));

    // Note: we aren't creating a Gpu here. This causes the resource provider & cache to
    // also not be created
    if (!context->init(proxy->priv().refCaps(),
                       proxy,
                       proxy->priv().fpFactoryCache())) {
        return nullptr;
    }
    return context;
}

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
class SK_API GrDDLContext : public GrRecordingContext {
public:
    GrDDLContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(proxy->priv().backend(),
                        proxy->priv().contextOptions(),
                        proxy->priv().contextUniqueID()) {
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
    bool initDDL(sk_sp<const GrCaps> caps,
                 sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                 sk_sp<GrSkSLFPFactoryCache> cache) {
        if (!INHERITED::initWeakest(std::move(caps), std::move(threadSafeProxy), std::move(cache))) {
            return false;
        }

        // Create drawingManager and proxyManager in here

        return true;
    }

private:
    typedef GrRecordingContext INHERITED;
};

sk_sp<GrRecordingContext> GrContextPriv::MakeDDL(const sk_sp<GrContextThreadSafeProxy>& proxy) {
    return GrDDLContext::Make(std::move(proxy));
}

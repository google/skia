/*
 * Copyright 2019 Google Inc.
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
class SK_API GrImageCreationContext : public GrImageContext {
public:
    GrImageCreationContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(proxy->priv().backend(),
                        proxy->priv().contextOptions(),
                        proxy->priv().contextUniqueID()) {
    }

    ~GrImageCreationContext() override { }

    static sk_sp<GrImageContext> Make(const sk_sp<GrContextThreadSafeProxy> proxy) {
        sk_sp<GrImageCreationContext> context(new GrImageCreationContext(proxy));

        if (!context->initImage(proxy->priv().refCaps(),
                              proxy,
                              proxy->priv().fpFactoryCache())) {
            return nullptr;
        }

        return context;
    }

protected:
    bool initImage(sk_sp<const GrCaps> caps,
                   sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                   sk_sp<GrSkSLFPFactoryCache> cache) {
        if (!this->initWeakest(std::move(caps), std::move(threadSafeProxy), std::move(cache))) {
            return false;
        }

        // Create proxyProvider here

        return true;
    }

private:
    typedef GrImageContext INHERITED;
};

sk_sp<GrImageContext> GrContextPriv::MakeImageContext(const sk_sp<GrContextThreadSafeProxy>& proxy) {
    return GrImageCreationContext::Make(std::move(proxy));
}

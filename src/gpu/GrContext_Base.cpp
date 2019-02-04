/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext_Base.h"

#include "GrCaps.h"
#include "GrSkSLFPFactoryCache.h"
#include "GrContextThreadSafeProxy.h"

static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};
    int32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidGenID);
    return id;
}

GrContext_Base::GrContext_Base(GrBackendApi backend,
                               const GrContextOptions& options,
                               uint32_t contextID)
        : fBackend(backend)
        , fOptions(options)
        , fContextID(SK_InvalidGenID == contextID ? next_id() : contextID) {
}

GrContext_Base::~GrContext_Base() { }

bool GrContext_Base::matches(GrContext_Base* context) const {
    return context->contextID() == this->contextID();
}

bool GrContext_Base::initWeakest(sk_sp<const GrCaps> caps,
                                 sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                                 sk_sp<GrSkSLFPFactoryCache> cache) {
    fCaps = std::move(caps);
    fThreadSafeProxy = std::move(threadSafeProxy);
    fFPFactoryCache = std::move(cache);
    return true;
}

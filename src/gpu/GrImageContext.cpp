/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageContext.h"

#include "GrCaps.h"
#include "GrImageContextPriv.h"
#include "GrProxyProvider.h"
#include "GrSkSLFPFactoryCache.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)

///////////////////////////////////////////////////////////////////////////////////////////////////
GrImageContext::GrImageContext(GrBackendApi backend,
                               const GrContextOptions& options,
                               uint32_t contextID)
            : INHERITED(backend, options, contextID) {
    fProxyProvider.reset(new GrProxyProvider(this));
}

GrImageContext::~GrImageContext() {}

void GrImageContext::abandonContext() {
    ASSERT_SINGLE_OWNER

    fAbandoned = true;
}

bool GrImageContext::abandoned() const {
    ASSERT_SINGLE_OWNER

    return fAbandoned;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrImageContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrImageContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

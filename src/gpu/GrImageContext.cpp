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

GrImageContext::GrImageContext(GrBackendApi backend,
                               const GrContextOptions& options,
                               uint32_t contextID)
            : INHERITED(backend, options, contextID) {
    fProxyProvider.reset(new GrProxyProvider(this));
}

GrImageContext::~GrImageContext() {}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrImageContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrImageContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

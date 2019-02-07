/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRecordingContext.h"

#include "GrCaps.h"
#include "GrRecordingContextPriv.h"
#include "GrSkSLFPFactoryCache.h"

GrRecordingContext::GrRecordingContext(GrBackendApi backend,
                                       const GrContextOptions& options,
                                       uint32_t contextID)
        : INHERITED(backend, options, contextID) {
}

GrRecordingContext::~GrRecordingContext() { }

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrRecordingContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrRecordingContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

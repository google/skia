/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrImageContext.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/effects/GrSkSLFP.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
GrImageContext::GrImageContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(std::move(proxy)) {
    fProxyProvider.reset(new GrProxyProvider(this));
}

GrImageContext::~GrImageContext() {}

void GrImageContext::abandonContext() {
    fThreadSafeProxy->priv().abandonContext();
}

bool GrImageContext::abandoned() {
    return fThreadSafeProxy->priv().abandoned();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrImageContextPriv::refCaps() const {
    return fContext->refCaps();
}

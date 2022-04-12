/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>

#include "include/private/gpu/ganesh/GrImageContext.h"

#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"

GrImageContext::GrImageContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(std::move(proxy)) {
}

GrImageContext::~GrImageContext() {}

void GrImageContext::abandonContext() {
    fThreadSafeProxy->priv().abandonContext();
}

bool GrImageContext::abandoned() {
    return fThreadSafeProxy->priv().abandoned();
}

sk_sp<GrImageContext> GrImageContext::MakeForPromiseImage(sk_sp<GrContextThreadSafeProxy> tsp) {
    return sk_sp<GrImageContext>(new GrImageContext(std::move(tsp)));
}

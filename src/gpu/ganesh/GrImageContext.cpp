/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/private/gpu/ganesh/GrImageContext.h"

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"

#include <utility>

GrImageContext::GrImageContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : GrContext_Base(std::move(proxy)) {
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

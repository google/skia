/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageContext.h"

#include "GrProxyProvider.h"

GrImageContext::GrImageContext(GrBackendApi backend,
                               const GrContextOptions& options,
                               uint32_t uniqueID)
            : INHERITED(backend, options, uniqueID) {
    fProxyProvider1.reset(new GrProxyProvider(this));
}

GrImageContext::~GrImageContext() {}

//GrProxyProvider* GrImageContext::proxyProvider() {
//    return fProxyProvider1.get();
//}

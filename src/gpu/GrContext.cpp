/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/text/GrStrikeCache.h"

GrContext::GrContext(sk_sp<GrContextThreadSafeProxy> proxy) : INHERITED(std::move(proxy)) { }

GrContext::~GrContext() = default;

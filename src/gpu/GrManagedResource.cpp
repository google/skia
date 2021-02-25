/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrManagedResource.h"

#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrTexture.h"


#ifdef SK_TRACE_MANAGED_RESOURCES
std::atomic<uint32_t> GrManagedResource::fKeyCounter{0};
#endif

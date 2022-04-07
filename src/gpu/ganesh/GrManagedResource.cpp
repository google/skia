/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrManagedResource.h"

#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrTexture.h"


#ifdef SK_TRACE_MANAGED_RESOURCES
std::atomic<uint32_t> GrManagedResource::fKeyCounter{0};
#endif

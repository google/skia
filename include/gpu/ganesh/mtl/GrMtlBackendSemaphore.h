/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#ifndef GrMtlBackendSemaphore_DEFINED
#define GrMtlBackendSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "include/private/base/SkAPI.h"

namespace GrBackendSemaphores {
// It is the creator's responsibility to ref the MTLEvent passed in here, via __bridge_retained.
// The other end will wrap this BackendSemaphore and take the ref, via __bridge_transfer.
SK_API GrBackendSemaphore MakeMtl(GrMTLHandle event, uint64_t value);
SK_API GrMTLHandle GetMtlHandle(const GrBackendSemaphore&);
SK_API uint64_t GetMtlValue(const GrBackendSemaphore&);
}  // namespace GrBackendSemaphores

#endif

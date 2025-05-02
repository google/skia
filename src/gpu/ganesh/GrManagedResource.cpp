/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrManagedResource.h"

#ifdef SK_TRACE_MANAGED_RESOURCES
std::atomic<uint32_t> GrManagedResource::fKeyCounter{0};

GrManagedResource::Trace* GrManagedResource::GetTrace() {
    static Trace kTrace;
    return &kTrace;
}

#endif

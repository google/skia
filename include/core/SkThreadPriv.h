/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadPriv_DEFINED
#define SkThreadPriv_DEFINED

#include "SkTypes.h"

// SK_ATOMICS_PLATFORM_H must provide inline implementations for the following declarations.

/** Atomic compare and set, for pointers.
 *  If *addr == before, set *addr to after.  Always returns previous value of *addr.
 *  This must issue a release barrier on success, acquire on failure, and always a compiler barrier.
 */
static void* sk_atomic_cas(void** addr, void* before, void* after);

#include SK_ATOMICS_PLATFORM_H

#endif//SkThreadPriv_DEFINED

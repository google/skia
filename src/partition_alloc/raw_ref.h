/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_RAW_REF_H_
#define SRC_PARTITION_ALLOC_RAW_REF_H_

#if defined(SK_USE_PARTITION_ALLOC)
#include "partition_alloc/pointers/raw_ref.h"
#else
#include "src/partition_alloc/noop/raw_ref.h"
#endif

#include "include/private/SkTypeTraits.h"

template <typename T, auto Traits>
struct sk_is_trivially_relocatable<raw_ref<T, Traits>> : std::true_type {};

#endif  // SRC_PARTITION_ALLOC_RAW_REF_H_

/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_RAW_PTR_H_
#define SRC_PARTITION_ALLOC_RAW_PTR_H_

#if defined(SK_USE_PARTITION_ALLOC)
#include "partition_alloc/pointers/raw_ptr.h"
#else
#include "src/partition_alloc/noop/raw_ptr.h"
#endif

#endif  // SRC_PARTITION_ALLOC_RAW_PTR_H_

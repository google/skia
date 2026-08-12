/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_SAFETY_CHECKS_H_
#define SRC_PARTITION_ALLOC_SAFETY_CHECKS_H_

#if defined(SK_USE_PARTITION_ALLOC)
#include "partition_alloc/safety_checks.h"
#define SK_SCOPED_DISABLE_PARTITION_ALLOC_SAFETY_CHECKS \
  partition_alloc::ScopedSafetyChecksExclusion _miracleObjExclusion
#endif

#endif  // SRC_PARTITION_ALLOC_SAFETY_CHECKS_H_

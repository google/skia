/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PartitionAlloc_TestSupport_DEFINED
#define PartitionAlloc_TestSupport_DEFINED

namespace skiatest {

// Enable using PartitionAlloc's allocator.
void InitializePartitionAllocForTesting();

// Install handlers causing a crash when a raw_ptr<T> becomes dangling.
void InitializeDanglingPointerChecksForTesting();

}  // namespace skiatest

#endif  // PartitionAlloc_TestSupport_DEFINED

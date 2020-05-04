/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBRDAllocator_DEFINED
#define SkBRDAllocator_DEFINED

#include "client_utils/android/BRDAllocator.h"

// Temporary until Android switches over to BRDAllocator directly.
class SkBRDAllocator : public android::skia::BRDAllocator {};

#endif // SkBRDAllocator_DEFINED

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadID_DEFINED
#define SkThreadID_DEFINED

#include "SkTypes.h"

typedef int64_t SkThreadID;

SkThreadID SkGetThreadID();

const SkThreadID kIllegalThreadID = 0;

#endif  // SkThreadID_DEFINED

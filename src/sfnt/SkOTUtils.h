/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTUtils_DEFINED
#define SkOTUtils_DEFINED

#include "SkOTTableTypes.h"

struct SkOTUtils {
    static uint32_t CalcTableChecksum(SK_OT_ULONG *data, size_t length);
};

#endif

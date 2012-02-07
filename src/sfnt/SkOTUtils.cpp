/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOTTableTypes.h"
#include "SkOTUtils.h"

uint32_t SkOTUtils::CalcTableChecksum(SK_OT_ULONG *data, size_t length) {
    uint32_t sum = 0;
    SK_OT_ULONG *dataEnd = data + ((length + 3) & ~3) / sizeof(SK_OT_ULONG);
    for (; data < dataEnd; ++data) {
        sum += SkEndian_SwapBE32(*data);
    }
    return sum;
}

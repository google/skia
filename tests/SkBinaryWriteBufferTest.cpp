/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"

DEF_TEST(SkBinaryWriteBufferTest, reporter) {
    SkBinaryWriteBuffer buffer;
    buffer.writeTypeface(nullptr);
    REPORTER_ASSERT(reporter, sizeof(int32_t) == buffer.bytesWritten());
}
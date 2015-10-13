/*
 * Copyright 2011 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include <algorithm>

#include "gtest/gtest.h"
#include "sfntly/port/memory_input_stream.h"
#include "sfntly/port/memory_output_stream.h"
#include "sfntly/port/type.h"

namespace {
  const char* kTestData =
"01234567890123456789012345678901234567890123456789"  // 50
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwx"  // 100
"yz";                                                 // 102
  const size_t kTestBufferLen = 102;
}

namespace sfntly {

bool TestMemoryInputStream() {
  ByteVector test_buffer;
  test_buffer.resize(kTestBufferLen);
  std::copy(kTestData, kTestData + kTestBufferLen, test_buffer.begin());

  MemoryInputStream is;
  is.Attach(&(test_buffer[0]), kTestBufferLen);
  EXPECT_EQ(is.Available(), (int32_t)kTestBufferLen);

  // Read one byte
  EXPECT_EQ(is.Read(), '0');  // position 1
  EXPECT_EQ(is.Read(), '1');  // position 2
  EXPECT_EQ(is.Read(), '2');  // position 3

  // Read byte vector
  ByteVector b;
  b.resize(7);
  EXPECT_EQ(is.Read(&b), 7);  // position 10
  EXPECT_EQ(memcmp(&(b[0]), &(test_buffer[0]) + 3, 7), 0);

  b.resize(17);
  EXPECT_EQ(is.Read(&b, 7, 10), 10);  // position 20
  EXPECT_EQ(memcmp(&(b[0]), &(test_buffer[0]) + 3, 17), 0);

  // Test skip
  b.clear();
  b.resize(10);
  EXPECT_EQ(is.Skip(30), 30);  // position 50
  EXPECT_EQ(is.Read(&b), 10);  // position 60
  EXPECT_EQ(memcmp(&(b[0]), &(test_buffer[0]) + 50, 10), 0);
  b.clear();
  b.resize(10);
  EXPECT_EQ(is.Skip(-20), -20);  // position 40
  EXPECT_EQ(is.Read(&b), 10);  // position 50
  EXPECT_EQ(memcmp(&(b[0]), &(test_buffer[0]) + 40, 10), 0);

  EXPECT_EQ(is.Available(), (int32_t)kTestBufferLen - 50);
  EXPECT_EQ(is.Skip(-60), -50);  // Out of bound, position 0
  EXPECT_EQ(is.Skip(kTestBufferLen + 10), (int32_t)kTestBufferLen);

  b.clear();
  b.resize(10);
  is.Unread(&b);
  EXPECT_EQ(memcmp(&(b[0]), &(test_buffer[0]) + kTestBufferLen - 10, 10), 0);

  return true;
}

bool TestMemoryOutputStream() {
  ByteVector test_buffer;
  test_buffer.resize(kTestBufferLen);
  std::copy(kTestData, kTestData + kTestBufferLen, test_buffer.begin());

  MemoryOutputStream os;
  os.Write(&(test_buffer[0]), (int32_t)50, (int32_t)(kTestBufferLen - 50));
  EXPECT_EQ(os.Size(), kTestBufferLen - 50);
  EXPECT_EQ(memcmp(os.Get(), &(test_buffer[0]) + 50, kTestBufferLen - 50), 0);

  return true;
}

}  // namespace sfntly

TEST(MemoryIO, All) {
  ASSERT_TRUE(sfntly::TestMemoryInputStream());
  ASSERT_TRUE(sfntly::TestMemoryOutputStream());
}

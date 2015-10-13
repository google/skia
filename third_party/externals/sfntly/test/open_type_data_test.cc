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

#include "gtest/gtest.h"
#include "sfntly/data/writable_font_data.h"
#include "sfntly/data/memory_byte_array.h"

namespace sfntly {

const byte_t TEST_OTF_DATA[] =
    {0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

bool TestOTFRead() {
  ByteVector bytes;
  for (size_t i = 0; i < sizeof(TEST_OTF_DATA) / sizeof(byte_t); ++i) {
    bytes.push_back(TEST_OTF_DATA[i]);
  }
  ByteArrayPtr array = new MemoryByteArray(&(bytes[0]), bytes.size());
  ReadableFontDataPtr data = new ReadableFontData(array);

  EXPECT_EQ(-1, data->ReadByte(0));
  EXPECT_EQ(0xff, data->ReadUByte(0));
  EXPECT_EQ(0x01, data->ReadByte(1));
  EXPECT_EQ(65281, data->ReadUShort(0));
  EXPECT_EQ(-255, data->ReadShort(0));
  EXPECT_EQ(16711937, data->ReadUInt24(0));
  EXPECT_EQ(4278255873LL, data->ReadULong(0));
  EXPECT_EQ(-16711423, data->ReadLong(0));
  return true;
}

bool TestOTFCopy() {
  ByteVector source_bytes(1024);
  for (size_t i = 0; i < source_bytes.size(); ++i) {
    source_bytes[i] = (byte_t)(i & 0xff);
  }
  ByteArrayPtr source_array = new MemoryByteArray(&(source_bytes[0]), 1024);
  ReadableFontDataPtr source = new ReadableFontData(source_array);

  ByteVector destination_bytes(1024);
  ByteArrayPtr destination_array =
      new MemoryByteArray(&(destination_bytes[0]), 1024);
  WritableFontDataPtr destination = new WritableFontData(destination_array);

  int32_t length = source->CopyTo(destination);
  EXPECT_EQ(1024, length);
  EXPECT_TRUE(std::equal(source_bytes.begin(), source_bytes.end(),
                         destination_bytes.begin()));
  return true;
}

}  // namespace sfntly

TEST(OpenTypeData, All) {
  ASSERT_TRUE(sfntly::TestOTFRead());
  ASSERT_TRUE(sfntly::TestOTFCopy());
}

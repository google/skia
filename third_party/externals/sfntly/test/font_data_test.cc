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

#include <vector>
#include <algorithm>

#include "gtest/gtest.h"
#include "sfntly/port/type.h"
#include "sfntly/data/writable_font_data.h"
#include "sfntly/data/memory_byte_array.h"

namespace sfntly {

const int32_t BYTE_ARRAY_SIZES[] =
  {1, 7, 127, 128, 129, 255, 256, 257, 666, 1023, 0x10000};

// array data for searching
const int32_t LOWER_BYTE_ARRAY_FOR_SEARCHING[] = {2, 4, 7, 13, 127};
const int32_t UPPER_BYTE_ARRAY_FOR_SEARCHING[] = {2, 5, 12, 16, 256};
const int32_t kLowerByteArrayForSearchingLength = 5;
const int32_t kUpperByteArrayForSearchingLength = 5;

// search test result pairs - number to search for; index found at
const int32_t SEARCH_TEST_PAIRS[][2] = {
  {0, -1}, {1, -1}, {2, 0}, {3, -1}, {4, 1}, {5, 1}, {6, -1}, {12, 2},
  {13, 3}, {17, -1}, {126, -1}, {127, 4}, {256, 4}, {257, -1}, {0x1000, -1}
};
const int32_t kSearchTestPairsLength = 15;

// offset and start index data for searching data
// array data size, lower_start_index, lower_offset, upper_start_index,
// upper_offset
const int32_t SEARCH_TEST_OFFSETS[][5] = {
  // lower[], upper[]
  { (kLowerByteArrayForSearchingLength + kUpperByteArrayForSearchingLength)
    * DataSize::kUSHORT,
    0,
    DataSize::kUSHORT,
    kLowerByteArrayForSearchingLength * DataSize::kUSHORT,
    DataSize::kUSHORT },

  // {lower, upper} []
  { (kLowerByteArrayForSearchingLength + kUpperByteArrayForSearchingLength)
    * DataSize::kUSHORT,
    0,
    2 * DataSize::kUSHORT,
    DataSize::kUSHORT,
    2 * DataSize::kUSHORT },

  // upper[], lower[]
  { (kLowerByteArrayForSearchingLength + kUpperByteArrayForSearchingLength)
    * DataSize::kUSHORT,
    kLowerByteArrayForSearchingLength * DataSize::kUSHORT,
    DataSize::kUSHORT,
    0,
    DataSize::kUSHORT },

  // {upper, lower} []
  { (kLowerByteArrayForSearchingLength + kUpperByteArrayForSearchingLength)
    * DataSize::kUSHORT,
    DataSize::kUSHORT,
    2 * DataSize::kUSHORT,
    0,
    2 * DataSize::kUSHORT }
};
const int32_t kSearchTestOffsetLength = 4;

ReadableFontData*
FillTestFontDataWithShortsForSearching(WritableFontData* wfd,
                                       const int32_t* lower_data,
                                       int32_t lower_start_index,
                                       int32_t lower_offset,
                                       const int32_t* upper_data,
                                       int32_t upper_start_index,
                                       int32_t upper_offset) {
  // lower data
  int offset = lower_start_index;
  for (int32_t i = 0; i < kLowerByteArrayForSearchingLength; ++i) {
    wfd->WriteUShort(offset, lower_data[i]);
    offset += lower_offset;
  }

  // upper data
  offset = upper_start_index;
  for (int32_t i = 0; i < kUpperByteArrayForSearchingLength; ++i) {
    wfd->WriteUShort(offset, upper_data[i]);
    offset += upper_offset;
  }

  return wfd;
}

bool TestReadableFontDataSearching() {
  for (int32_t i = 0; i < kSearchTestOffsetLength; ++i) {
    const int32_t* array_setup_offset = SEARCH_TEST_OFFSETS[i];
    WritableFontDataPtr wfd;
    wfd.Attach(WritableFontData::CreateWritableFontData(array_setup_offset[0]));
    FillTestFontDataWithShortsForSearching(wfd,
                                           LOWER_BYTE_ARRAY_FOR_SEARCHING,
                                           array_setup_offset[1],
                                           array_setup_offset[2],
                                           UPPER_BYTE_ARRAY_FOR_SEARCHING,
                                           array_setup_offset[3],
                                           array_setup_offset[4]);
    for (int32_t j = 0; j < kSearchTestPairsLength; ++j) {
      const int32_t* test_case = SEARCH_TEST_PAIRS[j];
      int32_t found = wfd->SearchUShort(array_setup_offset[1],
                                        array_setup_offset[2],
                                        array_setup_offset[3],
                                        array_setup_offset[4],
                                        kLowerByteArrayForSearchingLength,
                                        test_case[0]);
#if defined (SFNTLY_DEBUG_FONTDATA)
      fprintf(stderr, "Searching for %d; Got %d; Expected %d; "
              "[test %d][offset %d]\n",
              test_case[0], found, test_case[1], j, i);
#endif
      EXPECT_EQ(test_case[1], found);
    }
  }
  return true;
}

void FillTestByteArray(ByteArray* ba, int32_t size) {
  for (int32_t i = 0; i < size; ++i) {
    ba->Put(i, (byte_t)(i % 256));
  }
}

void ReadFontDataWithSingleByte(ReadableFontData* rfd, ByteVector* buffer) {
  buffer->resize(rfd->Length());
  for (int32_t index = 0; index < rfd->Length(); ++index) {
    (*buffer)[index] = (byte_t)(rfd->ReadByte(index));
  }
}

void ReadFontDataWithBuffer(ReadableFontData* rfd,
                            int32_t buffer_size,
                            ByteVector* b) {
  ByteVector buffer(buffer_size);
  b->resize(rfd->Length());

  int32_t index = 0;
  while (index < rfd->Length()) {
    int32_t bytes_read = rfd->ReadBytes(index, &(buffer[0]), 0, buffer.size());
    EXPECT_GE(bytes_read, 0);
    std::copy(buffer.begin(), buffer.begin() + bytes_read, b->begin() + index);
    index += bytes_read;
  }
}

void ReadFontDataWithSlidingWindow(ReadableFontData* rfd, int32_t window_size,
                                   ByteVector* b) {
  b->resize(rfd->Length());
  int32_t index = 0;
  while (index < rfd->Length()) {
    int32_t actual_window_size =
        std::min<int32_t>(window_size, b->size() - index);
    int32_t bytes_read =
        rfd->ReadBytes(index, &((*b)[0]), index, actual_window_size);
    EXPECT_GE(bytes_read, 0);
    index += bytes_read;
  }
}

void WriteFontDataWithSingleByte(ReadableFontData* rfd, WritableFontData* wfd) {
  for (int32_t index = 0; index < rfd->Length(); ++index) {
    byte_t b = (byte_t)(rfd->ReadByte(index));
    wfd->WriteByte(index, b);
  }
}

void WriteFontDataWithBuffer(ReadableFontData* rfd,
                             WritableFontData* wfd,
                             int32_t buffer_size) {
  ByteVector buffer(buffer_size);
  int32_t index = 0;
  while (index < rfd->Length()) {
    int32_t bytesRead = rfd->ReadBytes(index, &(buffer[0]), 0, buffer.size());
    wfd->WriteBytes(index, &(buffer[0]), 0, buffer.size());
    index += bytesRead;
  }
}

void WriteFontDataWithSlidingWindow(ReadableFontData* rfd,
                                    WritableFontData* wfd,
                                    int32_t window_size) {
  ByteVector b(rfd->Length());
  int32_t index = 0;
  while (index < rfd->Length()) {
    int32_t sliding_size = std::min<int32_t>(window_size, b.size() - index);
    int32_t bytes_read = rfd->ReadBytes(index, &(b[0]), index, sliding_size);
    wfd->WriteBytes(index, &(b[0]), index, sliding_size);
    index += bytes_read;
  }
}

bool ReadComparison(int32_t offset,
                    int32_t length,
                    ReadableFontData* rfd1,
                    ReadableFontData* rfd2) {
  EXPECT_TRUE(length == rfd2->Length());
  ByteVector b1, b2;
  b1.resize(length);
  b2.resize(length);

  // single byte reads
  ReadFontDataWithSingleByte(rfd1, &b1);
  ReadFontDataWithSingleByte(rfd2, &b2);
  EXPECT_EQ(memcmp(&(b1[offset]), &(b2[0]), length), 0);

  // buffer reads
  int32_t increments = std::max<int32_t>(length / 11, 1);
  for (int32_t buffer_size = 1; buffer_size <= length;
       buffer_size += increments) {
    b1.clear();
    b2.clear();
    b1.resize(length);
    b2.resize(length);
    ReadFontDataWithBuffer(rfd1, buffer_size, &b1);
    ReadFontDataWithBuffer(rfd2, buffer_size, &b2);
    int result = memcmp(&(b1[offset]), &(b2[0]), length);
    EXPECT_EQ(result, 0);
  }

  // sliding window reads
  for (int32_t window_size = 1; window_size <= length;
       window_size += increments) {
    b1.clear();
    b2.clear();
    b1.resize(length);
    b2.resize(length);
    ReadFontDataWithSlidingWindow(rfd1, window_size, &b1);
    ReadFontDataWithSlidingWindow(rfd2, window_size, &b2);
    int result = memcmp(&(b1[offset]), &(b2[0]), length);
    EXPECT_EQ(result, 0);
  }
  return true;
}

void SlicingReadTest(ReadableFontData* rfd) {
  fprintf(stderr, "read - trim = ");
  for (int32_t trim = 0; trim < (rfd->Length() / 2) + 1;
       trim += (rfd->Length() / 21) + 1) {
    fprintf(stderr, "%d ", trim);
    int32_t length = rfd->Length() - 2 * trim;
    ReadableFontDataPtr slice;
    slice.Attach(down_cast<ReadableFontData*>(rfd->Slice(trim, length)));
    EXPECT_TRUE(ReadComparison(trim, length, rfd, slice));
  }
  fprintf(stderr, "\n");
}

void SlicingWriteTest(ReadableFontData* rfd, WritableFontData* wfd) {
  fprintf(stderr, "write - trim = ");
  for (int32_t trim = 0; trim < (rfd->Length() / 2) + 1;
       trim += (rfd->Length() / 21) + 1) {
    fprintf(stderr, "%d ", trim);
    int32_t length = rfd->Length() - 2 * trim;
    WritableFontDataPtr w_slice;
    ReadableFontDataPtr r_slice;

    // single byte writes
    w_slice.Attach(down_cast<WritableFontData*>(wfd->Slice(trim, length)));
    r_slice.Attach(down_cast<ReadableFontData*>(rfd->Slice(trim, length)));
    WriteFontDataWithSingleByte(r_slice, w_slice);
    EXPECT_TRUE(ReadComparison(trim, length, rfd, w_slice));

    // buffer writes
    int32_t increments = std::max<int32_t>(length / 11, 1);
    for (int32_t buffer_size = 1; buffer_size < length;
         buffer_size += increments) {
      w_slice.Attach(down_cast<WritableFontData*>(wfd->Slice(trim, length)));
      r_slice.Attach(down_cast<ReadableFontData*>(rfd->Slice(trim, length)));
      WriteFontDataWithBuffer(r_slice, w_slice, buffer_size);
      EXPECT_TRUE(ReadComparison(trim, length, rfd, w_slice));
    }

    // sliding window writes
    for (int window_size = 1; window_size < length; window_size += increments) {
      w_slice.Attach(down_cast<WritableFontData*>(wfd->Slice(trim, length)));
      r_slice.Attach(down_cast<ReadableFontData*>(rfd->Slice(trim, length)));
      WriteFontDataWithSlidingWindow(r_slice, w_slice, window_size);
      EXPECT_TRUE(ReadComparison(trim, length, rfd, w_slice));
    }
  }
  fprintf(stderr, "\n");
}

bool TestReadableFontData() {
  for (size_t i = 0; i < sizeof(BYTE_ARRAY_SIZES) / sizeof(int32_t); ++i) {
    int32_t size = BYTE_ARRAY_SIZES[i];
    ByteArrayPtr ba = new MemoryByteArray(size);
    FillTestByteArray(ba, size);
    ReadableFontDataPtr rfd = new ReadableFontData(ba);
    SlicingReadTest(rfd);
  }
  return true;
}

bool TestWritableFontData() {
  for (size_t i = 0; i < sizeof(BYTE_ARRAY_SIZES) / sizeof(int32_t); ++i) {
    int32_t size = BYTE_ARRAY_SIZES[i];
    ByteArrayPtr ba = new MemoryByteArray(size);
    FillTestByteArray(ba, size);
    WritableFontDataPtr wfd = new WritableFontData(ba);
    SlicingReadTest(wfd);
    ByteArrayPtr temp = new MemoryByteArray(size);
    WritableFontDataPtr wfd_copy = new WritableFontData(temp);
    SlicingWriteTest(wfd, wfd_copy);
  }
  return true;
}

}  // namespace sfntly

TEST(FontData, ReadableFontDataSearching) {
  ASSERT_TRUE(sfntly::TestReadableFontDataSearching());
}

TEST(FontData, All) {
  ASSERT_TRUE(sfntly::TestReadableFontData());
  ASSERT_TRUE(sfntly::TestWritableFontData());
}

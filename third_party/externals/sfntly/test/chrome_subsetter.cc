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
#include "sample/chromium/font_subsetter.h"
#include "test/test_data.h"
#include "test/test_font_utils.h"

namespace {
  // Use an additional variable to easily change name for testing.
  const char* kInputFileName = sfntly::SAMPLE_TTF_FILE;
  const char* kFontName = "Tuffy";
  const char* kOutputFileName = "tuffy-s.ttf";
  // The subset we want: Hello, world!
  // The array is unsorted to verify that the subsetter gets the glyph id
  // correctly.
  const unsigned int kGlyphIds[] = { 43, 72, 79, 82, 15, 3, 90, 85, 71, 4 };
  const unsigned int kGlyphIdsCount = sizeof(kGlyphIds) / sizeof(unsigned int);
}

// This function is deliberately located at global namespace.
bool TestChromeSubsetter() {
  sfntly::ByteVector input_buffer;
  sfntly::LoadFile(kInputFileName, &input_buffer);
  EXPECT_GT(input_buffer.size(), (size_t)0);

  unsigned char* output_buffer = NULL;
  int output_length =
      SfntlyWrapper::SubsetFont(kFontName,
                                &(input_buffer[0]),
                                input_buffer.size(),
                                kGlyphIds,
                                kGlyphIdsCount,
                                &output_buffer);

  EXPECT_GT(output_length, 0);

  if (output_length > 0) {
    FILE* output_file = NULL;
#if defined WIN32
    fopen_s(&output_file, kOutputFileName, "wb");
#else
    output_file = fopen(kOutputFileName, "wb");
#endif
    EXPECT_TRUE((output_file != NULL));
    if (output_file) {
      int byte_count = fwrite(output_buffer, 1, output_length, output_file);
      EXPECT_EQ(byte_count, output_length);
      fflush(output_file);
      fclose(output_file);
    }

    delete[] output_buffer;
    return true;
  }

  return false;
}

TEST(ChromeSubsetter, All) {
  EXPECT_TRUE(TestChromeSubsetter());
}

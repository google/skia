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

#include "test/test_utils.h"

#include <stdio.h>
#include <unicode/ucnv.h>
#include <unicode/uchar.h>

#include "gtest/gtest.h"
#include "sfntly/font.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/data/growable_memory_byte_array.h"
#include "sfntly/port/file_input_stream.h"

namespace sfntly {
TestUtils::TestUtils() {}

// static
// OutputStream CreateOutputStream(const char *file_path) {
// }

// static
// void TestUtils::CreateNewFile(const char* file_path) {
// }

// static
int32_t TestUtils::EncodeOneChar(UConverter* encoder, int16_t uchar) {
  char* target = new char[ucnv_getMaxCharSize(encoder) * 2];
  char* target_end;
  UChar* source = new UChar[2];
  UChar* source_end;
  source[0] = (UChar)uchar;
  source[1] = 0;
  UErrorCode status = U_ZERO_ERROR;
  source_end = source;
  target_end = target;
  ucnv_fromUnicode(encoder, &target_end, target + 4,
                   (const UChar**)&source_end, source + sizeof(UChar),
                   NULL, TRUE, &status);
  if (!U_SUCCESS(status)) {
    fprintf(stderr, "Error occured in conversion of %d: %s\n",
            uchar, u_errorName(status));
    delete[] source;
    delete[] target;
    return 0;
  }
  int32_t enc_char = 0;
  for (int32_t position = 0; position < target_end - target; ++position) {
    enc_char <<= 8;
    enc_char |= (target[position] & 0xff);
  }
  delete[] source;
  delete[] target;
  return enc_char;
}

// static
UConverter* TestUtils::GetEncoder(const char* charset_name) {
  if (charset_name == NULL || strcmp(charset_name, "") == 0)
    return NULL;
  UErrorCode status = U_ZERO_ERROR;
  UConverter* conv = ucnv_open(charset_name, &status);
  // if (!U_SUCCESS(status))
  //   return NULL;
  return conv;  // returns NULL @ error anyway
}

// Get a file's extension
// static
const char* TestUtils::Extension(const char* file_path) {
  if (!file_path)
    return NULL;
  return strrchr(file_path, EXTENSION_SEPARATOR);
}
}

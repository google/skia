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

#ifndef SFNTLY_CPP_SRC_TEST_TEST_UTILS_H_
#define SFNTLY_CPP_SRC_TEST_TEST_UTILS_H_

// Must include this before ICU to avoid stdint redefinition issue.
#include "sfntly/port/type.h"

#include <unicode/ucnv.h>

#include <string>

#include "sfntly/font.h"
#include "sfntly/data/memory_byte_array.h"

namespace sfntly {
class TestUtils {
  TestUtils();

 public:
  // Compare sections of two byte arrays for equality
  // @param b1 byte array 1
  // @param offset1 offset for comparison in byte array 1
  // @param b2 byte array 2
  // @param offset2 offset for comparison in byte array 2
  // @param length the length of the byte arrays to compare
  // @return true if the array segments are equal; false otherwise
  // TODO(dfilimon): implement
  static bool Equals(ByteArray* b1,
                     int32_t offset1,
                     ByteArray* b2,
                     int32_t offset2);

  // @param offset1 offset to start comparing the first ByteArray from
  // @param ba1 the first ByteArray
  // @param offset2 offset to start comparing the second ByteArray from
  // @param ba2 the second ByteArray
  // @param length the number of bytes to compare
  // @return true if all bytes in the ranges given are equal; false otherwise
  // TODO(dfilimon): implement
  static bool Equals(ByteArray* b1,
                     int32_t offset1,
                     ByteArray* b2,
                     int32_t offset2,
                     int32_t length);

  // TODO(dfilimon): implement FileOutputStream in port/file_output_stream.*
  // static OutputStream createOutputStream(const char* file_path);

  // TODO(dfilimon): adapt & implement
  // static FileChannel createFilechannelForWriting(File file);

  // Creates a new file including deleting an already existing file with the
  // same path and name and creating any needed directories.
  // TODO(dfilimon): implement
  static void CreateNewFile(const char* file_path);

  // Converts an integer into a 4 character string using the ASCII encoding.
  // @param i the value to convert
  // @return the String based on the number
  // TODO(dfilimon): implement
  static void DumpLongAsString(int32_t i, std::string* result);

  // Calculate an OpenType checksum from the array.
  // @param b the array to calculate checksum on
  // @param offset the starting index in the array
  // @param length the number of bytes to check; must be a multiple of 4
  // @return checksum
  // TODO(dfilimon): implement
  static int64_t CheckSum(ByteArray* b, int32_t offset, int32_t length);

  // Encode a single character in UTF-16.
  // We only support the BMP for now
  // @param encoder the encoder to use for the encoding
  // @param uchar the Unicode character to encode
  // @return the encoded character
  static int32_t EncodeOneChar(UConverter* encoder, int16_t uchar);

  // Get an encoder for the charset name.
  // If the name is null or the empty string then just return null.
  // @param charsetName the charset to get an encoder for
  // @return an encoder or null if no encoder available for charset name
  static UConverter* GetEncoder(const char* charsetName);

 private:
  static const char EXTENSION_SEPARATOR = '.';

 public:
  // Get the extension of a file's name.
  // @param file the whose name to process
  // @return string containing the extension or an empty string if
  // there is no extension
  static const char* Extension(const char* file_path);
};
}
#endif  // SFNTLY_CPP_SRC_TEST_TEST_UTILS_H_

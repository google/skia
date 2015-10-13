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

#ifndef SFNTLY_CPP_SRC_TEST_TEST_FONT_UTILS_H_
#define SFNTLY_CPP_SRC_TEST_TEST_FONT_UTILS_H_

#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/port/memory_output_stream.h"

namespace sfntly {

void BuilderForFontFile(const char* font_path, FontFactory* factory,
                        FontBuilderArray* builders);
void SerializeFont(const char* font_path, FontFactory* factory, Font* font);
void LoadFont(const char* font_path, FontFactory* factory, FontArray* fonts);
void LoadFontUsingByteVector(const char* font_path,
                            bool fingerprint,
                            FontArray* fonts);

void LoadFile(const char* input_file_path, ByteVector* input_buffer);
void SerializeToFile(MemoryOutputStream* output_stream, const char* file_path);

void HexDump(const unsigned char* byte_data, size_t length);

}  // namespace sfntly

#endif  // SFNTLY_CPP_SRC_TEST_TEST_FONT_UTILS_H_

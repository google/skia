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

#ifndef SFNTLY_CPP_SRC_TEST_TEST_DATA_H_
#define SFNTLY_CPP_SRC_TEST_TEST_DATA_H_

#include "sfntly/tag.h"

namespace sfntly {

extern const char* SAMPLE_TTF_FILE;
extern const char* SAMPLE_BITMAP_FONT;

extern const size_t SAMPLE_TTF_SIZE;
extern const size_t SAMPLE_TTF_TABLES;
extern const size_t SAMPLE_TTF_KNOWN_TAGS;
extern const size_t SAMPLE_BITMAP_KNOWN_TAGS;
extern const size_t SAMPLE_TTF_FEAT;
extern const size_t SAMPLE_TTF_HEAD;
extern const size_t SAMPLE_TTF_POST;

extern const int32_t TTF_KNOWN_TAGS[];
extern const int32_t BITMAP_KNOWN_TAGS[];
extern const int64_t TTF_CHECKSUM[];
extern const int64_t TTF_OFFSET[];
extern const int32_t TTF_LENGTH[];
extern const unsigned char TTF_FEAT_DATA[];

}  // namespace sfntly

#endif  // SFNTLY_CPP_SRC_TEST_TEST_DATA_H_

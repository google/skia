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

#ifndef TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_UTILS_H_
#define TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_UTILS_H_

#include "sfntly/font.h"
#include "sfntly/font_factory.h"

namespace subtly {
CALLER_ATTACH sfntly::Font* LoadFont(const char* font_path);
CALLER_ATTACH sfntly::Font::Builder* LoadFontBuilder(const char* font_path);

void LoadFonts(const char* font_path, sfntly::FontFactory* factory,
               sfntly::FontArray* fonts);
void LoadFontBuilders(const char* font_path,
                      sfntly::FontFactory* factory,
                      sfntly::FontBuilderArray* builders);

bool SerializeFont(const char* font_path, sfntly::Font* font);
bool SerializeFont(const char* font_path, sfntly::FontFactory* factory,
                   sfntly::Font* font);
}

#endif  // TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_UTILS_H_

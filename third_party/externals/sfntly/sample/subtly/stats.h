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

#ifndef TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_STATS_H_
#define TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_STATS_H_

#include <stdio.h>

#include "sfntly/port/type.h"

namespace sfntly {
class Font;
}

namespace subtly {
using namespace sfntly;

int32_t TotalFontSize(Font* font);

double TableSizePercent(Font* font, int32_t tag);

void PrintComparison(FILE* out, Font* font, Font* new_font);

void PrintStats(FILE* out, Font* font);
}

#endif  // TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_STATS_H_

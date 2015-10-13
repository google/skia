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

#ifndef TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_MERGER_H_
#define TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_MERGER_H_

#include "subtly/character_predicate.h"
#include "subtly/font_info.h"

namespace sfntly {
class Font;
}

namespace subtly {
// Merges the subsets in the font array into a single font.
class Merger : public sfntly::RefCounted<Merger> {
 public:
  explicit Merger(sfntly::FontArray* fonts);
  virtual ~Merger() { }

  // Performs merging returning the subsetted font.
  virtual CALLER_ATTACH sfntly::Font* Merge();

 protected:
  virtual CALLER_ATTACH FontInfo* MergeFontInfos();

 private:
  FontIdMap fonts_;
};
}

#endif  // TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_MERGER_H_

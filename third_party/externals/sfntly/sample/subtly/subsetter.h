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

#ifndef TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_SUBSETTER_H_
#define TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_SUBSETTER_H_

#include "sfntly/font.h"
// Cannot remove this header due to Ptr<T> instantiation issue
#include "subtly/character_predicate.h"

namespace subtly {
// Subsets a given font using a character predicate.
class Subsetter : public sfntly::RefCounted<Subsetter> {
 public:
  Subsetter(sfntly::Font* font, CharacterPredicate* predicate);
  Subsetter(const char* font_path, CharacterPredicate* predicate);
  virtual ~Subsetter() { }

  // Performs subsetting returning the subsetted font.
  virtual CALLER_ATTACH sfntly::Font* Subset();

 private:
  sfntly::Ptr<sfntly::Font> font_;
  sfntly::Ptr<CharacterPredicate> predicate_;
};
}

#endif  // TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_SUBSETTER_H_

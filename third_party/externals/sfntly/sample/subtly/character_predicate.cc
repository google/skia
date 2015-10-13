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

#include "sfntly/port/refcount.h"
#include "subtly/character_predicate.h"

namespace subtly {
using namespace sfntly;

// AcceptRange predicate
AcceptRange::AcceptRange(int32_t start, int32_t end)
    : start_(start),
      end_(end) {
}

AcceptRange::~AcceptRange() {}

bool AcceptRange::operator()(int32_t character) const {
  return start_ <= character && character <= end_;
}

// AcceptSet predicate
AcceptSet::AcceptSet(IntegerSet* characters)
    : characters_(characters) {
}

AcceptSet::~AcceptSet() {
  delete characters_;
}

bool AcceptSet::operator()(int32_t character) const {
  return characters_->find(character) != characters_->end();
}

// AcceptAll predicate
bool AcceptAll::operator()(int32_t character) const {
  UNREFERENCED_PARAMETER(character);
  return true;
}
}

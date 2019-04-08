/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <unicode/brkiter.h>
#include <algorithm>
#include "SkLine.h"
#include "SkParagraphImpl.h"

void SkLine::breakLineByWords(UBreakIteratorType type, std::function<void(SkWord& word)> apply) {

  // TODO: do not create more breakers that we have to
  SkTextBreaker breaker;
  if (!breaker.initialize(fText, type)) {
    return;
  }
  fWords.reset();
  size_t currentPos = 0;
  while (true) {
    auto start = currentPos;
    currentPos = breaker.next(currentPos);
    if (breaker.eof()) {
      break;
    }
    SkSpan<const char> text(fText.begin() + start, currentPos - start);
    fWords.emplace_back(text);
    apply(fWords.back());
  }
}

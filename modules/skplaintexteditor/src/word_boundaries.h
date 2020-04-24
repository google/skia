// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef word_boundaries_DEFINED
#define word_boundaries_DEFINED

#include <cstddef>
#include <vector>

// TODO: Decide if this functionality should be moved into SkShaper as an extra utility.
std::vector<bool> GetUtf8WordBoundaries(const char* begin, std::size_t byteLen, const char* locale);

#endif  // word_boundaries_DEFINED

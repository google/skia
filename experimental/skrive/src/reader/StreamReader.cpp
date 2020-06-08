/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"

namespace skrive::internal {

extern std::unique_ptr<StreamReader> MakeJsonStreamReader(const char[], size_t);

std::unique_ptr<StreamReader> StreamReader::Make(const char data[], size_t len) {
    return MakeJsonStreamReader(data, len);
}

} // namespace skrive::internal

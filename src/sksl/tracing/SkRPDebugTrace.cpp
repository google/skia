/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkRPDebugTrace.h"

#include <sstream>
#include <utility>

namespace SkSL {

void SkRPDebugTrace::writeTrace(SkWStream* o) const {
    // Not yet implemented.
}

void SkRPDebugTrace::dump(SkWStream* o) const {
    // Not yet implemented.
}

void SkRPDebugTrace::setSource(std::string source) {
    fSource.clear();
    std::stringstream stream{std::move(source)};
    while (stream.good()) {
        fSource.push_back({});
        std::getline(stream, fSource.back(), '\n');
    }
}

}  // namespace SkSL

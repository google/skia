/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLNoOpErrorReporter_DEFINED
#define SkSLNoOpErrorReporter_DEFINED

#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"

namespace SkSL {

// We can use a no-op error reporter to silently ignore errors.
class NoOpErrorReporter : public ErrorReporter {
public:
    void handleError(std::string_view, Position) override {}
};

} // namespace SkSL

#endif

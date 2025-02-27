/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// This file is a part of a POC for more automated generation of binding code.
// It can be edited manually (for now).

#include "modules/skunicode/include/SkUnicode.h"

#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(CodeUnitsGen) {
    enum_<SkUnicode::CodeUnitFlags>("CodeUnitFlags")
            .value("NoCodeUnitFlag", SkUnicode::CodeUnitFlags::kNoCodeUnitFlag)
            .value("Whitespace", SkUnicode::CodeUnitFlags::kPartOfWhiteSpaceBreak)
            .value("Space", SkUnicode::CodeUnitFlags::kPartOfIntraWordBreak)
            .value("Control", SkUnicode::CodeUnitFlags::kControl)
            .value("Ideographic", SkUnicode::CodeUnitFlags::kIdeographic);
}

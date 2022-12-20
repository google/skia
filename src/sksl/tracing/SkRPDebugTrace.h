/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKRPDEBUGTRACE
#define SKRPDEBUGTRACE

#include "include/sksl/SkSLDebugTrace.h"
#include "src/sksl/tracing/SkSLDebugInfo.h"

#include <string>
#include <vector>

class SkWStream;

namespace SkSL {

class SkRPDebugTrace : public DebugTrace {
public:
    /** Serializes a debug trace to JSON which can be parsed by our debugger. */
    void writeTrace(SkWStream* o) const override;

    /** Generates a human-readable dump of the debug trace. */
    void dump(SkWStream* o) const override;

    /** Attaches the SkSL source to be debugged. */
    void setSource(std::string source);

    /** A 1:1 mapping of slot numbers to debug information. */
    std::vector<SlotDebugInfo> fSlotInfo;
    std::vector<FunctionDebugInfo> fFuncInfo;

    /** SkVM uniforms live in fSlotInfo; SkRP has dedicated a uniform slot map in fUniformInfo. */
    std::vector<SlotDebugInfo> fUniformInfo;

    /** The SkSL code, split line-by-line. */
    std::vector<std::string> fSource;
};

}  // namespace SkSL

#endif

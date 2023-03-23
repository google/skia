/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKVMDEBUGTRACE
#define SKVMDEBUGTRACE

#include "include/sksl/SkSLDebugTrace.h"

#include "include/core/SkPoint.h"
#include "src/sksl/tracing/SkSLDebugInfo.h"
#include "src/sksl/tracing/SkSLTraceHook.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class SkStream;
class SkWStream;

namespace SkSL {

class SkVMDebugTrace : public DebugTrace {
public:
    /**
     * Sets the device-coordinate pixel to trace. If it's not set, the point at (0, 0) will be used.
     */
    void setTraceCoord(const SkIPoint& coord);

    /** Attaches the SkSL source to be debugged. */
    void setSource(std::string source);

    /** Serializes a debug trace to JSON which can be parsed by our debugger. */
    bool readTrace(SkStream* r);
    void writeTrace(SkWStream* w) const override;

    /** Generates a human-readable dump of the debug trace. */
    void dump(SkWStream* o) const override;

    /** Returns a slot's component as a variable-name suffix, e.g. ".x" or "[2][2]". */
    std::string getSlotComponentSuffix(int slotIndex) const;

    /** Bit-casts a slot's value, then converts to text, e.g. "3.14" or "true" or "12345". */
    std::string getSlotValue(int slotIndex, int32_t value) const;

    /** Bit-casts a value for a given slot into a double, honoring the slot's NumberKind. */
    double interpretValueBits(int slotIndex, int32_t valueBits) const;

    /** Converts a numeric value into text, based on the slot's NumberKind. */
    std::string slotValueToString(int slotIndex, double value) const;

    /** The device-coordinate pixel to trace (controlled by setTraceCoord) */
    SkIPoint fTraceCoord = {};

    /** A 1:1 mapping of slot numbers to debug information. */
    std::vector<SlotDebugInfo> fSlotInfo;
    std::vector<FunctionDebugInfo> fFuncInfo;

    /** The SkSL debug trace. */
    std::vector<TraceInfo> fTraceInfo;

    /** The SkSL code, split line-by-line. */
    std::vector<std::string> fSource;

    /**
     * A trace hook which populates fTraceInfo during SkVM program evaluation. This will be created
     * automatically by the SkSLVMCodeGenerator.
     */
    std::unique_ptr<SkSL::TraceHook> fTraceHook;
};

}  // namespace SkSL

#endif

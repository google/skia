/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSLDEBUGTRACEPRIV
#define SKSLDEBUGTRACEPRIV

#include "include/core/SkPoint.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/tracing/SkSLTraceHook.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class SkWStream;

namespace SkSL {

struct TraceInfo {
    enum class Op {
        kLine,  /** data: line number, (unused) */
        kVar,   /** data: slot, value */
        kEnter, /** data: function index, (unused) */
        kExit,  /** data: function index, (unused) */
        kScope, /** data: scope delta, (unused) */
    };
    Op op;
    int32_t data[2];
};

struct SlotDebugInfo {
    /** The full name of this variable (without component), e.g. `myArray[3].myStruct.myVector` */
    std::string             name;
    /** The dimensions of this variable: 1x1 is a scalar, Nx1 is a vector, NxM is a matrix. */
    uint8_t                 columns = 1, rows = 1;
    /** Which component of the variable is this slot? (e.g. `vec4.z` is component 2) */
    uint8_t                 componentIndex = 0;
    /** Complex types (arrays/structs) can be tracked as a "group" of adjacent slots. */
    int                     groupIndex = 0;
    /** What kind of numbers belong in this slot? */
    SkSL::Type::NumberKind  numberKind = SkSL::Type::NumberKind::kNonnumeric;
    /** Where is this variable located in the program? */
    int                     line = 0;
    Position                pos = {};
    /** If this slot holds a function's return value, contains 1; if not, -1. */
    int                     fnReturnValue = -1;
};

struct FunctionDebugInfo {
    /** Full function declaration: `float myFunction(half4 color)`) */
    std::string             name;
};

class DebugTracePriv : public DebugTrace {
public:
    /**
     * Sets the device-coordinate pixel to trace. If it's not set, the point at (0, 0) will be used.
     */
    void setTraceCoord(const SkIPoint& coord);

    /** Attaches the SkSL source to be debugged. */
    void setSource(const std::string& source);

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

    /** SkRP stores uniform slot info in fUniformInfo. (In SkVM, they were mixed into fSlotInfo.) */
    std::vector<SlotDebugInfo> fUniformInfo;

    /** A 1:1 mapping of slot numbers to debug information. */
    std::vector<SlotDebugInfo> fSlotInfo;
    std::vector<FunctionDebugInfo> fFuncInfo;

    /** The SkSL debug trace. */
    std::vector<TraceInfo> fTraceInfo;

    /** The SkSL code, split line-by-line. */
    std::vector<std::string> fSource;

    /**
     * A trace hook which populates fTraceInfo during shader evaluation. This will be created
     * automatically during code generation.
     */
    std::unique_ptr<SkSL::TraceHook> fTraceHook;
};

}  // namespace SkSL

#endif

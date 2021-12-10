/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKVMDEBUGTRACE
#define SKVMDEBUGTRACE

#include "include/core/SkPoint.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "src/core/SkVM.h"
#include "src/sksl/ir/SkSLType.h"

#include <string>
#include <vector>

class SkStream;
class SkWStream;

namespace SkSL {

struct SkVMSlotInfo {
    /** The full name of this variable (without component): (e.g. `myArray[3].myStruct.myVector`) */
    std::string             name;
    /** The dimensions of this variable: 1x1 is a scalar, Nx1 is a vector, NxM is a matrix. */
    uint8_t                 columns = 1, rows = 1;
    /** Which component of the variable is this slot? (e.g. `vec4.z` is component 2) */
    uint8_t                 componentIndex = 0;
    /** What kind of numbers belong in this slot? */
    SkSL::Type::NumberKind  numberKind = SkSL::Type::NumberKind::kNonnumeric;
    /** Where is this variable located in the program? */
    int                     line;
    /** If this slot holds a function's return value, its FunctionInfo index; if not, -1. */
    int                     fnReturnValue;
};

struct SkVMFunctionInfo {
    /** Full function declaration: `float myFunction(half4 color)`) */
    std::string             name;
};

struct SkVMTraceInfo {
    enum class Op {
        kLine,  /** data: line number, (unused) */
        kVar,   /** data: slot, value */
        kEnter, /** data: function index, (unused) */
        kExit,  /** data: function index, (unused) */
    };
    Op op;
    int32_t data[2];
};

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

    /** The device-coordinate pixel to trace (controlled by setTraceCoord) */
    SkIPoint fTraceCoord = {};

    /** A 1:1 mapping of slot numbers to debug information. */
    std::vector<SkVMSlotInfo> fSlotInfo;
    std::vector<SkVMFunctionInfo> fFuncInfo;

    /** The SkSL debug trace. */
    std::vector<SkVMTraceInfo> fTraceInfo;

    /** The SkSL code, split line-by-line. */
    std::vector<std::string> fSource;

    /**
     * A trace hook which populates fTraceInfo during SkVM program evaluation. This will be created
     * automatically by the SkSLVMCodeGenerator.
     */
    std::unique_ptr<skvm::TraceHook> fTraceHook;
};

}  // namespace SkSL

#endif

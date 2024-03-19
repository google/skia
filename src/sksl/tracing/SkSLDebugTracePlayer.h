/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSLDebugTracePlayer_DEFINED
#define SkSLDebugTracePlayer_DEFINED

#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/utils/SkBitSet.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SkSL {

/**
 * Plays back a SkSL debug trace, allowing its contents to be viewed like a traditional debugger.
 */
class SkSLDebugTracePlayer {
public:
    /** Resets playback to the start of the trace. Breakpoints are not cleared. */
    void reset(sk_sp<DebugTracePriv> trace);

    /** Advances the simulation to the next Line op. */
    void step();

    /**
     * Advances the simulation to the next Line op, skipping past matched Enter/Exit pairs.
     * Breakpoints will also stop the simulation even if we haven't reached an Exit.
     */
    void stepOver();

    /**
     * Advances the simulation until we exit from the current stack frame.
     * Breakpoints will also stop the simulation even if we haven't left the stack frame.
     */
    void stepOut();

    /** Advances the simulation until we hit a breakpoint, or the trace completes. */
    void run();

    /** Breakpoints will force the simulation to stop whenever a desired line is reached. */
    void setBreakpoints(std::unordered_set<int> breakpointLines);
    void addBreakpoint(int line);
    void removeBreakpoint(int line);
    using BreakpointSet = std::unordered_set<int>;
    const BreakpointSet& getBreakpoints() { return fBreakpointLines; }

    /** Returns true if we have reached the end of the trace. */
    bool traceHasCompleted() const;

    /** Returns true if there is a breakpoint set at the current line. */
    bool atBreakpoint() const;

    /** Retrieves the cursor position. */
    size_t cursor() { return fCursor; }

    /** Retrieves the current line. */
    int32_t getCurrentLine() const;

    /** Retrieves the current line for a given stack frame. */
    int32_t getCurrentLineInStackFrame(int stackFrameIndex) const;

    /** Returns the call stack as an array of FunctionInfo indices. */
    std::vector<int> getCallStack() const;

    /** Returns the size of the call stack. */
    int getStackDepth() const;

    /**
     * Returns every line number reached inside this debug trace, along with the remaining number of
     * times that this trace will reach it. e.g. {100, 2} means line 100 will be reached twice.
     */
    using LineNumberMap = std::unordered_map<int, int>;
    const LineNumberMap& getLineNumbersReached() const { return fLineNumbers; }

    /** Returns variables from a stack frame, or from global scope. */
    struct VariableData {
        int     fSlotIndex;
        bool    fDirty;  // has this slot been written-to since the last step call?
        double  fValue;  // value in slot (with type-conversion applied)
    };
    std::vector<VariableData> getLocalVariables(int stackFrameIndex) const;
    std::vector<VariableData> getGlobalVariables() const;

private:
    /**
     * Executes the trace op at the passed-in cursor position. Returns true if we've reached a line
     * or exit trace op, which indicate a stopping point.
     */
    bool execute(size_t position);

    /**
     * Cleans up temporary state between steps, such as the dirty mask and function return values.
     */
    void tidyState();

    /** Updates fWriteTime for the entire variable at a given slot. */
    void updateVariableWriteTime(int slotIdx, size_t writeTime);

    /** Returns a vector of the indices and values of each slot that is enabled in `bits`. */
    std::vector<VariableData> getVariablesForDisplayMask(const SkBitSet& bits) const;

    struct StackFrame {
        int32_t   fFunction;     // from fFuncInfo
        int32_t   fLine;         // our current line number within the function
        SkBitSet  fDisplayMask;  // the variable slots which have been touched in this function
    };
    struct Slot {
        int32_t   fValue;        // values in each slot
        int       fScope;        // the scope value of each slot
        size_t    fWriteTime;    // when was the variable in this slot most recently written?
                                 // (by cursor position)
    };
    sk_sp<DebugTracePriv>      fDebugTrace;
    size_t                     fCursor = 0;      // position of the read head
    int                        fScope = 0;       // the current scope depth (as tracked by
                                                 // trace_scope)
    std::vector<Slot>          fSlots;           // the array of all slots
    std::vector<StackFrame>    fStack;           // the execution stack
    std::optional<SkBitSet>    fDirtyMask;       // variable slots touched during the most-recently
                                                 // executed step
    std::optional<SkBitSet>    fReturnValues;    // variable slots containing return values
    LineNumberMap              fLineNumbers;     // holds [line number, the remaining number of
                                                 // times to reach this line during the trace]
    BreakpointSet              fBreakpointLines; // all breakpoints set by setBreakpointLines
};

}  // namespace SkSL

#endif  // SkSLDebugTracePlayer_DEFINED

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkVMDebugTrace.h"
#include "src/utils/SkBitSet.h"

namespace SkSL {

/**
 * Plays back a SkVM debug trace, allowing its contents to be viewed like a traditional debugger.
 */
class SkVMDebugTracePlayer {
public:
    /** Resets playback. */
    void reset(sk_sp<SkVMDebugTrace> trace);

    /** Advances the simulation to the next Line op. */
    void step();

    /** Advances the simulation to the next Line op, skipping past matched Enter/Exit pairs. */
    void stepOver();

    /** Returns true if we have reached the end of the trace. */
    bool traceHasCompleted() const;

    /** Retrieves the cursor position. */
    size_t cursor() { return fCursor; }

    /** Retrieves the current line. */
    int32_t getCurrentLine() const;

    /** Returns the call stack as an array of FunctionInfo indices. */
    std::vector<int> getCallStack() const;

    /** Returns the size of the call stack. */
    int getStackDepth() const;

    /** Returns variables from a stack frame, or from global scope. */
    struct VariableData {
        int      fSlotIndex;
        int32_t  fValue;  // caller must type-pun bits to float/bool based on slot type
    };
    std::vector<VariableData> getLocalVariables(int stackFrameIndex) const;
    std::vector<VariableData> getGlobalVariables() const;

private:
    /**
     * Executes the trace op at the passed-in cursor position. Returns true if we've reached a line
     * or exit trace op, which indicate a stopping point.
     */
    bool execute(size_t position);

    /** Returns a vector of the indices and values of each slot that is enabled in `bits`. */
    std::vector<VariableData> getVariablesForDisplayMask(const SkBitSet& bits) const;

    struct StackFrame {
        int32_t   fFunction;     // from fFuncInfo
        int32_t   fLine;         // our current line number within the function
        SkBitSet  fDisplayMask;  // the variable slots which have been touched in this function
    };
    sk_sp<SkVMDebugTrace>       fDebugTrace;
    size_t                      fCursor = 0;   // position of the read head
    std::vector<int32_t>        fSlots;        // values in each slot
    std::vector<StackFrame>     fStack;        // the execution stack
};

}  // namespace SkSL

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkSLDebugTracePlayer.h"

#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include <limits.h>
#include <algorithm>
#include <utility>

namespace SkSL {

void SkSLDebugTracePlayer::reset(sk_sp<DebugTracePriv> debugTrace) {
    size_t nslots = debugTrace ? debugTrace->fSlotInfo.size() : 0;
    fDebugTrace = debugTrace;
    fCursor = 0;
    fScope = 0;
    fSlots.clear();
    fSlots.resize(nslots, {/*fValue=*/0,
                           /*fScope=*/INT_MAX,
                           /*fWriteTime=*/0});
    fStack.clear();
    fStack.push_back({/*fFunction=*/-1,
                      /*fLine=*/-1,
                      /*fDisplayMask=*/SkBitSet(nslots)});
    fDirtyMask.emplace(nslots);
    fReturnValues.emplace(nslots);

    if (fDebugTrace) {
        for (size_t slotIdx = 0; slotIdx < nslots; ++slotIdx) {
            if (fDebugTrace->fSlotInfo[slotIdx].fnReturnValue >= 0) {
                fReturnValues->set(slotIdx);
            }
        }

        for (const TraceInfo& trace : fDebugTrace->fTraceInfo) {
            if (trace.op == TraceInfo::Op::kLine) {
                fLineNumbers[trace.data[0]] += 1;
            }
        }
    }
}

void SkSLDebugTracePlayer::step() {
    this->tidyState();
    while (!this->traceHasCompleted()) {
        if (this->execute(fCursor++)) {
            break;
        }
    }
}

void SkSLDebugTracePlayer::stepOver() {
    this->tidyState();
    size_t initialStackDepth = fStack.size();
    while (!this->traceHasCompleted()) {
        bool canEscapeFromThisStackDepth = (fStack.size() <= initialStackDepth);
        if (this->execute(fCursor++)) {
            if (canEscapeFromThisStackDepth || this->atBreakpoint()) {
                break;
            }
        }
    }
}

void SkSLDebugTracePlayer::stepOut() {
    this->tidyState();
    size_t initialStackDepth = fStack.size();
    while (!this->traceHasCompleted()) {
        if (this->execute(fCursor++)) {
            bool hasEscapedFromInitialStackDepth = (fStack.size() < initialStackDepth);
            if (hasEscapedFromInitialStackDepth || this->atBreakpoint()) {
                break;
            }
        }
    }
}

void SkSLDebugTracePlayer::run() {
    this->tidyState();
    while (!this->traceHasCompleted()) {
        if (this->execute(fCursor++)) {
            if (this->atBreakpoint()) {
                break;
            }
        }
    }
}

void SkSLDebugTracePlayer::tidyState() {
    fDirtyMask->reset();

    // Conceptually this is `fStack.back().fDisplayMask &= ~fReturnValues`, but SkBitSet doesn't
    // support masking one set of bits against another.
    fReturnValues->forEachSetIndex([&](int slot) {
        fStack.back().fDisplayMask.reset(slot);
    });
}

bool SkSLDebugTracePlayer::traceHasCompleted() const {
    return !fDebugTrace || fCursor >= fDebugTrace->fTraceInfo.size();
}

int32_t SkSLDebugTracePlayer::getCurrentLine() const {
    SkASSERT(!fStack.empty());
    return fStack.back().fLine;
}

int32_t SkSLDebugTracePlayer::getCurrentLineInStackFrame(int stackFrameIndex) const {
    // The first entry on the stack is the "global" frame before we enter main, so offset our index
    // by one to account for it.
    ++stackFrameIndex;
    SkASSERT(stackFrameIndex > 0);
    SkASSERT((size_t)stackFrameIndex < fStack.size());
    return fStack[stackFrameIndex].fLine;
}

bool SkSLDebugTracePlayer::atBreakpoint() const {
    return fBreakpointLines.count(this->getCurrentLine());
}

void SkSLDebugTracePlayer::setBreakpoints(std::unordered_set<int> breakpointLines) {
    fBreakpointLines = std::move(breakpointLines);
}

void SkSLDebugTracePlayer::addBreakpoint(int line) {
    fBreakpointLines.insert(line);
}

void SkSLDebugTracePlayer::removeBreakpoint(int line) {
    fBreakpointLines.erase(line);
}

std::vector<int> SkSLDebugTracePlayer::getCallStack() const {
    SkASSERT(!fStack.empty());
    std::vector<int> funcs;
    funcs.reserve(fStack.size() - 1);
    for (size_t index = 1; index < fStack.size(); ++index) {
        funcs.push_back(fStack[index].fFunction);
    }
    return funcs;
}

int SkSLDebugTracePlayer::getStackDepth() const {
    SkASSERT(!fStack.empty());
    return fStack.size() - 1;
}

std::vector<SkSLDebugTracePlayer::VariableData> SkSLDebugTracePlayer::getVariablesForDisplayMask(
        const SkBitSet& displayMask) const {
    SkASSERT(displayMask.size() == fSlots.size());

    std::vector<VariableData> vars;
    displayMask.forEachSetIndex([&](int slot) {
        double typedValue = fDebugTrace->interpretValueBits(slot, fSlots[slot].fValue);
        vars.push_back({slot, fDirtyMask->test(slot), typedValue});
    });
    // Order the variable list so that the most recently-written variables are shown at the top.
    std::stable_sort(vars.begin(), vars.end(), [&](const VariableData& a, const VariableData& b) {
        return fSlots[a.fSlotIndex].fWriteTime > fSlots[b.fSlotIndex].fWriteTime;
    });
    return vars;
}

std::vector<SkSLDebugTracePlayer::VariableData> SkSLDebugTracePlayer::getLocalVariables(
        int stackFrameIndex) const {
    // The first entry on the stack is the "global" frame before we enter main, so offset our index
    // by one to account for it.
    ++stackFrameIndex;
    if (stackFrameIndex <= 0 || (size_t)stackFrameIndex >= fStack.size()) {
        SkDEBUGFAILF("stack frame %d doesn't exist", stackFrameIndex - 1);
        return {};
    }
    return this->getVariablesForDisplayMask(fStack[stackFrameIndex].fDisplayMask);
}

std::vector<SkSLDebugTracePlayer::VariableData> SkSLDebugTracePlayer::getGlobalVariables() const {
    if (fStack.empty()) {
        return {};
    }
    return this->getVariablesForDisplayMask(fStack.front().fDisplayMask);
}

void SkSLDebugTracePlayer::updateVariableWriteTime(int slotIdx, size_t cursor) {
    // The slotIdx could point to any slot within a variable.
    // We want to update the write time on EVERY slot associated with this variable.
    // The SlotInfo's groupIndex gives us enough information to find the affected range.
    const SkSL::SlotDebugInfo& changedSlot = fDebugTrace->fSlotInfo[slotIdx];
    slotIdx -= changedSlot.groupIndex;
    SkASSERT(slotIdx >= 0);
    SkASSERT(slotIdx < (int)fDebugTrace->fSlotInfo.size());

    for (;;) {
        fSlots[slotIdx++].fWriteTime = cursor;

        // Stop if we've reached the final slot.
        if (slotIdx >= (int)fDebugTrace->fSlotInfo.size()) {
            break;
        }
        // Each separate variable-group starts with a groupIndex of 0; stop when we detect this.
        if (fDebugTrace->fSlotInfo[slotIdx].groupIndex == 0) {
            break;
        }
    }
}

bool SkSLDebugTracePlayer::execute(size_t position) {
    if (position >= fDebugTrace->fTraceInfo.size()) {
        SkDEBUGFAILF("position %zu out of range", position);
        return true;
    }

    const TraceInfo& trace = fDebugTrace->fTraceInfo[position];
    switch (trace.op) {
        case TraceInfo::Op::kLine: { // data: line number, (unused)
            SkASSERT(!fStack.empty());
            int lineNumber = trace.data[0];
            SkASSERT(lineNumber >= 0);
            SkASSERT((size_t)lineNumber < fDebugTrace->fSource.size());
            SkASSERT(fLineNumbers[lineNumber] > 0);
            fStack.back().fLine = lineNumber;
            fLineNumbers[lineNumber] -= 1;
            return true;
        }
        case TraceInfo::Op::kVar: {  // data: slot, value
            int slotIdx = trace.data[0];
            int value = trace.data[1];
            SkASSERT(slotIdx >= 0);
            SkASSERT((size_t)slotIdx < fDebugTrace->fSlotInfo.size());
            fSlots[slotIdx].fValue = value;
            fSlots[slotIdx].fScope = std::min<>(fSlots[slotIdx].fScope, fScope);
            this->updateVariableWriteTime(slotIdx, position);
            if (fDebugTrace->fSlotInfo[slotIdx].fnReturnValue < 0) {
                // Normal variables are associated with the current function.
                SkASSERT(!fStack.empty());
                fStack.rbegin()[0].fDisplayMask.set(slotIdx);
            } else {
                // Return values are associated with the parent function (since the current function
                // is exiting and we won't see them there).
                SkASSERT(fStack.size() > 1);
                fStack.rbegin()[1].fDisplayMask.set(slotIdx);
            }
            fDirtyMask->set(slotIdx);
            break;
        }
        case TraceInfo::Op::kEnter: { // data: function index, (unused)
            int fnIdx = trace.data[0];
            SkASSERT(fnIdx >= 0);
            SkASSERT((size_t)fnIdx < fDebugTrace->fFuncInfo.size());
            fStack.push_back({/*fFunction=*/fnIdx,
                              /*fLine=*/-1,
                              /*fDisplayMask=*/SkBitSet(fDebugTrace->fSlotInfo.size())});
            break;
        }
        case TraceInfo::Op::kExit: { // data: function index, (unused)
            SkASSERT(!fStack.empty());
            SkASSERT(fStack.back().fFunction == trace.data[0]);
            fStack.pop_back();
            return true;
        }
        case TraceInfo::Op::kScope: { // data: scope delta, (unused)
            SkASSERT(!fStack.empty());
            fScope += trace.data[0];
            if (trace.data[0] < 0) {
                // If the scope is being reduced, discard variables that are now out of scope.
                for (size_t slotIdx = 0; slotIdx < fSlots.size(); ++slotIdx) {
                    if (fScope < fSlots[slotIdx].fScope) {
                        fSlots[slotIdx].fScope = INT_MAX;
                        fStack.back().fDisplayMask.reset(slotIdx);
                    }
                }
            }
            return false;
        }
    }

    return false;
}

}  // namespace SkSL

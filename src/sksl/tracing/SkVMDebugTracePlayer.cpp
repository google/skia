/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkVMDebugTracePlayer.h"

namespace SkSL {

void SkVMDebugTracePlayer::reset(sk_sp<SkVMDebugTrace> debugTrace) {
    size_t nslots = debugTrace ? debugTrace->fSlotInfo.size() : 0;
    fDebugTrace = debugTrace;
    fCursor = 0;
    fSlots.clear();
    fSlots.resize(nslots);
    fStack.clear();
    fStack.push_back({/*fFunction=*/-1,
                      /*fLine=*/-1,
                      /*fDisplayMask=*/SkBitSet(nslots)});
}

void SkVMDebugTracePlayer::step() {
    while (!this->traceHasCompleted()) {
        if (this->execute(fCursor++)) {
            break;
        }
    }
}

void SkVMDebugTracePlayer::stepOver() {
    size_t initialStackDepth = fStack.size();
    while (!this->traceHasCompleted()) {
        bool canEscapeFromThisStackDepth = (fStack.size() <= initialStackDepth);
        if (this->execute(fCursor++) && canEscapeFromThisStackDepth) {
            break;
        }
    }
}

bool SkVMDebugTracePlayer::traceHasCompleted() const {
    return !fDebugTrace || fCursor >= fDebugTrace->fTraceInfo.size();
}

int32_t SkVMDebugTracePlayer::getCurrentLine() const {
    SkASSERT(!fStack.empty());
    return fStack.back().fLine;
}

std::vector<int> SkVMDebugTracePlayer::getCallStack() const {
    SkASSERT(!fStack.empty());
    std::vector<int> funcs;
    funcs.reserve(fStack.size() - 1);
    for (size_t index = 1; index < fStack.size(); ++index) {
        funcs.push_back(fStack[index].fFunction);
    }
    return funcs;
}

int SkVMDebugTracePlayer::getStackDepth() const {
    SkASSERT(!fStack.empty());
    return fStack.size() - 1;
}

std::vector<SkVMDebugTracePlayer::VariableData> SkVMDebugTracePlayer::getVariablesForDisplayMask(
        const SkBitSet& bits) const {
    SkASSERT(bits.size() == fSlots.size());

    std::vector<VariableData> vars;
    bits.forEachSetIndex([&](int slot) {
        vars.push_back({slot, fSlots[slot]});
    });
    return vars;
}

std::vector<SkVMDebugTracePlayer::VariableData> SkVMDebugTracePlayer::getLocalVariables(
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

std::vector<SkVMDebugTracePlayer::VariableData> SkVMDebugTracePlayer::getGlobalVariables() const {
    if (fStack.empty()) {
        return {};
    }
    return this->getVariablesForDisplayMask(fStack.front().fDisplayMask);
}

bool SkVMDebugTracePlayer::execute(size_t position) {
    if (position >= fDebugTrace->fTraceInfo.size()) {
        SkDEBUGFAILF("position %zu out of range", position);
        return true;
    }

    const SkVMTraceInfo& trace = fDebugTrace->fTraceInfo[position];
    switch (trace.op) {
        case SkVMTraceInfo::Op::kLine: { // data: line number, (unused)
            SkASSERT(!fStack.empty());
            int lineNumber = trace.data[0];
            SkASSERT(lineNumber >= 0);
            SkASSERT((size_t)lineNumber < fDebugTrace->fSource.size());
            fStack.back().fLine = lineNumber;
            return true;
        }
        case SkVMTraceInfo::Op::kVar: {  // data: slot, value
            int slot = trace.data[0];
            int value = trace.data[1];
            SkASSERT(slot >= 0);
            SkASSERT((size_t)slot < fDebugTrace->fSlotInfo.size());
            fSlots[slot] = value;
            if (fDebugTrace->fSlotInfo[slot].fnReturnValue < 0) {
                // Normal variables are associated with the current function.
                SkASSERT(fStack.size() > 0);
                fStack.rbegin()[0].fDisplayMask.set(slot);
            } else {
                // Return values are associated with the parent function (since the current function
                // is exiting and we won't see them there).
                SkASSERT(fStack.size() > 1);
                fStack.rbegin()[1].fDisplayMask.set(slot);
            }
            break;
        }
        case SkVMTraceInfo::Op::kEnter: { // data: function index, (unused)
            int fnIdx = trace.data[0];
            SkASSERT(fnIdx >= 0);
            SkASSERT((size_t)fnIdx < fDebugTrace->fFuncInfo.size());
            fStack.push_back({/*fFunction=*/fnIdx,
                              /*fLine=*/-1,
                              /*fDisplayMask=*/SkBitSet(fDebugTrace->fSlotInfo.size())});
            break;
        }
        case SkVMTraceInfo::Op::kExit: { // data: function index, (unused)
            SkASSERT(!fStack.empty());
            SkASSERT(fStack.back().fFunction == trace.data[0]);
            fStack.pop_back();
            return true;
        }
    }

    return false;
}

}  // namespace SkSL

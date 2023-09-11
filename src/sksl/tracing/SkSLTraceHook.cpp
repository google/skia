/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "src/sksl/tracing/SkSLTraceHook.h"

namespace SkSL {

std::unique_ptr<Tracer> Tracer::Make(std::vector<TraceInfo>* traceInfo) {
    auto hook = std::make_unique<Tracer>();
    hook->fTraceInfo = traceInfo;
    return hook;
}

void Tracer::line(int lineNum) {
    fTraceInfo->push_back({TraceInfo::Op::kLine, /*data=*/{lineNum, 0}});
}
void Tracer::var(int slot, int32_t val) {
    fTraceInfo->push_back({TraceInfo::Op::kVar, /*data=*/{slot, val}});
}
void Tracer::enter(int fnIdx) {
    fTraceInfo->push_back({TraceInfo::Op::kEnter, /*data=*/{fnIdx, 0}});
}
void Tracer::exit(int fnIdx) {
    fTraceInfo->push_back({TraceInfo::Op::kExit, /*data=*/{fnIdx, 0}});
}
void Tracer::scope(int delta) {
    fTraceInfo->push_back({TraceInfo::Op::kScope, /*data=*/{delta, 0}});
}

}  // namespace SkSL

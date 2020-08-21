/*
 * Copyright 2020 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTraceMemoryDump.h"
#include "include/xamarin/SkManagedTraceMemoryDump.h"

SkManagedTraceMemoryDump::Procs SkManagedTraceMemoryDump::fProcs;

void SkManagedTraceMemoryDump::setProcs(SkManagedTraceMemoryDump::Procs procs) {
    fProcs = procs;
}

SkManagedTraceMemoryDump::SkManagedTraceMemoryDump(SkTraceMemoryDump::LevelOfDetail level, bool dumpWrapped, void* context) {
    fLevel = level;
    fDumpWrapped = dumpWrapped;
    fContext = context;
}

SkManagedTraceMemoryDump::~SkManagedTraceMemoryDump() {
}

void SkManagedTraceMemoryDump::dumpNumericValue(const char* dumpName, const char* valueName, const char* units, uint64_t value) {
    if (!fProcs.fDumpNumericValue) return;
    fProcs.fDumpNumericValue(this, fContext, dumpName, valueName, units, value);
}

void SkManagedTraceMemoryDump::dumpStringValue(const char* dumpName, const char* valueName, const char* value) {
    if (!fProcs.fDumpStringValue) return;
    fProcs.fDumpStringValue(this, fContext, dumpName, valueName, value);
}

void SkManagedTraceMemoryDump::setMemoryBacking(const char* dumpName, const char* backingType, const char* backingObjectId) {
    // no op
}

void SkManagedTraceMemoryDump::setDiscardableMemoryBacking(const char* dumpName, const SkDiscardableMemory& discardableMemoryObject)  {
    // no op
}

SkTraceMemoryDump::LevelOfDetail SkManagedTraceMemoryDump::getRequestedDetails() const {
    return fLevel;
}

bool SkManagedTraceMemoryDump::shouldDumpWrappedObjects() const {
    return fDumpWrapped;
}

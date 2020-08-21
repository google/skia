/*
 * Copyright 2020 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/xamarin/SkManagedTraceMemoryDump.h"

#include "include/xamarin/sk_managedtracememorydump.h"
#include "src/c/sk_types_priv.h"

static inline SkManagedTraceMemoryDump* AsManagedTraceMemoryDump(sk_managedtracememorydump_t* d) {
    return reinterpret_cast<SkManagedTraceMemoryDump*>(d);
}
static inline sk_managedtracememorydump_t* ToManagedTraceMemoryDump(SkManagedTraceMemoryDump* d) {
    return reinterpret_cast<sk_managedtracememorydump_t*>(d);
}

static sk_managedtracememorydump_procs_t gProcs;

void dDumpNumericValue(SkManagedTraceMemoryDump* d, void* context, const char* dumpName, const char* valueName, const char* units, uint64_t value) {
    if (!gProcs.fDumpNumericValue) return;
    gProcs.fDumpNumericValue(ToManagedTraceMemoryDump(d), context, dumpName, valueName, units, value);
}

void dDumpStringValue(SkManagedTraceMemoryDump* d, void* context, const char* dumpName, const char* valueName, const char* value) {
    if (!gProcs.fDumpStringValue) return;
    gProcs.fDumpStringValue(ToManagedTraceMemoryDump(d), context, dumpName, valueName, value);
}

sk_managedtracememorydump_t* sk_managedtracememorydump_new(bool detailed, bool dumpWrapped, void* context) {
    SkTraceMemoryDump::LevelOfDetail level = detailed
        ? SkTraceMemoryDump::LevelOfDetail::kObjectsBreakdowns_LevelOfDetail
        : SkTraceMemoryDump::LevelOfDetail::kLight_LevelOfDetail;
    return ToManagedTraceMemoryDump(new SkManagedTraceMemoryDump(level, dumpWrapped, context));
}

void sk_managedtracememorydump_delete(sk_managedtracememorydump_t* d) {
    delete AsManagedTraceMemoryDump(d);
}

void sk_managedtracememorydump_set_procs(sk_managedtracememorydump_procs_t procs) {
    gProcs = procs;

    SkManagedTraceMemoryDump::Procs p;
    p.fDumpNumericValue = dDumpNumericValue;
    p.fDumpStringValue = dDumpStringValue;

    SkManagedTraceMemoryDump::setProcs(p);
}

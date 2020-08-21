/*
 * Copyright 2020 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkManagedTraceMemoryDump_h
#define SkManagedTraceMemoryDump_h

#include "include/core/SkTypes.h"
#include "include/core/SkTraceMemoryDump.h"

class SkCanvas;
class SkPicture;
struct SkRect;


class SK_API SkManagedTraceMemoryDump : public SkTraceMemoryDump {
public:
    SkManagedTraceMemoryDump(SkTraceMemoryDump::LevelOfDetail level, bool dumpWrapped, void* context);
    ~SkManagedTraceMemoryDump() override;

public:
    typedef void (*DumpNumericValueProc) (SkManagedTraceMemoryDump* d, void* context, const char* dumpName, const char* valueName, const char* units, uint64_t value);
    typedef void (*DumpStringValueProc)  (SkManagedTraceMemoryDump* d, void* context, const char* dumpName, const char* valueName, const char* value);

    struct Procs {
        DumpNumericValueProc fDumpNumericValue = nullptr;
        DumpStringValueProc fDumpStringValue = nullptr;
    };

    static void setProcs(Procs procs);

    void dumpNumericValue(const char* dumpName, const char* valueName, const char* units, uint64_t value) override;
    void dumpStringValue(const char* dumpName, const char* valueName, const char* value) override;

    // "internal"
    void setMemoryBacking(const char* dumpName, const char* backingType, const char* backingObjectId) override;
    void setDiscardableMemoryBacking(const char* dumpName, const SkDiscardableMemory& discardableMemoryObject) override;

    SkTraceMemoryDump::LevelOfDetail getRequestedDetails() const override;
    bool shouldDumpWrappedObjects() const override;

private:
    static Procs fProcs;

    SkTraceMemoryDump::LevelOfDetail fLevel;
    bool fDumpWrapped;
    void* fContext;

    typedef SkTraceMemoryDump INHERITED;
};


#endif

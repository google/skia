/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTraceMemoryDump_DEFINED
#define SkTraceMemoryDump_DEFINED

#include "SkTypes.h"

class SkDiscardableMemory;

/**
 * Interface for memory tracing.
 * This interface is meant to be passed as argument to the memory dump methods of Skia objects.
 * The implementation of this interface is provided by the embedder.
 */
class SK_API SkTraceMemoryDump {
public:
    /**
     *  Appends a new memory dump (i.e. a row) to the trace memory infrastructure.
     *  If dumpName does not exist yet, a new one is created. Otherwise, a new column is appended to
     *  the previously created dump.
     *  Arguments:
     *    dumpName: an absolute, slash-separated, name for the item being dumped
     *        e.g., "skia/CacheX/EntryY".
     *    valueName: a string indicating the name of the column.
     *        e.g., "size", "active_size", "number_of_objects".
     *        This string is supposed to be long lived and is NOT copied.
     *    units: a string indicating the units for the value.
     *        e.g., "bytes", "objects".
     *        This string is supposed to be long lived and is NOT copied.
     *    value: the actual value being dumped.
     */
    virtual void dumpNumericValue(const char* dumpName,
                                  const char* valueName,
                                  const char* units,
                                  uint64_t value) = 0;

    /**
     * Sets the memory backing for an existing dump.
     * backingType and backingObjectId are used by the embedder to associate the memory dumped via
     * dumpNumericValue with the corresponding dump that backs the memory.
     */
    virtual void setMemoryBacking(const char* dumpName,
                                  const char* backingType,
                                  const char* backingObjectId) = 0;

    /**
     * Specialization for memory backed by discardable memory.
     */
    virtual void setDiscardableMemoryBacking(
        const char* dumpName,
        const SkDiscardableMemory& discardableMemoryObject) = 0;

protected:
    virtual ~SkTraceMemoryDump() { }
};

#endif

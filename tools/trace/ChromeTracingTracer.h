/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ChromeTracingTracer_DEFINED
#define ChromeTracingTracer_DEFINED

#include "EventTracingPriv.h"
#include "SkEventTracer.h"
#include "SkSpinlock.h"
#include "SkString.h"
#include "SkTHash.h"

class SkJSONWriter;

/**
 * A SkEventTracer implementation that logs events to JSON for viewing with chrome://tracing.
 */
class ChromeTracingTracer : public SkEventTracer {
public:
    ChromeTracingTracer(const char* filename);
    ~ChromeTracingTracer() override;

    SkEventTracer::Handle addTraceEvent(char            phase,
                                        const uint8_t*  categoryEnabledFlag,
                                        const char*     name,
                                        uint64_t        id,
                                        int             numArgs,
                                        const char**    argNames,
                                        const uint8_t*  argTypes,
                                        const uint64_t* argValues,
                                        uint8_t         flags) override;

    void updateTraceEventDuration(const uint8_t*        categoryEnabledFlag,
                                  const char*           name,
                                  SkEventTracer::Handle handle) override;

    const uint8_t* getCategoryGroupEnabled(const char* name) override {
        return fCategories.getCategoryGroupEnabled(name);
    }

    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag) override {
        return fCategories.getCategoryGroupName(categoryEnabledFlag);
    }

private:
    void flush();

    enum {
        // Events are variable size, but most commonly 48 bytes, assuming 64-bit pointers and
        // reasonable packing. This is a first guess at a number that balances memory usage vs.
        // time overhead of allocating blocks.
        kBlockSize = 512 * 1024,
    };

    typedef std::unique_ptr<uint8_t[]> BlockPtr;
    struct TraceEventBlock {
        BlockPtr fBlock;
        int      fEventsInBlock;
    };

    void createBlock();

    Handle appendEvent(const void* data, size_t size);

    SkString                 fFilename;
    SkSpinlock               fMutex;
    SkEventTracingCategories fCategories;

    TraceEventBlock fCurBlock;
    size_t          fCurBlockUsed;

    SkTArray<TraceEventBlock> fBlocks;
};

#endif

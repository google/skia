/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkATrace_DEFINED
#define SkATrace_DEFINED

#include "SkEventTracer.h"

class SkATrace : public SkEventTracer {
public:
    SkATrace();

    SkEventTracer::Handle addTraceEvent(char phase,
                                        const uint8_t* categoryEnabledFlag,
                                        const char* name,
                                        uint64_t id,
                                        int numArgs,
                                        const char** argNames,
                                        const uint8_t* argTypes,
                                        const uint64_t* argValues,
                                        uint8_t flags) override;


    void updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                  const char* name,
                                  SkEventTracer::Handle handle) override;

    const uint8_t* getCategoryGroupEnabled(const char* name) override;

    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag) override {
        static const char* category = "skiaATrace";
        return category;
    }

private:
    static void (*fBeginSection)(const char*);
    static void (*fEndSection)(void);
    static bool (*fIsEnabled)(void);
};

#endif


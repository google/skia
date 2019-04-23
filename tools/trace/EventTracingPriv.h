/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef EventTracingPriv_DEFINED
#define EventTracingPriv_DEFINED

#include "include/private/SkMutex.h"

/**
 * Construct and install an SkEventTracer, based on the mode,
 * defaulting to the --trace command line argument.
 */
void initializeEventTracingForTools(const char* mode = nullptr);

/**
 * Helper class used by internal implementations of SkEventTracer to manage categories.
 */
class SkEventTracingCategories {
public:
    SkEventTracingCategories() : fNumCategories(0) {}

    uint8_t*    getCategoryGroupEnabled(const char* name);
    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag);

private:
    enum { kMaxCategories = 256 };

    struct CategoryState {
        uint8_t     fEnabled;
        const char* fName;
    };

    CategoryState fCategories[kMaxCategories];
    int           fNumCategories;
    SkMutex       fMutex;
};

#endif

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEventTracingPriv_DEFINED
#define SkEventTracingPriv_DEFINED

#include "SkTypes.h"

/**
 * Construct and install an SkEventTracer, based on the 'trace' command line argument.
 *
 * @param threadsFlag Pointer to the FLAGS_threads variable (or nullptr). This is used to disable
 *                    threading when tracing to JSON. (Remove this param when JSON tracer is thread
 *                    safe).
 */
void initializeEventTracingForTools(int32_t* threadsFlag);

#endif

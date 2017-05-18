/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFuzzLogging_DEFINED
#define SkFuzzLogging_DEFINED

// Utilities for Skia's fuzzer

// When SK_FUZZ_LOGGING is defined SkDebugfs relevant to image filter fuzzing
// will be enabled. This allows the filter fuzzing code to white list fuzzer
// failures based on the output logs.
// Define this flag in your SkUserConfig.h or in your Make/Build system.
#ifdef SK_FUZZ_LOGGING
    #define SkFUZZF(args) SkDebugf("SkFUZZ: "); SkDebugf args
#else
    #define SkFUZZF(args)
#endif

#endif

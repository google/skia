/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if !defined(__has_feature)
    #define __has_feature(x) 0
#endif

#if __has_feature(address_sanitizer)

extern "C" {

    const char* __lsan_default_suppressions();
    const char* __lsan_default_suppressions() {
        return "leak:libfontconfig\n"    // FontConfig looks like it leaks, but it doesn't.
               "leak:libGL.so\n"         // For NVidia driver.
               "leak:__strdup\n"         // An eternal mystery, skia:2916.
               ;
    }

}

#endif

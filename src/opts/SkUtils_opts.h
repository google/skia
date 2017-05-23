/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_opts_DEFINED
#define SkUtils_opts_DEFINED

namespace SK_OPTS_NS {

    static void memset16(uint16_t buffer[], uint16_t value, int count) {
        for (int i = 0; i < count; i++) {
            buffer[i] = value;
        }
    }
    static void memset32(uint32_t buffer[], uint32_t value, int count) {
        for (int i = 0; i < count; i++) {
            buffer[i] = value;
        }
    }
    static void memset64(uint64_t buffer[], uint64_t value, int count) {
        for (int i = 0; i < count; i++) {
            buffer[i] = value;
        }
    }

}

#endif//SkUtils_opts_DEFINED

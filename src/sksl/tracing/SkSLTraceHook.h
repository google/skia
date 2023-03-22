/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSLTRACEHOOK
#define SKSLTRACEHOOK

#include <cstdint>

namespace SkSL {

class TraceHook {
public:
    virtual ~TraceHook() = default;
    virtual void line(int lineNum) = 0;
    virtual void var(int slot, int32_t val) = 0;
    virtual void enter(int fnIdx) = 0;
    virtual void exit(int fnIdx) = 0;
    virtual void scope(int delta) = 0;
};

}  // namespace SkSL

#endif

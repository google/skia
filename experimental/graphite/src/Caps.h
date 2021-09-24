/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Caps_DEFINED
#define skgpu_Caps_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class Caps : public SkRefCnt {
public:
    ~Caps() override {}

protected:
    Caps();

private:
};

} // namespace skgpu

#endif // skgpu_Caps_DEFINED

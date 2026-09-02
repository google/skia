/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skcpu_RecorderImpl_DEFINED
#define skcpu_RecorderImpl_DEFINED

#include "include/core/SkCPURecorder.h"
#include "src/core/SkCPUContextImpl.h"

namespace skcpu {

class RecorderImpl final : public skcpu::Recorder {
public:
    RecorderImpl(const ContextImpl* ctx) : fCtx(ctx) {}

    const ContextImpl* ctx() const { return fCtx; }

private:
    const ContextImpl* const fCtx;
};

}  // namespace skcpu

inline skcpu::RecorderImpl* asRRI(skcpu::Recorder* rr) {
    return static_cast<skcpu::RecorderImpl*>(rr);
}

#endif

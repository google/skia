/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RefCntedCallback_DEFINED
#define skgpu_RefCntedCallback_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {
/**
 * Ref-counted object that calls a callback from its destructor.
 */
class RefCntedCallback : public SkNVRefCnt<RefCntedCallback> {
public:
    using Context = void*;
    using Callback = void (*)(Context);

    static sk_sp<RefCntedCallback> Make(Callback proc, Context ctx) {
        if (!proc) {
            return nullptr;
        }
        return sk_sp<RefCntedCallback>(new RefCntedCallback(proc, ctx));
    }

    ~RefCntedCallback() { fReleaseProc(fReleaseCtx); }

    Context context() const { return fReleaseCtx; }

private:
    RefCntedCallback(Callback proc, Context ctx) : fReleaseProc(proc), fReleaseCtx(ctx) {}
    RefCntedCallback(const RefCntedCallback&) = delete;
    RefCntedCallback(RefCntedCallback&&) = delete;
    RefCntedCallback& operator=(const RefCntedCallback&) = delete;
    RefCntedCallback& operator=(RefCntedCallback&&) = delete;

    Callback fReleaseProc;
    Context fReleaseCtx;
};

} // namespace skgpu

#endif // skgpu_RefCntedCallback_DEFINED


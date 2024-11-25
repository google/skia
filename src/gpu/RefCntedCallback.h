/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RefCntedCallback_DEFINED
#define skgpu_RefCntedCallback_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"

namespace skgpu {
/**
 * Move-only type that calls a callback from its destructor.
 */
class AutoCallback {
public:
    using Context                 = void*;
    using Callback                = void (*)(Context);
    using CallbackWithStats       = void (*)(Context, const GpuStats&);
    using ResultCallback          = void (*)(Context, CallbackResult);
    using ResultCallbackWithStats = void (*)(Context, CallbackResult, const GpuStats&);

    AutoCallback() = default;
    AutoCallback(const AutoCallback&) = delete;
    AutoCallback(AutoCallback&& that) { *this = std::move(that); }

    AutoCallback(Callback proc, Context ctx) : fReleaseProc(proc), fReleaseCtx(ctx) {}
    AutoCallback(CallbackWithStats proc, Context ctx)
            : fReleaseWithStatsProc(proc), fReleaseCtx(ctx) {}
    AutoCallback(ResultCallback proc, Context ctx) : fResultReleaseProc(proc), fReleaseCtx(ctx) {}
    AutoCallback(ResultCallbackWithStats proc, Context ctx)
            : fResultReleaseWithStatsProc(proc), fReleaseCtx(ctx) {}

    ~AutoCallback() {
        SkASSERT(this->operator bool() || true);  // run assert in the operator

        if (fResultReleaseWithStatsProc) {
            fResultReleaseWithStatsProc(fReleaseCtx, fResult, fGpuStats);
        } else if (fReleaseWithStatsProc) {
            fReleaseWithStatsProc(fReleaseCtx, fGpuStats);
        } else if (fResultReleaseProc) {
            fResultReleaseProc(fReleaseCtx, fResult);
        } else if (fReleaseProc) {
            fReleaseProc(fReleaseCtx);
        }
    }

    AutoCallback& operator=(const AutoCallback&) = delete;
    AutoCallback& operator=(AutoCallback&& that) {
        fReleaseCtx                 = that.fReleaseCtx;
        fReleaseProc                = that.fReleaseProc;
        fReleaseWithStatsProc       = that.fReleaseWithStatsProc;
        fResultReleaseProc          = that.fResultReleaseProc;
        fResultReleaseWithStatsProc = that.fResultReleaseWithStatsProc;
        fResult                     = that.fResult;
        fGpuStats                   = that.fGpuStats;

        that.fReleaseProc                = nullptr;
        that.fReleaseWithStatsProc       = nullptr;
        that.fResultReleaseProc          = nullptr;
        that.fResultReleaseWithStatsProc = nullptr;
        return *this;
    }

    Context context() const { return fReleaseCtx; }

    bool receivesGpuStats() const { return fReleaseWithStatsProc || fResultReleaseWithStatsProc; }

    void setFailureResult() {
        SkASSERT(fResultReleaseProc || fResultReleaseWithStatsProc);
        // Shouldn't really be calling this multiple times.
        SkASSERT(fResult == CallbackResult::kSuccess);
        fResult = CallbackResult::kFailed;
    }

    void setStats(const GpuStats& stats) {
        SkASSERT(this->receivesGpuStats());
        fGpuStats = stats;
    }

    explicit operator bool() const {
        auto toInt = [](auto p) { return p ? 1U : 0U; };
        auto total = toInt(fReleaseProc) + toInt(fReleaseWithStatsProc) + toInt(fResultReleaseProc);
        SkASSERT(total <= 1);
        return total == 1;
    }

private:
    Callback                fReleaseProc                = nullptr;
    CallbackWithStats       fReleaseWithStatsProc       = nullptr;
    ResultCallback          fResultReleaseProc          = nullptr;
    ResultCallbackWithStats fResultReleaseWithStatsProc = nullptr;

    Context        fReleaseCtx = nullptr;
    CallbackResult fResult     = CallbackResult::kSuccess;
    GpuStats       fGpuStats   = {};
};

/**
 * Ref-counted object that calls a callback from its destructor.
 */
class RefCntedCallback : public SkNVRefCnt<RefCntedCallback> {
public:
    using Context                 = AutoCallback::Context;
    using Callback                = AutoCallback::Callback;
    using CallbackWithStats       = AutoCallback::CallbackWithStats;
    using ResultCallback          = AutoCallback::ResultCallback;
    using ResultCallbackWithStats = AutoCallback::ResultCallbackWithStats;

    static sk_sp<RefCntedCallback> Make(Callback proc, Context ctx) { return MakeImpl(proc, ctx); }

    static sk_sp<RefCntedCallback> Make(CallbackWithStats proc, Context ctx) {
        return MakeImpl(proc, ctx);
    }

    static sk_sp<RefCntedCallback> Make(ResultCallback proc, Context ctx) {
        return MakeImpl(proc, ctx);
    }

    static sk_sp<RefCntedCallback> Make(ResultCallbackWithStats proc, Context ctx) {
        return MakeImpl(proc, ctx);
    }

    static sk_sp<RefCntedCallback> Make(AutoCallback&& callback) {
        if (!callback) {
            return nullptr;
        }
        return sk_sp<RefCntedCallback>(new RefCntedCallback(std::move(callback)));
    }

    Context context() const { return fCallback.context(); }

    bool receivesGpuStats() const { return fCallback.receivesGpuStats(); }

    void setFailureResult() { fCallback.setFailureResult(); }

    void setStats(const GpuStats& stats) { fCallback.setStats(stats); }

private:
    template <typename R, typename... Args>
    static sk_sp<RefCntedCallback> MakeImpl(R proc(Args...), Context ctx) {
        if (!proc) {
            return nullptr;
        }
        return sk_sp<RefCntedCallback>(new RefCntedCallback({proc, ctx}));
    }

    RefCntedCallback(AutoCallback callback) : fCallback(std::move(callback)) {}

    AutoCallback fCallback;
};

} // namespace skgpu

#endif // skgpu_RefCntedCallback_DEFINED


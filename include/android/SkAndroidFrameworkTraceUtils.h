/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAndroidFrameworkTraceUtils_DEFINED
#define SkAndroidFrameworkTraceUtils_DEFINED

#include <atomic>

#include "include/core/SkTypes.h"

class SkAndroidFrameworkTraceUtils {
public:
    SkAndroidFrameworkTraceUtils() = delete;

    // Controls whether broad tracing is enabled.
    //
    // Some key trace events may still be recorded when this is disabled, if a relevant tracing
    // session is active.
    //
    // ATrace is used by default, but can be replaced with Perfetto by calling
    // SetUsePerfettoTrackEvents(true)
    static void SetEnableDetailedTracing(bool enableDetailedTracing) {
        gEnableDetailedTracing.store(enableDetailedTracing, std::memory_order_release);
    }

    // Deprecated: use SetEnableDetailedTracing
    static inline void setEnableTracing(bool enableAndroidTracing) {
        SetEnableDetailedTracing(enableAndroidTracing);
    }

    // Controls whether tracing uses Perfetto instead of ATrace.
    //
    // Returns true if Skia was built with Perfetto, false otherwise.
    static bool SetUsePerfettoTrackEvents(bool usePerfettoTrackEvents) {
#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
        // Ensure Perfetto is initialized if it wasn't already the preferred tracing backend.
        if (usePerfettoTrackEvents && !gUsePerfettoTrackEvents.load(std::memory_order_acquire)) {
            InitPerfetto();
        }
        gUsePerfettoTrackEvents.store(usePerfettoTrackEvents, std::memory_order_release);
        return true;
#else // !SK_ANDROID_FRAMEWORK_USE_PERFETTO
        return false;
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO
    }

    // Deprecated: use SetUsePerfettoTrackEvents
    static inline bool setUsePerfettoTrackEvents(bool usePerfettoTrackEvents) {
        return SetUsePerfettoTrackEvents(usePerfettoTrackEvents);
    }

    static bool GetEnableDetailedTracing() {
        return gEnableDetailedTracing.load(std::memory_order_acquire);
    }

    static bool GetUsePerfettoTrackEvents() {
        return gUsePerfettoTrackEvents.load(std::memory_order_acquire);
    }

private:
    static std::atomic<bool> gEnableDetailedTracing;
    static std::atomic<bool> gUsePerfettoTrackEvents;

#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
    // Initializes tracing systems, and establishes a connection to the 'traced' daemon.
    //
    // Can be called multiple times.
    static void InitPerfetto();
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO
};

// TODO(nscobie): remove this after migrating clients
using SkAndroidFrameworkTraceUtil = SkAndroidFrameworkTraceUtils;

#endif // SkAndroidFrameworkTraceUtils_DEFINED

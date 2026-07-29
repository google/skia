/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLogHandler_DEFINED
#define SkLogHandler_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkLogPriority.h"

#include <cstdarg>

/**
 * SkLogHandler defines an interface for receiving Skia log messages.
 * Clients can implement this interface and install it via SetInstance.
 */
class SK_API SkLogHandler : public SkRefCnt {
public:
    ~SkLogHandler() override = default;

    /**
     * Called when a log message is generated (must pass compile-time priority).
     * Must be thread-safe since multiple threads may be logging at the same time.
     * @param priority The priority of the log message.
     * @param format   The format string of the log message.
     * @param args     The arguments for the format string.
     */
    virtual void onLog(SkLogPriority priority, const char format[], va_list args) = 0;

    /**
     * Installs a custom log handler. Returns true if successful (i.e., it's the first call).
     * Skia only allows one SkLogHandler to be set and it cannot change after being set.
     */
    static bool SetInstance(sk_sp<SkLogHandler>);

    /**
     * Returns the current log handler. If none has been set, returns nullptr.
     */
    static sk_sp<SkLogHandler> GetInstance();

protected:
    SkLogHandler() = default;

private:
    SkLogHandler(const SkLogHandler&) = delete;
    SkLogHandler& operator=(const SkLogHandler&) = delete;
};

#endif // SkLogHandler_DEFINED

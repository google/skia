/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkLog.h"
#include "include/utils/SkLogHandler.h"

#include <cstdarg>

// Redefinitions of SkLog must invoke SkLogHandler::onLog if they still want on log callbacks. It is
// more likely that the onLog callback is sufficient, and you don't need to redefine this function.
#if !defined(SkLog)
void SkLog(SkLogPriority priority, const char format[], ...) {
    va_list args;
    va_start(args, format);

    if (auto handler = SkLogHandler::GetInstance()) {
        va_list args_copy;
        va_copy(args_copy, args);
        handler->onLog(priority, format, args_copy);
        va_end(args_copy);
    }

    SkLogVAList(priority, format, args);
    va_end(args);
}
#endif

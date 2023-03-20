/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkProcessWorklist_DEFINED
#define SkProcessWorklist_DEFINED

#include "include/core/SkSpan.h"

#include <functional>
#include <string>

enum class ResultCode {
    kSuccess = 0,
    kCompileError = 1,
    kInputError = 2,
    kOutputError = 3,
    kConfigurationError = 4,
};

/**
 * Processes multiple inputs in a single invocation by reading a worklist file.
 * The processCommand is invoked with a set of arguments, read from the worklist.
 * A blank line is used to separate one group of arguments from the next group.
 */
ResultCode ProcessWorklist(
        const char* worklistPath,
        const std::function<ResultCode(SkSpan<std::string> args)>& processCommandFn);

#endif

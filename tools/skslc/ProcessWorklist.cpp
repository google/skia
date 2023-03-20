/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkStringView.h"
#include "tools/skslc/ProcessWorklist.h"

#include <algorithm>
#include <fstream>
#include <vector>

ResultCode ProcessWorklist(
        const char* worklistPath,
        const std::function<ResultCode(SkSpan<std::string> args)>& processCommandFn) {
    std::string inputPath(worklistPath);
    if (!skstd::ends_with(inputPath, ".worklist")) {
        printf("expected .worklist file, found: %s\n\n", worklistPath);
        return ResultCode::kConfigurationError;
    }

    // The worklist contains one line per argument to pass. When a blank line is reached, those
    // arguments will be passed to `processCommandFn`.
    auto resultCode = ResultCode::kSuccess;
    std::vector<std::string> args = {"sksl"};
    std::ifstream in(worklistPath);
    for (std::string line; std::getline(in, line); ) {
        if (in.rdstate()) {
            printf("error reading '%s'\n", worklistPath);
            return ResultCode::kInputError;
        }

        if (!line.empty()) {
            // We found an argument. Remember it.
            args.push_back(std::move(line));
        } else {
            // We found a blank line. If we have any arguments stored up, process them as a command.
            if (!args.empty()) {
                ResultCode outcome = processCommandFn(args);
                resultCode = std::max(resultCode, outcome);

                // Clear every argument except the first.
                args.resize(1);
            }
        }
    }

    // If the worklist ended with a list of arguments but no blank line, process those now.
    if (args.size() > 1) {
        ResultCode outcome = processCommandFn(args);
        resultCode = std::max(resultCode, outcome);
    }

    // Return the "worst" status we encountered. For our purposes, compilation errors are the least
    // serious, because they are expected to occur in unit tests. Other types of errors are not
    // expected at all during a build.
    return resultCode;
}

/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/testrunners/common/TestRunner.h"
#include <regex>

bool TestRunner::ShouldRunTestCase(const char* name,
                                   CommandLineFlags::StringArray& matchFlag,
                                   CommandLineFlags::StringArray& skipFlag) {
    for (int i = 0; i < skipFlag.size(); i++) {
        std::regex re(skipFlag[i]);
        if (std::regex_search(name, re)) {
            return false;
        }
    }

    if (matchFlag.isEmpty()) {
        return true;
    }

    for (int i = 0; i < matchFlag.size(); i++) {
        std::regex re(matchFlag[i]);
        if (std::regex_search(name, re)) {
            return true;
        }
    }

    return false;
}

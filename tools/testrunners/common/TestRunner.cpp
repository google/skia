/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkAttributes.h"
#include "include/private/base/SkDebug.h"
#include "tools/testrunners/common/TestRunner.h"

#include <cstdarg>
#include <iomanip>
#include <regex>
#include <sstream>

void TestRunner::FlagValidators::StringNonEmpty(std::string name,
                                                CommandLineFlags::StringArray flag) {
    if (flag.size() == 0) {
        SK_ABORT("Flag %s cannot be empty.\n", name.c_str());
    }
}

void TestRunner::FlagValidators::StringAtMostOne(std::string name,
                                                 CommandLineFlags::StringArray flag) {
    if (flag.size() > 1) {
        SK_ABORT("Flag %s takes one single value, got: %d.\n", name.c_str(), flag.size());
    }
}

void TestRunner::FlagValidators::StringEven(std::string name, CommandLineFlags::StringArray flag) {
    if (flag.size() % 2 == 1) {
        SK_ABORT(
                "Flag %s takes an even number of arguments, got: %d.\n", name.c_str(), flag.size());
    }
}

void TestRunner::FlagValidators::IntGreaterOrEqual(std::string name, int flag, int min) {
    if (flag < min) {
        SK_ABORT("Flag %s must be greater or equal than %d, got: %d.\n", name.c_str(), min, flag);
    }
}

void TestRunner::FlagValidators::AllOrNone(std::map<std::string, bool> flags) {
    std::string names;
    unsigned int numFlagsSet = 0;
    for (auto const& [name, isSet] : flags) {
        if (names == "") {
            names = name;
        } else {
            names += ", " + name;
        }
        if (isSet) {
            numFlagsSet++;
        }
    }
    if (numFlagsSet != flags.size() && numFlagsSet != 0) {
        SK_ABORT("Either all or none of the following flags must be set: %s.\n", names.c_str());
    }
}

void TestRunner::FlagValidators::ExactlyOne(std::map<std::string, bool> flags) {
    std::string names;
    unsigned int numFlagsSet = 0;
    for (auto const& [name, isSet] : flags) {
        if (names == "") {
            names = name;
        } else {
            names += ", " + name;
        }
        if (isSet) {
            numFlagsSet++;
        }
    }
    if (numFlagsSet != 1) {
        SK_ABORT("Exactly one of the following flags must be set: %s.\n", names.c_str());
    }
}

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

void TestRunner::Log(const char* format, ...) {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::gmtime(&t);
    std::ostringstream oss;
    oss << std::put_time(now, "%Y-%m-%d %H:%M:%S UTC");
    printf("[%s] ", oss.str().c_str());

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}

/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestRunner_DEFINED
#define TestRunner_DEFINED

#include "tools/flags/CommandLineFlags.h"

#include <map>

namespace TestRunner {
namespace FlagValidators {

void StringNonEmpty(std::string name, CommandLineFlags::StringArray flag);

void StringAtMostOne(std::string name, CommandLineFlags::StringArray flag);

void StringEven(std::string name, CommandLineFlags::StringArray flag);

void IntGreaterOrEqual(std::string name, int flag, int min);

void AllOrNone(std::map<std::string, bool> flags);

void ExactlyOne(std::map<std::string, bool> flags);

}  // namespace FlagValidators

// Performs some common initialization steps, and logs all command-line arguments.
//
// It should be called by all test runners at the beginning of their main() function.
void InitAndLogCmdlineArgs(int argc, char** argv);

// Determines whether a test case should be run based on the --match and --skip command-line flags.
bool ShouldRunTestCase(const char* name,
                       CommandLineFlags::StringArray& matchFlag,
                       CommandLineFlags::StringArray& skipFlag);

// Logs a message to stdout.
//
// It prefixes the message with a timestamp, and suffixes it with a line break ("\n").
void Log(const char* format, ...) SK_PRINTF_LIKE(1, 2);

}  // namespace TestRunner

#endif  // TestRunner_DEFINED

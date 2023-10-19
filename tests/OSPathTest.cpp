/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"

#include <cstddef>

/**
 *  Test SkOSPath::Join, SkOSPath::Basename, and SkOSPath::Dirname.
 *  Will use SkOSPath::Join to append filename to dir, test that it works correctly,
 *  and tests using SkOSPath::Basename on the result.
 *  @param reporter Reporter for test conditions.
 *  @param dir String representing the path to a folder. May or may not
 *      end with SkOSPath::SEPARATOR.
 *  @param filename String representing the basename of a file. Must NOT
 *      contain SkOSPath::SEPARATOR.
 */
static void test_dir_with_file(skiatest::Reporter* reporter, const SkString& dir,
                               const SkString& filename) {
    // If filename contains SkOSPath::SEPARATOR, the tests will fail.
    SkASSERT(!filename.contains(SkOSPath::SEPARATOR));

    // Tests for SkOSPath::Join and SkOSPath::Basename

    // fullName should be "dir<SkOSPath::SEPARATOR>file"
    SkString fullName = SkOSPath::Join(dir.c_str(), filename.c_str());

    // fullName should be the combined size of dir and file, plus one if
    // dir did not include the final path separator.
    size_t expectedSize = dir.size() + filename.size();
    if (!dir.endsWith(SkOSPath::SEPARATOR) && !dir.isEmpty()) {
        expectedSize++;
    }
    REPORTER_ASSERT(reporter, fullName.size() == expectedSize);

    SkString basename = SkOSPath::Basename(fullName.c_str());
    SkString dirname = SkOSPath::Dirname(fullName.c_str());

    // basename should be the same as filename
    REPORTER_ASSERT(reporter, basename.equals(filename));

    // dirname should be the same as dir with any trailing seperators removed.
    // Except when the the string is just "/".
    SkString strippedDir = dir;
    while (strippedDir.size() > 2 && strippedDir[strippedDir.size() - 1] == SkOSPath::SEPARATOR) {
        strippedDir.remove(strippedDir.size() - 1, 1);
    }
    if (!dirname.equals(strippedDir)) {
        SkDebugf("OOUCH %s %s %s\n", dir.c_str(), strippedDir.c_str(), dirname.c_str());
    }
    REPORTER_ASSERT(reporter, dirname.equals(strippedDir));

    // basename will not contain a path separator
    REPORTER_ASSERT(reporter, !basename.contains(SkOSPath::SEPARATOR));

    // Now take the basename of filename, which should be the same as filename.
    basename = SkOSPath::Basename(filename.c_str());
    REPORTER_ASSERT(reporter, basename.equals(filename));
}

DEF_TEST(OSPath, reporter) {
    SkString dir("dir");
    SkString filename("file");
    test_dir_with_file(reporter, dir, filename);

    // Now make sure this works with a path separator at the end of dir.
    dir.appendUnichar(SkOSPath::SEPARATOR);
    test_dir_with_file(reporter, dir, filename);

    // Test using no filename.
    test_dir_with_file(reporter, dir, SkString());

    // Testing using no directory.
    test_dir_with_file(reporter, SkString(), filename);

    // Test with a sub directory.
    dir.append("subDir");
    test_dir_with_file(reporter, dir, filename);

    // Basename of a directory with a path separator at the end is empty.
    dir.appendUnichar(SkOSPath::SEPARATOR);
    SkString baseOfDir = SkOSPath::Basename(dir.c_str());
    REPORTER_ASSERT(reporter, baseOfDir.size() == 0);

    // Basename of nullptr is an empty string.
    SkString empty = SkOSPath::Basename(nullptr);
    REPORTER_ASSERT(reporter, empty.size() == 0);

    // File in root dir
    dir.printf("%c", SkOSPath::SEPARATOR);
    filename.set("file");
    test_dir_with_file(reporter, dir, filename);

    // Just the root dir
    filename.reset();
    test_dir_with_file(reporter, dir, filename);

    // Test that nullptr can be used for the directory and filename.
    SkString emptyPath = SkOSPath::Join(nullptr, nullptr);
    REPORTER_ASSERT(reporter, emptyPath.isEmpty());
}

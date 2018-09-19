/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSPath_DEFINED
#define SkOSPath_DEFINED

#include "include/core/SkString.h"

/**
 *  Functions for modifying SkStrings which represent paths on the filesystem.
 */
class SkOSPath {
public:
#ifdef _WIN32
    const static char SEPARATOR = '\\';
#else
    const static char SEPARATOR = '/';
#endif

    /**
     * Assembles rootPath and relativePath into a single path, like this:
     * rootPath/relativePath.
     * It is okay to call with a NULL rootPath and/or relativePath. A path
     * separator will still be inserted.
     *
     * Uses SkPATH_SEPARATOR, to work on all platforms.
     */
    static SkString Join(const char* rootPath, const char* relativePath);

    /**
     *  Return the name of the file, ignoring the directory structure.
     *  Behaves like python's os.path.basename. If the fullPath is
     *  /dir/subdir/, an empty string is returned.
     *  @param fullPath Full path to the file.
     *  @return SkString The basename of the file - anything beyond the
     *      final slash, or the full name if there is no slash.
     */
    static SkString Basename(const char* fullPath);

    /**
     *  Given a qualified file name returns the directory.
     *  Behaves like python's os.path.dirname. If the fullPath is
     *  /dir/subdir/ the return will be /dir/subdir/
     *  @param fullPath Full path to the file.
     *  @return SkString The dir containing the file - anything preceding the
     *      final slash, or the full name if ending in a slash.
     */
    static SkString Dirname(const char* fullPath);
};

#endif

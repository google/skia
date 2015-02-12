/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOSFile.h"

SkString SkOSPath::Join(const char *rootPath, const char *relativePath) {
    SkString result(rootPath);
    if (!result.endsWith(SkPATH_SEPARATOR) && !result.isEmpty()) {
        result.appendUnichar(SkPATH_SEPARATOR);
    }
    result.append(relativePath);
    return result;
}

SkString SkOSPath::Basename(const char* fullPath) {
    if (!fullPath) {
        return SkString();
    }
    const char* filename = strrchr(fullPath, SkPATH_SEPARATOR);
    if (NULL == filename) {
        filename = fullPath;
    } else {
        ++filename;
    }
    return SkString(filename);
}

SkString SkOSPath::Dirname(const char* fullPath) {
    if (!fullPath) {
        return SkString();
    }
    const char* end = strrchr(fullPath, SkPATH_SEPARATOR);
    if (NULL == end) {
        return SkString();
    }
    if (end == fullPath) {
        SkASSERT(fullPath[0] == SkPATH_SEPARATOR);
        ++end;
    }
    return SkString(fullPath, end - fullPath);
}

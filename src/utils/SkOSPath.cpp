/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSPath.h"

SkString SkOSPath::Join(const char *rootPath, const char *relativePath) {
    SkString result(rootPath);
    if (!result.endsWith(SEPARATOR) && ('\\' != SEPARATOR || !result.endsWith('/')) &&
            !result.isEmpty()) {
        result.appendUnichar(SEPARATOR);
    }
    result.append(relativePath);
    return result;
}

SkString SkOSPath::Basename(const char* fullPath) {
    if (!fullPath) {
        return SkString();
    }
    const char* filename = strrchr(fullPath, SEPARATOR);
    if ('\\' == SEPARATOR) {
        const char* alternate = strrchr(fullPath, '/');
        if (filename < alternate) {
            filename = alternate;
        }
    }
    if (nullptr == filename) {
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
    const char* end = strrchr(fullPath, SEPARATOR);
    if ('\\' == SEPARATOR) {
        const char* alternate = strrchr(fullPath, '/');
        if (end < alternate) {
            end = alternate;
        }
    }
    if (nullptr == end) {
        return SkString();
    }
    if (end == fullPath) {
        SkASSERT(fullPath[0] == SEPARATOR || ('\\' == SEPARATOR && fullPath[0] == '/'));
        ++end;
    }
    return SkString(fullPath, end - fullPath);
}

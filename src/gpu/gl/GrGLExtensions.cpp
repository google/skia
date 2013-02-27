/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLExtensions.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

bool GrGLExtensions::init(GrGLBinding binding,
                          GrGLGetStringProc getString,
                          GrGLGetStringiProc getStringi,
                          GrGLGetIntegervProc getIntegerv) {
    fStrings.reset();
    if (NULL == getString) {
        return false;
    }
    bool indexed = false;
    if (kDesktop_GrGLBinding == binding) {
        const GrGLubyte* verString = getString(GR_GL_VERSION);
        if (NULL == verString) {
            return false;
        }
        GrGLVersion version = GrGLGetVersionFromString((const char*) verString);
        indexed = version >= GR_GL_VER(3, 0);
    }
    if (indexed) {
        if (NULL == getStringi || NULL == getIntegerv) {
            return false;
        }
        GrGLint extensionCnt = 0;
        getIntegerv(GR_GL_NUM_EXTENSIONS, &extensionCnt);
        fStrings.push_back_n(extensionCnt);
        for (int i = 0; i < extensionCnt; ++i) {
            const char* ext = (const char*) getStringi(GR_GL_EXTENSIONS, i);
            fStrings[i] = ext;
        }
    } else {
        const char* extensions = (const char*) getString(GR_GL_EXTENSIONS);
        if (NULL == extensions) {
            return false;
        }
        // First count the extensions so that we don't cause the array to malloc multiple times.
        int extensionCnt = 1;
        const char* e = (const char*) extensions;
        while (NULL != (e = strchr(e+1, ' '))) {
            e += 1;
            ++extensionCnt;
        }
        fStrings.push_back_n(extensionCnt);

        int i = 0;
        while (true) {
            size_t length = strcspn(extensions, " ");
            GrAssert(i < extensionCnt);
            fStrings[i].set(extensions, length);
            ++i;
            if ('\0' == extensions[length]) {
                break;
            }
            extensions += length + 1;
        }
        GrAssert(i == extensionCnt);
    }
    return true;
}

bool GrGLExtensions::has(const char* ext) const {
    // TODO: Sort the extensions and binary search.
    int count = fStrings.count();
    for (int i = 0; i < count; ++i) {
        if (fStrings[i].equals(ext)) {
            return true;
        }
    }
    return false;
}

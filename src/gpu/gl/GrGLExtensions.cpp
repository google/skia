/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLExtensions.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

#include "SkTSearch.h"
#include "SkTSort.h"

namespace {
inline bool extension_compare(const SkString& a, const SkString& b) {
    return strcmp(a.c_str(), b.c_str()) < 0;
}
}

bool GrGLExtensions::init(GrGLBinding binding,
                          GrGLGetStringProc getString,
                          GrGLGetStringiProc getStringi,
                          GrGLGetIntegervProc getIntegerv) {
    fStrings.reset();
    if (NULL == getString) {
        return false;
    }

    // glGetStringi and indexed extensions were added in version 3.0 of desktop GL and ES.
    const GrGLubyte* verString = getString(GR_GL_VERSION);
    if (NULL == verString) {
        return false;
    }
    GrGLVersion version = GrGLGetVersionFromString((const char*) verString);
    bool indexed = version >= GR_GL_VER(3, 0);

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
        while (true) {
            // skip over multiple spaces between extensions
            while (' ' == *extensions) {
                ++extensions;
            }
            // quit once we reach the end of the string.
            if ('\0' == *extensions) {
                break;
            }
            // we found an extension
            size_t length = strcspn(extensions, " ");
            fStrings.push_back().set(extensions, length);
            extensions += length;
        }
    }
    if (!fStrings.empty()) {
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fStrings.front(), &fStrings.back(), cmp);
    }
    return true;
}

bool GrGLExtensions::has(const char* ext) const {
    if (fStrings.empty()) {
        return false;
    }
    SkString extensionStr(ext);
    int idx = SkTSearch<SkString, extension_compare>(&fStrings.front(),
                                                     fStrings.count(),
                                                     extensionStr,
                                                     sizeof(SkString));
    return idx >= 0;
}

void GrGLExtensions::print(const char* sep) const {
    if (NULL == sep) {
        sep = " ";
    }
    int cnt = fStrings.count();
    for (int i = 0; i < cnt; ++i) {
        GrPrintf("%s%s", fStrings[i].c_str(), (i < cnt - 1) ? sep : "");
    }
}

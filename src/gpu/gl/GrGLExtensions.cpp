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

namespace { // This cannot be static because it is used as a template parameter.
inline bool extension_compare(const SkString& a, const SkString& b) {
    return strcmp(a.c_str(), b.c_str()) < 0;
}
}

// finds the index of ext in strings or a negative result if ext is not found.
static int find_string(const SkTArray<SkString>& strings, const char ext[]) {
    if (strings.empty()) {
        return -1;
    }
    SkString extensionStr(ext);
    int idx = SkTSearch<SkString, extension_compare>(&strings.front(),
                                                     strings.count(),
                                                     extensionStr,
                                                     sizeof(SkString));
    return idx;
}

GrGLExtensions::GrGLExtensions(const GrGLExtensions& that) : fStrings(SkNEW(SkTArray<SkString>)) {
    *this = that;
}

GrGLExtensions& GrGLExtensions::operator=(const GrGLExtensions& that) {
    *fStrings = *that.fStrings;
    fInitialized = that.fInitialized;
    return *this;
}

bool GrGLExtensions::init(GrGLStandard standard,
                          GrGLGetStringProc getString,
                          GrGLGetStringiProc getStringi,
                          GrGLGetIntegervProc getIntegerv) {
    fInitialized = false;
    fStrings->reset();

    if (NULL == getString) {
        return false;
    }

    // glGetStringi and indexed extensions were added in version 3.0 of desktop GL and ES.
    const GrGLubyte* verString = getString(GR_GL_VERSION);
    GrGLVersion version = GrGLGetVersionFromString((const char*) verString);
    if (GR_GL_INVALID_VER == version) {
        return false;
    }

    bool indexed = version >= GR_GL_VER(3, 0);

    if (indexed) {
        if (NULL == getStringi || NULL == getIntegerv) {
            return false;
        }
        GrGLint extensionCnt = 0;
        getIntegerv(GR_GL_NUM_EXTENSIONS, &extensionCnt);
        fStrings->push_back_n(extensionCnt);
        for (int i = 0; i < extensionCnt; ++i) {
            const char* ext = (const char*) getStringi(GR_GL_EXTENSIONS, i);
            (*fStrings)[i] = ext;
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
            fStrings->push_back().set(extensions, length);
            extensions += length;
        }
    }
    if (!fStrings->empty()) {
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fStrings->front(), &fStrings->back(), cmp);
    }
    fInitialized = true;
    return true;
}

bool GrGLExtensions::has(const char ext[]) const {
    SkASSERT(fInitialized);
    return find_string(*fStrings, ext) >= 0;
}

bool GrGLExtensions::remove(const char ext[]) {
    SkASSERT(fInitialized);
    int idx = find_string(*fStrings, ext);
    if (idx >= 0) {
        // This is not terribly effecient but we really only expect this function to be called at
        // most a handful of times when our test programs start.
        SkAutoTDelete< SkTArray<SkString> > oldStrings(fStrings.detach());
        fStrings.reset(SkNEW(SkTArray<SkString>(oldStrings->count() - 1)));
        fStrings->push_back_n(idx, &oldStrings->front());
        fStrings->push_back_n(oldStrings->count() - idx - 1, &(*oldStrings)[idx] + 1);
        return true;
    } else {
        return false;
    }
}

void GrGLExtensions::add(const char ext[]) {
    int idx = find_string(*fStrings, ext);
    if (idx < 0) {
        // This is not the most effecient approach since we end up doing a full sort of the
        // extensions after the add
        fStrings->push_back().set(ext);
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fStrings->front(), &fStrings->back(), cmp);
    }
}

void GrGLExtensions::print(const char* sep) const {
    if (NULL == sep) {
        sep = " ";
    }
    int cnt = fStrings->count();
    for (int i = 0; i < cnt; ++i) {
        GrPrintf("%s%s", (*fStrings)[i].c_str(), (i < cnt - 1) ? sep : "");
    }
}

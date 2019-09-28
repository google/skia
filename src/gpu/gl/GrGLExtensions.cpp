/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLExtensions.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"

#include "src/core/SkMakeUnique.h"
#include "src/core/SkTSearch.h"
#include "src/core/SkTSort.h"
#include "src/utils/SkJSONWriter.h"

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

GrGLExtensions::GrGLExtensions(const GrGLExtensions& that) {
    *this = that;
}

GrGLExtensions& GrGLExtensions::operator=(const GrGLExtensions& that) {
    if (this != &that) {
        fStrings = that.fStrings;
        fInitialized = that.fInitialized;
    }
    return *this;
}

static void eat_space_sep_strings(SkTArray<SkString>* out, const char in[]) {
    if (!in) {
        return;
    }
    while (true) {
        // skip over multiple spaces between extensions
        while (' ' == *in) {
            ++in;
        }
        // quit once we reach the end of the string.
        if ('\0' == *in) {
            break;
        }
        // we found an extension
        size_t length = strcspn(in, " ");
        out->push_back().set(in, length);
        in += length;
    }
}

bool GrGLExtensions::init(GrGLStandard standard,
                          GrGLFunction<GrGLGetStringFn> getString,
                          GrGLFunction<GrGLGetStringiFn> getStringi,
                          GrGLFunction<GrGLGetIntegervFn> getIntegerv,
                          GrGLFunction<GrEGLQueryStringFn> queryString,
                          GrEGLDisplay eglDisplay) {
    fInitialized = false;
    fStrings.reset();

    if (!getString) {
        return false;
    }

    const GrGLubyte* verString = getString(GR_GL_VERSION);
    GrGLVersion version = GrGLGetVersionFromString((const char*) verString);
    if (GR_GL_INVALID_VER == version) {
        return false;
    }

    bool indexed = false;
    if (GR_IS_GR_GL(standard) || GR_IS_GR_GL_ES(standard)) {
        // glGetStringi and indexed extensions were added in version 3.0 of desktop GL and ES.
        indexed = version >= GR_GL_VER(3, 0);
    } else if (GR_IS_GR_WEBGL(standard)) {
        // WebGL (1.0 or 2.0) doesn't natively support glGetStringi, but enscripten adds it in
        // https://github.com/emscripten-core/emscripten/issues/3472
        indexed = version >= GR_GL_VER(2, 0);
    }

    if (indexed) {
        if (!getStringi || !getIntegerv) {
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
        if (!extensions) {
            return false;
        }
        eat_space_sep_strings(&fStrings, extensions);
    }
    if (queryString) {
        const char* extensions = queryString(eglDisplay, GR_EGL_EXTENSIONS);

        eat_space_sep_strings(&fStrings, extensions);
    }
    if (!fStrings.empty()) {
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fStrings.front(), &fStrings.back(), cmp);
    }
    fInitialized = true;
    return true;
}

bool GrGLExtensions::has(const char ext[]) const {
    SkASSERT(fInitialized);
    return find_string(fStrings, ext) >= 0;
}

bool GrGLExtensions::remove(const char ext[]) {
    SkASSERT(fInitialized);
    int idx = find_string(fStrings, ext);
    if (idx < 0) {
        return false;
    }

    // This is not terribly effecient but we really only expect this function to be called at
    // most a handful of times when our test programs start.
    fStrings.removeShuffle(idx);
    if (idx != fStrings.count()) {
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTInsertionSort(&(fStrings.operator[](idx)), &fStrings.back(), cmp);
    }
    return true;
}

void GrGLExtensions::add(const char ext[]) {
    int idx = find_string(fStrings, ext);
    if (idx < 0) {
        // This is not the most effecient approach since we end up looking at all of the
        // extensions after the add
        fStrings.emplace_back(ext);
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTInsertionSort(&fStrings.front(), &fStrings.back(), cmp);
    }
}

#ifdef SK_ENABLE_DUMP_GPU
void GrGLExtensions::dumpJSON(SkJSONWriter* writer) const {
    writer->beginArray();
    for (int i = 0; i < fStrings.count(); ++i) {
        writer->appendString(fStrings[i].c_str());
    }
    writer->endArray();
}
#else
void GrGLExtensions::dumpJSON(SkJSONWriter* writer) const { }
#endif

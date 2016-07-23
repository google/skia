/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLExtensions_DEFINED
#define GrGLExtensions_DEFINED

#include "../../private/SkTArray.h"
#include "GrGLFunctions.h"
#include "SkString.h"

struct GrGLInterface;

/**
 * This helper queries the current GL context for its extensions, remembers them, and can be
 * queried. It supports both glGetString- and glGetStringi-style extension string APIs and will
 * use the latter if it is available. It also will query for EGL extensions if a eglQueryString
 * implementation is provided.
 */
class SK_API GrGLExtensions {
public:
    GrGLExtensions() : fInitialized(false), fStrings(new SkTArray<SkString>) {}

    GrGLExtensions(const GrGLExtensions&);

    GrGLExtensions& operator=(const GrGLExtensions&);

    void swap(GrGLExtensions* that) {
        fStrings.swap(that->fStrings);
        SkTSwap(fInitialized, that->fInitialized);
    }

    /**
     * We sometimes need to use this class without having yet created a GrGLInterface. This version
     * of init expects that getString is always non-NULL while getIntegerv and getStringi are non-
     * NULL if on desktop GL with version 3.0 or higher. Otherwise it will fail.
     */
    bool init(GrGLStandard standard,
              GrGLFunction<GrGLGetStringProc> getString,
              GrGLFunction<GrGLGetStringiProc> getStringi,
              GrGLFunction<GrGLGetIntegervProc> getIntegerv,
              GrGLFunction<GrEGLQueryStringProc> queryString = nullptr,
              GrEGLDisplay eglDisplay = nullptr);

    bool isInitialized() const { return fInitialized; }

    /**
     * Queries whether an extension is present. This will fail if init() has not been called.
     */
    bool has(const char[]) const;

    /**
     * Removes an extension if present. Returns true if the extension was present before the call.
     */
    bool remove(const char[]);

    /**
     * Adds an extension to list
     */
    void add(const char[]);

    void reset() { fStrings->reset(); }

    void print(const char* sep = "\n") const;

private:
    bool                                fInitialized;
    SkAutoTDelete<SkTArray<SkString> >  fStrings;
};

#endif

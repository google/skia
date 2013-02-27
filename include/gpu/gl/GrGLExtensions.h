/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLExtensions_DEFINED
#define GrGLExtensions_DEFINED

#include "GrGLInterface.h"
#include "SkString.h"
#include "SkTArray.h"

/**
 * This helper queries the current GL context for its extensions, remembers them, and can be
 * queried. It supports both glGetString- and glGetStringi-style extension string APIs and will
 * use the latter if it is available.
 */
class GrGLExtensions {
public:
    bool init(GrGLBinding binding, const GrGLInterface* iface) {
        GrAssert(binding & iface->fBindingsExported);
        return this->init(binding, iface->fGetString, iface->fGetStringi, iface->fGetIntegerv);
    }
    /**
     * We sometimes need to use this class without having yet created a GrGLInterface. This version
     * of init expects that getString is always non-NULL while getIntegerv and getStringi are non-
     * NULL if on desktop GL with version 3.0 or higher. Otherwise it will fail.
     */
    bool init(GrGLBinding binding,
              GrGLGetStringProc getString,
              GrGLGetStringiProc getStringi,
              GrGLGetIntegervProc getIntegerv);

    /**
     * Queries whether an extension is present. This will fail if init() has not been called.
     */
    bool has(const char*) const;

    void reset() { fStrings.reset(); }

private:
    SkTArray<SkString> fStrings;
};

#endif

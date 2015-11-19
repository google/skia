/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAutoLocaleSetter_DEFINED
#define GrAutoLocaleSetter_DEFINED

#include "GrTypes.h"

#if !defined(SK_BUILD_FOR_ANDROID)
#include <locale.h>
#endif

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <xlocale.h>
#endif

/**
 * Helper class for ensuring that we don't use the wrong locale when building shaders. Android
 * doesn't support locale in the NDK, so this is a no-op there.
 */
class GrAutoLocaleSetter : public SkNoncopyable {
public:
    GrAutoLocaleSetter (const char* name) {
#if defined(SK_BUILD_FOR_WIN)
        fOldPerThreadLocale = _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
        fOldLocale = setlocale(LC_ALL, name);
#elif !defined(SK_BUILD_FOR_ANDROID) && !defined(__UCLIBC__)
        fLocale = newlocale(LC_ALL, name, 0);
        if (fLocale) {
            fOldLocale = uselocale(fLocale);
        } else {
            fOldLocale = static_cast<locale_t>(0);
        }
#else
        (void) name; // suppress unused param warning.
#endif
    }

    ~GrAutoLocaleSetter () {
#if defined(SK_BUILD_FOR_WIN)
        setlocale(LC_ALL, fOldLocale);
        _configthreadlocale(fOldPerThreadLocale);
#elif !defined(SK_BUILD_FOR_ANDROID) && !defined(__UCLIBC__)
        if (fLocale) {
             uselocale(fOldLocale);
             freelocale(fLocale);
        }
#endif
    }

private:
#if defined(SK_BUILD_FOR_WIN)
    int fOldPerThreadLocale;
    const char* fOldLocale;
#elif !defined(SK_BUILD_FOR_ANDROID) && !defined(__UCLIBC__)
    locale_t fOldLocale;
    locale_t fLocale;
#endif
};

#endif


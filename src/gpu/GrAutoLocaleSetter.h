/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAutoLocaleSetter_DEFINED
#define GrAutoLocaleSetter_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/private/SkNoncopyable.h"

#if defined(SK_BUILD_FOR_WIN)
#include "include/core/SkString.h"
#endif

#if !defined(SK_BUILD_FOR_ANDROID)
#include <locale.h>
#endif

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <xlocale.h>
#include <cstring>
#define HAVE_XLOCALE 1
#else
#define HAVE_XLOCALE 0
#endif

#if defined(SK_BUILD_FOR_ANDROID) || defined(__UCLIBC__) || defined(_NEWLIB_VERSION)
#define HAVE_LOCALE_T 0
#else
#define HAVE_LOCALE_T 1
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
        char* oldLocale = setlocale(LC_ALL, name);
        if (oldLocale) {
            fOldLocale = oldLocale;
            fShouldRestoreLocale = true;
        } else {
            fShouldRestoreLocale = false;
        }
#elif HAVE_LOCALE_T
#if HAVE_XLOCALE
        // In xlocale nullptr means the C locale.
        if (0 == strcmp(name, "C")) {
            name = nullptr;
        }
#endif
        fLocale = newlocale(LC_ALL_MASK, name, nullptr);
        if (fLocale) {
            fOldLocale = uselocale(fLocale);
        } else {
            fOldLocale = static_cast<locale_t>(nullptr);
        }
#else
        (void) name; // suppress unused param warning.
#endif
    }

    ~GrAutoLocaleSetter () {
#if defined(SK_BUILD_FOR_WIN)
        if (fShouldRestoreLocale) {
            setlocale(LC_ALL, fOldLocale.c_str());
        }
        _configthreadlocale(fOldPerThreadLocale);
#elif HAVE_LOCALE_T
        if (fLocale) {
             uselocale(fOldLocale);
             freelocale(fLocale);
        }
#endif
    }

private:
#if defined(SK_BUILD_FOR_WIN)
    int fOldPerThreadLocale;
    bool fShouldRestoreLocale;
    SkString fOldLocale;
#elif HAVE_LOCALE_T
    locale_t fOldLocale;
    locale_t fLocale;
#endif
};

#undef HAVE_LOCALE_T
#undef HAVE_XLOCALE

#endif

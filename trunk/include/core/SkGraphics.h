
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkGraphics_DEFINED
#define SkGraphics_DEFINED

#include "SkTypes.h"

class SK_API SkGraphics {
public:
    /**
     *  Call this at process initialization time if your environment does not
     *  permit static global initializers that execute code. Note that
     *  Init() is not thread-safe.
     */
    static void Init();

    /**
     *  Call this to release any memory held privately, such as the font cache.
     */
    static void Term();

    /**
     *  Return the version numbers for the library. If the parameter is not
     *  null, it is set to the version number.
     */
    static void GetVersion(int32_t* major, int32_t* minor, int32_t* patch);

    /**
     *  Return the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *  This max can be changed by calling SetFontCacheLimit().
     */
    static size_t GetFontCacheLimit();

    /**
     *  Specify the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *
     *  This function returns the previous setting, as if GetFontCacheLimit()
     *  had be called before the new limit was set.
     */
    static size_t SetFontCacheLimit(size_t bytes);

    /**
     *  Return the number of bytes currently used by the font cache.
     */
    static size_t GetFontCacheUsed();

    /**
     *  For debugging purposes, this will attempt to purge the font cache. It
     *  does not change the limit, but will cause subsequent font measures and
     *  draws to be recreated, since they will no longer be in the cache.
     */
    static void PurgeFontCache();

    /**
     *  Applications with command line options may pass optional state, such
     *  as cache sizes, here, for instance:
     *  font-cache-limit=12345678
     *
     *  The flags format is name=value[;name=value...] with no spaces.
     *  This format is subject to change.
     */
    static void SetFlags(const char* flags);

    /**
     *  Return the max number of bytes that should be used by the thread-local
     *  font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *  This max can be changed by calling SetFontCacheLimit().
     *
     *  If this thread has never called SetTLSFontCacheLimit, or has called it
     *  with 0, then this thread is using the shared font cache. In that case,
     *  this function will always return 0, and the caller may want to call
     *  GetFontCacheLimit.
     */
    static size_t GetTLSFontCacheLimit();

    /**
     *  Specify the max number of bytes that should be used by the thread-local
     *  font cache. If this value is 0, then this thread will use the shared
     *  global font cache.
     */
    static void SetTLSFontCacheLimit(size_t bytes);

private:
    /** This is automatically called by SkGraphics::Init(), and must be
        implemented by the host OS. This allows the host OS to register a callback
        with the C++ runtime to call SkGraphics::FreeCaches()
    */
    static void InstallNewHandler();
};

class SkAutoGraphics {
public:
    SkAutoGraphics() {
        SkGraphics::Init();
    }
    ~SkAutoGraphics() {
        SkGraphics::Term();
    }
};

#endif


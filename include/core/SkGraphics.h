
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkGraphics_DEFINED
#define SkGraphics_DEFINED

#include "SkTypes.h"

class SkGraphics {
public:
    static void Init();
    static void Term();

    /** Return the (approximate) number of bytes used by the font cache.
    */
    static size_t GetFontCacheUsed();
    
    /** Attempt to purge the font cache until <= the specified amount remains
        in the cache. Specifying 0 will attempt to purge the entire cache.
        Returns true if some amount was purged from the font cache.
    */
    static bool SetFontCacheUsed(size_t usageInBytes);

    /** Return the version numbers for the library. If the parameter is not
        null, it is set to the version number.
     */
    static void GetVersion(int32_t* major, int32_t* minor, int32_t* patch);

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



/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



/*
 ** Mac Text API
 **
 **
 ** Two text APIs are available on the Mac, ATSUI and CoreText.
 **
 ** ATSUI is available on all versions of Mac OS X, but is 32-bit only.
 **
 ** The replacement API, CoreText, supports both 32-bit and 64-bit builds
 ** but is only available from Mac OS X 10.5 onwards.
 **
 ** To maintain support for Mac OS X 10.4, we default to ATSUI in 32-bit
 ** builds unless SK_USE_CORETEXT is defined.
*/
#ifndef SK_USE_CORETEXT
    #if TARGET_RT_64_BIT || defined(SK_USE_MAC_CORE_TEXT)
        #define SK_USE_CORETEXT                                     1
    #else
        #define SK_USE_CORETEXT                                     0
    #endif
#endif

#if SK_USE_CORETEXT
    #include "SkFontHost_mac_coretext.cpp"
#else
    #include "SkFontHost_mac_atsui.cpp"
#endif



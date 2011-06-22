/*
 ** Copyright 2006, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License"); 
 ** you may not use this file except in compliance with the License. 
 ** You may obtain a copy of the License at 
 **
 **     http://www.apache.org/licenses/LICENSE-2.0 
 **
 ** Unless required by applicable law or agreed to in writing, software 
 ** distributed under the License is distributed on an "AS IS" BASIS, 
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 ** See the License for the specific language governing permissions and 
 ** limitations under the License.
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



/* include/corecg/SkUserConfig.h
**
** Copyright 2006, Google Inc.
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

#ifndef SkUserConfig_DEFINED
#define SkUserConfig_DEFINED

/*  This file is included before all other headers, except for SkPreConfig.h.
    That file uses various heuristics to make a "best guess" at settings for
    the following build defines.

    However, in this file you can override any of those decisions by either
    defining new symbols, or #undef symbols that were already set.
*/

// experimental for now
#define SK_SUPPORT_MIPMAP

// android specific defines and tests

#ifdef SK_FORCE_SCALARFIXED
    #define SK_SCALAR_IS_FIXED
    #undef SK_SCALAR_IS_FLOAT
    #undef SK_CAN_USE_FLOAT
#endif

#ifdef SK_FORCE_SCALARFLOAT
    #define SK_SCALAR_IS_FLOAT
    #define SK_CAN_USE_FLOAT
    #undef SK_SCALAR_IS_FIXED
#endif

#ifdef ANDROID
    #include <utils/misc.h>

    #if __BYTE_ORDER == __BIG_ENDIAN
        #define SK_CPU_BENDIAN
        #undef  SK_CPU_LENDIAN
    #else
        #define SK_CPU_LENDIAN
        #undef  SK_CPU_BENDIAN
    #endif
#endif

#ifdef SK_DEBUG
    #define SK_SUPPORT_UNITTEST
    /* Define SK_SIMULATE_FAILED_MALLOC to have
     * sk_malloc throw an exception. Use this to
     * detect unhandled memory leaks. */
    //#define SK_SIMULATE_FAILED_MALLOC
    //#define SK_FIND_MEMORY_LEAKS
#endif

#ifdef SK_BUILD_FOR_BREW
    #include "SkBrewUserConfig.h"
#endif

#ifdef SK_BUILD_FOR_MAC
    #define SK_CAN_USE_FLOAT
#endif

#endif


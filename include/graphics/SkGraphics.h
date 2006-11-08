/* include/graphics/SkGraphics.h
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

#ifndef SkGraphics_DEFINED
#define SkGraphics_DEFINED

#include "SkTypes.h"

class SkGraphics {
public:
    static void Init(bool runUnitTests);
    static void Term();

    /** Call this if the heap that the graphics engine uses is low on memory.
        It will attempt to free some of its caches. Returns true if it was
        able to, or false if it could do nothing.

        This may be called from any thread, and guarantees not to call
        new or sk_malloc (though it will hopefully call delete and/or sk_free).
        It also will never throw an exception.
    */
    static bool FreeCaches(size_t bytesNeeded);

private:
    /** This is automatically called by SkGraphics::Init(), and must be
        implemented by the host OS. This allows the host OS to register a callback
        with the C++ runtime to call SkGraphics::FreeCaches()
    */
    static void InstallNewHandler();
};

#endif


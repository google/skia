/* libs/corecg/SkDebug_brew.cpp
**
** Copyright 2009, The Android Open Source Project
** Copyright 2009, Company 100, Inc.
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

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_BREW

static const size_t kBufferSize = 256;

#include <AEEStdLib.h>
#include <stdarg.h>

void SkDebugf(const char format[], ...) {
    char    buffer[kBufferSize + 1];
    va_list args;
    va_start(args, format);
    VSNPRINTF(buffer, kBufferSize, format, args);
    va_end(args);
    DBGPRINTF(buffer);
}

#endif SK_BUILD_FOR_BREW

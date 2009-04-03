/* libs/corecg/SkDebug_stdio.cpp
**
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

#include "SkTypes.h"

static const size_t kBufferSize = 256;

#define LOG_TAG "skia"
#include <utils/Log.h>

void Android_SkDebugf(const char* file, int line, const char* function, 
    const char* format, ...)
{
    if (format[0] == '\n' && format[1] == '\0')
        return;
    va_list args;
    va_start(args, format);
#ifdef HAVE_ANDROID_OS
    char buffer[kBufferSize + 1];
    vsnprintf(buffer, kBufferSize, format, args);
    if (buffer[0] != 0)
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", buffer);
#else
    android_vprintLog(ANDROID_LOG_DEBUG, NULL, LOG_TAG, format, args);
#endif
    va_end(args);
}



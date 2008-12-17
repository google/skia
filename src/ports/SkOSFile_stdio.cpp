/* libs/graphics/ports/SkOSFile_stdio.cpp
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

#include "SkOSFile.h"

#ifndef SK_BUILD_FOR_BREW

#include <stdio.h>
#include <errno.h>

SkFILE* sk_fopen(const char path[], SkFILE_Flags flags)
{
    char    perm[4];
    char*   p = perm;

    if (flags & kRead_SkFILE_Flag)
        *p++ = 'r';
    if (flags & kWrite_SkFILE_Flag)
        *p++ = 'w';
    *p++ = 'b';
    *p = 0;

    SkFILE* f = (SkFILE*)::fopen(path, perm);
#if 0
    if (NULL == f)
        SkDebugf("sk_fopen failed for %s (%s), errno=%s\n", path, perm, strerror(errno));
#endif
    return f;
}

size_t sk_fgetsize(SkFILE* f)
{
    SkASSERT(f);

    size_t  curr = ::ftell((FILE*)f);       // remember where we are
    ::fseek((FILE*)f, 0, SEEK_END);         // go to the end
    size_t size = ::ftell((FILE*)f);        // record the size
    ::fseek((FILE*)f, (long)curr, SEEK_SET);        // go back to our prev loc
    return size;
}

bool sk_frewind(SkFILE* f)
{
    SkASSERT(f);
    ::rewind((FILE*)f);
//  ::fseek((FILE*)f, 0, SEEK_SET);
    return true;
}

size_t sk_fread(void* buffer, size_t byteCount, SkFILE* f)
{
    SkASSERT(f);
    if (buffer == NULL)
    {
        size_t curr = ::ftell((FILE*)f);
        if ((long)curr == -1) {
            SkDEBUGF(("sk_fread: ftell(%p) returned -1 feof:%d ferror:%d\n", f, feof((FILE*)f), ferror((FILE*)f)));
            return 0;
        }
    //  ::fseek((FILE*)f, (long)(curr + byteCount), SEEK_SET);
        int err = ::fseek((FILE*)f, (long)byteCount, SEEK_CUR);
        if (err != 0) {
            SkDEBUGF(("sk_fread: fseek(%d) tell:%d failed with feof:%d ferror:%d returned:%d\n",
                        byteCount, curr, feof((FILE*)f), ferror((FILE*)f), err));
            return 0;
        }
        return byteCount;
    }
    else
        return ::fread(buffer, 1, byteCount, (FILE*)f);
}

size_t sk_fwrite(const void* buffer, size_t byteCount, SkFILE* f)
{
    SkASSERT(f);
    return ::fwrite(buffer, 1, byteCount, (FILE*)f);
}

void sk_fflush(SkFILE* f)
{
    SkASSERT(f);
    ::fflush((FILE*)f);
}

void sk_fclose(SkFILE* f)
{
    SkASSERT(f);
    ::fclose((FILE*)f);
}

#endif


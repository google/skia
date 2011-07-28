/* libs/graphics/ports/SkOSFile_brew.cpp
 *
 * Copyright 2006, The Android Open Source Project
 * Copyright 2009, Company 100, Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkOSFile.h"

#ifdef SK_BUILD_FOR_BREW

#include <AEEAppGen.h>
#include <AEEFile.h>
#include <AEEStdLib.h>

SkFILE* sk_fopen(const char path[], SkFILE_Flags flags)
{
    int err;
    OpenFileMode mode;
    IFileMgr* fileMgr;
    IFile* file;
    IShell* shell;

    shell = reinterpret_cast<AEEApplet*>(GETAPPINSTANCE())->m_pIShell;
    err = ISHELL_CreateInstance(shell, AEECLSID_FILEMGR, (void**)&fileMgr);
    if (err!= SUCCESS)
        return NULL;

    if (flags & kWrite_SkFILE_Flag)
        mode = _OFM_READWRITE;
    else /* kRead_SkFILE_Flag */
        mode = _OFM_READ;

    file = IFILEMGR_OpenFile(fileMgr, path, mode);
    IFILEMGR_Release(fileMgr);

    return (SkFILE*)file;
}

size_t sk_fgetsize(SkFILE* f)
{
    FileInfo fileInfo;

    IFILE_GetInfo((IFile*)f, &fileInfo);
    return fileInfo.dwSize;
}

bool sk_frewind(SkFILE* f)
{
    SkASSERT(f);
    return IFILE_Seek((IFile*)f,  _SEEK_START, 0) == SUCCESS;
}

size_t sk_fread(void* buffer, size_t byteCount, SkFILE* f)
{
    SkASSERT(f);
    if (buffer == NULL)
    {
        int err = IFILE_Seek((IFile*)f, _SEEK_CURRENT, (int)byteCount);
        if (err == EFAILED) {
            SkDEBUGF(("sk_fread: IFILE_Seek(%d) failed returned:%d\n", byteCount, err));
            return 0;
        }
        return byteCount;
    }
    else
        return IFILE_Read((IFile*)f, buffer, byteCount);
}

size_t sk_fwrite(const void* buffer, size_t byteCount, SkFILE* f)
{
    SkASSERT(f);
    return IFILE_Write((IFile*)f, buffer, byteCount);
}

void sk_fflush(SkFILE* f)
{
    SkASSERT(f);
}

void sk_fclose(SkFILE* f)
{
    SkASSERT(f);
    IFILE_Release((IFile*)f);
}

#endif

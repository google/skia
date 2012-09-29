
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMMapStream.h"

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

SkMMAPStream::SkMMAPStream(const char filename[])
{
    fAddr = NULL;   // initialize to failure case
    fSize = 0;

    int fildes = open(filename, O_RDONLY);
    if (fildes < 0)
    {
        SkDEBUGF(("---- failed to open(%s) for mmap stream error=%d\n", filename, errno));
        return;
    }

    off_t offset = lseek(fildes, 0, SEEK_END);    // find the file size
    if (offset == -1)
    {
        SkDEBUGF(("---- failed to lseek(%s) for mmap stream error=%d\n", filename, errno));
        close(fildes);
        return;
    }
    (void)lseek(fildes, 0, SEEK_SET);   // restore file offset to beginning

    // to avoid a 64bit->32bit warning, I explicitly create a size_t size
    size_t size = static_cast<size_t>(offset);

    void* addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fildes, 0);

    // According to the POSIX documentation of mmap it adds an extra reference
    // to the file associated with the fildes which is not removed by a
    // subsequent close() on that fildes. This reference is removed when there
    // are no more mappings to the file.
    close(fildes);

    if (MAP_FAILED == addr)
    {
        SkDEBUGF(("---- failed to mmap(%s) for mmap stream error=%d\n", filename, errno));
        return;
    }

    this->INHERITED::setMemory(addr, size);

    fAddr = addr;
    fSize = size;
}

SkMMAPStream::~SkMMAPStream()
{
    this->closeMMap();
}

void SkMMAPStream::setMemory(const void* data, size_t length, bool copyData)
{
    this->closeMMap();
    this->INHERITED::setMemory(data, length, copyData);
}

void SkMMAPStream::closeMMap()
{
    if (fAddr)
    {
        munmap(fAddr, fSize);
        fAddr = NULL;
    }
}


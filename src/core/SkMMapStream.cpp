#include "SkMMapStream.h"

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

SkMMAPStream::SkMMAPStream(const char filename[])
{
    fFildes = -1;   // initialize to failure case

    int fildes = open(filename, O_RDONLY);
    if (fildes < 0)
    {
        SkDEBUGF(("---- failed to open(%s) for mmap stream error=%d\n", filename, errno));
        return;
    }

    off_t size = lseek(fildes, 0, SEEK_END);    // find the file size
    if (size == -1)
    {
        SkDEBUGF(("---- failed to lseek(%s) for mmap stream error=%d\n", filename, errno));
        close(fildes);
        return;
    }
    (void)lseek(fildes, 0, SEEK_SET);   // restore file offset to beginning

    void* addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fildes, 0);
    if (MAP_FAILED == addr)
    {
        SkDEBUGF(("---- failed to mmap(%s) for mmap stream error=%d\n", filename, errno));
        close(fildes);
        return;
    }

    this->INHERITED::setMemory(addr, size);

    fFildes = fildes;
    fAddr = addr;
    fSize = size;
}

SkMMAPStream::~SkMMAPStream()
{
    this->closeMMap();
}

void SkMMAPStream::setMemory(const void* data, size_t length)
{
    this->closeMMap();
    this->INHERITED::setMemory(data, length);
}

void SkMMAPStream::closeMMap()
{
    if (fFildes >= 0)
    {
        munmap(fAddr, fSize);
        close(fFildes);
        fFildes = -1;
    }
}


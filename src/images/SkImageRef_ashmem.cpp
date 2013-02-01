
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkImageRef_ashmem.h"
#include "SkImageDecoder.h"
#include "SkFlattenableBuffers.h"
#include "SkThread.h"

#include "android/ashmem.h"

#include <sys/mman.h>
#include <unistd.h>

//#define TRACE_ASH_PURGE     // just trace purges

#ifdef DUMP_IMAGEREF_LIFECYCLE
    #define DUMP_ASHMEM_LIFECYCLE
#else
//    #define DUMP_ASHMEM_LIFECYCLE
#endif

// ashmem likes lengths on page boundaries
static size_t roundToPageSize(size_t size) {
    const size_t mask = getpagesize() - 1;
    size_t newsize = (size + mask) & ~mask;
//    SkDebugf("---- oldsize %d newsize %d\n", size, newsize);
    return newsize;
}

SkImageRef_ashmem::SkImageRef_ashmem(SkStream* stream,
                                             SkBitmap::Config config,
                                             int sampleSize)
        : SkImageRef(stream, config, sampleSize) {

    fRec.fFD = -1;
    fRec.fAddr = NULL;
    fRec.fSize = 0;
    fRec.fPinned = false;

    fCT = NULL;
}

SkImageRef_ashmem::~SkImageRef_ashmem() {
    SkSafeUnref(fCT);
    this->closeFD();
}

void SkImageRef_ashmem::closeFD() {
    if (-1 != fRec.fFD) {
#ifdef DUMP_ASHMEM_LIFECYCLE
        SkDebugf("=== ashmem close %d\n", fRec.fFD);
#endif
        SkASSERT(fRec.fAddr);
        SkASSERT(fRec.fSize);
        munmap(fRec.fAddr, fRec.fSize);
        close(fRec.fFD);
        fRec.fFD = -1;
    }
}

///////////////////////////////////////////////////////////////////////////////

class AshmemAllocator : public SkBitmap::Allocator {
public:
    AshmemAllocator(SkAshmemRec* rec, const char name[])
        : fRec(rec), fName(name) {}

    virtual bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) {
        const size_t size = roundToPageSize(bm->getSize());
        int fd = fRec->fFD;
        void* addr = fRec->fAddr;

        SkASSERT(!fRec->fPinned);

        if (-1 == fd) {
            SkASSERT(NULL == addr);
            SkASSERT(0 == fRec->fSize);

            fd = ashmem_create_region(fName, size);
#ifdef DUMP_ASHMEM_LIFECYCLE
            SkDebugf("=== ashmem_create_region %s size=%d fd=%d\n", fName, size, fd);
#endif
            if (-1 == fd) {
                SkDebugf("------- imageref_ashmem create failed <%s> %d\n",
                         fName, size);
                return false;
            }

            int err = ashmem_set_prot_region(fd, PROT_READ | PROT_WRITE);
            if (err) {
                SkDebugf("------ ashmem_set_prot_region(%d) failed %d\n",
                         fd, err);
                close(fd);
                return false;
            }

            addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
            if (-1 == (long)addr) {
                SkDebugf("---------- mmap failed for imageref_ashmem size=%d\n",
                         size);
                close(fd);
                return false;
            }

            fRec->fFD = fd;
            fRec->fAddr = addr;
            fRec->fSize = size;
        } else {
            SkASSERT(addr);
            SkASSERT(size == fRec->fSize);
            (void)ashmem_pin_region(fd, 0, 0);
        }

        bm->setPixels(addr, ct);
        fRec->fPinned = true;
        return true;
    }

private:
    // we just point to our caller's memory, these are not copies
    SkAshmemRec* fRec;
    const char*  fName;
};

bool SkImageRef_ashmem::onDecode(SkImageDecoder* codec, SkStream* stream,
                                 SkBitmap* bitmap, SkBitmap::Config config,
                                 SkImageDecoder::Mode mode) {

    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return this->INHERITED::onDecode(codec, stream, bitmap, config, mode);
    }

    AshmemAllocator alloc(&fRec, this->getURI());

    codec->setAllocator(&alloc);
    bool success = this->INHERITED::onDecode(codec, stream, bitmap, config,
                                             mode);
    // remove the allocator, since its on the stack
    codec->setAllocator(NULL);

    if (success) {
        // remember the colortable (if any)
        SkRefCnt_SafeAssign(fCT, bitmap->getColorTable());
        return true;
    } else {
        if (fRec.fPinned) {
            ashmem_unpin_region(fRec.fFD, 0, 0);
            fRec.fPinned = false;
        }
        this->closeFD();
        return false;
    }
}

void* SkImageRef_ashmem::onLockPixels(SkColorTable** ct) {
    SkASSERT(fBitmap.getPixels() == NULL);
    SkASSERT(fBitmap.getColorTable() == NULL);

    // fast case: check if we can just pin and get the cached data
    if (-1 != fRec.fFD) {
        SkASSERT(fRec.fAddr);
        SkASSERT(!fRec.fPinned);
        int pin = ashmem_pin_region(fRec.fFD, 0, 0);

        if (ASHMEM_NOT_PURGED == pin) { // yea, fast case!
            fBitmap.setPixels(fRec.fAddr, fCT);
            fRec.fPinned = true;
        } else if (ASHMEM_WAS_PURGED == pin) {
            ashmem_unpin_region(fRec.fFD, 0, 0);
            // let go of our colortable if we lost the pixels. Well get it back
            // again when we re-decode
            if (fCT) {
                fCT->unref();
                fCT = NULL;
            }
#if defined(DUMP_ASHMEM_LIFECYCLE) || defined(TRACE_ASH_PURGE)
            SkDebugf("===== ashmem purged %d\n", fBitmap.getSize());
#endif
        } else {
            SkDebugf("===== ashmem pin_region(%d) returned %d\n", fRec.fFD, pin);
            // return null result for failure
            if (ct) {
                *ct = NULL;
            }
            return NULL;
        }
    } else {
        // no FD, will create an ashmem region in allocator
    }

    return this->INHERITED::onLockPixels(ct);
}

void SkImageRef_ashmem::onUnlockPixels() {
    this->INHERITED::onUnlockPixels();

    if (-1 != fRec.fFD) {
        SkASSERT(fRec.fAddr);
        SkASSERT(fRec.fPinned);

        ashmem_unpin_region(fRec.fFD, 0, 0);
        fRec.fPinned = false;
    }

    // we clear this with or without an error, since we've either closed or
    // unpinned the region
    fBitmap.setPixels(NULL, NULL);
}

void SkImageRef_ashmem::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeString(getURI());
}

SkImageRef_ashmem::SkImageRef_ashmem(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
    fRec.fFD = -1;
    fRec.fAddr = NULL;
    fRec.fSize = 0;
    fRec.fPinned = false;
    fCT = NULL;
    char* uri = buffer.readString();
    if (uri) {
        setURI(uri);
        sk_free(uri);
    }
}

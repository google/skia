/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkMallocPixelRef.h"
#include "Test.h"

static void delete_uint8_proc(void* ptr, void*) {
    delete[] static_cast<uint8_t*>(ptr);
}

static void set_to_one_proc(void*, void* context) {
    *(static_cast<int*>(context)) = 1;
}

/**
 *  This test contains basic sanity checks concerning SkMallocPixelRef.
 */
DEF_TEST(MallocPixelRef, reporter) {
    REPORTER_ASSERT(reporter, true);
    SkImageInfo info = SkImageInfo::MakeN32Premul(10, 13);
    {
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewAllocate(info, info.minRowBytes() - 1, NULL));
        // rowbytes too small.
        REPORTER_ASSERT(reporter, NULL == pr.get());
    }
    {
        size_t rowBytes = info.minRowBytes() - 1;
        size_t size = info.getSafeSize(rowBytes);
        SkAutoDataUnref data(SkData::NewUninitialized(size));
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewWithData(info, rowBytes, NULL, data));
        // rowbytes too small.
        REPORTER_ASSERT(reporter, NULL == pr.get());
    }
    {
        size_t rowBytes = info.minRowBytes() + 2;
        size_t size = info.getSafeSize(rowBytes) - 1;
        SkAutoDataUnref data(SkData::NewUninitialized(size));
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewWithData(info, rowBytes, NULL, data));
        // data too small.
        REPORTER_ASSERT(reporter, NULL == pr.get());
    }
    size_t rowBytes = info.minRowBytes() + 7;
    size_t size = info.getSafeSize(rowBytes) + 9;
    {
        SkAutoMalloc memory(size);
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewDirect(info, memory.get(), rowBytes, NULL));
        REPORTER_ASSERT(reporter, pr.get() != NULL);
        REPORTER_ASSERT(reporter, memory.get() == pr->pixels());
    }
    {
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewAllocate(info, rowBytes, NULL));
        REPORTER_ASSERT(reporter, pr.get() != NULL);
        REPORTER_ASSERT(reporter, pr->pixels());
    }
    {
        void* addr = static_cast<void*>(new uint8_t[size]);
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewWithProc(info, rowBytes, NULL, addr,
                                          delete_uint8_proc, NULL));
        REPORTER_ASSERT(reporter, pr.get() != NULL);
        REPORTER_ASSERT(reporter, addr == pr->pixels());
    }
    {
        int x = 0;
        SkAutoMalloc memory(size);
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewWithProc(info, rowBytes, NULL,
                                          memory.get(), set_to_one_proc,
                                          static_cast<void*>(&x)));
        REPORTER_ASSERT(reporter, pr.get() != NULL);
        REPORTER_ASSERT(reporter, memory.get() == pr->pixels());
        REPORTER_ASSERT(reporter, 0 == x);
        pr.reset(NULL);
        // make sure that set_to_one_proc was called.
        REPORTER_ASSERT(reporter, 1 == x);
    }
    {
        void* addr = static_cast<void*>(new uint8_t[size]);
        REPORTER_ASSERT(reporter, addr != NULL);
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewWithProc(info, rowBytes, NULL, addr,
                                          delete_uint8_proc, NULL));
        REPORTER_ASSERT(reporter, addr == pr->pixels());
    }
    {
        SkAutoDataUnref data(SkData::NewUninitialized(size));
        SkData* dataPtr = data.get();
        REPORTER_ASSERT(reporter, dataPtr->unique());
        SkAutoTUnref<SkMallocPixelRef> pr(
            SkMallocPixelRef::NewWithData(info, rowBytes, NULL, data.get()));
        REPORTER_ASSERT(reporter, !(dataPtr->unique()));
        data.reset(NULL);
        REPORTER_ASSERT(reporter, dataPtr->unique());
        REPORTER_ASSERT(reporter, dataPtr->data() == pr->pixels());
    }
}

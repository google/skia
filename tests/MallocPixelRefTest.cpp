/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
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
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeAllocate(info, info.minRowBytes() - 1));
        // rowbytes too small.
        REPORTER_ASSERT(reporter, nullptr == pr.get());
    }
    {
        size_t rowBytes = info.minRowBytes() - 1;
        size_t size = info.getSafeSize(rowBytes);
        sk_sp<SkData> data(SkData::MakeUninitialized(size));
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithData(info, rowBytes, data));
        // rowbytes too small.
        REPORTER_ASSERT(reporter, nullptr == pr.get());
    }
    {
        size_t rowBytes = info.minRowBytes() + 2;
        size_t size = info.getSafeSize(rowBytes) - 1;
        sk_sp<SkData> data(SkData::MakeUninitialized(size));
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithData(info, rowBytes, data));
        // data too small.
        REPORTER_ASSERT(reporter, nullptr == pr.get());
    }
    size_t rowBytes = info.minRowBytes() + 7;
    size_t size = info.getSafeSize(rowBytes) + 9;
    {
        SkAutoMalloc memory(size);
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeDirect(info, memory.get(), rowBytes));
        REPORTER_ASSERT(reporter, pr.get() != nullptr);
        REPORTER_ASSERT(reporter, memory.get() == pr->pixels());
    }
    {
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeAllocate(info, rowBytes));
        REPORTER_ASSERT(reporter, pr.get() != nullptr);
        REPORTER_ASSERT(reporter, pr->pixels());
    }
    {
        void* addr = static_cast<void*>(new uint8_t[size]);
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithProc(info, rowBytes, addr, delete_uint8_proc, nullptr));
        REPORTER_ASSERT(reporter, pr.get() != nullptr);
        REPORTER_ASSERT(reporter, addr == pr->pixels());
    }
    {
        int x = 0;
        SkAutoMalloc memory(size);
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithProc(info, rowBytes,
                                           memory.get(), set_to_one_proc,
                                           static_cast<void*>(&x)));
        REPORTER_ASSERT(reporter, pr.get() != nullptr);
        REPORTER_ASSERT(reporter, memory.get() == pr->pixels());
        REPORTER_ASSERT(reporter, 0 == x);
        pr.reset(nullptr);
        // make sure that set_to_one_proc was called.
        REPORTER_ASSERT(reporter, 1 == x);
    }
    {
        int x = 0;
        SkAutoMalloc memory(size);
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithProc(SkImageInfo::MakeN32Premul(-1, -1), rowBytes,
                                           memory.get(), set_to_one_proc,
                                           static_cast<void*>(&x)));
        REPORTER_ASSERT(reporter, pr.get() == nullptr);
        // make sure that set_to_one_proc was called.
        REPORTER_ASSERT(reporter, 1 == x);
    }
    {
        void* addr = static_cast<void*>(new uint8_t[size]);
        REPORTER_ASSERT(reporter, addr != nullptr);
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithProc(info, rowBytes, addr,
                                           delete_uint8_proc, nullptr));
        REPORTER_ASSERT(reporter, addr == pr->pixels());
    }
    {
        sk_sp<SkData> data(SkData::MakeUninitialized(size));
        SkData* dataPtr = data.get();
        REPORTER_ASSERT(reporter, dataPtr->unique());
        sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeWithData(info, rowBytes, data);
        REPORTER_ASSERT(reporter, !(dataPtr->unique()));
        data.reset(nullptr);
        REPORTER_ASSERT(reporter, dataPtr->unique());
        REPORTER_ASSERT(reporter, dataPtr->data() == pr->pixels());
    }
}

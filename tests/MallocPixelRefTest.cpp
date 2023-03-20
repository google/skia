/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRefCnt.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkPixelRefPriv.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>

static void delete_uint8_proc(void* ptr, void*) {
    delete[] static_cast<uint8_t*>(ptr);
}

static void set_to_one_proc(void*, void* context) {
    *(static_cast<int*>(context)) = 1;
}

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
        size_t size = info.computeByteSize(rowBytes);
        sk_sp<SkData> data(SkData::MakeUninitialized(size));
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithData(info, rowBytes, data));
        // rowbytes too small.
        REPORTER_ASSERT(reporter, nullptr == pr.get());
    }
    {
        size_t rowBytes = info.minRowBytes() + info.bytesPerPixel();
        size_t size = info.computeByteSize(rowBytes) - 1;
        sk_sp<SkData> data(SkData::MakeUninitialized(size));
        sk_sp<SkPixelRef> pr(
            SkMallocPixelRef::MakeWithData(info, rowBytes, data));
        // data too small.
        REPORTER_ASSERT(reporter, nullptr == pr.get());
    }
    size_t rowBytes = info.minRowBytes() + info.bytesPerPixel();
    size_t size = info.computeByteSize(rowBytes) + 9;
    {
        SkAutoMalloc memory(size);
        auto pr = sk_make_sp<SkPixelRef>(info.width(), info.height(), memory.get(), rowBytes);
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
            SkMakePixelRefWithProc(info.width(), info.height(), rowBytes, addr, delete_uint8_proc,
                                   nullptr));
        REPORTER_ASSERT(reporter, pr.get() != nullptr);
        REPORTER_ASSERT(reporter, addr == pr->pixels());
    }
    {
        int x = 0;
        SkAutoMalloc memory(size);
        sk_sp<SkPixelRef> pr(
            SkMakePixelRefWithProc(info.width(), info.height(), rowBytes, memory.get(),
                                   set_to_one_proc, static_cast<void*>(&x)));
        REPORTER_ASSERT(reporter, pr.get() != nullptr);
        REPORTER_ASSERT(reporter, memory.get() == pr->pixels());
        REPORTER_ASSERT(reporter, 0 == x);
        pr.reset(nullptr);
        // make sure that set_to_one_proc was called.
        REPORTER_ASSERT(reporter, 1 == x);
    }
    {
        void* addr = static_cast<void*>(new uint8_t[size]);
        REPORTER_ASSERT(reporter, addr != nullptr);
        sk_sp<SkPixelRef> pr(
            SkMakePixelRefWithProc(info.width(), info.height(), rowBytes, addr, delete_uint8_proc,
                                   nullptr));
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

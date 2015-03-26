/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapHeap.h"
#include "SkColor.h"
#include "SkFlattenable.h"
#include "SkWriteBuffer.h"
#include "SkPictureFlat.h"
#include "SkRefCnt.h"
#include "SkShader.h"
#include "Test.h"

struct SimpleFlatController : public SkFlatController {
    SimpleFlatController() : SkFlatController() {}
    ~SimpleFlatController() { fAllocations.freeAll(); }
    void* allocThrow(size_t bytes) override {
        fAllocations.push(sk_malloc_throw(bytes));
        return fAllocations.top();
    }
    void unalloc(void*) override { }
    void setBitmapStorage(SkBitmapHeap* h) { this->setBitmapHeap(h); }
private:
    SkTDArray<void*> fAllocations;
};

struct SkShaderTraits {
    static void Flatten(SkWriteBuffer& buffer, const SkShader& shader) {
        buffer.writeFlattenable(&shader);
    }
};
typedef SkFlatDictionary<SkShader, SkShaderTraits> FlatDictionary;

class SkBitmapHeapTester {
public:
    static int32_t GetRefCount(const SkBitmapHeapEntry* entry) {
        return entry->fRefCount;
    }
};

DEF_TEST(BitmapHeap, reporter) {
    // Create a bitmap shader.
    SkBitmap bm;
    bm.allocN32Pixels(2, 2);
    bm.eraseColor(SK_ColorRED);
    uint32_t* pixel = bm.getAddr32(1,0);
    *pixel = SK_ColorBLUE;

    SkShader* bitmapShader = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                                          SkShader::kRepeat_TileMode);
    SkAutoTUnref<SkShader> aur(bitmapShader);

    // Flatten, storing it in the bitmap heap.
    SkBitmapHeap heap(1, 1);
    SimpleFlatController controller;
    controller.setBitmapStorage(&heap);
    FlatDictionary dictionary(&controller);

    // Dictionary and heap start off empty.
    REPORTER_ASSERT(reporter, heap.count() == 0);
    REPORTER_ASSERT(reporter, dictionary.count() == 0);

    heap.deferAddingOwners();
    int index = dictionary.find(*bitmapShader);
    heap.endAddingOwnersDeferral(true);

    // The dictionary and heap should now each have one entry.
    REPORTER_ASSERT(reporter, 1 == index);
    REPORTER_ASSERT(reporter, heap.count() == 1);
    REPORTER_ASSERT(reporter, dictionary.count() == 1);

    // The bitmap entry's refcount should be 1, then 0 after release.
    SkBitmapHeapEntry* entry = heap.getEntry(0);
    REPORTER_ASSERT(reporter, SkBitmapHeapTester::GetRefCount(entry) == 1);

    entry->releaseRef();
    REPORTER_ASSERT(reporter, SkBitmapHeapTester::GetRefCount(entry) == 0);

    // Now clear out the heap, after which it should be empty.
    heap.freeMemoryIfPossible(~0U);
    REPORTER_ASSERT(reporter, heap.count() == 0);

    // Now attempt to flatten the shader again.
    heap.deferAddingOwners();
    index = dictionary.find(*bitmapShader);
    heap.endAddingOwnersDeferral(false);

    // The dictionary should report the same index since the new entry is identical.
    // The bitmap heap should contain the bitmap, but with no references.
    REPORTER_ASSERT(reporter, 1 == index);
    REPORTER_ASSERT(reporter, heap.count() == 1);
    REPORTER_ASSERT(reporter, SkBitmapHeapTester::GetRefCount(heap.getEntry(0)) == 0);
}

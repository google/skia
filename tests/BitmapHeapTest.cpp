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
#include "SkOrderedWriteBuffer.h"
#include "SkPictureFlat.h"
#include "SkRefCnt.h"
#include "SkShader.h"
#include "Test.h"
#include "TestClassDef.h"

class FlatDictionary : public SkFlatDictionary<SkShader> {

public:
    FlatDictionary(SkFlatController* controller)
    : SkFlatDictionary<SkShader>(controller) {
        fFlattenProc = &flattenFlattenableProc;
        // No need for an unflattenProc
    }
    static void flattenFlattenableProc(SkOrderedWriteBuffer& buffer, const void* obj) {
        buffer.writeFlattenable((SkFlattenable*)obj);
    }
};

class SkBitmapHeapTester {

public:
    static int32_t GetRefCount(const SkBitmapHeapEntry* entry) {
        return entry->fRefCount;
    }
};

DEF_TEST(BitmapHeap, reporter) {
    // Create a bitmap shader.
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 2, 2);
    bm.allocPixels();
    bm.eraseColor(SK_ColorRED);
    uint32_t* pixel = bm.getAddr32(1,0);
    *pixel = SK_ColorBLUE;

    SkShader* bitmapShader = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                                          SkShader::kRepeat_TileMode);
    SkAutoTUnref<SkShader> aur(bitmapShader);

    // Flatten, storing it in the bitmap heap.
    SkBitmapHeap heap(1, 1);
    SkChunkFlatController controller(1024);
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

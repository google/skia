/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorDataManager_DEFINED
#define GrProcessorDataManager_DEFINED

#include "SkRefCnt.h"
#include "SkTArray.h"

class GrProcessorDataManager : public SkRefCnt {
public:
    GrProcessorDataManager() {}
    GrProcessorDataManager(const GrProcessorDataManager& procDataManager) {
        fIndices = procDataManager.fIndices;
        fStorage = procDataManager.fStorage;
    }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

private:
    uint32_t currentSaveMarker() const { return 0; }
    void restoreToSaveMarker(/*uint32_t marker*/) {}

    // For GrPipelineBuilder::AutoRestoreProcessorDataManager
    friend class GrPipelineBuilder;
    static const uint32_t kNumProcessor = 1;
    static const uint32_t kDataIndicesPerProcessor = 1;
    static const uint32_t kPreAllocDataPerProcessor = 1;

    /*static const size_t kPreAllocStorage = kNumProcessor * kPreAllocDataPerProcessor;
    static const uint32_t kNumProcessor = 8;
    static const uint32_t kDataIndicesPerProcessor = 4;
    static const uint32_t kPreAllocDataPerProcessor = kDataIndicesPerProcessor *
                                                      sizeof(GrCoordTransform);*/
    static const size_t kPreAllocIndices = kNumProcessor * kDataIndicesPerProcessor;
    static const size_t kPreAllocStorage = kNumProcessor * kPreAllocDataPerProcessor;
    SkSTArray<kPreAllocIndices, uint32_t, true> fIndices;
    SkSTArray<kPreAllocStorage, unsigned char, true> fStorage;

    typedef SkRefCnt INHERITED;
};

#endif

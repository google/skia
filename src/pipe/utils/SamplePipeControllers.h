/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkChunkAlloc.h"
#include "SkGPipe.h"
#include "SkTDArray.h"

class SkCanvas;
class SkMatrix;

class PipeController : public SkGPipeController {
public:
    PipeController(SkCanvas* target);
    virtual ~PipeController();
    virtual void* requestBlock(size_t minRequest, size_t* actual) SK_OVERRIDE;
    virtual void notifyWritten(size_t bytes) SK_OVERRIDE;
protected:
    const void* getData() { return (const char*) fBlock + fBytesWritten; }
    SkGPipeReader fReader;
private:
    void* fBlock;
    size_t fBlockSize;
    size_t fBytesWritten;
    SkGPipeReader::Status fStatus;
};

////////////////////////////////////////////////////////////////////////////////

class TiledPipeController : public PipeController {
public:
    TiledPipeController(const SkBitmap&, const SkMatrix* initialMatrix = NULL);
    virtual ~TiledPipeController() {};
    virtual void notifyWritten(size_t bytes) SK_OVERRIDE;
    virtual int numberOfReaders() const SK_OVERRIDE { return NumberOfTiles; }
private:
    enum {
        NumberOfTiles = 10
    };
    SkGPipeReader fReaders[NumberOfTiles - 1];
    SkBitmap fBitmaps[NumberOfTiles];
    typedef PipeController INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Borrowed (and modified) from SkDeferredCanvas.cpp::DeferredPipeController.
 * Allows playing back from multiple threads, but does not do the threading itself.
 */
class ThreadSafePipeController : public SkGPipeController {
public:
    ThreadSafePipeController(int numberOfReaders);
    virtual void* requestBlock(size_t minRequest, size_t* actual) SK_OVERRIDE;
    virtual void notifyWritten(size_t bytes) SK_OVERRIDE;
    virtual int numberOfReaders() const SK_OVERRIDE { return fNumberOfReaders; }

    /**
     * Play the stored drawing commands to the specified canvas. If SkGPipeWriter::startRecording
     * used the flag SkGPipeWriter::kSimultaneousReaders_Flag, this can be called from different
     * threads simultaneously.
     */
    void playback(SkCanvas*);
private:
    enum {
        kMinBlockSize = 4096
    };
    struct PipeBlock {
        PipeBlock(void* block, size_t bytes) { fBlock = block, fBytes = bytes; }
        // Stream of draw commands written by the SkGPipeWriter. Allocated by fAllocator, which will
        // handle freeing it.
        void* fBlock;
        // Number of bytes that were written to fBlock.
        size_t fBytes;
    };
    void* fBlock;
    size_t fBytesWritten;
    SkChunkAlloc fAllocator;
    SkTDArray<PipeBlock> fBlockList;
    int fNumberOfReaders;
};

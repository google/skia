/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkGPipe.h"

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

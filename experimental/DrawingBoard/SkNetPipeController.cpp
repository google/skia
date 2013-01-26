/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkNetPipeController.h"

SkNetPipeController::SkNetPipeController(SkCanvas* target) : fReader(target) {
    fBlock = NULL;
    fBlockSize = fBytesWritten = 0;
    fPlayback = true;
    fStatus = SkGPipeReader::kDone_Status;
    fTotalWritten = 0;
    fAtomsWritten = 0;
}
SkNetPipeController::~SkNetPipeController() {
    sk_free(fBlock);
}

int SkNetPipeController::writeToSocket(SkSocket* sockfd, SkSocket::DataType type) {
    if (NULL != sockfd && fTotalWritten > 4)
        return sockfd->writePacket(fBlock, fBytesWritten, type);
    else
        return -1;
}

void* SkNetPipeController::requestBlock(size_t minRequest, size_t* actual) {
    sk_free(fBlock);

    fBlockSize = minRequest * 4;
    fBlock = sk_malloc_throw(fBlockSize);
    fBytesWritten = 0;
    *actual = fBlockSize;
    return fBlock;
}

void SkNetPipeController::notifyWritten(size_t bytes) {
    SkASSERT(fBytesWritten + bytes <= fBlockSize);

    if (fPlayback) {
        fStatus = fReader.playback((const char*)fBlock + fBytesWritten, bytes);
    }

    SkASSERT(SkGPipeReader::kError_Status != fStatus);
    fBytesWritten += bytes;
    fTotalWritten += bytes;

    fAtomsWritten += 1;
}

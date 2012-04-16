
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBufferObj.h"

void GrBufferObj::allocate(GrGLint size, const GrGLchar *dataPtr) {
    GrAlwaysAssert(size >= 0);

    // delete pre-existing data
    delete[] fDataPtr;

    fSize = size;
    fDataPtr = new GrGLchar[size];
    if (dataPtr) {
        memcpy(fDataPtr, dataPtr, fSize);
    }
    // TODO: w/ no dataPtr the data is unitialized - this could be tracked
}

void GrBufferObj::deleteAction() {

    // buffers are automatically unmapped when deleted
    this->resetMapped();

    this->INHERITED::deleteAction();
}

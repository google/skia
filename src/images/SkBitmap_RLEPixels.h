
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmap_RLEPixels_DEFINED
#define SkBitmap_RLEPixels_DEFINED

#include "SkChunkAlloc.h"

class SkBitmap_RLEPixels {
public:
    SkBitmap_RLEPixels(int width, int height);
    ~SkBitmap_RLEPixels();
    
    uint8_t* yptrs() const { return fYPtrs; }
    uint8_t* allocChunk(size_t chunk);
    
private:
    SkChunkAlloc    fChunk;
    uint8_t**       fYPtrs;
};

#endif

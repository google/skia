/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageDiffer_DEFINED
#define SkImageDiffer_DEFINED

#include "SkBitmap.h"

/**
 * Encapsulates an image difference metric algorithm that can be potentially run asynchronously.
 */
class SkImageDiffer {
public:
    SkImageDiffer();
    virtual ~SkImageDiffer();

    static const double RESULT_CORRECT;
    static const double RESULT_INCORRECT;

    struct Result {
        double result;
        int poiCount;
        SkBitmap poiAlphaMask; // optional
        double timeElapsed; // optional
    };

    /**
     * Gets a unique and descriptive name of this differ
     * @return A statically allocated null terminated string that is the name of this differ
     */
    virtual const char* getName() const = 0;

    /**
     * Gets if this differ needs to be initialized with and OpenCL device and context.
     */
    virtual bool requiresOpenCL() const { return false; }

    /**
     * diff on a pair of bitmaps.
     * @param  baseline    The correct bitmap
     * @param  test        The bitmap whose difference is being tested
     * @param  computeMask true if the differ is to attempt to create poiAlphaMask
     * @return             true on success, and false in the case of failure
     */
    virtual bool diff(SkBitmap* baseline, SkBitmap* test, bool computeMask,
                      Result* result) const = 0;
};

#endif

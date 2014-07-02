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
        // TODO(djsollen): Figure out a way that the differ can report which of the
        // optional fields it has filled in.  See http://skbug.com/2712 ('allow
        // skpdiff to report different sets of result fields for different comparison algorithms')
        SkBitmap poiAlphaMask; // optional
        SkBitmap rgbDiffBitmap; // optional
        SkBitmap whiteDiffBitmap; // optional
        int maxRedDiff; // optional
        int maxGreenDiff; // optional
        int maxBlueDiff; // optional
        double timeElapsed; // optional
    };

    // A bitfield indicating which bitmap types we want a differ to create.
    //
    // TODO(epoger): Remove whiteDiffBitmap, because alphaMask can provide
    // the same functionality and more.
    // It will be a little bit tricky, because the rebaseline_server client
    // and server side code will both need to change to use the alphaMask.
    struct BitmapsToCreate {
        bool alphaMask;
        bool rgbDiff;
        bool whiteDiff;
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
     * @param  bitmapsToCreate  Which bitmaps the differ should attempt to create
     * @return             true on success, and false in the case of failure
     */
    virtual bool diff(SkBitmap* baseline, SkBitmap* test, const BitmapsToCreate& bitmapsToCreate,
                      Result* result) const = 0;
};

#endif

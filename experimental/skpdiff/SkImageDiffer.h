/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageDiffer_DEFINED
#define SkImageDiffer_DEFINED

class SkBitmap;
struct SkIPoint;

/**
 * Encapsulates an image difference metric algorithm that can be potentially run asynchronously.
 */
class SkImageDiffer {
public:
    SkImageDiffer();
    virtual ~SkImageDiffer();

    /**
     * Gets a unique and descriptive name of this differ
     * @return A statically allocated null terminated string that is the name of this differ
     */
    virtual const char* getName() = 0;

    /**
     * Gets if this differ is in a usable state
     * @return True if this differ can be used, false otherwise
     */
    bool isGood() { return fIsGood; }

    /**
     * Gets if this differ needs to be initialized with and OpenCL device and context.
     */
    virtual bool requiresOpenCL() { return false; }

    /**
     * Wraps a call to queueDiff by loading the given filenames into SkBitmaps
     * @param  baseline The file path of the baseline image
     * @param  test     The file path of the test image
     * @return          The results of queueDiff with the loaded bitmaps
     */
    int queueDiffOfFile(const char baseline[], const char test[]);

    /**
     * Queues a diff on a pair of bitmaps to be done at some future time.
     * @param  baseline The correct bitmap
     * @param  test     The bitmap whose difference is being tested
     * @return          An non-negative diff ID on success, a negative integer on failure.
     */
    virtual int queueDiff(SkBitmap* baseline, SkBitmap* test) = 0;

    /**
     * Gets whether a queued diff of the given id has finished
     * @param  id The id of the queued diff to query
     * @return    True if the queued diff is finished and has results, false otherwise
     */
    virtual bool isFinished(int id) = 0;

    /**
     * Deletes memory associated with a diff and its results. This may block execution until the
     * diff is finished,
     * @param id The id of the diff to query
     */
    virtual void deleteDiff(int id) = 0;

    /**
     * Gets the results of the queued diff of the given id. The results are only meaningful after
     * the queued diff has finished.
     * @param  id The id of the queued diff to query
     */
    virtual double getResult(int id) = 0;

    /**
     * Gets the number of points of interest for the diff of the given id. The results are only
     * meaningful after the queued diff has finished.
     * @param  id The id of the queued diff to query
     */
    virtual int getPointsOfInterestCount(int id) = 0;

    /**
     * Gets an array of the points of interest for the diff of the given id. The results are only
     * meaningful after the queued diff has finished.
     * @param  id The id of the queued diff to query
     */
    virtual SkIPoint* getPointsOfInterest(int id) = 0;



protected:
    bool fIsGood;
};


#endif

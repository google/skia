/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SubsetPath_DEFINED
#define SubsetPath_DEFINED

#include "include/core/SkPath.h"
#include "include/private/SkTDArray.h"

/* Given a path, generate a the desired minimal subset of the original.

   This does a binary divide-and-conquer on the path, first splitting by
   contours, and then by verbs. The caller passes whether the previous subset
   behaved the same as the original. If not, the subset() call restores the
   prior state before returning a new subset.

   If a path fails a local test, this reduces the data to the
   minimal set that fails using a pattern like:

   bool testFailed = true;
   SkPath minimal;
   SubsetContours subsetContours(testPath);
   while (subsetContours.subset(testFailed, &minimal)) {
       testFailed = myPathTest(minimal);
   }
   testFailed = true;
   SubsetVerbs subsetVerbs(testPath);
   while (subsetVerbs.subset(testFailed, &minimal)) {
       testFailed = myPathTest(minimal);
   }
*/

class SubsetPath {
public:
    SubsetPath(const SkPath& path);
    virtual ~SubsetPath() {}
    bool subset(bool testFailed, SkPath* sub);
protected:
    int range(int* end) const;
    virtual SkPath getSubsetPath() const = 0;

    const SkPath& fPath;
    SkTDArray<bool> fSelected;
    int fSubset;
    int fTries;

};

class SubsetContours : public SubsetPath {
public:
    SubsetContours(const SkPath& path);
protected:
    SkPath getSubsetPath() const override;
};

class SubsetVerbs : public SubsetPath {
public:
    SubsetVerbs(const SkPath& path);
protected:
    SkPath getSubsetPath() const override;
};

#endif

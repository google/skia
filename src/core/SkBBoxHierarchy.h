
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBBoxHierarchy_DEFINED
#define SkBBoxHierarchy_DEFINED

#include "SkRect.h"
#include "SkTDArray.h"
#include "SkRefCnt.h"

/**
 * Interface for a client class that implements utility methods needed
 * by SkBBoxHierarchy that require intrinsic knowledge of the data
 * object type that is stored in the bounding box hierarchy.
 */
class SkBBoxHierarchyClient {
public:
    virtual ~SkBBoxHierarchyClient() {}

    /**
     * Implements a rewind stop condition used by rewindInserts
     * Must returns true if 'data' points to an object that should be re-wound
     * by rewinfInserts.
     */
    virtual bool shouldRewind(void* data) = 0;
};

/**
 * Interface for a spatial data structure that associates user data pointers with axis-aligned
 * bounding boxes, and allows efficient retrieval of intersections with query rectangles.
 */
class SkBBoxHierarchy : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkBBoxHierarchy)

    SkBBoxHierarchy() : fClient(NULL) {}

    /**
     * Insert a data pointer and corresponding bounding box
     * @param data The data pointer, may be NULL
     * @param bounds The bounding box, should not be empty
     * @param defer Whether or not it is acceptable to delay insertion of this element (building up
     *        an entire spatial data structure at once is often faster and produces better
     *        structures than repeated inserts) until flushDeferredInserts is called or the first
     *        search.
     */
    virtual void insert(void* data, const SkRect& bounds, bool defer = false) = 0;

    /**
     * If any insertions have been deferred, this forces them to be inserted
     */
    virtual void flushDeferredInserts() = 0;

    /**
     * Populate 'results' with data pointers corresponding to bounding boxes that intersect 'query'
     */
    virtual void search(const SkRect& query, SkTDArray<void*>* results) const = 0;

    virtual void clear() = 0;

    /**
     * Gets the number of insertions actually made (does not include deferred insertions)
     */
    virtual int getCount() const = 0;

    /**
     * Returns the depth of the currently allocated tree. The root node counts for 1 level,
     * so it should be 1 or more if there's a root node. This information provides details
     * about the underlying structure, which is useful mainly for testing purposes.
     *
     * Returns 0 if there are currently no nodes in the tree.
     * Returns -1 if the structure isn't a tree.
     */
    virtual int getDepth() const = 0;

    /**
     * Rewinds all the most recently inserted data elements until an element
     * is encountered for which client->shouldRewind(data) returns false. May
     * not rewind elements that were inserted prior to the last call to
     * flushDeferredInserts.
     */
    virtual void rewindInserts() = 0;

    void setClient(SkBBoxHierarchyClient* client) { fClient = client; }

protected:
    SkBBoxHierarchyClient* fClient;

private:
    typedef SkRefCnt INHERITED;
};

#endif


/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFCatalog_DEFINED
#define SkPDFCatalog_DEFINED

#include "SkPDFTypes.h"
#include "SkTDArray.h"
#include "SkTHash.h"

/** \class SkPDFCatalog

    The PDF catalog manages object numbers.  It is used
    to create the PDF cross reference table.
*/
class SkPDFCatalog {
public:
    SkPDFCatalog();
    ~SkPDFCatalog();

    /** Add the passed object to the catalog.
     *  @param obj         The object to add.
     *  @return True iff the object was not already added to the catalog.
     */
    bool addObject(SkPDFObject* obj);

    /** Get the object number for the passed object.
     *  @param obj         The object of interest.
     */
    int32_t getObjectNumber(SkPDFObject* obj) const;

    /** Set substitute object for the passed object.
        Refs substitute.
     */
    void setSubstitute(SkPDFObject* original, SkPDFObject* substitute);

    /** Find and return any substitute object set for the passed object. If
     *  there is none, return the passed object.
     */
    SkPDFObject* getSubstituteObject(SkPDFObject* object) const;

    const SkTDArray<SkPDFObject*>& objects() const { return fObjects; }

private:
    SkTDArray<SkPDFObject*> fObjects;
    SkTHashMap<SkPDFObject*, int32_t> fObjectNumbers;
    SkTHashMap<SkPDFObject*, SkPDFObject*> fSubstituteMap;
};

#endif

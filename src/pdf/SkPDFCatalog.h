
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFCatalog_DEFINED
#define SkPDFCatalog_DEFINED

#include <sys/types.h>

#include "SkPDFTypes.h"
#include "SkTDArray.h"

/** \class SkPDFCatalog

    The PDF catalog manages object numbers and file offsets.  It is used
    to create the PDF cross reference table.
*/
class SkPDFCatalog {
public:
    /** Create a PDF catalog.
     */
    SkPDFCatalog();
    ~SkPDFCatalog();

    /** Add the passed object to the catalog.  Refs obj.
     *  @param obj         The object to add.
     *  @param onFirstPage Is the object on the first page.
     *  @return The obj argument is returned.
     */
    SkPDFObject* addObject(SkPDFObject* obj, bool onFirstPage);

    /** Inform the catalog of the object's position in the final stream.
     *  The object should already have been added to the catalog.
     *  @param obj         The object to add.
     *  @param offset      The byte offset in the output stream of this object.
     */
    void setFileOffset(SkPDFObject* obj, off_t offset);

    /** Get the object number for the passed object.
     *  @param obj         The object of interest.
     */
    int32_t getObjectNumber(SkPDFObject* obj);

    /** Output the cross reference table for objects in the catalog.
     *  Returns the total number of objects.
     *  @param stream      The writable output stream to send the output to.
     *  @param firstPage   If true, include first page objects only, otherwise
     *                     include all objects not on the first page.
     */
    int32_t emitXrefTable(SkWStream* stream, bool firstPage);

    /** Set substitute object for the passed object.
     */
    void setSubstitute(SkPDFObject* original, SkPDFObject* substitute);

    /** Find and return any substitute object set for the passed object. If
     *  there is none, return the passed object.
     */
    SkPDFObject* getSubstituteObject(SkPDFObject* object);

private:
    struct Rec {
        Rec(SkPDFObject* object, bool onFirstPage)
            : fObject(object),
              fFileOffset(0),
              fObjNumAssigned(false),
              fOnFirstPage(onFirstPage) {
        }
        SkPDFObject* fObject;
        off_t fFileOffset;
        bool fObjNumAssigned;
        bool fOnFirstPage;
    };

    struct SubstituteMapping {
        SubstituteMapping(SkPDFObject* original, SkPDFObject* substitute)
            : fOriginal(original), fSubstitute(substitute) {
        }
        SkPDFObject* fOriginal;
        SkPDFObject* fSubstitute;
    };

    // TODO(vandebo): Make this a hash if it's a performance problem.
    SkTDArray<Rec> fCatalog;

    // TODO(arthurhsu): Make this a hash if it's a performance problem.
    SkTDArray<SubstituteMapping> fSubstituteMap;
    SkTSet<SkPDFObject*> fSubstituteResourcesFirstPage;
    SkTSet<SkPDFObject*> fSubstituteResourcesRemaining;

    // Number of objects on the first page.
    uint32_t fFirstPageCount;
    // Next object number to assign (on page > 1).
    uint32_t fNextObjNum;
    // Next object number to assign on the first page.
    uint32_t fNextFirstPageObjNum;

    int findObjectIndex(SkPDFObject* obj);

    int assignObjNum(SkPDFObject* obj);
};

#endif


/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFFormXObject_DEFINED
#define SkPDFFormXObject_DEFINED

#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkRefCnt.h"
#include "SkString.h"

class SkMatrix;
class SkPDFDevice;
class SkPDFCatalog;

/** \class SkPDFFormXObject

    A form XObject; a self contained description of graphics objects.  A form
    XObject is basically a page object with slightly different syntax, that
    can be drawn onto a page.
*/

// The caller could keep track of the form XObjects it creates and
// canonicalize them, but the Skia API doesn't provide enough context to
// automatically do it (trivially).
class SkPDFFormXObject : public SkPDFStream {
public:
    /** Create a PDF form XObject. Entries for the dictionary entries are
     *  automatically added.
     *  @param device      The set of graphical elements on this form.
     */
    explicit SkPDFFormXObject(SkPDFDevice* device);
    virtual ~SkPDFFormXObject();

    // The SkPDFObject interface.
    virtual void getResources(const SkTSet<SkPDFObject*>& knownResourceObjects,
                              SkTSet<SkPDFObject*>* newResourceObjects);

private:
    SkTSet<SkPDFObject*> fResources;
};

#endif

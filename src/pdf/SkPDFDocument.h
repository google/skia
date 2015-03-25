
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFDocument_DEFINED
#define SkPDFDocument_DEFINED

#include "SkAdvancedTypefaceMetrics.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"
#include "SkTemplates.h"

class SkPDFCatalog;
class SkPDFDevice;
class SkPDFDict;
class SkPDFPage;
class SkPDFObject;
class SkWStream;
template <typename T> class SkTSet;

/** \class SkPDFDocument

    A SkPDFDocument assembles pages together and generates the final PDF file.
*/
class SkPDFDocument {
public:
    SkPDFDocument();
    ~SkPDFDocument();

    /** Output the PDF to the passed stream.  It is an error to call this (it
     *  will return false and not modify stream) if no pages have been added
     *  or there are pages missing (i.e. page 1 and 3 have been added, but not
     *  page 2).
     *
     *  @param stream    The writable output stream to send the PDF to.
     */
    bool emitPDF(SkWStream* stream);

    /** Append the passed pdf device to the document as a new page.  Returns
     *  true if successful.  Will fail if the document has already been emitted.
     *
     *  @param pdfDevice The page to add to this document.
     */
    bool appendPage(SkPDFDevice* pdfDevice) {
        fPageDevices.push(SkRef(pdfDevice));
        return true;
    }

    /** Get the count of unique font types used in the document.
     */
    void getCountOfFontTypes(
        int counts[SkAdvancedTypefaceMetrics::kOther_Font + 1],
        int* notSubsettableCount,
        int* notEmbedddableCount) const;

private:
    SkTDArray<SkPDFDevice*> fPageDevices;
};

#endif


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
    SkPDFDocument() {}
    ~SkPDFDocument() { fPageDevices.unrefAll(); }

    /** Output the PDF to the passed stream.  It is an error to call this (it
     *  will return false and not modify stream) if pageDevices is empty.
     *  No device pointer can be NULL.
     *
     *  @param pageDevices An array of pages, in order.  All pages
     *                     should be created using the same SkPDFCanon.
     *                     TODO(halcanary): ASSERT this condition.
     *  @param SkWStream   The writable output stream to send the PDF to.
     */
    static bool EmitPDF(const SkTDArray<SkPDFDevice*>& pageDevices, SkWStream*);

    /** Output the PDF to the passed stream.  It is an error to call this (it
     *  will return false and not modify stream) if no pages have been added.
     *
     *  @param stream    The writable output stream to send the PDF to.
     */
    bool emitPDF(SkWStream* stream) const {
        return SkPDFDocument::EmitPDF(fPageDevices, stream);
    }

    /** Append the passed pdf device to the document as a new page.
     *
     *  @param pdfDevice The page to add to this document. All pages
     *                   added to this document should be created
     *                   using the same SkPDFCanon.
     */
    void appendPage(SkPDFDevice* pdfDevice) {
        fPageDevices.push(SkRef(pdfDevice));
    }

    /** Get the count of unique font types used in the given pages.
     */
    static void GetCountOfFontTypes(
            const SkTDArray<SkPDFDevice*>& pageDevices,
            int counts[SkAdvancedTypefaceMetrics::kOther_Font + 1],
            int* notSubsettableCount,
            int* notEmbedddableCount);

    /** Get the count of unique font types used in the document.
     */
    void getCountOfFontTypes(
            int counts[SkAdvancedTypefaceMetrics::kOther_Font + 1],
            int* notSubsettableCount,
            int* notEmbedddableCount) const {
        return SkPDFDocument::GetCountOfFontTypes(
                fPageDevices, counts, notSubsettableCount, notEmbedddableCount);
    }

private:
    SkTDArray<SkPDFDevice*> fPageDevices;
};

#endif

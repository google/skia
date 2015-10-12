/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFMetadata_DEFINED
#define SkPDFMetadata_DEFINED

#include "SkDocument.h"
#include "SkTime.h"

class SkPDFObject;

struct SkPDFMetadata {
    SkTArray<SkDocument::Attribute> fInfo;
    SkAutoTDelete<const SkTime::DateTime> fCreation;
    SkAutoTDelete<const SkTime::DateTime> fModified;

    SkPDFObject* createDocumentInformationDict() const;

#ifdef SK_PDF_GENERATE_PDFA
    struct UUID {
        uint8_t fData[16];
    };
    UUID uuid() const;
    static SkPDFObject* CreatePdfId(const UUID& doc, const UUID& instance);
    SkPDFObject* createXMPObject(const UUID& doc, const UUID& instance) const;
#endif  // SK_PDF_GENERATE_PDFA
};

#endif  // SkPDFMetadata_DEFINED

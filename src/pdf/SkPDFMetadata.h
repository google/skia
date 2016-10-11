/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFMetadata_DEFINED
#define SkPDFMetadata_DEFINED

#include "SkDocument.h"

class SkPDFObject;

namespace SkPDFMetadata {
sk_sp<SkPDFObject> MakeDocumentInformationDict(const SkDocument::PDFMetadata&);

struct UUID {
    uint8_t fData[16];
};

UUID CreateUUID(const SkDocument::PDFMetadata&);

sk_sp<SkPDFObject> MakePdfId(const UUID& doc, const UUID& instance);

sk_sp<SkPDFObject> MakeXMPObject(const SkDocument::PDFMetadata&,
                                 const UUID& doc,
                                 const UUID& instance);
}
#endif  // SkPDFMetadata_DEFINED

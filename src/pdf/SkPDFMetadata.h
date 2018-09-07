/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFMetadata_DEFINED
#define SkPDFMetadata_DEFINED

#include "SkPDFDocument.h"

class SkPDFObject;

namespace SkPDFMetadata {
sk_sp<SkPDFObject> MakeDocumentInformationDict(const SkPDF::Metadata&);

struct UUID {
    uint8_t fData[16];
};

UUID CreateUUID(const SkPDF::Metadata&);

sk_sp<SkPDFObject> MakePdfId(const UUID& doc, const UUID& instance);

sk_sp<SkPDFObject> MakeXMPObject(const SkPDF::Metadata&,
                                 const UUID& doc,
                                 const UUID& instance);
}
#endif  // SkPDFMetadata_DEFINED

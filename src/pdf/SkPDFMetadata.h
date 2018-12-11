/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFMetadata_DEFINED
#define SkPDFMetadata_DEFINED

#include "SkPDFDocument.h"
#include "SkUUID.h"

class SkPDFObject;

namespace SkPDFMetadata {
sk_sp<SkPDFObject> MakeDocumentInformationDict(const SkPDF::Metadata&);

SkUUID CreateUUID(const SkPDF::Metadata&);

sk_sp<SkPDFObject> MakePdfId(const SkUUID& doc, const SkUUID& instance);

sk_sp<SkPDFObject> MakeXMPObject(const SkPDF::Metadata&,
                                 const SkUUID& doc,
                                 const SkUUID& instance);
}
#endif  // SkPDFMetadata_DEFINED

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFMetadata_DEFINED
#define SkPDFMetadata_DEFINED

#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkUUID.h"

#include <memory>

class SkPDFDocument;

namespace SkPDF { struct Metadata; }

namespace SkPDFMetadata {
std::unique_ptr<SkPDFObject> MakeDocumentInformationDict(const SkPDF::Metadata&);

SkUUID CreateUUID(const SkPDF::Metadata&);

std::unique_ptr<SkPDFObject> MakePdfId(const SkUUID& doc, const SkUUID& instance);

SkPDFIndirectReference MakeXMPObject(const SkPDF::Metadata& metadata,
                                     const SkUUID& doc,
                                     const SkUUID& instance,
                                     SkPDFDocument*);
}  // namespace SkPDFMetadata
#endif  // SkPDFMetadata_DEFINED

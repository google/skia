/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfComponentsWithMetadataDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfStream* SkPdfComponentsWithMetadataDictionary::Metadata(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Metadata", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfComponentsWithMetadataDictionary::has_Metadata() const {
  return get("Metadata", "") != NULL;
}

/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfStructureElementAccessDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfStructureElementAccessDictionary::StructParent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructParent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfStructureElementAccessDictionary::has_StructParent() const {
  return get("StructParent", "") != NULL;
}

int64_t SkPdfStructureElementAccessDictionary::StructParents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructParents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfStructureElementAccessDictionary::has_StructParents() const {
  return get("StructParents", "") != NULL;
}

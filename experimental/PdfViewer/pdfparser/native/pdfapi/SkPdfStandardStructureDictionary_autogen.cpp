/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfStandardStructureDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfStandardStructureDictionary::Placement(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Placement", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStandardStructureDictionary::has_Placement() const {
  return get("Placement", "") != NULL;
}

SkString SkPdfStandardStructureDictionary::WritingMode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("WritingMode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStandardStructureDictionary::has_WritingMode() const {
  return get("WritingMode", "") != NULL;
}

/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFDFNamedPageReferenceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfFDFNamedPageReferenceDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFDFNamedPageReferenceDictionary::has_Name() const {
  return get("Name", "") != NULL;
}

SkPdfFileSpec SkPdfFDFNamedPageReferenceDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFileSpec();
}

bool SkPdfFDFNamedPageReferenceDictionary::has_F() const {
  return get("F", "") != NULL;
}

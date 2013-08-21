/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPrinterMarkAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPrinterMarkAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfPrinterMarkAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfPrinterMarkAnnotationDictionary::MN(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("MN", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPrinterMarkAnnotationDictionary::has_MN() const {
  return get("MN", "") != NULL;
}

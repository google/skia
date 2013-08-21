/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPrinterMarkFormDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPrinterMarkFormDictionary::MarkStyle(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("MarkStyle", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPrinterMarkFormDictionary::has_MarkStyle() const {
  return get("MarkStyle", "") != NULL;
}

SkPdfDictionary* SkPdfPrinterMarkFormDictionary::Colorants(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Colorants", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPrinterMarkFormDictionary::has_Colorants() const {
  return get("Colorants", "") != NULL;
}

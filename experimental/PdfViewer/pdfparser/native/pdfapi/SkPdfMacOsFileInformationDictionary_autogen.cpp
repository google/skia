/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMacOsFileInformationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfMacOsFileInformationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfMacOsFileInformationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfMacOsFileInformationDictionary::Creator(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Creator", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfMacOsFileInformationDictionary::has_Creator() const {
  return get("Creator", "") != NULL;
}

SkPdfStream* SkPdfMacOsFileInformationDictionary::ResFork(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ResFork", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMacOsFileInformationDictionary::has_ResFork() const {
  return get("ResFork", "") != NULL;
}

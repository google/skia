/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfReferenceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfFileSpec SkPdfReferenceDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

bool SkPdfReferenceDictionary::has_F() const {
  return get("F", "") != NULL;
}

bool SkPdfReferenceDictionary::isPageAInteger(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Page", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isInteger();
}

int64_t SkPdfReferenceDictionary::getPageAsInteger(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Page", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfReferenceDictionary::isPageAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Page", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfReferenceDictionary::getPageAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Page", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfReferenceDictionary::has_Page() const {
  return get("Page", "") != NULL;
}

SkPdfArray* SkPdfReferenceDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfReferenceDictionary::has_ID() const {
  return get("ID", "") != NULL;
}

/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPageTreeNodeDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPageTreeNodeDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfPageTreeNodeDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkPdfDictionary* SkPdfPageTreeNodeDictionary::Parent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Parent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageTreeNodeDictionary::has_Parent() const {
  return get("Parent", "") != NULL;
}

SkPdfArray* SkPdfPageTreeNodeDictionary::Kids(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Kids", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfPageTreeNodeDictionary::has_Kids() const {
  return get("Kids", "") != NULL;
}

int64_t SkPdfPageTreeNodeDictionary::Count(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Count", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfPageTreeNodeDictionary::has_Count() const {
  return get("Count", "") != NULL;
}

/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfNameTreeNodeDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfNameTreeNodeDictionary::Kids(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Kids", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfNameTreeNodeDictionary::has_Kids() const {
  return get("Kids", "") != NULL;
}

SkPdfArray* SkPdfNameTreeNodeDictionary::Names(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Names", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfNameTreeNodeDictionary::has_Names() const {
  return get("Names", "") != NULL;
}

SkPdfArray* SkPdfNameTreeNodeDictionary::Limits(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Limits", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfNameTreeNodeDictionary::has_Limits() const {
  return get("Limits", "") != NULL;
}

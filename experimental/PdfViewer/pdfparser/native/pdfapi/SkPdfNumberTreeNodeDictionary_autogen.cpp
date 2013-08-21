/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfNumberTreeNodeDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfNumberTreeNodeDictionary::Kids(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Kids", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfNumberTreeNodeDictionary::has_Kids() const {
  return get("Kids", "") != NULL;
}

SkPdfArray* SkPdfNumberTreeNodeDictionary::Nums(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Nums", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfNumberTreeNodeDictionary::has_Nums() const {
  return get("Nums", "") != NULL;
}

SkPdfArray* SkPdfNumberTreeNodeDictionary::Limits(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Limits", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfNumberTreeNodeDictionary::has_Limits() const {
  return get("Limits", "") != NULL;
}

/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfURIActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfURIActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfURIActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkString SkPdfURIActionDictionary::URI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("URI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfURIActionDictionary::has_URI() const {
  return get("URI", "") != NULL;
}

bool SkPdfURIActionDictionary::IsMap(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IsMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfURIActionDictionary::has_IsMap() const {
  return get("IsMap", "") != NULL;
}

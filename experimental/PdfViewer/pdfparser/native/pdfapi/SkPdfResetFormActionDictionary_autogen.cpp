/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfResetFormActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfResetFormActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfResetFormActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfArray* SkPdfResetFormActionDictionary::Fields(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Fields", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResetFormActionDictionary::has_Fields() const {
  return get("Fields", "") != NULL;
}

int64_t SkPdfResetFormActionDictionary::Flags(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Flags", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfResetFormActionDictionary::has_Flags() const {
  return get("Flags", "") != NULL;
}

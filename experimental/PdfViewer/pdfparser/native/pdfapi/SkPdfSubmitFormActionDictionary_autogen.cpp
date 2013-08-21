/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSubmitFormActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfSubmitFormActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSubmitFormActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfFileSpec SkPdfSubmitFormActionDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

bool SkPdfSubmitFormActionDictionary::has_F() const {
  return get("F", "") != NULL;
}

SkPdfArray* SkPdfSubmitFormActionDictionary::Fields(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Fields", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSubmitFormActionDictionary::has_Fields() const {
  return get("Fields", "") != NULL;
}

int64_t SkPdfSubmitFormActionDictionary::Flags(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Flags", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfSubmitFormActionDictionary::has_Flags() const {
  return get("Flags", "") != NULL;
}

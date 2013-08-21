/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfChoiceFieldDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfChoiceFieldDictionary::Opt(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Opt", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfChoiceFieldDictionary::has_Opt() const {
  return get("Opt", "") != NULL;
}

int64_t SkPdfChoiceFieldDictionary::TI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfChoiceFieldDictionary::has_TI() const {
  return get("TI", "") != NULL;
}

SkPdfArray* SkPdfChoiceFieldDictionary::I(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("I", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfChoiceFieldDictionary::has_I() const {
  return get("I", "") != NULL;
}

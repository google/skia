/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfEncodingDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfEncodingDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfEncodingDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfEncodingDictionary::BaseEncoding(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BaseEncoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfEncodingDictionary::has_BaseEncoding() const {
  return get("BaseEncoding", "") != NULL;
}

SkPdfArray* SkPdfEncodingDictionary::Differences(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Differences", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfEncodingDictionary::has_Differences() const {
  return get("Differences", "") != NULL;
}

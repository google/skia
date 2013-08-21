/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfEmbeddedFileStreamDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfEmbeddedFileStreamDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfEmbeddedFileStreamDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfEmbeddedFileStreamDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfEmbeddedFileStreamDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkPdfDictionary* SkPdfEmbeddedFileStreamDictionary::Params(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Params", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfEmbeddedFileStreamDictionary::has_Params() const {
  return get("Params", "") != NULL;
}

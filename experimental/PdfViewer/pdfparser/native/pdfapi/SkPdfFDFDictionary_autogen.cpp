/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFDFDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfFileSpec SkPdfFDFDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFileSpec();
}

bool SkPdfFDFDictionary::has_F() const {
  return get("F", "") != NULL;
}

SkPdfArray* SkPdfFDFDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFDictionary::has_ID() const {
  return get("ID", "") != NULL;
}

SkPdfArray* SkPdfFDFDictionary::Fields(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Fields", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFDictionary::has_Fields() const {
  return get("Fields", "") != NULL;
}

SkString SkPdfFDFDictionary::Status(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Status", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFDFDictionary::has_Status() const {
  return get("Status", "") != NULL;
}

SkPdfArray* SkPdfFDFDictionary::Pages(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pages", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFDictionary::has_Pages() const {
  return get("Pages", "") != NULL;
}

SkString SkPdfFDFDictionary::Encoding(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFDFDictionary::has_Encoding() const {
  return get("Encoding", "") != NULL;
}

SkPdfArray* SkPdfFDFDictionary::Annots(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Annots", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFDictionary::has_Annots() const {
  return get("Annots", "") != NULL;
}

SkPdfStream* SkPdfFDFDictionary::Differences(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Differences", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFDictionary::has_Differences() const {
  return get("Differences", "") != NULL;
}

SkString SkPdfFDFDictionary::Target(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Target", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFDFDictionary::has_Target() const {
  return get("Target", "") != NULL;
}

SkPdfArray* SkPdfFDFDictionary::EmbeddedFDFs(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EmbeddedFDFs", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFDictionary::has_EmbeddedFDFs() const {
  return get("EmbeddedFDFs", "") != NULL;
}

SkPdfDictionary* SkPdfFDFDictionary::JavaScript(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JavaScript", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFDictionary::has_JavaScript() const {
  return get("JavaScript", "") != NULL;
}

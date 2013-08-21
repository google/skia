/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfALinkAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfALinkAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfALinkAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfALinkAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfALinkAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

bool SkPdfALinkAnnotationDictionary::isDestAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfALinkAnnotationDictionary::getDestAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfALinkAnnotationDictionary::isDestAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfALinkAnnotationDictionary::getDestAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfALinkAnnotationDictionary::isDestAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfALinkAnnotationDictionary::getDestAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfALinkAnnotationDictionary::has_Dest() const {
  return get("Dest", "") != NULL;
}

SkString SkPdfALinkAnnotationDictionary::H(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("H", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfALinkAnnotationDictionary::has_H() const {
  return get("H", "") != NULL;
}

SkPdfDictionary* SkPdfALinkAnnotationDictionary::PA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfALinkAnnotationDictionary::has_PA() const {
  return get("PA", "") != NULL;
}

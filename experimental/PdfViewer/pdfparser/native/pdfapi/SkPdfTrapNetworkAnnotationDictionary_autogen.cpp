/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfTrapNetworkAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfTrapNetworkAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfTrapNetworkAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfTrapNetworkAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfTrapNetworkAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkPdfDate SkPdfTrapNetworkAnnotationDictionary::LastModified(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LastModified", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfTrapNetworkAnnotationDictionary::has_LastModified() const {
  return get("LastModified", "") != NULL;
}

SkPdfArray* SkPdfTrapNetworkAnnotationDictionary::Version(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Version", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfTrapNetworkAnnotationDictionary::has_Version() const {
  return get("Version", "") != NULL;
}

SkPdfArray* SkPdfTrapNetworkAnnotationDictionary::AnnotStates(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AnnotStates", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfTrapNetworkAnnotationDictionary::has_AnnotStates() const {
  return get("AnnotStates", "") != NULL;
}

SkPdfArray* SkPdfTrapNetworkAnnotationDictionary::FontFauxing(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontFauxing", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfTrapNetworkAnnotationDictionary::has_FontFauxing() const {
  return get("FontFauxing", "") != NULL;
}

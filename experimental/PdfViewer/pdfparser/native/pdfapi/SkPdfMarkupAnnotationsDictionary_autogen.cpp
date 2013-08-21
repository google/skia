/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMarkupAnnotationsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfMarkupAnnotationsDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfMarkupAnnotationsDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfMarkupAnnotationsDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfMarkupAnnotationsDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkPdfArray* SkPdfMarkupAnnotationsDictionary::QuadPoints(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("QuadPoints", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfMarkupAnnotationsDictionary::has_QuadPoints() const {
  return get("QuadPoints", "") != NULL;
}

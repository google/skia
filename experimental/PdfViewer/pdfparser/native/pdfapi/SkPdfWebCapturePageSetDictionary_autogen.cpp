/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfWebCapturePageSetDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfWebCapturePageSetDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfWebCapturePageSetDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkString SkPdfWebCapturePageSetDictionary::T(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWebCapturePageSetDictionary::has_T() const {
  return get("T", "") != NULL;
}

SkString SkPdfWebCapturePageSetDictionary::TID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWebCapturePageSetDictionary::has_TID() const {
  return get("TID", "") != NULL;
}

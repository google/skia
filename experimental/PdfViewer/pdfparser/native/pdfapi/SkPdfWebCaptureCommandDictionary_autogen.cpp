/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfWebCaptureCommandDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfWebCaptureCommandDictionary::URL(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("URL", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfWebCaptureCommandDictionary::has_URL() const {
  return get("URL", "") != NULL;
}

int64_t SkPdfWebCaptureCommandDictionary::L(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("L", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfWebCaptureCommandDictionary::has_L() const {
  return get("L", "") != NULL;
}

int64_t SkPdfWebCaptureCommandDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfWebCaptureCommandDictionary::has_F() const {
  return get("F", "") != NULL;
}

bool SkPdfWebCaptureCommandDictionary::isPAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfWebCaptureCommandDictionary::getPAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWebCaptureCommandDictionary::isPAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfWebCaptureCommandDictionary::getPAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfWebCaptureCommandDictionary::has_P() const {
  return get("P", "") != NULL;
}

SkString SkPdfWebCaptureCommandDictionary::CT(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWebCaptureCommandDictionary::has_CT() const {
  return get("CT", "") != NULL;
}

SkString SkPdfWebCaptureCommandDictionary::H(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("H", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWebCaptureCommandDictionary::has_H() const {
  return get("H", "") != NULL;
}

SkPdfDictionary* SkPdfWebCaptureCommandDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfWebCaptureCommandDictionary::has_S() const {
  return get("S", "") != NULL;
}

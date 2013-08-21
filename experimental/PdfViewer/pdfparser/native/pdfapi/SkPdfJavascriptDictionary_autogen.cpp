/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfJavascriptDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

bool SkPdfJavascriptDictionary::isBeforeAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Before", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfJavascriptDictionary::getBeforeAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Before", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfJavascriptDictionary::isBeforeAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Before", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfJavascriptDictionary::getBeforeAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Before", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfJavascriptDictionary::has_Before() const {
  return get("Before", "") != NULL;
}

bool SkPdfJavascriptDictionary::isAfterAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("After", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfJavascriptDictionary::getAfterAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("After", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfJavascriptDictionary::isAfterAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("After", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfJavascriptDictionary::getAfterAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("After", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfJavascriptDictionary::has_After() const {
  return get("After", "") != NULL;
}

SkPdfArray* SkPdfJavascriptDictionary::Doc(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Doc", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfJavascriptDictionary::has_Doc() const {
  return get("Doc", "") != NULL;
}

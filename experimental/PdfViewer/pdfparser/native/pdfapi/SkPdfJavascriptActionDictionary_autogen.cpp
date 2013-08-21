/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfJavascriptActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfJavascriptActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfJavascriptActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

bool SkPdfJavascriptActionDictionary::isJSAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfJavascriptActionDictionary::getJSAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfJavascriptActionDictionary::isJSAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfJavascriptActionDictionary::getJSAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfJavascriptActionDictionary::has_JS() const {
  return get("JS", "") != NULL;
}

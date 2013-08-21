/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfWindowsLaunchActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfWindowsLaunchActionDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfWindowsLaunchActionDictionary::has_F() const {
  return get("F", "") != NULL;
}

SkString SkPdfWindowsLaunchActionDictionary::D(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWindowsLaunchActionDictionary::has_D() const {
  return get("D", "") != NULL;
}

SkString SkPdfWindowsLaunchActionDictionary::O(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("O", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWindowsLaunchActionDictionary::has_O() const {
  return get("O", "") != NULL;
}

SkString SkPdfWindowsLaunchActionDictionary::P(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfWindowsLaunchActionDictionary::has_P() const {
  return get("P", "") != NULL;
}

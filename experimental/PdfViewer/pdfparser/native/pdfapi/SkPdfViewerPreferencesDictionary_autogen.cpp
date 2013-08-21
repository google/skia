/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfViewerPreferencesDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

bool SkPdfViewerPreferencesDictionary::HideToolbar(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HideToolbar", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfViewerPreferencesDictionary::has_HideToolbar() const {
  return get("HideToolbar", "") != NULL;
}

bool SkPdfViewerPreferencesDictionary::HideMenubar(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HideMenubar", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfViewerPreferencesDictionary::has_HideMenubar() const {
  return get("HideMenubar", "") != NULL;
}

bool SkPdfViewerPreferencesDictionary::HideWindowUI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HideWindowUI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfViewerPreferencesDictionary::has_HideWindowUI() const {
  return get("HideWindowUI", "") != NULL;
}

bool SkPdfViewerPreferencesDictionary::FitWindow(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FitWindow", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfViewerPreferencesDictionary::has_FitWindow() const {
  return get("FitWindow", "") != NULL;
}

bool SkPdfViewerPreferencesDictionary::CenterWindow(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CenterWindow", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfViewerPreferencesDictionary::has_CenterWindow() const {
  return get("CenterWindow", "") != NULL;
}

bool SkPdfViewerPreferencesDictionary::DisplayDocTitle(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DisplayDocTitle", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfViewerPreferencesDictionary::has_DisplayDocTitle() const {
  return get("DisplayDocTitle", "") != NULL;
}

SkString SkPdfViewerPreferencesDictionary::NonFullScreenPageMode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("NonFullScreenPageMode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfViewerPreferencesDictionary::has_NonFullScreenPageMode() const {
  return get("NonFullScreenPageMode", "") != NULL;
}

SkString SkPdfViewerPreferencesDictionary::Direction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Direction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfViewerPreferencesDictionary::has_Direction() const {
  return get("Direction", "") != NULL;
}

SkString SkPdfViewerPreferencesDictionary::ViewArea(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ViewArea", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfViewerPreferencesDictionary::has_ViewArea() const {
  return get("ViewArea", "") != NULL;
}

SkString SkPdfViewerPreferencesDictionary::ViewClip(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ViewClip", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfViewerPreferencesDictionary::has_ViewClip() const {
  return get("ViewClip", "") != NULL;
}

SkString SkPdfViewerPreferencesDictionary::PrintArea(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PrintArea", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfViewerPreferencesDictionary::has_PrintArea() const {
  return get("PrintArea", "") != NULL;
}

SkString SkPdfViewerPreferencesDictionary::PrintClip(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PrintClip", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfViewerPreferencesDictionary::has_PrintClip() const {
  return get("PrintClip", "") != NULL;
}

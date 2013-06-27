#include "SkPdfHideActionDictionary_autogen.h"

std::string SkPdfHideActionDictionary::S() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "S", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfHideActionDictionary::getTAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "T", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfHideActionDictionary::getTAsString() const {
  std::string ret = "";
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "T", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfArray* SkPdfHideActionDictionary::getTAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "T", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfHideActionDictionary::H() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "H", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}


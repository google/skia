#include "SkPdfBeadDictionary_autogen.h"

std::string SkPdfBeadDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfBeadDictionary::T() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "T", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfBeadDictionary::N() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "N", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfBeadDictionary::V() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "V", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfBeadDictionary::P() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "P", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkRect* SkPdfBeadDictionary::R() const {
  SkRect* ret;
  if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "R", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}


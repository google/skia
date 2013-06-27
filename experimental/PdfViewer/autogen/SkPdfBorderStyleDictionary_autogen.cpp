#include "SkPdfBorderStyleDictionary_autogen.h"

std::string SkPdfBorderStyleDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

double SkPdfBorderStyleDictionary::W() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "W", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfBorderStyleDictionary::S() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "S", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfArray* SkPdfBorderStyleDictionary::D() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "D", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}


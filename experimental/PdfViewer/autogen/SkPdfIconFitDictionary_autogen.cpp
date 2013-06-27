#include "SkPdfIconFitDictionary_autogen.h"

std::string SkPdfIconFitDictionary::SW() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SW", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfIconFitDictionary::S() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "S", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfArray* SkPdfIconFitDictionary::A() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "A", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}


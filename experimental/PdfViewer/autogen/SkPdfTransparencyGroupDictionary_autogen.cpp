#include "SkPdfTransparencyGroupDictionary_autogen.h"

std::string SkPdfTransparencyGroupDictionary::S() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "S", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfTransparencyGroupDictionary::getCSAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfArray* SkPdfTransparencyGroupDictionary::getCSAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

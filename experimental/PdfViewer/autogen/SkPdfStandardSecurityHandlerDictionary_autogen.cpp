#include "SkPdfStandardSecurityHandlerDictionary_autogen.h"

double SkPdfStandardSecurityHandlerDictionary::R() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "R", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfStandardSecurityHandlerDictionary::O() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "O", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfStandardSecurityHandlerDictionary::U() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "U", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

long SkPdfStandardSecurityHandlerDictionary::P() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "P", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

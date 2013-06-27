#include "SkPdfWebCaptureImageSetDictionary_autogen.h"

std::string SkPdfWebCaptureImageSetDictionary::S() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "S", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

long SkPdfWebCaptureImageSetDictionary::getRAsInteger() const {
  long ret = 0;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "R", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfArray* SkPdfWebCaptureImageSetDictionary::getRAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "R", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

#include "SkPdfCalgrayColorSpaceDictionary_autogen.h"

SkPdfArray* SkPdfCalgrayColorSpaceDictionary::WhitePoint() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "WhitePoint", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfCalgrayColorSpaceDictionary::BlackPoint() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BlackPoint", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

double SkPdfCalgrayColorSpaceDictionary::Gamma() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Gamma", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}


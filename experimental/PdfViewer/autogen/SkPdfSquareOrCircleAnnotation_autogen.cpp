#include "SkPdfSquareOrCircleAnnotation_autogen.h"

std::string SkPdfSquareOrCircleAnnotation::Subtype() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfSquareOrCircleAnnotation::Contents() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Contents", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfSquareOrCircleAnnotation::BS() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfSquareOrCircleAnnotation::IC() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "IC", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}


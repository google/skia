#include "SkPdfStreamCommonDictionary_autogen.h"

long SkPdfStreamCommonDictionary::Length() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Length", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfStreamCommonDictionary::getFilterAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Filter", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfArray* SkPdfStreamCommonDictionary::getFilterAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Filter", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfStreamCommonDictionary::getDecodeParmsAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DecodeParms", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfStreamCommonDictionary::getDecodeParmsAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DecodeParms", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfFileSpec SkPdfStreamCommonDictionary::F() const {
  SkPdfFileSpec ret;
  if (FileSpecFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "F", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

std::string SkPdfStreamCommonDictionary::getFFilterAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FFilter", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfArray* SkPdfStreamCommonDictionary::getFFilterAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FFilter", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfStreamCommonDictionary::getFDecodeParmsAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FDecodeParms", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfStreamCommonDictionary::getFDecodeParmsAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FDecodeParms", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

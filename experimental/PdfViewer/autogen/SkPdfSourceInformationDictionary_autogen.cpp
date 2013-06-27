#include "SkPdfSourceInformationDictionary_autogen.h"

std::string SkPdfSourceInformationDictionary::getAUAsString() const {
  std::string ret = "";
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "AU", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfSourceInformationDictionary::getAUAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "AU", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDate SkPdfSourceInformationDictionary::TS() const {
  SkPdfDate ret;
  if (DateFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfDate();
}

SkPdfDate SkPdfSourceInformationDictionary::E() const {
  SkPdfDate ret;
  if (DateFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "E", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfDate();
}

long SkPdfSourceInformationDictionary::S() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "S", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfDictionary* SkPdfSourceInformationDictionary::C() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "C", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

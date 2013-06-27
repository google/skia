#include "SkPdfCMapDictionary_autogen.h"

std::string SkPdfCMapDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfCMapDictionary::CMapName() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CMapName", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfCMapDictionary::getCIDSystemInfoAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDSystemInfo", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfCMapDictionary::getCIDSystemInfoAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDSystemInfo", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

long SkPdfCMapDictionary::WMode() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "WMode", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfCMapDictionary::getUseCMapAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "UseCMap", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfStream* SkPdfCMapDictionary::getUseCMapAsStream() const {
  SkPdfStream* ret = NULL;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "UseCMap", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}


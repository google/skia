#include "SkPdfAppearanceDictionary_autogen.h"

SkPdfStream* SkPdfAppearanceDictionary::getNAsStream() const {
  SkPdfStream* ret = NULL;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "N", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfAppearanceDictionary::getNAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "N", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfStream* SkPdfAppearanceDictionary::getRAsStream() const {
  SkPdfStream* ret = NULL;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "R", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfAppearanceDictionary::getRAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "R", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfStream* SkPdfAppearanceDictionary::getDAsStream() const {
  SkPdfStream* ret = NULL;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "D", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfAppearanceDictionary::getDAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "D", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

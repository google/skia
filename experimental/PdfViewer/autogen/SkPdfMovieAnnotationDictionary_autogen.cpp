#include "SkPdfMovieAnnotationDictionary_autogen.h"

std::string SkPdfMovieAnnotationDictionary::Subtype() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfMovieAnnotationDictionary::Movie() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Movie", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfMovieAnnotationDictionary::Contents() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Contents", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

bool SkPdfMovieAnnotationDictionary::getAAsBoolean() const {
  bool ret = false;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "A", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

SkPdfDictionary* SkPdfMovieAnnotationDictionary::getAAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "A", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}


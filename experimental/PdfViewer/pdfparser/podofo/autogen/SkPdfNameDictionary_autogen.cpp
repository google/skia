#include "SkPdfNameDictionary_autogen.h"

std::string SkPdfNameDictionary::getDestsAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Dests", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getDestsAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Dests", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfNameDictionary::getAPAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "AP", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getAPAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "AP", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfNameDictionary::getJavaScriptAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "JavaScript", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getJavaScriptAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "JavaScript", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfNameDictionary::getPagesAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Pages", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getPagesAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Pages", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfNameDictionary::getTemplatesAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Templates", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getTemplatesAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Templates", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfNameDictionary::getIDSAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "IDS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getIDSAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "IDS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfNameDictionary::getURLSAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "URLS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getURLSAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "URLS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfNameDictionary::getEmbeddedFilesAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EmbeddedFiles", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfNameDictionary::getEmbeddedFilesAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EmbeddedFiles", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

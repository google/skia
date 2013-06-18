#ifndef __DEFINED__SkPdfImageDictionary
#define __DEFINED__SkPdfImageDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfXObjectDictionary_autogen.h"

class SkPdfImageDictionary : public SkPdfXObjectDictionary {
public:
  virtual SkPdfObjectType getType() const { return kObjectDictionaryXObjectDictionaryImageDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectDictionaryXObjectDictionaryImageDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfImageDictionary* asImageDictionary() {return this;}
  virtual const SkPdfImageDictionary* asImageDictionary() const {return this;}

private:
  virtual SkPdfType1FormDictionary* asType1FormDictionary() {return NULL;}
  virtual const SkPdfType1FormDictionary* asType1FormDictionary() const {return NULL;}

public:
private:
public:
  SkPdfImageDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfXObjectDictionary(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfImageDictionary& operator=(const SkPdfImageDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

  std::string Type() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  std::string Subtype() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  long Width() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Width", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

  long Height() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Height", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

  bool isColorSpaceAName() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ColorSpace", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Name;
  }

  std::string getColorSpaceAsName() const {
    std::string ret = "";
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ColorSpace", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  bool isColorSpaceAArray() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ColorSpace", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Array;
  }

  SkPdfArray getColorSpaceAsArray() const {
    SkPdfArray ret = SkPdfArray();
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ColorSpace", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

  long BitsPerComponent() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

  std::string Intent() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Intent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  bool ImageMask() const {
    bool ret;
    if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ImageMask", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return false;
  }

  bool isMaskAStream() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Mask", "", &ret)) return false;
    return ret->podofo()->HasStream();
  }

  SkPdfStream getMaskAsStream() const {
    SkPdfStream ret = SkPdfStream();
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Mask", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfStream();
  }

  bool isMaskAArray() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Mask", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Array;
  }

  SkPdfArray getMaskAsArray() const {
    SkPdfArray ret = SkPdfArray();
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Mask", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

  SkPdfStream SMask() const {
    SkPdfStream ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SMask", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfStream();
  }

  SkPdfArray Decode() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Decode", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

  bool Interpolate() const {
    bool ret;
    if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Interpolate", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return false;
  }

  SkPdfArray Alternates() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Alternates", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

  std::string Name() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Name", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  long StructParent() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "StructParent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

  std::string ID() const {
    std::string ret;
    if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ID", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  SkPdfDictionary* OPI() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "OPI", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

  SkPdfStream Metadata() const {
    SkPdfStream ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Metadata", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfStream();
  }

};

#endif  // __DEFINED__SkPdfImageDictionary

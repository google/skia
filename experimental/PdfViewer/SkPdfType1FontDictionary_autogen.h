#ifndef __DEFINED__SkPdfType1FontDictionary
#define __DEFINED__SkPdfType1FontDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfFontDictionary_autogen.h"

class SkPdfType1FontDictionary : public SkPdfFontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kObjectDictionaryFontDictionaryType1FontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return kObjectDictionaryFontDictionaryType1FontDictionary__End_SkPdfObjectType;}
public:
  virtual SkPdfType1FontDictionary* asType1FontDictionary() {return this;}
  virtual const SkPdfType1FontDictionary* asType1FontDictionary() const {return this;}

private:
  virtual SkPdfCIDFontDictionary* asCIDFontDictionary() {return NULL;}
  virtual const SkPdfCIDFontDictionary* asCIDFontDictionary() const {return NULL;}

  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

  virtual SkPdfType0FontDictionary* asType0FontDictionary() {return NULL;}
  virtual const SkPdfType0FontDictionary* asType0FontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfType1FontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfFontDictionary(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfType1FontDictionary& operator=(const SkPdfType1FontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

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

  std::string Name() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Name", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  std::string BaseFont() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BaseFont", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  long FirstChar() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FirstChar", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

  long LastChar() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LastChar", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

  SkPdfArray Widths() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Widths", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

  SkPdfDictionary* FontDescriptor() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontDescriptor", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

  bool isEncodingAName() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Name;
  }

  std::string getEncodingAsName() const {
    std::string ret = "";
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  bool isEncodingADictionary() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Dictionary;
  }

  SkPdfDictionary* getEncodingAsDictionary() const {
    SkPdfDictionary* ret = NULL;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

  SkPdfStream ToUnicode() const {
    SkPdfStream ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ToUnicode", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfStream();
  }

};

#endif  // __DEFINED__SkPdfType1FontDictionary

#ifndef __DEFINED__SkPdfDictionary
#define __DEFINED__SkPdfDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfObject_autogen.h"

class SkPdfDictionary : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return kObjectDictionary__End_SkPdfObjectType;}
public:
  virtual SkPdfDictionary* asDictionary() {return this;}
  virtual const SkPdfDictionary* asDictionary() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfHexString* asHexString() {return NULL;}
  virtual const SkPdfHexString* asHexString() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfNumber* asNumber() {return NULL;}
  virtual const SkPdfNumber* asNumber() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfString* asString() {return NULL;}
  virtual const SkPdfString* asString() const {return NULL;}

public:
private:
public:
  SkPdfDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfObject(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfDictionary& operator=(const SkPdfDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

};

#endif  // __DEFINED__SkPdfDictionary

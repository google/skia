enum SkPdfObjectType {
  kObject_SkPdfObjectType,
  kObjectArray_SkPdfObjectType,
  kObjectBoolean_SkPdfObjectType,
  kObjectDictionary_SkPdfObjectType,
  kObjectDictionaryXObject_SkPdfObjectType,
  kObjectDictionaryXObjectForm_SkPdfObjectType,
  kObjectDictionaryXObjectImage_SkPdfObjectType,
  kObjectDictionaryXObject__End_SkPdfObjectType,
  kObjectDictionary__End_SkPdfObjectType,
  kObjectInteger_SkPdfObjectType,
  kObjectName_SkPdfObjectType,
  kObjectNull_SkPdfObjectType,
  kObjectReal_SkPdfObjectType,
  kObjectReference_SkPdfObjectType,
  kObjectStream_SkPdfObjectType,
  kObject__End_SkPdfObjectType,
};

class SkPdfObject;
class SkPdfNull;
class SkPdfBoolean;
class SkPdfInteger;
class SkPdfReal;
class SkPdfName;
class SkPdfStream;
class SkPdfReference;
class SkPdfArray;
class SkPdfDictionary;
class SkPdfXObject;
class SkPdfImage;
class SkPdfForm;

class SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObject_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return kObject__End_SkPdfObjectType;}
public:
  virtual SkPdfObject* asObject() {return this;}
  virtual const SkPdfObject* asObject() const {return this;}

  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
protected:
  const PdfMemDocument* fPodofoDoc;
  const PdfObject* fPodofoObj;

public:
  SkPdfObject(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : fPodofoDoc(podofoDoc), fPodofoObj(podofoObj) {}
  const PdfObject* podofo() const { return fPodofoObj;}
  virtual bool valid() const {return true;}
};


class SkPdfNull : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectNull_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectNull_SkPdfObjectType + 1);}
public:
  virtual SkPdfNull* asNull() {return this;}
  virtual const SkPdfNull* asNull() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfNull(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfBoolean : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectBoolean_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectBoolean_SkPdfObjectType + 1);}
public:
  virtual SkPdfBoolean* asBoolean() {return this;}
  virtual const SkPdfBoolean* asBoolean() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfBoolean(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfInteger : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectInteger_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectInteger_SkPdfObjectType + 1);}
public:
  virtual SkPdfInteger* asInteger() {return this;}
  virtual const SkPdfInteger* asInteger() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfInteger(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfReal : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectReal_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectReal_SkPdfObjectType + 1);}
public:
  virtual SkPdfReal* asReal() {return this;}
  virtual const SkPdfReal* asReal() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfReal(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfName : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectName_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectName_SkPdfObjectType + 1);}
public:
  virtual SkPdfName* asName() {return this;}
  virtual const SkPdfName* asName() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfName(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfStream : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectStream_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectStream_SkPdfObjectType + 1);}
public:
  virtual SkPdfStream* asStream() {return this;}
  virtual const SkPdfStream* asStream() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

public:
private:
public:
 SkPdfStream(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfReference : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectReference_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectReference_SkPdfObjectType + 1);}
public:
  virtual SkPdfReference* asReference() {return this;}
  virtual const SkPdfReference* asReference() const {return this;}

private:
  virtual SkPdfArray* asArray() {return NULL;}
  virtual const SkPdfArray* asArray() const {return NULL;}

  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfReference(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfArray : public SkPdfObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectArray_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectArray_SkPdfObjectType + 1);}
public:
  virtual SkPdfArray* asArray() {return this;}
  virtual const SkPdfArray* asArray() const {return this;}

private:
  virtual SkPdfBoolean* asBoolean() {return NULL;}
  virtual const SkPdfBoolean* asBoolean() const {return NULL;}

  virtual SkPdfDictionary* asDictionary() {return NULL;}
  virtual const SkPdfDictionary* asDictionary() const {return NULL;}

  virtual SkPdfXObject* asXObject() {return NULL;}
  virtual const SkPdfXObject* asXObject() const {return NULL;}

  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfArray(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


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

  virtual SkPdfInteger* asInteger() {return NULL;}
  virtual const SkPdfInteger* asInteger() const {return NULL;}

  virtual SkPdfName* asName() {return NULL;}
  virtual const SkPdfName* asName() const {return NULL;}

  virtual SkPdfNull* asNull() {return NULL;}
  virtual const SkPdfNull* asNull() const {return NULL;}

  virtual SkPdfReal* asReal() {return NULL;}
  virtual const SkPdfReal* asReal() const {return NULL;}

  virtual SkPdfReference* asReference() {return NULL;}
  virtual const SkPdfReference* asReference() const {return NULL;}

  virtual SkPdfStream* asStream() {return NULL;}
  virtual const SkPdfStream* asStream() const {return NULL;}

public:
private:
public:
 SkPdfDictionary(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
};


class SkPdfXObject : public SkPdfDictionary {
public:
  virtual SkPdfObjectType getType() const { return kObjectDictionaryXObject_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return kObjectDictionaryXObject__End_SkPdfObjectType;}
public:
  virtual SkPdfXObject* asXObject() {return this;}
  virtual const SkPdfXObject* asXObject() const {return this;}

private:
public:
private:
public:
 SkPdfXObject(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfDictionary(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
  std::string t() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

};


class SkPdfImage : public SkPdfXObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectDictionaryXObjectImage_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectDictionaryXObjectImage_SkPdfObjectType + 1);}
public:
  virtual SkPdfImage* asImage() {return this;}
  virtual const SkPdfImage* asImage() const {return this;}

private:
  virtual SkPdfForm* asForm() {return NULL;}
  virtual const SkPdfForm* asForm() const {return NULL;}

public:
private:
  SkBitmap bitmap;
public:
 SkPdfImage(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfXObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
  std::string t() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  std::string s() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  long w() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Width", "W", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return -1;
  }

  long h() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Height", "H", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return -1;
  }

  std::string cs() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ColorSpace", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  long bpc() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "BPC", &ret)) return ret;
    return 1;
  }

};


class SkPdfForm : public SkPdfXObject {
public:
  virtual SkPdfObjectType getType() const { return kObjectDictionaryXObjectForm_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kObjectDictionaryXObjectForm_SkPdfObjectType + 1);}
public:
  virtual SkPdfForm* asForm() {return this;}
  virtual const SkPdfForm* asForm() const {return this;}

private:
  virtual SkPdfImage* asImage() {return NULL;}
  virtual const SkPdfImage* asImage() const {return NULL;}

public:
  void test() {}
private:
public:
 SkPdfForm(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdfXObject(podofoDoc, podofoObj) {}
  virtual bool valid() const {return true;}
  std::string t() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  std::string s() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

};


class PodofoMapper {
public:
  static bool mapObject(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAObject(podofoDoc, podofoObj)) return false;

    if (mapArray(podofoDoc, podofoObj, out)) return true;
    if (mapBoolean(podofoDoc, podofoObj, out)) return true;
    if (mapDictionary(podofoDoc, podofoObj, out)) return true;
    if (mapInteger(podofoDoc, podofoObj, out)) return true;
    if (mapName(podofoDoc, podofoObj, out)) return true;
    if (mapNull(podofoDoc, podofoObj, out)) return true;
    if (mapReal(podofoDoc, podofoObj, out)) return true;
    if (mapReference(podofoDoc, podofoObj, out)) return true;
    if (mapStream(podofoDoc, podofoObj, out)) return true;

    *out = new SkPdfObject(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapNull(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isANull(podofoDoc, podofoObj)) return false;


    *out = new SkPdfNull(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapBoolean(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isABoolean(podofoDoc, podofoObj)) return false;


    *out = new SkPdfBoolean(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapInteger(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAInteger(podofoDoc, podofoObj)) return false;


    *out = new SkPdfInteger(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapReal(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAReal(podofoDoc, podofoObj)) return false;


    *out = new SkPdfReal(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapName(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAName(podofoDoc, podofoObj)) return false;


    *out = new SkPdfName(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapStream(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAStream(podofoDoc, podofoObj)) return false;


    *out = new SkPdfStream(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapReference(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAReference(podofoDoc, podofoObj)) return false;


    *out = new SkPdfReference(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapArray(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAArray(podofoDoc, podofoObj)) return false;


    *out = new SkPdfArray(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isADictionary(podofoDoc, podofoObj)) return false;

    if (mapXObject(podofoDoc, podofoObj, out)) return true;

    *out = new SkPdfDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapXObject(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAXObject(podofoDoc, podofoObj)) return false;

    if (mapForm(podofoDoc, podofoObj, out)) return true;
    if (mapImage(podofoDoc, podofoObj, out)) return true;

    *out = new SkPdfXObject(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapImage(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAImage(podofoDoc, podofoObj)) return false;


    *out = new SkPdfImage(&podofoDoc, &podofoObj);
    return true;
  }

  static bool mapForm(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isAForm(podofoDoc, podofoObj)) return false;


    *out = new SkPdfForm(&podofoDoc, &podofoObj);
    return true;
  }

  static bool isAObject(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isANull(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isABoolean(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isAInteger(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isAReal(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isAName(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isAStream(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isAReference(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isAArray(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return false;
  }

  static bool isADictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isAXObject(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    std::string t;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Type", "", &t)) return false;
    if (t != "XObject") return false;

    return true;
  }

  static bool isAImage(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    std::string t;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Type", "", &t)) return false;
    if (t != "XObject") return false;

    std::string s;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &s)) return false;
    if (s != "Image") return false;

    return true;
  }

  static bool isAForm(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    std::string t;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Type", "", &t)) return false;
    if (t != "XObject") return false;

    std::string s;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &s)) return false;
    if (s != "Form") return false;

    return true;
  }

};

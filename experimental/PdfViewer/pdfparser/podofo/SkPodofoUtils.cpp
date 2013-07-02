
#include "SkPodofoUtils.h"
#include "SkMatrix.h"
#include "SkPdfHeaders_autogen.h"
#include "SkPdfMapper_autogen.h"

#include "podofo.h"

const PoDoFo::PdfObject* resolveReferenceObject(const SkPodofoParsedPDF* pdfDoc,
                                  const PoDoFo::PdfObject* obj,
                                  bool resolveOneElementArrays) {
    while (obj && (obj->IsReference() || (resolveOneElementArrays &&
                                          obj->IsArray() &&
                                          obj->GetArray().GetSize() == 1))) {
        if (obj->IsReference()) {
            // We need to force the non const, the only update we will do is for recurssion checks.
            PoDoFo::PdfReference& ref = (PoDoFo::PdfReference&)obj->GetReference();
            obj = pdfDoc->podofo()->GetObjects().GetObject(ref);
        } else {
            obj = &obj->GetArray()[0];
        }
    }

    return obj;
}

// TODO(edisonn): deal with synonyms (/BPC == /BitsPerComponent), here or in GetKey?
// Always pass long form in key, and have a map of long -> short key
bool LongFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        long* data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)));

    if (value == NULL || !value->IsNumber()) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    *data = value->GetNumber();
    return true;
}

bool LongFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        long* data) {
    if (LongFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return LongFromDictionary(pdfDoc, dict, abr, data);
}

bool DoubleFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          double* data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)));

    if (value == NULL || (!value->IsReal() && !value->IsNumber())) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    *data = value->GetReal();
    return true;
}

bool DoubleFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          double* data) {
    if (DoubleFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return DoubleFromDictionary(pdfDoc, dict, abr, data);
}


bool BoolFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        bool* data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)));

    if (value == NULL || !value->IsBool()) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    *data = value->GetBool();
    return true;
}

bool BoolFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        bool* data) {
    if (BoolFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return BoolFromDictionary(pdfDoc, dict, abr, data);
}

bool NameFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        std::string* data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)),
                                              true);
    if (value == NULL || !value->IsName()) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    *data = value->GetName().GetName();
    return true;
}

bool NameFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        std::string* data) {
    if (NameFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return NameFromDictionary(pdfDoc, dict, abr, data);
}

bool StringFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          std::string* data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)),
                                              true);
    if (value == NULL || (!value->IsString() && !value->IsHexString())) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    *data = value->GetString().GetString();
    return true;
}

bool StringFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          std::string* data) {
    if (StringFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return StringFromDictionary(pdfDoc, dict, abr, data);
}


bool SkMatrixFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                            const PoDoFo::PdfDictionary& dict,
                            const char* key,
                            SkMatrix** matrix) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                                    dict.GetKey(PoDoFo::PdfName(key)));

    if (value == NULL || !value->IsArray()) {
        return false;
    }

    if (value->GetArray().GetSize() != 6) {
        return false;
    }

    double array[6];
    for (int i = 0; i < 6; i++) {
        const PoDoFo::PdfObject* elem = resolveReferenceObject(pdfDoc, &value->GetArray()[i]);
        if (elem == NULL || (!elem->IsReal() && !elem->IsNumber())) {
            return false;
        }
        array[i] = elem->GetReal();
    }

    *matrix = new SkMatrix();
    **matrix = SkMatrixFromPdfMatrix(array);
    return true;
}

bool SkMatrixFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkMatrix** data) {
    if (SkMatrixFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return SkMatrixFromDictionary(pdfDoc, dict, abr, data);

}

bool SkRectFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          SkRect** rect) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                                    dict.GetKey(PoDoFo::PdfName(key)));

    if (value == NULL || !value->IsArray()) {
        return false;
    }

    if (value->GetArray().GetSize() != 4) {
        return false;
    }

    double array[4];
    for (int i = 0; i < 4; i++) {
        const PoDoFo::PdfObject* elem = resolveReferenceObject(pdfDoc, &value->GetArray()[i]);
        if (elem == NULL || (!elem->IsReal() && !elem->IsNumber())) {
            return false;
        }
        array[i] = elem->GetReal();
    }

    *rect = new SkRect();
    **rect = SkRect::MakeLTRB(SkDoubleToScalar(array[0]),
                              SkDoubleToScalar(array[1]),
                              SkDoubleToScalar(array[2]),
                              SkDoubleToScalar(array[3]));
    return true;
}

bool SkRectFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkRect** data) {
    if (SkRectFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return SkRectFromDictionary(pdfDoc, dict, abr, data);

}


SkPdfObject* get(const SkPdfObject* obj, const char* key, const char* abr = "") {
    PoDoFo::PdfObject* podofoObj = NULL;
    if (obj == NULL) return NULL;
    const SkPdfDictionary* dict = obj->asDictionary();
    if (dict == NULL) return NULL;
    if (!dict->podofo()->IsDictionary()) return NULL;
    ObjectFromDictionary(dict->doc(), dict->podofo()->GetDictionary(), key, abr, &podofoObj);
    SkPdfObject* ret = NULL;
    obj->doc()->mapper()->mapObject(podofoObj, &ret);
    return ret;
}

bool ArrayFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                         const PoDoFo::PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfArray* data) {return false;}

bool FileSpecFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                         const PoDoFo::PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfFileSpec* data) {return false;}

bool StreamFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                         const PoDoFo::PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfStream** data);

bool TreeFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                         const PoDoFo::PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfTree** data) {return false;}

bool DateFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                         const PoDoFo::PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfDate* data) {return false;}

bool FunctionFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                         const PoDoFo::PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfFunction* data) {return false;}




bool ArrayFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                              const PoDoFo::PdfDictionary& dict,
                              const char* key,
                              SkPdfArray** data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)),
                                              true);
    if (value == NULL || !value->IsArray()) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    return pdfDoc->mapper()->mapArray(value, data);
}


bool ArrayFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfArray** data) {
    if (ArrayFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return ArrayFromDictionary(pdfDoc, dict, abr, data);
}

/*
bool DictionaryFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                              const PoDoFo::PdfDictionary& dict,
                              const char* key,
                              SkPoDoFo::PdfDictionary** data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)),
                                              true);
    if (value == NULL || !value->IsDictionary()) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    return pdfDoc->mapper()->mapDictionary(value, data);
}

bool DictionaryFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPoDoFo::PdfDictionary** data) {
    if (DictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return DictionaryFromDictionary(pdfDoc, dict, abr, data);
}
*/

bool ObjectFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          PoDoFo::PdfObject** data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)),
                                              true);
    if (value == NULL) {
        return false;
    }
    if (data == NULL) {
        return true;
    }
    *data = (PoDoFo::PdfObject*)value;
    return true;
}

bool ObjectFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        PoDoFo::PdfObject** data) {
    if (ObjectFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return ObjectFromDictionary(pdfDoc, dict, abr, data);
}

bool StreamFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          SkPdfStream** data) {
    const PoDoFo::PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PoDoFo::PdfName(key)),
                                              true);
    if (value == NULL) {
        return false;
    }
    if (data == NULL) {
        return true;
    }
    return pdfDoc->mapper()->mapStream(value, data);
}

bool StreamFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfStream** data) {
    if (StreamFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return StreamFromDictionary(pdfDoc, dict, abr, data);
}



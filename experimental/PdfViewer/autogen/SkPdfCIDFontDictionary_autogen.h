#ifndef __DEFINED__SkPdfCIDFontDictionary
#define __DEFINED__SkPdfCIDFontDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfFontDictionary_autogen.h"

// Entries in a CIDFont dictionary
class SkPdfCIDFontDictionary : public SkPdfFontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kCIDFontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kCIDFontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfCIDFontDictionary* asCIDFontDictionary() {return this;}
  virtual const SkPdfCIDFontDictionary* asCIDFontDictionary() const {return this;}

private:
  virtual SkPdfType0FontDictionary* asType0FontDictionary() {return NULL;}
  virtual const SkPdfType0FontDictionary* asType0FontDictionary() const {return NULL;}

  virtual SkPdfType1FontDictionary* asType1FontDictionary() {return NULL;}
  virtual const SkPdfType1FontDictionary* asType1FontDictionary() const {return NULL;}

  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfCIDFontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfFontDictionary(podofoDoc, podofoObj) {}

  SkPdfCIDFontDictionary(const SkPdfCIDFontDictionary& from) : SkPdfFontDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfCIDFontDictionary& operator=(const SkPdfCIDFontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Required) The type of PDF object that this dictionary describes; must be
 *  Font for a CIDFont dictionary.
**/
  bool has_Type() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", NULL));
  }

  std::string Type() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Required) The type of CIDFont; CIDFontType0 or CIDFontType2.
**/
  bool has_Subtype() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", NULL));
  }

  std::string Subtype() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Required) The PostScript name of the CIDFont. For Type 0 CIDFonts, this
 *  is usually the value of the CIDFontName entry in the CIDFont program. For
 *  Type 2 CIDFonts, it is derived the same way as for a simple TrueType font;
 *  see Section 5.5.2, "TrueType Fonts." In either case, the name can have a sub-
 *  set prefix if appropriate; see Section 5.5.3, "Font Subsets."
**/
  bool has_BaseFont() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BaseFont", "", NULL));
  }

  std::string BaseFont() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BaseFont", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Required) A dictionary containing entries that define the character collec-
 *  tion of the CIDFont. See Table 5.12 on page 337.
**/
  bool has_CIDSystemInfo() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDSystemInfo", "", NULL));
  }

  SkPdfDictionary* CIDSystemInfo() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDSystemInfo", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required; must be an indirect reference) A font descriptor describing the
 *  CIDFont's default metrics other than its glyph widths (see Section 5.7,
 *  "Font Descriptors").
**/
  bool has_FontDescriptor() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontDescriptor", "", NULL));
  }

  SkPdfDictionary* FontDescriptor() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontDescriptor", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional) The default width for glyphs in the CIDFont (see "Glyph Met-
 *  rics in CIDFonts" on page 340). Default value: 1000.
**/
  bool has_DW() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DW", "", NULL));
  }

  long DW() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DW", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Optional) A description of the widths for the glyphs in the CIDFont. The
 *  array's elements have a variable format that can specify individual widths
 *  for consecutive CIDs or one width for a range of CIDs (see "Glyph Metrics
 *  in CIDFonts" on page 340). Default value: none (the DW value is used for
 *  all glyphs).
**/
  bool has_W() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "W", "", NULL));
  }

  SkPdfArray* W() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "W", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; applies only to CIDFonts used for vertical writing) An array of two
 *  numbers specifying the default metrics for vertical writing (see "Glyph
 *  Metrics in CIDFonts" on page 340). Default value: [880 -1000].
**/
  bool has_DW2() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DW2", "", NULL));
  }

  SkPdfArray* DW2() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DW2", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; applies only to CIDFonts used for vertical writing) A description of
 *  the metrics for vertical writing for the glyphs in the CIDFont (see "Glyph
 *  Metrics in CIDFonts" on page 340). Default value: none (the DW2 value is
 *  used for all glyphs).
**/
  bool has_W2() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "W2", "", NULL));
  }

  SkPdfArray* W2() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "W2", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; Type 2 CIDFonts only) A specification of the mapping from CIDs
 *  to glyph indices. If the value is a stream, the bytes in the stream contain the
 *  mapping from CIDs to glyph indices: the glyph index for a particular CID
 *  value c is a 2-byte value stored in bytes 2 x c and 2 x c + 1, where the first
 *  byte is the high-order byte. If the value of CIDToGIDMap is a name, it must
 *  be Identity, indicating that the mapping between CIDs and glyph indices is
 *  the identity mapping. Default value: Identity.
 *  This entry may appear only in a Type 2 CIDFont whose associated True-
 *  Type font program is embedded in the PDF file (see the next section).
**/
  bool has_CIDToGIDMap() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDToGIDMap", "", NULL));
  }

  bool isCIDToGIDMapAStream() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDToGIDMap", "", &ret)) return false;
    return ret->podofo()->HasStream();
  }

  SkPdfStream* getCIDToGIDMapAsStream() const {
    SkPdfStream* ret = NULL;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDToGIDMap", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

  bool isCIDToGIDMapAName() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDToGIDMap", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Name;
  }

  std::string getCIDToGIDMapAsName() const {
    std::string ret = "";
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CIDToGIDMap", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

};

#endif  // __DEFINED__SkPdfCIDFontDictionary

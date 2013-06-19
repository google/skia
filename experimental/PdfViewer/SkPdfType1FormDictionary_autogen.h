#ifndef __DEFINED__SkPdfType1FormDictionary
#define __DEFINED__SkPdfType1FormDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfXObjectDictionary_autogen.h"

// Additional entries specific to a type 1 form dictionary
class SkPdfType1FormDictionary : public SkPdfXObjectDictionary {
public:
  virtual SkPdfObjectType getType() const { return kType1FormDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kType1FormDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfType1FormDictionary* asType1FormDictionary() {return this;}
  virtual const SkPdfType1FormDictionary* asType1FormDictionary() const {return this;}

private:
  virtual SkPdfImageDictionary* asImageDictionary() {return NULL;}
  virtual const SkPdfImageDictionary* asImageDictionary() const {return NULL;}

public:
private:
public:
  SkPdfType1FormDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfXObjectDictionary(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfType1FormDictionary& operator=(const SkPdfType1FormDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Optional) The type of PDF object that this dictionary describes; if present,
 *  must be XObject for a form XObject.
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

/** (Required) The type of XObject that this dictionary describes; must be Form
 *  for a form XObject.
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

/** (Optional) A code identifying the type of form XObject that this dictionary
 *  describes. The only valid value defined at the time of publication is 1. Default
 *  value: 1.
**/
  bool has_FormType() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FormType", "", NULL));
  }

  long FormType() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FormType", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required in PDF 1.0; optional otherwise) The name by which this form
 *  XObject is referenced in the XObject subdictionary of the current resource
 *  dictionary (see Section 3.7.2, "Resource Dictionaries").
 *  Note: This entry is obsolescent and its use is no longer recommended. (See
 *  implementation note 38 in Appendix H.)
**/
  bool has_Name() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Name", "", NULL));
  }

  std::string Name() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Name", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Required if PieceInfo is present; optional otherwise; PDF 1.3) The date and
 *  time (see Section 3.8.2, "Dates") when the form XObject's contents were
 *  most recently modified. If a page-piece dictionary (PieceInfo) is present, the
 *  modification date is used to ascertain which of the application data diction-
 *  aries it contains correspond to the current content of the form (see Section
 *  9.4, "Page-Piece Dictionaries").
**/
  bool has_LastModified() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LastModified", "", NULL));
  }

  SkPdfDate LastModified() const {
    SkPdfDate ret;
    if (DateFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LastModified", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfDate();
  }

/** (Required) An array of four numbers in the form coordinate system (see
 *  below), giving the coordinates of the left, bottom, right, and top edges,
 *  respectively, of the form XObject's bounding box. These boundaries are used
 *  to clip the form XObject and to determine its size for caching.
**/
  bool has_BBox() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BBox", "", NULL));
  }

  SkRect BBox() const {
    SkRect ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkRect();
  }

/** (Optional) An array of six numbers specifying the form matrix, which maps
 *  form space into user space (see Section 4.2.3, "Transformation Matrices").
 *  Default value: the identity matrix [1 0 0 1 0 0].
**/
  bool has_Matrix() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Matrix", "", NULL));
  }

  SkPdfArray Matrix() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Matrix", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

/** (Optional but strongly recommended; PDF 1.2) A dictionary specifying any
 *  resources (such as fonts and images) required by the form XObject (see Sec-
 *  tion 3.7, "Content Streams and Resources").
 *  In PDF 1.1 and earlier, all named resources used in the form XObject must be
 *  included in the resource dictionary of each page object on which the form
 *  XObject appears, whether or not they also appear in the resource dictionary
 *  of the form XObject itself. It can be useful to specify these resources in the
 *  form XObject's own resource dictionary as well, in order to determine which
 *  resources are used inside the form XObject. If a resource is included in both
 *  dictionaries, it should have the same name in both locations.
 *       In PDF 1.2 and later versions, form XObjects can be independent of the
 *       content streams in which they appear, and this is strongly recommended
 *       although not required. In an independent form XObject, the resource dic-
 *       tionary of the form XObject is required and contains all named resources
 *       used by the form XObject. These resources are not "promoted" to the outer
 *       content stream's resource dictionary, although that stream's resource diction-
 *       ary will refer to the form XObject itself.
**/
  bool has_Resources() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Resources", "", NULL));
  }

  SkPdfDictionary* Resources() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Resources", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.4) A group attributes dictionary indicating that the contents
 *  of the form XObject are to be treated as a group and specifying the attributes
 *  of that group (see Section 4.9.2, "Group XObjects").
 *  Note: If a Ref entry (see below) is present, the group attributes also apply to the
 *  external page imported by that entry. This allows such an imported page to be
 *  treated as a group without further modification.
**/
  bool has_Group() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Group", "", NULL));
  }

  SkPdfDictionary* Group() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Group", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.4) A reference dictionary identifying a page to be imported
 *  from another PDF file, and for which the form XObject serves as a proxy (see
 *  Section 4.9.3, "Reference XObjects").
**/
  bool has_Ref() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Ref", "", NULL));
  }

  SkPdfDictionary* Ref() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Ref", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.4) A metadata stream containing metadata for the form
 *  XObject (see Section 9.2.2, "Metadata Streams").
**/
  bool has_Metadata() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Metadata", "", NULL));
  }

  SkPdfStream Metadata() const {
    SkPdfStream ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Metadata", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfStream();
  }

/** (Optional; PDF 1.3) A page-piece dictionary associated with the form
 *  XObject (see Section 9.4, "Page-Piece Dictionaries").
**/
  bool has_PieceInfo() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "PieceInfo", "", NULL));
  }

  SkPdfDictionary* PieceInfo() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "PieceInfo", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required if the form XObject is a structural content item; PDF 1.3) The integer
 *  key of the form XObject's entry in the structural parent tree (see "Finding
 *  Structure Elements from Content Items" on page 600).
**/
  bool has_StructParent() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "StructParent", "", NULL));
  }

  long StructParent() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "StructParent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required if the form XObject contains marked-content sequences that are struc-
 *  tural content items; PDF 1.3) The integer key of the form XObject's entry in
 *  the structural parent tree (see "Finding Structure Elements from Content
 *  Items" on page 600).
 *  Note: At most one of the entries StructParent or StructParents may be present. A
 *  form XObject can be either a content item in its entirety or a container for
 *  marked-content sequences that are content items, but not both.
**/
  bool has_StructParents() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "StructParents", "", NULL));
  }

  long StructParents() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "StructParents", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Optional; PDF 1.2) An OPI version dictionary for the form XObject (see
 *  Section 9.10.6, "Open Prepress Interface (OPI)").
**/
  bool has_OPI() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "OPI", "", NULL));
  }

  SkPdfDictionary* OPI() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "OPI", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

};

#endif  // __DEFINED__SkPdfType1FormDictionary

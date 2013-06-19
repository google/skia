#ifndef __DEFINED__SkPdfImageDictionary
#define __DEFINED__SkPdfImageDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfXObjectDictionary_autogen.h"

// Additional entries specific to an image dictionary
class SkPdfImageDictionary : public SkPdfXObjectDictionary {
public:
  virtual SkPdfObjectType getType() const { return kImageDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kImageDictionary_SkPdfObjectType + 1);}
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

/** (Optional) The type of PDF object that this dictionary describes; if
 *  present, must be XObject for an image XObject.
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

/** (Required) The type of XObject that this dictionary describes; must be
 *  Image for an image XObject.
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

/** (Required) The width of the image, in samples.
**/
  bool has_Width() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Width", "", NULL));
  }

  long Width() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Width", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required) The height of the image, in samples.
**/
  bool has_Height() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Height", "", NULL));
  }

  long Height() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Height", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required except for image masks; not allowed for image masks) The color
 *  space in which image samples are specified. This may be any type of color
 *  space except Pattern.
**/
  bool has_ColorSpace() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ColorSpace", "", NULL));
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

/** (Required except for image masks; optional for image masks) The number of
 *  bits used to represent each color component. Only a single value may be
 *  specified; the number of bits is the same for all color components. Valid
 *  values are 1, 2, 4, and 8. If ImageMask is true, this entry is optional, and if
 *  specified, its value must be 1.
 *  If the image stream uses a filter, the value of BitsPerComponent must be
 *  consistent with the size of the data samples that the filter delivers. In par-
 *  ticular, a CCITTFaxDecode or JBIG2Decode filter always delivers 1-bit sam-
 *  ples, a RunLengthDecode or DCTDecode filter delivers 8-bit samples, and
 *  an LZWDecode or FlateDecode filter delivers samples of a specified size if
 *  a predictor function is used.
**/
  bool has_BitsPerComponent() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "", NULL));
  }

  long BitsPerComponent() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Optional; PDF 1.1) The name of a color rendering intent to be used in
 *  rendering the image (see "Rendering Intents" on page 197). Default value:
 *  the current rendering intent in the graphics state.
**/
  bool has_Intent() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Intent", "", NULL));
  }

  std::string Intent() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Intent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Optional) A flag indicating whether the image is to be treated as an image
 *  mask (see Section 4.8.5, "Masked Images"). If this flag is true, the value of
 *  BitsPerComponent must be 1 and Mask and ColorSpace should not be
 *  specified; unmasked areas will be painted using the current nonstroking
 *  color. Default value: false.
**/
  bool has_ImageMask() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ImageMask", "", NULL));
  }

  bool ImageMask() const {
    bool ret;
    if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ImageMask", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return false;
  }

/** (Optional except for image masks; not allowed for image masks; PDF 1.3) An
 *  image XObject defining an image mask to be applied to this image (see
 *  "Explicit Masking" on page 277), or an array specifying a range of colors
 *  to be applied to it as a color key mask (see "Color Key Masking" on page
 *  277). If ImageMask is true, this entry must not be present. (See
 *  implementation note 35 in Appendix H.)
**/
  bool has_Mask() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Mask", "", NULL));
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

/** (Optional; PDF 1.4) A subsidiary image XObject defining a soft-mask
 *  image (see "Soft-Mask Images" on page 447) to be used as a source of
 *  mask shape or mask opacity values in the transparent imaging model. The
 *  alpha source parameter in the graphics state determines whether the mask
 *  values are interpreted as shape or opacity.
 *  If present, this entry overrides the current soft mask in the graphics state,
 *  as well as the image's Mask entry, if any. (However, the other transparency-
 *  related graphics state parameters-blend mode and alpha constant-
 *  remain in effect.) If SMask is absent, the image has no associated soft mask
 *  (although the current soft mask in the graphics state may still apply).
**/
  bool has_SMask() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SMask", "", NULL));
  }

  SkPdfStream SMask() const {
    SkPdfStream ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SMask", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfStream();
  }

/** (Optional) An array of numbers describing how to map image samples
 *  into the range of values appropriate for the image's color space (see
 *  "Decode Arrays" on page 271). If ImageMask is true, the array must be
 *  either [0 1] or [1 0]; otherwise, its length must be twice the number of
 *  color components required by ColorSpace. Default value: see "Decode
 *  Arrays" on page 271.
**/
  bool has_Decode() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Decode", "", NULL));
  }

  SkPdfArray Decode() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Decode", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

/** (Optional) A flag indicating whether image interpolation is to be per-
 *  formed (see "Image Interpolation" on page 273). Default value: false.
**/
  bool has_Interpolate() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Interpolate", "", NULL));
  }

  bool Interpolate() const {
    bool ret;
    if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Interpolate", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return false;
  }

/** (Optional; PDF 1.3) An array of alternate image dictionaries for this image
 *  (see "Alternate Images" on page 273). The order of elements within the
 *  array has no significance. This entry may not be present in an image
 *  XObject that is itself an alternate image.
**/
  bool has_Alternates() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Alternates", "", NULL));
  }

  SkPdfArray Alternates() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Alternates", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

/** (Required in PDF 1.0; optional otherwise) The name by which this image
 *  XObject is referenced in the XObject subdictionary of the current resource
 *  dictionary (see Section 3.7.2, "Resource Dictionaries").
 *  Note: This entry is obsolescent and its use is no longer recommended. (See
 *  implementation note 36 in Appendix H.)
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

/** (Required if the image is a structural content item; PDF 1.3) The integer key
 *  of the image's entry in the structural parent tree (see "Finding Structure
 *  Elements from Content Items" on page 600).
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

/** (Optional; PDF 1.3; indirect reference preferred) The digital identifier of the
 *  image's parent Web Capture content set (see Section 9.9.5, "Object At-
 *  tributes Related to Web Capture").
**/
  bool has_ID() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ID", "", NULL));
  }

  std::string ID() const {
    std::string ret;
    if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ID", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Optional; PDF 1.2) An OPI version dictionary for the image (see Section
 *  9.10.6, "Open Prepress Interface (OPI)"). If ImageMask is true, this entry
 *  is ignored.
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

/** (Optional; PDF 1.4) A metadata stream containing metadata for the image
 *  (see Section 9.2.2, "Metadata Streams").
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

};

#endif  // __DEFINED__SkPdfImageDictionary

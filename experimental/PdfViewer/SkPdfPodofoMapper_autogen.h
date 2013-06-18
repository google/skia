#ifndef __DEFINED__SkPdfPodofoMapper
#define __DEFINED__SkPdfPodofoMapper

#include "SkPdfHeaders_autogen.h"
class PodofoMapper {
public:
  static bool map(const SkPdfObject& in, SkPdfObject** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    if (!isObject(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfArray**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfBoolean**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfInteger**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfName**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfNull**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfReference**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfString**)out)) return true;

    *out = new SkPdfObject(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfNull** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNull** out) {
    if (!isNull(podofoDoc, podofoObj)) return false;


    *out = new SkPdfNull(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfBoolean** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBoolean** out) {
    if (!isBoolean(podofoDoc, podofoObj)) return false;


    *out = new SkPdfBoolean(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfInteger** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInteger** out) {
    if (!isInteger(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfNumber**)out)) return true;

    *out = new SkPdfInteger(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfNumber** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNumber** out) {
    if (!isNumber(podofoDoc, podofoObj)) return false;


    *out = new SkPdfNumber(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfName** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfName** out) {
    if (!isName(podofoDoc, podofoObj)) return false;


    *out = new SkPdfName(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfReference** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfReference** out) {
    if (!isReference(podofoDoc, podofoObj)) return false;


    *out = new SkPdfReference(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfArray** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfArray** out) {
    if (!isArray(podofoDoc, podofoObj)) return false;


    *out = new SkPdfArray(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfString** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfString** out) {
    if (!isString(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfHexString**)out)) return true;

    *out = new SkPdfString(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfHexString** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfHexString** out) {
    if (!isHexString(podofoDoc, podofoObj)) return false;


    *out = new SkPdfHexString(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDictionary** out) {
    if (!isDictionary(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfALinkAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfAlternateImageDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfAnnotationActionsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfAppearanceCharacteristicsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfAppearanceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfApplicationDataDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfArtifactsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfAttributeObjectDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfBeadDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfBlockLevelStructureElementsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfBorderStyleDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfBoxColorInformationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfBoxStyleDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCIDFontDescriptorDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCIDSystemInfoDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCMapDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCalgrayColorSpaceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCalrgbColorSpaceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCatalogDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCcittfaxdecodeFilterDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfCheckboxFieldDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfChoiceFieldDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfComponentsWithMetadataDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfDctdecodeFilterDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfDeviceNColorSpaceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfDocumentCatalogActionsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfDocumentInformationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfEmbeddedFileParameterDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfEmbeddedFileStreamDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfEmbeddedFontStreamDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfEncodingDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfEncryptedEmbeddedFileStreamDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfEncryptionCommonDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFCatalogDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFFieldDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFFileAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFNamedPageReferenceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFPageDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFTemplateDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFDFTrailerDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFieldDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFileAttachmentAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFileSpecificationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFileTrailerDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFontDescriptorDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFontDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFormFieldActionsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFreeTextAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfFunctionCommonDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfGoToActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfGraphicsStateDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfGroupAttributesDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfHideActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfIccProfileStreamDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfIconFitDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfImportDataActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfInkAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfInlineLevelStructureElementsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfInteractiveFormDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfJavascriptActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfJavascriptDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfJbig2DecodeFilterDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfLabColorSpaceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfLaunchActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfLineAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfListAttributeDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfLzwdecodeAndFlatedecodeFiltersDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMacOsFileInformationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMarkInformationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMarkedContentReferenceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMarkupAnnotationsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMetadataStreamDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMovieActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMovieActivationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMovieAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfMovieDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfNameDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfNameTreeNodeDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfNamedActionsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfNumberTreeNodeDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfObjectReferenceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfOpiVersionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfOutlineDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfOutlineItemDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPDF_XOutputIntentDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPSXobjectDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPageLabelDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPageObjectActionsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPageObjectDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPagePieceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPageTreeNodeDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPopUpAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPrinterMarkAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfPrinterMarkFormDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfRadioButtonFieldDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfReferenceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfRemoteGoToActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfResetFormActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfResourceDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfRubberStampAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSeparationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfShadingDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSignatureDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSoftMaskDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSoftMaskImageDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSoundActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSoundAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSoundObjectDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSourceInformationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSquareOrCircleAnnotation**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfStandardSecurityHandlerDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfStandardStructureDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfStreamCommonDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfStructureElementAccessDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfStructureElementDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfStructureTreeRootDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfSubmitFormActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTableAttributesDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTextAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTextFieldDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfThreadActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfThreadDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTransitionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTransparencyGroupDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTrapNetworkAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTrapNetworkAppearanceStreamDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType0FunctionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType10HalftoneDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType16HalftoneDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType1HalftoneDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType1PatternDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType2FunctionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType2PatternDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType3FunctionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType5HalftoneDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType6HalftoneDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfURIActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfURIDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfURLAliasDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfVariableTextFieldDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfViewerPreferencesDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWebCaptureCommandDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWebCaptureCommandSettingsDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWebCaptureDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWebCaptureImageSetDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWebCaptureInformationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWebCapturePageSetDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWidgetAnnotationDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfWindowsLaunchActionDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfXObjectDictionary**)out)) return true;

    *out = new SkPdfDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfXObjectDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfXObjectDictionary** out) {
    if (!isXObjectDictionary(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfImageDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType1FormDictionary**)out)) return true;

    *out = new SkPdfXObjectDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFontDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFontDictionary** out) {
    if (!isFontDictionary(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfCIDFontDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfTrueTypeFontDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType0FontDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType1FontDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType3FontDictionary**)out)) return true;

    *out = new SkPdfFontDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTrueTypeFontDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTrueTypeFontDictionary** out) {
    if (!isTrueTypeFontDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTrueTypeFontDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfStreamCommonDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStreamCommonDictionary** out) {
    if (!isStreamCommonDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfStreamCommonDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** out) {
    if (!isLzwdecodeAndFlatedecodeFiltersDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfLzwdecodeAndFlatedecodeFiltersDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCcittfaxdecodeFilterDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCcittfaxdecodeFilterDictionary** out) {
    if (!isCcittfaxdecodeFilterDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCcittfaxdecodeFilterDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfJbig2DecodeFilterDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfJbig2DecodeFilterDictionary** out) {
    if (!isJbig2DecodeFilterDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfJbig2DecodeFilterDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfDctdecodeFilterDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDctdecodeFilterDictionary** out) {
    if (!isDctdecodeFilterDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfDctdecodeFilterDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFileTrailerDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFileTrailerDictionary** out) {
    if (!isFileTrailerDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFileTrailerDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfEncryptionCommonDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEncryptionCommonDictionary** out) {
    if (!isEncryptionCommonDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfEncryptionCommonDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfStandardSecurityHandlerDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStandardSecurityHandlerDictionary** out) {
    if (!isStandardSecurityHandlerDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfStandardSecurityHandlerDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCatalogDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCatalogDictionary** out) {
    if (!isCatalogDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCatalogDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPageTreeNodeDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageTreeNodeDictionary** out) {
    if (!isPageTreeNodeDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPageTreeNodeDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPageObjectDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageObjectDictionary** out) {
    if (!isPageObjectDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPageObjectDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfNameDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNameDictionary** out) {
    if (!isNameDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfNameDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfResourceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfResourceDictionary** out) {
    if (!isResourceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfResourceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfNameTreeNodeDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNameTreeNodeDictionary** out) {
    if (!isNameTreeNodeDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfNameTreeNodeDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfNumberTreeNodeDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNumberTreeNodeDictionary** out) {
    if (!isNumberTreeNodeDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfNumberTreeNodeDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFunctionCommonDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFunctionCommonDictionary** out) {
    if (!isFunctionCommonDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFunctionCommonDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType0FunctionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType0FunctionDictionary** out) {
    if (!isType0FunctionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType0FunctionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType2FunctionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType2FunctionDictionary** out) {
    if (!isType2FunctionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType2FunctionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType3FunctionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType3FunctionDictionary** out) {
    if (!isType3FunctionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType3FunctionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFileSpecificationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFileSpecificationDictionary** out) {
    if (!isFileSpecificationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFileSpecificationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfEmbeddedFileStreamDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEmbeddedFileStreamDictionary** out) {
    if (!isEmbeddedFileStreamDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfEmbeddedFileStreamDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfEmbeddedFileParameterDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEmbeddedFileParameterDictionary** out) {
    if (!isEmbeddedFileParameterDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfEmbeddedFileParameterDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMacOsFileInformationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMacOsFileInformationDictionary** out) {
    if (!isMacOsFileInformationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMacOsFileInformationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfGraphicsStateDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfGraphicsStateDictionary** out) {
    if (!isGraphicsStateDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfGraphicsStateDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCalgrayColorSpaceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCalgrayColorSpaceDictionary** out) {
    if (!isCalgrayColorSpaceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCalgrayColorSpaceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCalrgbColorSpaceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCalrgbColorSpaceDictionary** out) {
    if (!isCalrgbColorSpaceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCalrgbColorSpaceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfLabColorSpaceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLabColorSpaceDictionary** out) {
    if (!isLabColorSpaceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfLabColorSpaceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfIccProfileStreamDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfIccProfileStreamDictionary** out) {
    if (!isIccProfileStreamDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfIccProfileStreamDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfDeviceNColorSpaceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDeviceNColorSpaceDictionary** out) {
    if (!isDeviceNColorSpaceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfDeviceNColorSpaceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType1PatternDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1PatternDictionary** out) {
    if (!isType1PatternDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType1PatternDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType2PatternDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType2PatternDictionary** out) {
    if (!isType2PatternDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType2PatternDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfShadingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfShadingDictionary** out) {
    if (!isShadingDictionary(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfType1ShadingDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType2ShadingDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType3ShadingDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType4ShadingDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType5ShadingDictionary**)out)) return true;
    if (map(podofoDoc, podofoObj, (SkPdfType6ShadingDictionary**)out)) return true;

    *out = new SkPdfShadingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType1ShadingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1ShadingDictionary** out) {
    if (!isType1ShadingDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType1ShadingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType2ShadingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType2ShadingDictionary** out) {
    if (!isType2ShadingDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType2ShadingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType3ShadingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType3ShadingDictionary** out) {
    if (!isType3ShadingDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType3ShadingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType4ShadingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType4ShadingDictionary** out) {
    if (!isType4ShadingDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType4ShadingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType5ShadingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType5ShadingDictionary** out) {
    if (!isType5ShadingDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType5ShadingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType6ShadingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType6ShadingDictionary** out) {
    if (!isType6ShadingDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType6ShadingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfImageDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfImageDictionary** out) {
    if (!isImageDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfImageDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfAlternateImageDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAlternateImageDictionary** out) {
    if (!isAlternateImageDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfAlternateImageDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType1FormDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1FormDictionary** out) {
    if (!isType1FormDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType1FormDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfGroupAttributesDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfGroupAttributesDictionary** out) {
    if (!isGroupAttributesDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfGroupAttributesDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfReferenceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfReferenceDictionary** out) {
    if (!isReferenceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfReferenceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPSXobjectDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPSXobjectDictionary** out) {
    if (!isPSXobjectDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPSXobjectDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType1FontDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1FontDictionary** out) {
    if (!isType1FontDictionary(podofoDoc, podofoObj)) return false;

    if (map(podofoDoc, podofoObj, (SkPdfMultiMasterFontDictionary**)out)) return true;

    *out = new SkPdfType1FontDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType3FontDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType3FontDictionary** out) {
    if (!isType3FontDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType3FontDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfEncodingDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEncodingDictionary** out) {
    if (!isEncodingDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfEncodingDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCIDSystemInfoDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCIDSystemInfoDictionary** out) {
    if (!isCIDSystemInfoDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCIDSystemInfoDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCIDFontDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCIDFontDictionary** out) {
    if (!isCIDFontDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCIDFontDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCMapDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCMapDictionary** out) {
    if (!isCMapDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCMapDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType0FontDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType0FontDictionary** out) {
    if (!isType0FontDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType0FontDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFontDescriptorDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFontDescriptorDictionary** out) {
    if (!isFontDescriptorDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFontDescriptorDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCIDFontDescriptorDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCIDFontDescriptorDictionary** out) {
    if (!isCIDFontDescriptorDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCIDFontDescriptorDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfEmbeddedFontStreamDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEmbeddedFontStreamDictionary** out) {
    if (!isEmbeddedFontStreamDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfEmbeddedFontStreamDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType1HalftoneDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1HalftoneDictionary** out) {
    if (!isType1HalftoneDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType1HalftoneDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType6HalftoneDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType6HalftoneDictionary** out) {
    if (!isType6HalftoneDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType6HalftoneDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType10HalftoneDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType10HalftoneDictionary** out) {
    if (!isType10HalftoneDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType10HalftoneDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType16HalftoneDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType16HalftoneDictionary** out) {
    if (!isType16HalftoneDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType16HalftoneDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfType5HalftoneDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType5HalftoneDictionary** out) {
    if (!isType5HalftoneDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfType5HalftoneDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSoftMaskDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoftMaskDictionary** out) {
    if (!isSoftMaskDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSoftMaskDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSoftMaskImageDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoftMaskImageDictionary** out) {
    if (!isSoftMaskImageDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSoftMaskImageDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTransparencyGroupDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTransparencyGroupDictionary** out) {
    if (!isTransparencyGroupDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTransparencyGroupDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfViewerPreferencesDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfViewerPreferencesDictionary** out) {
    if (!isViewerPreferencesDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfViewerPreferencesDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfOutlineDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfOutlineDictionary** out) {
    if (!isOutlineDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfOutlineDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfOutlineItemDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfOutlineItemDictionary** out) {
    if (!isOutlineItemDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfOutlineItemDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPageLabelDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageLabelDictionary** out) {
    if (!isPageLabelDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPageLabelDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfThreadDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfThreadDictionary** out) {
    if (!isThreadDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfThreadDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfBeadDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBeadDictionary** out) {
    if (!isBeadDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfBeadDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTransitionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTransitionDictionary** out) {
    if (!isTransitionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTransitionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAnnotationDictionary** out) {
    if (!isAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfBorderStyleDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBorderStyleDictionary** out) {
    if (!isBorderStyleDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfBorderStyleDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfAppearanceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAppearanceDictionary** out) {
    if (!isAppearanceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfAppearanceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTextAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTextAnnotationDictionary** out) {
    if (!isTextAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTextAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfALinkAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfALinkAnnotationDictionary** out) {
    if (!isALinkAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfALinkAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFreeTextAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFreeTextAnnotationDictionary** out) {
    if (!isFreeTextAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFreeTextAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfLineAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLineAnnotationDictionary** out) {
    if (!isLineAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfLineAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSquareOrCircleAnnotation** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSquareOrCircleAnnotation** out) {
    if (!isSquareOrCircleAnnotation(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSquareOrCircleAnnotation(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMarkupAnnotationsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMarkupAnnotationsDictionary** out) {
    if (!isMarkupAnnotationsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMarkupAnnotationsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfRubberStampAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfRubberStampAnnotationDictionary** out) {
    if (!isRubberStampAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfRubberStampAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfInkAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInkAnnotationDictionary** out) {
    if (!isInkAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfInkAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPopUpAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPopUpAnnotationDictionary** out) {
    if (!isPopUpAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPopUpAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFileAttachmentAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFileAttachmentAnnotationDictionary** out) {
    if (!isFileAttachmentAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFileAttachmentAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSoundAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoundAnnotationDictionary** out) {
    if (!isSoundAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSoundAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMovieAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieAnnotationDictionary** out) {
    if (!isMovieAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMovieAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWidgetAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWidgetAnnotationDictionary** out) {
    if (!isWidgetAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWidgetAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfActionDictionary** out) {
    if (!isActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfAnnotationActionsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAnnotationActionsDictionary** out) {
    if (!isAnnotationActionsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfAnnotationActionsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPageObjectActionsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageObjectActionsDictionary** out) {
    if (!isPageObjectActionsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPageObjectActionsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFormFieldActionsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFormFieldActionsDictionary** out) {
    if (!isFormFieldActionsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFormFieldActionsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfDocumentCatalogActionsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDocumentCatalogActionsDictionary** out) {
    if (!isDocumentCatalogActionsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfDocumentCatalogActionsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfGoToActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfGoToActionDictionary** out) {
    if (!isGoToActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfGoToActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfRemoteGoToActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfRemoteGoToActionDictionary** out) {
    if (!isRemoteGoToActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfRemoteGoToActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfLaunchActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLaunchActionDictionary** out) {
    if (!isLaunchActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfLaunchActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWindowsLaunchActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWindowsLaunchActionDictionary** out) {
    if (!isWindowsLaunchActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWindowsLaunchActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfThreadActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfThreadActionDictionary** out) {
    if (!isThreadActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfThreadActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfURIActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfURIActionDictionary** out) {
    if (!isURIActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfURIActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfURIDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfURIDictionary** out) {
    if (!isURIDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfURIDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSoundActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoundActionDictionary** out) {
    if (!isSoundActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSoundActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMovieActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieActionDictionary** out) {
    if (!isMovieActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMovieActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfHideActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfHideActionDictionary** out) {
    if (!isHideActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfHideActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfNamedActionsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNamedActionsDictionary** out) {
    if (!isNamedActionsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfNamedActionsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfInteractiveFormDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInteractiveFormDictionary** out) {
    if (!isInteractiveFormDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfInteractiveFormDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFieldDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFieldDictionary** out) {
    if (!isFieldDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFieldDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfVariableTextFieldDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfVariableTextFieldDictionary** out) {
    if (!isVariableTextFieldDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfVariableTextFieldDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfAppearanceCharacteristicsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAppearanceCharacteristicsDictionary** out) {
    if (!isAppearanceCharacteristicsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfAppearanceCharacteristicsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfCheckboxFieldDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCheckboxFieldDictionary** out) {
    if (!isCheckboxFieldDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfCheckboxFieldDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfRadioButtonFieldDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfRadioButtonFieldDictionary** out) {
    if (!isRadioButtonFieldDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfRadioButtonFieldDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTextFieldDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTextFieldDictionary** out) {
    if (!isTextFieldDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTextFieldDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfChoiceFieldDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfChoiceFieldDictionary** out) {
    if (!isChoiceFieldDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfChoiceFieldDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSignatureDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSignatureDictionary** out) {
    if (!isSignatureDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSignatureDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSubmitFormActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSubmitFormActionDictionary** out) {
    if (!isSubmitFormActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSubmitFormActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfResetFormActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfResetFormActionDictionary** out) {
    if (!isResetFormActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfResetFormActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfImportDataActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfImportDataActionDictionary** out) {
    if (!isImportDataActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfImportDataActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfJavascriptActionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfJavascriptActionDictionary** out) {
    if (!isJavascriptActionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfJavascriptActionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFTrailerDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFTrailerDictionary** out) {
    if (!isFDFTrailerDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFTrailerDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFCatalogDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFCatalogDictionary** out) {
    if (!isFDFCatalogDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFCatalogDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFDictionary** out) {
    if (!isFDFDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfEncryptedEmbeddedFileStreamDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEncryptedEmbeddedFileStreamDictionary** out) {
    if (!isEncryptedEmbeddedFileStreamDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfEncryptedEmbeddedFileStreamDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfJavascriptDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfJavascriptDictionary** out) {
    if (!isJavascriptDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfJavascriptDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFFieldDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFFieldDictionary** out) {
    if (!isFDFFieldDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFFieldDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfIconFitDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfIconFitDictionary** out) {
    if (!isIconFitDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfIconFitDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFPageDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFPageDictionary** out) {
    if (!isFDFPageDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFPageDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFTemplateDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFTemplateDictionary** out) {
    if (!isFDFTemplateDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFTemplateDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFNamedPageReferenceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFNamedPageReferenceDictionary** out) {
    if (!isFDFNamedPageReferenceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFNamedPageReferenceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfFDFFileAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFFileAnnotationDictionary** out) {
    if (!isFDFFileAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfFDFFileAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSoundObjectDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoundObjectDictionary** out) {
    if (!isSoundObjectDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSoundObjectDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMovieDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieDictionary** out) {
    if (!isMovieDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMovieDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMovieActivationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieActivationDictionary** out) {
    if (!isMovieActivationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMovieActivationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfDocumentInformationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDocumentInformationDictionary** out) {
    if (!isDocumentInformationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfDocumentInformationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMetadataStreamDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMetadataStreamDictionary** out) {
    if (!isMetadataStreamDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMetadataStreamDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfComponentsWithMetadataDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfComponentsWithMetadataDictionary** out) {
    if (!isComponentsWithMetadataDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfComponentsWithMetadataDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPagePieceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPagePieceDictionary** out) {
    if (!isPagePieceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPagePieceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfApplicationDataDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfApplicationDataDictionary** out) {
    if (!isApplicationDataDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfApplicationDataDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfStructureTreeRootDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStructureTreeRootDictionary** out) {
    if (!isStructureTreeRootDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfStructureTreeRootDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfStructureElementDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStructureElementDictionary** out) {
    if (!isStructureElementDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfStructureElementDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMarkedContentReferenceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMarkedContentReferenceDictionary** out) {
    if (!isMarkedContentReferenceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMarkedContentReferenceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfObjectReferenceDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObjectReferenceDictionary** out) {
    if (!isObjectReferenceDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfObjectReferenceDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfStructureElementAccessDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStructureElementAccessDictionary** out) {
    if (!isStructureElementAccessDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfStructureElementAccessDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfAttributeObjectDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAttributeObjectDictionary** out) {
    if (!isAttributeObjectDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfAttributeObjectDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMarkInformationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMarkInformationDictionary** out) {
    if (!isMarkInformationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMarkInformationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfArtifactsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfArtifactsDictionary** out) {
    if (!isArtifactsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfArtifactsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfStandardStructureDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStandardStructureDictionary** out) {
    if (!isStandardStructureDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfStandardStructureDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfBlockLevelStructureElementsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBlockLevelStructureElementsDictionary** out) {
    if (!isBlockLevelStructureElementsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfBlockLevelStructureElementsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfInlineLevelStructureElementsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInlineLevelStructureElementsDictionary** out) {
    if (!isInlineLevelStructureElementsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfInlineLevelStructureElementsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfListAttributeDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfListAttributeDictionary** out) {
    if (!isListAttributeDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfListAttributeDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTableAttributesDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTableAttributesDictionary** out) {
    if (!isTableAttributesDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTableAttributesDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWebCaptureInformationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureInformationDictionary** out) {
    if (!isWebCaptureInformationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWebCaptureInformationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWebCaptureDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureDictionary** out) {
    if (!isWebCaptureDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWebCaptureDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWebCapturePageSetDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCapturePageSetDictionary** out) {
    if (!isWebCapturePageSetDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWebCapturePageSetDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWebCaptureImageSetDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureImageSetDictionary** out) {
    if (!isWebCaptureImageSetDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWebCaptureImageSetDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSourceInformationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSourceInformationDictionary** out) {
    if (!isSourceInformationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSourceInformationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfURLAliasDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfURLAliasDictionary** out) {
    if (!isURLAliasDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfURLAliasDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWebCaptureCommandDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureCommandDictionary** out) {
    if (!isWebCaptureCommandDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWebCaptureCommandDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfWebCaptureCommandSettingsDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureCommandSettingsDictionary** out) {
    if (!isWebCaptureCommandSettingsDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfWebCaptureCommandSettingsDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfBoxColorInformationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBoxColorInformationDictionary** out) {
    if (!isBoxColorInformationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfBoxColorInformationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfBoxStyleDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBoxStyleDictionary** out) {
    if (!isBoxStyleDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfBoxStyleDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPrinterMarkAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPrinterMarkAnnotationDictionary** out) {
    if (!isPrinterMarkAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPrinterMarkAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPrinterMarkFormDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPrinterMarkFormDictionary** out) {
    if (!isPrinterMarkFormDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPrinterMarkFormDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfSeparationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSeparationDictionary** out) {
    if (!isSeparationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfSeparationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfPDF_XOutputIntentDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPDF_XOutputIntentDictionary** out) {
    if (!isPDF_XOutputIntentDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfPDF_XOutputIntentDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTrapNetworkAnnotationDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTrapNetworkAnnotationDictionary** out) {
    if (!isTrapNetworkAnnotationDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTrapNetworkAnnotationDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfTrapNetworkAppearanceStreamDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTrapNetworkAppearanceStreamDictionary** out) {
    if (!isTrapNetworkAppearanceStreamDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfTrapNetworkAppearanceStreamDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfOpiVersionDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfOpiVersionDictionary** out) {
    if (!isOpiVersionDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfOpiVersionDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool map(const SkPdfObject& in, SkPdfMultiMasterFontDictionary** out) {
    return map(*in.doc(), *in.podofo(), out);
  }

  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMultiMasterFontDictionary** out) {
    if (!isMultiMasterFontDictionary(podofoDoc, podofoObj)) return false;


    *out = new SkPdfMultiMasterFontDictionary(&podofoDoc, &podofoObj);
    return true;
  }

  static bool isObject(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isNull(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Null;
  }

  static bool isBoolean(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Bool;
  }

  static bool isInteger(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Number || podofoObj.GetDataType() == ePdfDataType_Real;
  }

  static bool isNumber(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Number || podofoObj.GetDataType() == ePdfDataType_Real;
  }

  static bool isName(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Name;
  }

  static bool isReference(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Reference;
  }

  static bool isArray(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Array;
  }

  static bool isString(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_String || podofoObj.GetDataType() == ePdfDataType_HexString;
  }

  static bool isHexString(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_HexString;
  }

  static bool isDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return podofoObj.GetDataType() == ePdfDataType_Dictionary;
  }

  static bool isXObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTrueTypeFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isStreamCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isLzwdecodeAndFlatedecodeFiltersDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCcittfaxdecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isJbig2DecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isDctdecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFileTrailerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isEncryptionCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isStandardSecurityHandlerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCatalogDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPageTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPageObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isNameDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isResourceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isNameTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isNumberTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFunctionCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType0FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType2FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType3FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFileSpecificationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isEmbeddedFileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isEmbeddedFileParameterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMacOsFileInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isGraphicsStateDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCalgrayColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCalrgbColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isLabColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isIccProfileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isDeviceNColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType1PatternDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType2PatternDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType1ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType2ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType3ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType4ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType5ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType6ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    std::string Subtype;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
    if (Subtype != "Image") return false;

    return true;
  }

  static bool isAlternateImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType1FormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    std::string Subtype;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
    if (Subtype != "Form") return false;

    return true;
  }

  static bool isGroupAttributesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPSXobjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType1FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType3FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isEncodingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCIDSystemInfoDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCIDFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCMapDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType0FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFontDescriptorDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCIDFontDescriptorDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isEmbeddedFontStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType1HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType6HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType10HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType16HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isType5HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSoftMaskDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSoftMaskImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTransparencyGroupDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isViewerPreferencesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isOutlineDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isOutlineItemDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPageLabelDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isThreadDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isBeadDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTransitionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isBorderStyleDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isAppearanceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTextAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isALinkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFreeTextAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isLineAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSquareOrCircleAnnotation(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMarkupAnnotationsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isRubberStampAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isInkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPopUpAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFileAttachmentAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSoundAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMovieAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWidgetAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isAnnotationActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPageObjectActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFormFieldActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isDocumentCatalogActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isGoToActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isRemoteGoToActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isLaunchActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWindowsLaunchActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isThreadActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isURIActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isURIDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSoundActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMovieActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isHideActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isNamedActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isInteractiveFormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isVariableTextFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isAppearanceCharacteristicsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isCheckboxFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isRadioButtonFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTextFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isChoiceFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSignatureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSubmitFormActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isResetFormActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isImportDataActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isJavascriptActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFTrailerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFCatalogDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isEncryptedEmbeddedFileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isJavascriptDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isIconFitDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFPageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFTemplateDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFNamedPageReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isFDFFileAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSoundObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMovieDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMovieActivationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isDocumentInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMetadataStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isComponentsWithMetadataDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPagePieceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isApplicationDataDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isStructureTreeRootDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isStructureElementDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMarkedContentReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isObjectReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isStructureElementAccessDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isAttributeObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMarkInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isArtifactsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isStandardStructureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isBlockLevelStructureElementsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isInlineLevelStructureElementsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isListAttributeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTableAttributesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWebCaptureInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWebCaptureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWebCapturePageSetDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWebCaptureImageSetDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSourceInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isURLAliasDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWebCaptureCommandDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isWebCaptureCommandSettingsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isBoxColorInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isBoxStyleDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPrinterMarkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPrinterMarkFormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isSeparationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isPDF_XOutputIntentDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTrapNetworkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isTrapNetworkAppearanceStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isOpiVersionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    return true;
  }

  static bool isMultiMasterFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
    std::string Subtype;
    if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
    if (Subtype != "MMType1") return false;

    return true;
  }

};

#endif  // __DEFINED__SkPdfPodofoMapper

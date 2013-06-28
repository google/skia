#include "SkPdfPodofoMapper_autogen.h"
bool mapObject(const SkPdfObject& in, SkPdfObject** out) {
  return mapObject(*in.doc(), *in.podofo(), out);
}

bool mapObject(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
  if (!isObject(podofoDoc, podofoObj)) return false;

  if (mapArray(podofoDoc, podofoObj, (SkPdfArray**)out)) return true;
  if (mapBoolean(podofoDoc, podofoObj, (SkPdfBoolean**)out)) return true;
  if (mapDictionary(podofoDoc, podofoObj, (SkPdfDictionary**)out)) return true;
  if (mapInteger(podofoDoc, podofoObj, (SkPdfInteger**)out)) return true;
  if (mapName(podofoDoc, podofoObj, (SkPdfName**)out)) return true;
  if (mapNull(podofoDoc, podofoObj, (SkPdfNull**)out)) return true;
  if (mapReference(podofoDoc, podofoObj, (SkPdfReference**)out)) return true;
  if (mapString(podofoDoc, podofoObj, (SkPdfString**)out)) return true;
  if (mapStream(podofoDoc, podofoObj, (SkPdfStream**)out)) return true;

  *out = new SkPdfObject(&podofoDoc, &podofoObj);
  return true;
}

bool mapNull(const SkPdfObject& in, SkPdfNull** out) {
  return mapNull(*in.doc(), *in.podofo(), out);
}

bool mapNull(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNull** out) {
  if (!isNull(podofoDoc, podofoObj)) return false;


  *out = new SkPdfNull(&podofoDoc, &podofoObj);
  return true;
}

bool mapBoolean(const SkPdfObject& in, SkPdfBoolean** out) {
  return mapBoolean(*in.doc(), *in.podofo(), out);
}

bool mapBoolean(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBoolean** out) {
  if (!isBoolean(podofoDoc, podofoObj)) return false;


  *out = new SkPdfBoolean(&podofoDoc, &podofoObj);
  return true;
}

bool mapInteger(const SkPdfObject& in, SkPdfInteger** out) {
  return mapInteger(*in.doc(), *in.podofo(), out);
}

bool mapInteger(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInteger** out) {
  if (!isInteger(podofoDoc, podofoObj)) return false;

  if (mapNumber(podofoDoc, podofoObj, (SkPdfNumber**)out)) return true;

  *out = new SkPdfInteger(&podofoDoc, &podofoObj);
  return true;
}

bool mapNumber(const SkPdfObject& in, SkPdfNumber** out) {
  return mapNumber(*in.doc(), *in.podofo(), out);
}

bool mapNumber(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNumber** out) {
  if (!isNumber(podofoDoc, podofoObj)) return false;


  *out = new SkPdfNumber(&podofoDoc, &podofoObj);
  return true;
}

bool mapName(const SkPdfObject& in, SkPdfName** out) {
  return mapName(*in.doc(), *in.podofo(), out);
}

bool mapName(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfName** out) {
  if (!isName(podofoDoc, podofoObj)) return false;


  *out = new SkPdfName(&podofoDoc, &podofoObj);
  return true;
}

bool mapReference(const SkPdfObject& in, SkPdfReference** out) {
  return mapReference(*in.doc(), *in.podofo(), out);
}

bool mapReference(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfReference** out) {
  if (!isReference(podofoDoc, podofoObj)) return false;


  *out = new SkPdfReference(&podofoDoc, &podofoObj);
  return true;
}

bool mapArray(const SkPdfObject& in, SkPdfArray** out) {
  return mapArray(*in.doc(), *in.podofo(), out);
}

bool mapArray(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfArray** out) {
  if (!isArray(podofoDoc, podofoObj)) return false;


  *out = new SkPdfArray(&podofoDoc, &podofoObj);
  return true;
}

bool mapString(const SkPdfObject& in, SkPdfString** out) {
  return mapString(*in.doc(), *in.podofo(), out);
}

bool mapString(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfString** out) {
  if (!isString(podofoDoc, podofoObj)) return false;

  if (mapHexString(podofoDoc, podofoObj, (SkPdfHexString**)out)) return true;

  *out = new SkPdfString(&podofoDoc, &podofoObj);
  return true;
}

bool mapHexString(const SkPdfObject& in, SkPdfHexString** out) {
  return mapHexString(*in.doc(), *in.podofo(), out);
}

bool mapHexString(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfHexString** out) {
  if (!isHexString(podofoDoc, podofoObj)) return false;


  *out = new SkPdfHexString(&podofoDoc, &podofoObj);
  return true;
}

bool mapDictionary(const SkPdfObject& in, SkPdfDictionary** out) {
  return mapDictionary(*in.doc(), *in.podofo(), out);
}

bool mapDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDictionary** out) {
  if (!isDictionary(podofoDoc, podofoObj)) return false;

  if (mapALinkAnnotationDictionary(podofoDoc, podofoObj, (SkPdfALinkAnnotationDictionary**)out)) return true;
  if (mapActionDictionary(podofoDoc, podofoObj, (SkPdfActionDictionary**)out)) return true;
  if (mapAlternateImageDictionary(podofoDoc, podofoObj, (SkPdfAlternateImageDictionary**)out)) return true;
  if (mapAnnotationActionsDictionary(podofoDoc, podofoObj, (SkPdfAnnotationActionsDictionary**)out)) return true;
  if (mapAnnotationDictionary(podofoDoc, podofoObj, (SkPdfAnnotationDictionary**)out)) return true;
  if (mapAppearanceCharacteristicsDictionary(podofoDoc, podofoObj, (SkPdfAppearanceCharacteristicsDictionary**)out)) return true;
  if (mapAppearanceDictionary(podofoDoc, podofoObj, (SkPdfAppearanceDictionary**)out)) return true;
  if (mapApplicationDataDictionary(podofoDoc, podofoObj, (SkPdfApplicationDataDictionary**)out)) return true;
  if (mapArtifactsDictionary(podofoDoc, podofoObj, (SkPdfArtifactsDictionary**)out)) return true;
  if (mapAttributeObjectDictionary(podofoDoc, podofoObj, (SkPdfAttributeObjectDictionary**)out)) return true;
  if (mapBeadDictionary(podofoDoc, podofoObj, (SkPdfBeadDictionary**)out)) return true;
  if (mapBlockLevelStructureElementsDictionary(podofoDoc, podofoObj, (SkPdfBlockLevelStructureElementsDictionary**)out)) return true;
  if (mapBorderStyleDictionary(podofoDoc, podofoObj, (SkPdfBorderStyleDictionary**)out)) return true;
  if (mapBoxColorInformationDictionary(podofoDoc, podofoObj, (SkPdfBoxColorInformationDictionary**)out)) return true;
  if (mapBoxStyleDictionary(podofoDoc, podofoObj, (SkPdfBoxStyleDictionary**)out)) return true;
  if (mapCIDFontDescriptorDictionary(podofoDoc, podofoObj, (SkPdfCIDFontDescriptorDictionary**)out)) return true;
  if (mapCIDFontDictionary(podofoDoc, podofoObj, (SkPdfCIDFontDictionary**)out)) return true;
  if (mapCIDSystemInfoDictionary(podofoDoc, podofoObj, (SkPdfCIDSystemInfoDictionary**)out)) return true;
  if (mapCMapDictionary(podofoDoc, podofoObj, (SkPdfCMapDictionary**)out)) return true;
  if (mapCalgrayColorSpaceDictionary(podofoDoc, podofoObj, (SkPdfCalgrayColorSpaceDictionary**)out)) return true;
  if (mapCalrgbColorSpaceDictionary(podofoDoc, podofoObj, (SkPdfCalrgbColorSpaceDictionary**)out)) return true;
  if (mapCatalogDictionary(podofoDoc, podofoObj, (SkPdfCatalogDictionary**)out)) return true;
  if (mapCcittfaxdecodeFilterDictionary(podofoDoc, podofoObj, (SkPdfCcittfaxdecodeFilterDictionary**)out)) return true;
  if (mapCheckboxFieldDictionary(podofoDoc, podofoObj, (SkPdfCheckboxFieldDictionary**)out)) return true;
  if (mapChoiceFieldDictionary(podofoDoc, podofoObj, (SkPdfChoiceFieldDictionary**)out)) return true;
  if (mapComponentsWithMetadataDictionary(podofoDoc, podofoObj, (SkPdfComponentsWithMetadataDictionary**)out)) return true;
  if (mapDctdecodeFilterDictionary(podofoDoc, podofoObj, (SkPdfDctdecodeFilterDictionary**)out)) return true;
  if (mapDeviceNColorSpaceDictionary(podofoDoc, podofoObj, (SkPdfDeviceNColorSpaceDictionary**)out)) return true;
  if (mapDocumentCatalogActionsDictionary(podofoDoc, podofoObj, (SkPdfDocumentCatalogActionsDictionary**)out)) return true;
  if (mapDocumentInformationDictionary(podofoDoc, podofoObj, (SkPdfDocumentInformationDictionary**)out)) return true;
  if (mapEmbeddedFileParameterDictionary(podofoDoc, podofoObj, (SkPdfEmbeddedFileParameterDictionary**)out)) return true;
  if (mapEmbeddedFileStreamDictionary(podofoDoc, podofoObj, (SkPdfEmbeddedFileStreamDictionary**)out)) return true;
  if (mapEmbeddedFontStreamDictionary(podofoDoc, podofoObj, (SkPdfEmbeddedFontStreamDictionary**)out)) return true;
  if (mapEncodingDictionary(podofoDoc, podofoObj, (SkPdfEncodingDictionary**)out)) return true;
  if (mapEncryptedEmbeddedFileStreamDictionary(podofoDoc, podofoObj, (SkPdfEncryptedEmbeddedFileStreamDictionary**)out)) return true;
  if (mapEncryptionCommonDictionary(podofoDoc, podofoObj, (SkPdfEncryptionCommonDictionary**)out)) return true;
  if (mapFDFCatalogDictionary(podofoDoc, podofoObj, (SkPdfFDFCatalogDictionary**)out)) return true;
  if (mapFDFDictionary(podofoDoc, podofoObj, (SkPdfFDFDictionary**)out)) return true;
  if (mapFDFFieldDictionary(podofoDoc, podofoObj, (SkPdfFDFFieldDictionary**)out)) return true;
  if (mapFDFFileAnnotationDictionary(podofoDoc, podofoObj, (SkPdfFDFFileAnnotationDictionary**)out)) return true;
  if (mapFDFNamedPageReferenceDictionary(podofoDoc, podofoObj, (SkPdfFDFNamedPageReferenceDictionary**)out)) return true;
  if (mapFDFPageDictionary(podofoDoc, podofoObj, (SkPdfFDFPageDictionary**)out)) return true;
  if (mapFDFTemplateDictionary(podofoDoc, podofoObj, (SkPdfFDFTemplateDictionary**)out)) return true;
  if (mapFDFTrailerDictionary(podofoDoc, podofoObj, (SkPdfFDFTrailerDictionary**)out)) return true;
  if (mapFieldDictionary(podofoDoc, podofoObj, (SkPdfFieldDictionary**)out)) return true;
  if (mapFileAttachmentAnnotationDictionary(podofoDoc, podofoObj, (SkPdfFileAttachmentAnnotationDictionary**)out)) return true;
  if (mapFileSpecificationDictionary(podofoDoc, podofoObj, (SkPdfFileSpecificationDictionary**)out)) return true;
  if (mapFileTrailerDictionary(podofoDoc, podofoObj, (SkPdfFileTrailerDictionary**)out)) return true;
  if (mapFontDescriptorDictionary(podofoDoc, podofoObj, (SkPdfFontDescriptorDictionary**)out)) return true;
  if (mapFontDictionary(podofoDoc, podofoObj, (SkPdfFontDictionary**)out)) return true;
  if (mapFormFieldActionsDictionary(podofoDoc, podofoObj, (SkPdfFormFieldActionsDictionary**)out)) return true;
  if (mapFreeTextAnnotationDictionary(podofoDoc, podofoObj, (SkPdfFreeTextAnnotationDictionary**)out)) return true;
  if (mapFunctionCommonDictionary(podofoDoc, podofoObj, (SkPdfFunctionCommonDictionary**)out)) return true;
  if (mapGoToActionDictionary(podofoDoc, podofoObj, (SkPdfGoToActionDictionary**)out)) return true;
  if (mapGraphicsStateDictionary(podofoDoc, podofoObj, (SkPdfGraphicsStateDictionary**)out)) return true;
  if (mapGroupAttributesDictionary(podofoDoc, podofoObj, (SkPdfGroupAttributesDictionary**)out)) return true;
  if (mapHideActionDictionary(podofoDoc, podofoObj, (SkPdfHideActionDictionary**)out)) return true;
  if (mapIccProfileStreamDictionary(podofoDoc, podofoObj, (SkPdfIccProfileStreamDictionary**)out)) return true;
  if (mapIconFitDictionary(podofoDoc, podofoObj, (SkPdfIconFitDictionary**)out)) return true;
  if (mapImportDataActionDictionary(podofoDoc, podofoObj, (SkPdfImportDataActionDictionary**)out)) return true;
  if (mapInkAnnotationDictionary(podofoDoc, podofoObj, (SkPdfInkAnnotationDictionary**)out)) return true;
  if (mapInlineLevelStructureElementsDictionary(podofoDoc, podofoObj, (SkPdfInlineLevelStructureElementsDictionary**)out)) return true;
  if (mapInteractiveFormDictionary(podofoDoc, podofoObj, (SkPdfInteractiveFormDictionary**)out)) return true;
  if (mapJavascriptActionDictionary(podofoDoc, podofoObj, (SkPdfJavascriptActionDictionary**)out)) return true;
  if (mapJavascriptDictionary(podofoDoc, podofoObj, (SkPdfJavascriptDictionary**)out)) return true;
  if (mapJbig2DecodeFilterDictionary(podofoDoc, podofoObj, (SkPdfJbig2DecodeFilterDictionary**)out)) return true;
  if (mapLabColorSpaceDictionary(podofoDoc, podofoObj, (SkPdfLabColorSpaceDictionary**)out)) return true;
  if (mapLaunchActionDictionary(podofoDoc, podofoObj, (SkPdfLaunchActionDictionary**)out)) return true;
  if (mapLineAnnotationDictionary(podofoDoc, podofoObj, (SkPdfLineAnnotationDictionary**)out)) return true;
  if (mapListAttributeDictionary(podofoDoc, podofoObj, (SkPdfListAttributeDictionary**)out)) return true;
  if (mapLzwdecodeAndFlatedecodeFiltersDictionary(podofoDoc, podofoObj, (SkPdfLzwdecodeAndFlatedecodeFiltersDictionary**)out)) return true;
  if (mapMacOsFileInformationDictionary(podofoDoc, podofoObj, (SkPdfMacOsFileInformationDictionary**)out)) return true;
  if (mapMarkInformationDictionary(podofoDoc, podofoObj, (SkPdfMarkInformationDictionary**)out)) return true;
  if (mapMarkedContentReferenceDictionary(podofoDoc, podofoObj, (SkPdfMarkedContentReferenceDictionary**)out)) return true;
  if (mapMarkupAnnotationsDictionary(podofoDoc, podofoObj, (SkPdfMarkupAnnotationsDictionary**)out)) return true;
  if (mapMetadataStreamDictionary(podofoDoc, podofoObj, (SkPdfMetadataStreamDictionary**)out)) return true;
  if (mapMovieActionDictionary(podofoDoc, podofoObj, (SkPdfMovieActionDictionary**)out)) return true;
  if (mapMovieActivationDictionary(podofoDoc, podofoObj, (SkPdfMovieActivationDictionary**)out)) return true;
  if (mapMovieAnnotationDictionary(podofoDoc, podofoObj, (SkPdfMovieAnnotationDictionary**)out)) return true;
  if (mapMovieDictionary(podofoDoc, podofoObj, (SkPdfMovieDictionary**)out)) return true;
  if (mapNameDictionary(podofoDoc, podofoObj, (SkPdfNameDictionary**)out)) return true;
  if (mapNameTreeNodeDictionary(podofoDoc, podofoObj, (SkPdfNameTreeNodeDictionary**)out)) return true;
  if (mapNamedActionsDictionary(podofoDoc, podofoObj, (SkPdfNamedActionsDictionary**)out)) return true;
  if (mapNumberTreeNodeDictionary(podofoDoc, podofoObj, (SkPdfNumberTreeNodeDictionary**)out)) return true;
  if (mapObjectReferenceDictionary(podofoDoc, podofoObj, (SkPdfObjectReferenceDictionary**)out)) return true;
  if (mapOpiVersionDictionary(podofoDoc, podofoObj, (SkPdfOpiVersionDictionary**)out)) return true;
  if (mapOutlineDictionary(podofoDoc, podofoObj, (SkPdfOutlineDictionary**)out)) return true;
  if (mapOutlineItemDictionary(podofoDoc, podofoObj, (SkPdfOutlineItemDictionary**)out)) return true;
  if (mapPDF_XOutputIntentDictionary(podofoDoc, podofoObj, (SkPdfPDF_XOutputIntentDictionary**)out)) return true;
  if (mapPSXobjectDictionary(podofoDoc, podofoObj, (SkPdfPSXobjectDictionary**)out)) return true;
  if (mapPageLabelDictionary(podofoDoc, podofoObj, (SkPdfPageLabelDictionary**)out)) return true;
  if (mapPageObjectActionsDictionary(podofoDoc, podofoObj, (SkPdfPageObjectActionsDictionary**)out)) return true;
  if (mapPageObjectDictionary(podofoDoc, podofoObj, (SkPdfPageObjectDictionary**)out)) return true;
  if (mapPagePieceDictionary(podofoDoc, podofoObj, (SkPdfPagePieceDictionary**)out)) return true;
  if (mapPageTreeNodeDictionary(podofoDoc, podofoObj, (SkPdfPageTreeNodeDictionary**)out)) return true;
  if (mapPopUpAnnotationDictionary(podofoDoc, podofoObj, (SkPdfPopUpAnnotationDictionary**)out)) return true;
  if (mapPrinterMarkAnnotationDictionary(podofoDoc, podofoObj, (SkPdfPrinterMarkAnnotationDictionary**)out)) return true;
  if (mapPrinterMarkFormDictionary(podofoDoc, podofoObj, (SkPdfPrinterMarkFormDictionary**)out)) return true;
  if (mapRadioButtonFieldDictionary(podofoDoc, podofoObj, (SkPdfRadioButtonFieldDictionary**)out)) return true;
  if (mapReferenceDictionary(podofoDoc, podofoObj, (SkPdfReferenceDictionary**)out)) return true;
  if (mapRemoteGoToActionDictionary(podofoDoc, podofoObj, (SkPdfRemoteGoToActionDictionary**)out)) return true;
  if (mapResetFormActionDictionary(podofoDoc, podofoObj, (SkPdfResetFormActionDictionary**)out)) return true;
  if (mapResourceDictionary(podofoDoc, podofoObj, (SkPdfResourceDictionary**)out)) return true;
  if (mapRubberStampAnnotationDictionary(podofoDoc, podofoObj, (SkPdfRubberStampAnnotationDictionary**)out)) return true;
  if (mapSeparationDictionary(podofoDoc, podofoObj, (SkPdfSeparationDictionary**)out)) return true;
  if (mapShadingDictionary(podofoDoc, podofoObj, (SkPdfShadingDictionary**)out)) return true;
  if (mapSignatureDictionary(podofoDoc, podofoObj, (SkPdfSignatureDictionary**)out)) return true;
  if (mapSoftMaskDictionary(podofoDoc, podofoObj, (SkPdfSoftMaskDictionary**)out)) return true;
  if (mapSoftMaskImageDictionary(podofoDoc, podofoObj, (SkPdfSoftMaskImageDictionary**)out)) return true;
  if (mapSoundActionDictionary(podofoDoc, podofoObj, (SkPdfSoundActionDictionary**)out)) return true;
  if (mapSoundAnnotationDictionary(podofoDoc, podofoObj, (SkPdfSoundAnnotationDictionary**)out)) return true;
  if (mapSoundObjectDictionary(podofoDoc, podofoObj, (SkPdfSoundObjectDictionary**)out)) return true;
  if (mapSourceInformationDictionary(podofoDoc, podofoObj, (SkPdfSourceInformationDictionary**)out)) return true;
  if (mapSquareOrCircleAnnotation(podofoDoc, podofoObj, (SkPdfSquareOrCircleAnnotation**)out)) return true;
  if (mapStandardSecurityHandlerDictionary(podofoDoc, podofoObj, (SkPdfStandardSecurityHandlerDictionary**)out)) return true;
  if (mapStandardStructureDictionary(podofoDoc, podofoObj, (SkPdfStandardStructureDictionary**)out)) return true;
  if (mapStreamCommonDictionary(podofoDoc, podofoObj, (SkPdfStreamCommonDictionary**)out)) return true;
  if (mapStructureElementAccessDictionary(podofoDoc, podofoObj, (SkPdfStructureElementAccessDictionary**)out)) return true;
  if (mapStructureElementDictionary(podofoDoc, podofoObj, (SkPdfStructureElementDictionary**)out)) return true;
  if (mapStructureTreeRootDictionary(podofoDoc, podofoObj, (SkPdfStructureTreeRootDictionary**)out)) return true;
  if (mapSubmitFormActionDictionary(podofoDoc, podofoObj, (SkPdfSubmitFormActionDictionary**)out)) return true;
  if (mapTableAttributesDictionary(podofoDoc, podofoObj, (SkPdfTableAttributesDictionary**)out)) return true;
  if (mapTextAnnotationDictionary(podofoDoc, podofoObj, (SkPdfTextAnnotationDictionary**)out)) return true;
  if (mapTextFieldDictionary(podofoDoc, podofoObj, (SkPdfTextFieldDictionary**)out)) return true;
  if (mapThreadActionDictionary(podofoDoc, podofoObj, (SkPdfThreadActionDictionary**)out)) return true;
  if (mapThreadDictionary(podofoDoc, podofoObj, (SkPdfThreadDictionary**)out)) return true;
  if (mapTransitionDictionary(podofoDoc, podofoObj, (SkPdfTransitionDictionary**)out)) return true;
  if (mapTransparencyGroupDictionary(podofoDoc, podofoObj, (SkPdfTransparencyGroupDictionary**)out)) return true;
  if (mapTrapNetworkAnnotationDictionary(podofoDoc, podofoObj, (SkPdfTrapNetworkAnnotationDictionary**)out)) return true;
  if (mapTrapNetworkAppearanceStreamDictionary(podofoDoc, podofoObj, (SkPdfTrapNetworkAppearanceStreamDictionary**)out)) return true;
  if (mapType0FunctionDictionary(podofoDoc, podofoObj, (SkPdfType0FunctionDictionary**)out)) return true;
  if (mapType10HalftoneDictionary(podofoDoc, podofoObj, (SkPdfType10HalftoneDictionary**)out)) return true;
  if (mapType16HalftoneDictionary(podofoDoc, podofoObj, (SkPdfType16HalftoneDictionary**)out)) return true;
  if (mapType1HalftoneDictionary(podofoDoc, podofoObj, (SkPdfType1HalftoneDictionary**)out)) return true;
  if (mapType1PatternDictionary(podofoDoc, podofoObj, (SkPdfType1PatternDictionary**)out)) return true;
  if (mapType2FunctionDictionary(podofoDoc, podofoObj, (SkPdfType2FunctionDictionary**)out)) return true;
  if (mapType2PatternDictionary(podofoDoc, podofoObj, (SkPdfType2PatternDictionary**)out)) return true;
  if (mapType3FunctionDictionary(podofoDoc, podofoObj, (SkPdfType3FunctionDictionary**)out)) return true;
  if (mapType5HalftoneDictionary(podofoDoc, podofoObj, (SkPdfType5HalftoneDictionary**)out)) return true;
  if (mapType6HalftoneDictionary(podofoDoc, podofoObj, (SkPdfType6HalftoneDictionary**)out)) return true;
  if (mapURIActionDictionary(podofoDoc, podofoObj, (SkPdfURIActionDictionary**)out)) return true;
  if (mapURIDictionary(podofoDoc, podofoObj, (SkPdfURIDictionary**)out)) return true;
  if (mapURLAliasDictionary(podofoDoc, podofoObj, (SkPdfURLAliasDictionary**)out)) return true;
  if (mapVariableTextFieldDictionary(podofoDoc, podofoObj, (SkPdfVariableTextFieldDictionary**)out)) return true;
  if (mapViewerPreferencesDictionary(podofoDoc, podofoObj, (SkPdfViewerPreferencesDictionary**)out)) return true;
  if (mapWebCaptureCommandDictionary(podofoDoc, podofoObj, (SkPdfWebCaptureCommandDictionary**)out)) return true;
  if (mapWebCaptureCommandSettingsDictionary(podofoDoc, podofoObj, (SkPdfWebCaptureCommandSettingsDictionary**)out)) return true;
  if (mapWebCaptureDictionary(podofoDoc, podofoObj, (SkPdfWebCaptureDictionary**)out)) return true;
  if (mapWebCaptureImageSetDictionary(podofoDoc, podofoObj, (SkPdfWebCaptureImageSetDictionary**)out)) return true;
  if (mapWebCaptureInformationDictionary(podofoDoc, podofoObj, (SkPdfWebCaptureInformationDictionary**)out)) return true;
  if (mapWebCapturePageSetDictionary(podofoDoc, podofoObj, (SkPdfWebCapturePageSetDictionary**)out)) return true;
  if (mapWidgetAnnotationDictionary(podofoDoc, podofoObj, (SkPdfWidgetAnnotationDictionary**)out)) return true;
  if (mapWindowsLaunchActionDictionary(podofoDoc, podofoObj, (SkPdfWindowsLaunchActionDictionary**)out)) return true;
  if (mapXObjectDictionary(podofoDoc, podofoObj, (SkPdfXObjectDictionary**)out)) return true;

  *out = new SkPdfDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapStream(const SkPdfObject& in, SkPdfStream** out) {
  return mapStream(*in.doc(), *in.podofo(), out);
}

bool mapStream(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStream** out) {
  if (!isStream(podofoDoc, podofoObj)) return false;


  *out = new SkPdfStream(&podofoDoc, &podofoObj);
  return true;
}

bool mapXObjectDictionary(const SkPdfObject& in, SkPdfXObjectDictionary** out) {
  return mapXObjectDictionary(*in.doc(), *in.podofo(), out);
}

bool mapXObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfXObjectDictionary** out) {
  if (!isXObjectDictionary(podofoDoc, podofoObj)) return false;

  if (mapImageDictionary(podofoDoc, podofoObj, (SkPdfImageDictionary**)out)) return true;
  if (mapType1FormDictionary(podofoDoc, podofoObj, (SkPdfType1FormDictionary**)out)) return true;

  *out = new SkPdfXObjectDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFontDictionary(const SkPdfObject& in, SkPdfFontDictionary** out) {
  return mapFontDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFontDictionary** out) {
  if (!isFontDictionary(podofoDoc, podofoObj)) return false;

  if (mapType0FontDictionary(podofoDoc, podofoObj, (SkPdfType0FontDictionary**)out)) return true;
  if (mapType1FontDictionary(podofoDoc, podofoObj, (SkPdfType1FontDictionary**)out)) return true;

  *out = new SkPdfFontDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTrueTypeFontDictionary(const SkPdfObject& in, SkPdfTrueTypeFontDictionary** out) {
  return mapTrueTypeFontDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTrueTypeFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTrueTypeFontDictionary** out) {
  if (!isTrueTypeFontDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTrueTypeFontDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapStreamCommonDictionary(const SkPdfObject& in, SkPdfStreamCommonDictionary** out) {
  return mapStreamCommonDictionary(*in.doc(), *in.podofo(), out);
}

bool mapStreamCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStreamCommonDictionary** out) {
  if (!isStreamCommonDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfStreamCommonDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapLzwdecodeAndFlatedecodeFiltersDictionary(const SkPdfObject& in, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** out) {
  return mapLzwdecodeAndFlatedecodeFiltersDictionary(*in.doc(), *in.podofo(), out);
}

bool mapLzwdecodeAndFlatedecodeFiltersDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** out) {
  if (!isLzwdecodeAndFlatedecodeFiltersDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfLzwdecodeAndFlatedecodeFiltersDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCcittfaxdecodeFilterDictionary(const SkPdfObject& in, SkPdfCcittfaxdecodeFilterDictionary** out) {
  return mapCcittfaxdecodeFilterDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCcittfaxdecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCcittfaxdecodeFilterDictionary** out) {
  if (!isCcittfaxdecodeFilterDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCcittfaxdecodeFilterDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapJbig2DecodeFilterDictionary(const SkPdfObject& in, SkPdfJbig2DecodeFilterDictionary** out) {
  return mapJbig2DecodeFilterDictionary(*in.doc(), *in.podofo(), out);
}

bool mapJbig2DecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfJbig2DecodeFilterDictionary** out) {
  if (!isJbig2DecodeFilterDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfJbig2DecodeFilterDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapDctdecodeFilterDictionary(const SkPdfObject& in, SkPdfDctdecodeFilterDictionary** out) {
  return mapDctdecodeFilterDictionary(*in.doc(), *in.podofo(), out);
}

bool mapDctdecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDctdecodeFilterDictionary** out) {
  if (!isDctdecodeFilterDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfDctdecodeFilterDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFileTrailerDictionary(const SkPdfObject& in, SkPdfFileTrailerDictionary** out) {
  return mapFileTrailerDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFileTrailerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFileTrailerDictionary** out) {
  if (!isFileTrailerDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFileTrailerDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapEncryptionCommonDictionary(const SkPdfObject& in, SkPdfEncryptionCommonDictionary** out) {
  return mapEncryptionCommonDictionary(*in.doc(), *in.podofo(), out);
}

bool mapEncryptionCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEncryptionCommonDictionary** out) {
  if (!isEncryptionCommonDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfEncryptionCommonDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapStandardSecurityHandlerDictionary(const SkPdfObject& in, SkPdfStandardSecurityHandlerDictionary** out) {
  return mapStandardSecurityHandlerDictionary(*in.doc(), *in.podofo(), out);
}

bool mapStandardSecurityHandlerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStandardSecurityHandlerDictionary** out) {
  if (!isStandardSecurityHandlerDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfStandardSecurityHandlerDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCatalogDictionary(const SkPdfObject& in, SkPdfCatalogDictionary** out) {
  return mapCatalogDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCatalogDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCatalogDictionary** out) {
  if (!isCatalogDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCatalogDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPageTreeNodeDictionary(const SkPdfObject& in, SkPdfPageTreeNodeDictionary** out) {
  return mapPageTreeNodeDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPageTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageTreeNodeDictionary** out) {
  if (!isPageTreeNodeDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPageTreeNodeDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPageObjectDictionary(const SkPdfObject& in, SkPdfPageObjectDictionary** out) {
  return mapPageObjectDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPageObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageObjectDictionary** out) {
  if (!isPageObjectDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPageObjectDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapNameDictionary(const SkPdfObject& in, SkPdfNameDictionary** out) {
  return mapNameDictionary(*in.doc(), *in.podofo(), out);
}

bool mapNameDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNameDictionary** out) {
  if (!isNameDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfNameDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapResourceDictionary(const SkPdfObject& in, SkPdfResourceDictionary** out) {
  return mapResourceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapResourceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfResourceDictionary** out) {
  if (!isResourceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfResourceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapNameTreeNodeDictionary(const SkPdfObject& in, SkPdfNameTreeNodeDictionary** out) {
  return mapNameTreeNodeDictionary(*in.doc(), *in.podofo(), out);
}

bool mapNameTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNameTreeNodeDictionary** out) {
  if (!isNameTreeNodeDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfNameTreeNodeDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapNumberTreeNodeDictionary(const SkPdfObject& in, SkPdfNumberTreeNodeDictionary** out) {
  return mapNumberTreeNodeDictionary(*in.doc(), *in.podofo(), out);
}

bool mapNumberTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNumberTreeNodeDictionary** out) {
  if (!isNumberTreeNodeDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfNumberTreeNodeDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFunctionCommonDictionary(const SkPdfObject& in, SkPdfFunctionCommonDictionary** out) {
  return mapFunctionCommonDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFunctionCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFunctionCommonDictionary** out) {
  if (!isFunctionCommonDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFunctionCommonDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType0FunctionDictionary(const SkPdfObject& in, SkPdfType0FunctionDictionary** out) {
  return mapType0FunctionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType0FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType0FunctionDictionary** out) {
  if (!isType0FunctionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType0FunctionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType2FunctionDictionary(const SkPdfObject& in, SkPdfType2FunctionDictionary** out) {
  return mapType2FunctionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType2FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType2FunctionDictionary** out) {
  if (!isType2FunctionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType2FunctionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType3FunctionDictionary(const SkPdfObject& in, SkPdfType3FunctionDictionary** out) {
  return mapType3FunctionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType3FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType3FunctionDictionary** out) {
  if (!isType3FunctionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType3FunctionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFileSpecificationDictionary(const SkPdfObject& in, SkPdfFileSpecificationDictionary** out) {
  return mapFileSpecificationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFileSpecificationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFileSpecificationDictionary** out) {
  if (!isFileSpecificationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFileSpecificationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapEmbeddedFileStreamDictionary(const SkPdfObject& in, SkPdfEmbeddedFileStreamDictionary** out) {
  return mapEmbeddedFileStreamDictionary(*in.doc(), *in.podofo(), out);
}

bool mapEmbeddedFileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEmbeddedFileStreamDictionary** out) {
  if (!isEmbeddedFileStreamDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfEmbeddedFileStreamDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapEmbeddedFileParameterDictionary(const SkPdfObject& in, SkPdfEmbeddedFileParameterDictionary** out) {
  return mapEmbeddedFileParameterDictionary(*in.doc(), *in.podofo(), out);
}

bool mapEmbeddedFileParameterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEmbeddedFileParameterDictionary** out) {
  if (!isEmbeddedFileParameterDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfEmbeddedFileParameterDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMacOsFileInformationDictionary(const SkPdfObject& in, SkPdfMacOsFileInformationDictionary** out) {
  return mapMacOsFileInformationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMacOsFileInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMacOsFileInformationDictionary** out) {
  if (!isMacOsFileInformationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMacOsFileInformationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapGraphicsStateDictionary(const SkPdfObject& in, SkPdfGraphicsStateDictionary** out) {
  return mapGraphicsStateDictionary(*in.doc(), *in.podofo(), out);
}

bool mapGraphicsStateDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfGraphicsStateDictionary** out) {
  if (!isGraphicsStateDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfGraphicsStateDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCalgrayColorSpaceDictionary(const SkPdfObject& in, SkPdfCalgrayColorSpaceDictionary** out) {
  return mapCalgrayColorSpaceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCalgrayColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCalgrayColorSpaceDictionary** out) {
  if (!isCalgrayColorSpaceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCalgrayColorSpaceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCalrgbColorSpaceDictionary(const SkPdfObject& in, SkPdfCalrgbColorSpaceDictionary** out) {
  return mapCalrgbColorSpaceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCalrgbColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCalrgbColorSpaceDictionary** out) {
  if (!isCalrgbColorSpaceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCalrgbColorSpaceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapLabColorSpaceDictionary(const SkPdfObject& in, SkPdfLabColorSpaceDictionary** out) {
  return mapLabColorSpaceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapLabColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLabColorSpaceDictionary** out) {
  if (!isLabColorSpaceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfLabColorSpaceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapIccProfileStreamDictionary(const SkPdfObject& in, SkPdfIccProfileStreamDictionary** out) {
  return mapIccProfileStreamDictionary(*in.doc(), *in.podofo(), out);
}

bool mapIccProfileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfIccProfileStreamDictionary** out) {
  if (!isIccProfileStreamDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfIccProfileStreamDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapDeviceNColorSpaceDictionary(const SkPdfObject& in, SkPdfDeviceNColorSpaceDictionary** out) {
  return mapDeviceNColorSpaceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapDeviceNColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDeviceNColorSpaceDictionary** out) {
  if (!isDeviceNColorSpaceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfDeviceNColorSpaceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType1PatternDictionary(const SkPdfObject& in, SkPdfType1PatternDictionary** out) {
  return mapType1PatternDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType1PatternDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1PatternDictionary** out) {
  if (!isType1PatternDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType1PatternDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType2PatternDictionary(const SkPdfObject& in, SkPdfType2PatternDictionary** out) {
  return mapType2PatternDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType2PatternDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType2PatternDictionary** out) {
  if (!isType2PatternDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType2PatternDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapShadingDictionary(const SkPdfObject& in, SkPdfShadingDictionary** out) {
  return mapShadingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfShadingDictionary** out) {
  if (!isShadingDictionary(podofoDoc, podofoObj)) return false;

  if (mapType1ShadingDictionary(podofoDoc, podofoObj, (SkPdfType1ShadingDictionary**)out)) return true;
  if (mapType2ShadingDictionary(podofoDoc, podofoObj, (SkPdfType2ShadingDictionary**)out)) return true;
  if (mapType3ShadingDictionary(podofoDoc, podofoObj, (SkPdfType3ShadingDictionary**)out)) return true;
  if (mapType4ShadingDictionary(podofoDoc, podofoObj, (SkPdfType4ShadingDictionary**)out)) return true;
  if (mapType5ShadingDictionary(podofoDoc, podofoObj, (SkPdfType5ShadingDictionary**)out)) return true;
  if (mapType6ShadingDictionary(podofoDoc, podofoObj, (SkPdfType6ShadingDictionary**)out)) return true;

  *out = new SkPdfShadingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType1ShadingDictionary(const SkPdfObject& in, SkPdfType1ShadingDictionary** out) {
  return mapType1ShadingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType1ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1ShadingDictionary** out) {
  if (!isType1ShadingDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType1ShadingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType2ShadingDictionary(const SkPdfObject& in, SkPdfType2ShadingDictionary** out) {
  return mapType2ShadingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType2ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType2ShadingDictionary** out) {
  if (!isType2ShadingDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType2ShadingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType3ShadingDictionary(const SkPdfObject& in, SkPdfType3ShadingDictionary** out) {
  return mapType3ShadingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType3ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType3ShadingDictionary** out) {
  if (!isType3ShadingDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType3ShadingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType4ShadingDictionary(const SkPdfObject& in, SkPdfType4ShadingDictionary** out) {
  return mapType4ShadingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType4ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType4ShadingDictionary** out) {
  if (!isType4ShadingDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType4ShadingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType5ShadingDictionary(const SkPdfObject& in, SkPdfType5ShadingDictionary** out) {
  return mapType5ShadingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType5ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType5ShadingDictionary** out) {
  if (!isType5ShadingDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType5ShadingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType6ShadingDictionary(const SkPdfObject& in, SkPdfType6ShadingDictionary** out) {
  return mapType6ShadingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType6ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType6ShadingDictionary** out) {
  if (!isType6ShadingDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType6ShadingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapImageDictionary(const SkPdfObject& in, SkPdfImageDictionary** out) {
  return mapImageDictionary(*in.doc(), *in.podofo(), out);
}

bool mapImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfImageDictionary** out) {
  if (!isImageDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfImageDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapAlternateImageDictionary(const SkPdfObject& in, SkPdfAlternateImageDictionary** out) {
  return mapAlternateImageDictionary(*in.doc(), *in.podofo(), out);
}

bool mapAlternateImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAlternateImageDictionary** out) {
  if (!isAlternateImageDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfAlternateImageDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType1FormDictionary(const SkPdfObject& in, SkPdfType1FormDictionary** out) {
  return mapType1FormDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType1FormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1FormDictionary** out) {
  if (!isType1FormDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType1FormDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapGroupAttributesDictionary(const SkPdfObject& in, SkPdfGroupAttributesDictionary** out) {
  return mapGroupAttributesDictionary(*in.doc(), *in.podofo(), out);
}

bool mapGroupAttributesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfGroupAttributesDictionary** out) {
  if (!isGroupAttributesDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfGroupAttributesDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapReferenceDictionary(const SkPdfObject& in, SkPdfReferenceDictionary** out) {
  return mapReferenceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfReferenceDictionary** out) {
  if (!isReferenceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfReferenceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPSXobjectDictionary(const SkPdfObject& in, SkPdfPSXobjectDictionary** out) {
  return mapPSXobjectDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPSXobjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPSXobjectDictionary** out) {
  if (!isPSXobjectDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPSXobjectDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType1FontDictionary(const SkPdfObject& in, SkPdfType1FontDictionary** out) {
  return mapType1FontDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType1FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1FontDictionary** out) {
  if (!isType1FontDictionary(podofoDoc, podofoObj)) return false;

  if (mapMultiMasterFontDictionary(podofoDoc, podofoObj, (SkPdfMultiMasterFontDictionary**)out)) return true;
  if (mapTrueTypeFontDictionary(podofoDoc, podofoObj, (SkPdfTrueTypeFontDictionary**)out)) return true;
  if (mapType3FontDictionary(podofoDoc, podofoObj, (SkPdfType3FontDictionary**)out)) return true;

  *out = new SkPdfType1FontDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType3FontDictionary(const SkPdfObject& in, SkPdfType3FontDictionary** out) {
  return mapType3FontDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType3FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType3FontDictionary** out) {
  if (!isType3FontDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType3FontDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapEncodingDictionary(const SkPdfObject& in, SkPdfEncodingDictionary** out) {
  return mapEncodingDictionary(*in.doc(), *in.podofo(), out);
}

bool mapEncodingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEncodingDictionary** out) {
  if (!isEncodingDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfEncodingDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCIDSystemInfoDictionary(const SkPdfObject& in, SkPdfCIDSystemInfoDictionary** out) {
  return mapCIDSystemInfoDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCIDSystemInfoDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCIDSystemInfoDictionary** out) {
  if (!isCIDSystemInfoDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCIDSystemInfoDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCIDFontDictionary(const SkPdfObject& in, SkPdfCIDFontDictionary** out) {
  return mapCIDFontDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCIDFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCIDFontDictionary** out) {
  if (!isCIDFontDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCIDFontDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCMapDictionary(const SkPdfObject& in, SkPdfCMapDictionary** out) {
  return mapCMapDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCMapDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCMapDictionary** out) {
  if (!isCMapDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCMapDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType0FontDictionary(const SkPdfObject& in, SkPdfType0FontDictionary** out) {
  return mapType0FontDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType0FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType0FontDictionary** out) {
  if (!isType0FontDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType0FontDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFontDescriptorDictionary(const SkPdfObject& in, SkPdfFontDescriptorDictionary** out) {
  return mapFontDescriptorDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFontDescriptorDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFontDescriptorDictionary** out) {
  if (!isFontDescriptorDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFontDescriptorDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCIDFontDescriptorDictionary(const SkPdfObject& in, SkPdfCIDFontDescriptorDictionary** out) {
  return mapCIDFontDescriptorDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCIDFontDescriptorDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCIDFontDescriptorDictionary** out) {
  if (!isCIDFontDescriptorDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCIDFontDescriptorDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapEmbeddedFontStreamDictionary(const SkPdfObject& in, SkPdfEmbeddedFontStreamDictionary** out) {
  return mapEmbeddedFontStreamDictionary(*in.doc(), *in.podofo(), out);
}

bool mapEmbeddedFontStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEmbeddedFontStreamDictionary** out) {
  if (!isEmbeddedFontStreamDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfEmbeddedFontStreamDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType1HalftoneDictionary(const SkPdfObject& in, SkPdfType1HalftoneDictionary** out) {
  return mapType1HalftoneDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType1HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType1HalftoneDictionary** out) {
  if (!isType1HalftoneDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType1HalftoneDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType6HalftoneDictionary(const SkPdfObject& in, SkPdfType6HalftoneDictionary** out) {
  return mapType6HalftoneDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType6HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType6HalftoneDictionary** out) {
  if (!isType6HalftoneDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType6HalftoneDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType10HalftoneDictionary(const SkPdfObject& in, SkPdfType10HalftoneDictionary** out) {
  return mapType10HalftoneDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType10HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType10HalftoneDictionary** out) {
  if (!isType10HalftoneDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType10HalftoneDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType16HalftoneDictionary(const SkPdfObject& in, SkPdfType16HalftoneDictionary** out) {
  return mapType16HalftoneDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType16HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType16HalftoneDictionary** out) {
  if (!isType16HalftoneDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType16HalftoneDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapType5HalftoneDictionary(const SkPdfObject& in, SkPdfType5HalftoneDictionary** out) {
  return mapType5HalftoneDictionary(*in.doc(), *in.podofo(), out);
}

bool mapType5HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfType5HalftoneDictionary** out) {
  if (!isType5HalftoneDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfType5HalftoneDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSoftMaskDictionary(const SkPdfObject& in, SkPdfSoftMaskDictionary** out) {
  return mapSoftMaskDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSoftMaskDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoftMaskDictionary** out) {
  if (!isSoftMaskDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSoftMaskDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSoftMaskImageDictionary(const SkPdfObject& in, SkPdfSoftMaskImageDictionary** out) {
  return mapSoftMaskImageDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSoftMaskImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoftMaskImageDictionary** out) {
  if (!isSoftMaskImageDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSoftMaskImageDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTransparencyGroupDictionary(const SkPdfObject& in, SkPdfTransparencyGroupDictionary** out) {
  return mapTransparencyGroupDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTransparencyGroupDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTransparencyGroupDictionary** out) {
  if (!isTransparencyGroupDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTransparencyGroupDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapViewerPreferencesDictionary(const SkPdfObject& in, SkPdfViewerPreferencesDictionary** out) {
  return mapViewerPreferencesDictionary(*in.doc(), *in.podofo(), out);
}

bool mapViewerPreferencesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfViewerPreferencesDictionary** out) {
  if (!isViewerPreferencesDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfViewerPreferencesDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapOutlineDictionary(const SkPdfObject& in, SkPdfOutlineDictionary** out) {
  return mapOutlineDictionary(*in.doc(), *in.podofo(), out);
}

bool mapOutlineDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfOutlineDictionary** out) {
  if (!isOutlineDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfOutlineDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapOutlineItemDictionary(const SkPdfObject& in, SkPdfOutlineItemDictionary** out) {
  return mapOutlineItemDictionary(*in.doc(), *in.podofo(), out);
}

bool mapOutlineItemDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfOutlineItemDictionary** out) {
  if (!isOutlineItemDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfOutlineItemDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPageLabelDictionary(const SkPdfObject& in, SkPdfPageLabelDictionary** out) {
  return mapPageLabelDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPageLabelDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageLabelDictionary** out) {
  if (!isPageLabelDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPageLabelDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapThreadDictionary(const SkPdfObject& in, SkPdfThreadDictionary** out) {
  return mapThreadDictionary(*in.doc(), *in.podofo(), out);
}

bool mapThreadDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfThreadDictionary** out) {
  if (!isThreadDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfThreadDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapBeadDictionary(const SkPdfObject& in, SkPdfBeadDictionary** out) {
  return mapBeadDictionary(*in.doc(), *in.podofo(), out);
}

bool mapBeadDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBeadDictionary** out) {
  if (!isBeadDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfBeadDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTransitionDictionary(const SkPdfObject& in, SkPdfTransitionDictionary** out) {
  return mapTransitionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTransitionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTransitionDictionary** out) {
  if (!isTransitionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTransitionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapAnnotationDictionary(const SkPdfObject& in, SkPdfAnnotationDictionary** out) {
  return mapAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAnnotationDictionary** out) {
  if (!isAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapBorderStyleDictionary(const SkPdfObject& in, SkPdfBorderStyleDictionary** out) {
  return mapBorderStyleDictionary(*in.doc(), *in.podofo(), out);
}

bool mapBorderStyleDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBorderStyleDictionary** out) {
  if (!isBorderStyleDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfBorderStyleDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapAppearanceDictionary(const SkPdfObject& in, SkPdfAppearanceDictionary** out) {
  return mapAppearanceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapAppearanceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAppearanceDictionary** out) {
  if (!isAppearanceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfAppearanceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTextAnnotationDictionary(const SkPdfObject& in, SkPdfTextAnnotationDictionary** out) {
  return mapTextAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTextAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTextAnnotationDictionary** out) {
  if (!isTextAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTextAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapALinkAnnotationDictionary(const SkPdfObject& in, SkPdfALinkAnnotationDictionary** out) {
  return mapALinkAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapALinkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfALinkAnnotationDictionary** out) {
  if (!isALinkAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfALinkAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFreeTextAnnotationDictionary(const SkPdfObject& in, SkPdfFreeTextAnnotationDictionary** out) {
  return mapFreeTextAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFreeTextAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFreeTextAnnotationDictionary** out) {
  if (!isFreeTextAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFreeTextAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapLineAnnotationDictionary(const SkPdfObject& in, SkPdfLineAnnotationDictionary** out) {
  return mapLineAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapLineAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLineAnnotationDictionary** out) {
  if (!isLineAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfLineAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSquareOrCircleAnnotation(const SkPdfObject& in, SkPdfSquareOrCircleAnnotation** out) {
  return mapSquareOrCircleAnnotation(*in.doc(), *in.podofo(), out);
}

bool mapSquareOrCircleAnnotation(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSquareOrCircleAnnotation** out) {
  if (!isSquareOrCircleAnnotation(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSquareOrCircleAnnotation(&podofoDoc, &podofoObj);
  return true;
}

bool mapMarkupAnnotationsDictionary(const SkPdfObject& in, SkPdfMarkupAnnotationsDictionary** out) {
  return mapMarkupAnnotationsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMarkupAnnotationsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMarkupAnnotationsDictionary** out) {
  if (!isMarkupAnnotationsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMarkupAnnotationsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapRubberStampAnnotationDictionary(const SkPdfObject& in, SkPdfRubberStampAnnotationDictionary** out) {
  return mapRubberStampAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapRubberStampAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfRubberStampAnnotationDictionary** out) {
  if (!isRubberStampAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfRubberStampAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapInkAnnotationDictionary(const SkPdfObject& in, SkPdfInkAnnotationDictionary** out) {
  return mapInkAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapInkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInkAnnotationDictionary** out) {
  if (!isInkAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfInkAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPopUpAnnotationDictionary(const SkPdfObject& in, SkPdfPopUpAnnotationDictionary** out) {
  return mapPopUpAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPopUpAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPopUpAnnotationDictionary** out) {
  if (!isPopUpAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPopUpAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFileAttachmentAnnotationDictionary(const SkPdfObject& in, SkPdfFileAttachmentAnnotationDictionary** out) {
  return mapFileAttachmentAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFileAttachmentAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFileAttachmentAnnotationDictionary** out) {
  if (!isFileAttachmentAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFileAttachmentAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSoundAnnotationDictionary(const SkPdfObject& in, SkPdfSoundAnnotationDictionary** out) {
  return mapSoundAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSoundAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoundAnnotationDictionary** out) {
  if (!isSoundAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSoundAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMovieAnnotationDictionary(const SkPdfObject& in, SkPdfMovieAnnotationDictionary** out) {
  return mapMovieAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMovieAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieAnnotationDictionary** out) {
  if (!isMovieAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMovieAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWidgetAnnotationDictionary(const SkPdfObject& in, SkPdfWidgetAnnotationDictionary** out) {
  return mapWidgetAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWidgetAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWidgetAnnotationDictionary** out) {
  if (!isWidgetAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWidgetAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapActionDictionary(const SkPdfObject& in, SkPdfActionDictionary** out) {
  return mapActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfActionDictionary** out) {
  if (!isActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapAnnotationActionsDictionary(const SkPdfObject& in, SkPdfAnnotationActionsDictionary** out) {
  return mapAnnotationActionsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapAnnotationActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAnnotationActionsDictionary** out) {
  if (!isAnnotationActionsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfAnnotationActionsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPageObjectActionsDictionary(const SkPdfObject& in, SkPdfPageObjectActionsDictionary** out) {
  return mapPageObjectActionsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPageObjectActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPageObjectActionsDictionary** out) {
  if (!isPageObjectActionsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPageObjectActionsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFormFieldActionsDictionary(const SkPdfObject& in, SkPdfFormFieldActionsDictionary** out) {
  return mapFormFieldActionsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFormFieldActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFormFieldActionsDictionary** out) {
  if (!isFormFieldActionsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFormFieldActionsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapDocumentCatalogActionsDictionary(const SkPdfObject& in, SkPdfDocumentCatalogActionsDictionary** out) {
  return mapDocumentCatalogActionsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapDocumentCatalogActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDocumentCatalogActionsDictionary** out) {
  if (!isDocumentCatalogActionsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfDocumentCatalogActionsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapGoToActionDictionary(const SkPdfObject& in, SkPdfGoToActionDictionary** out) {
  return mapGoToActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapGoToActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfGoToActionDictionary** out) {
  if (!isGoToActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfGoToActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapRemoteGoToActionDictionary(const SkPdfObject& in, SkPdfRemoteGoToActionDictionary** out) {
  return mapRemoteGoToActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapRemoteGoToActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfRemoteGoToActionDictionary** out) {
  if (!isRemoteGoToActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfRemoteGoToActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapLaunchActionDictionary(const SkPdfObject& in, SkPdfLaunchActionDictionary** out) {
  return mapLaunchActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapLaunchActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfLaunchActionDictionary** out) {
  if (!isLaunchActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfLaunchActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWindowsLaunchActionDictionary(const SkPdfObject& in, SkPdfWindowsLaunchActionDictionary** out) {
  return mapWindowsLaunchActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWindowsLaunchActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWindowsLaunchActionDictionary** out) {
  if (!isWindowsLaunchActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWindowsLaunchActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapThreadActionDictionary(const SkPdfObject& in, SkPdfThreadActionDictionary** out) {
  return mapThreadActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapThreadActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfThreadActionDictionary** out) {
  if (!isThreadActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfThreadActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapURIActionDictionary(const SkPdfObject& in, SkPdfURIActionDictionary** out) {
  return mapURIActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapURIActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfURIActionDictionary** out) {
  if (!isURIActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfURIActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapURIDictionary(const SkPdfObject& in, SkPdfURIDictionary** out) {
  return mapURIDictionary(*in.doc(), *in.podofo(), out);
}

bool mapURIDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfURIDictionary** out) {
  if (!isURIDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfURIDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSoundActionDictionary(const SkPdfObject& in, SkPdfSoundActionDictionary** out) {
  return mapSoundActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSoundActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoundActionDictionary** out) {
  if (!isSoundActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSoundActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMovieActionDictionary(const SkPdfObject& in, SkPdfMovieActionDictionary** out) {
  return mapMovieActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMovieActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieActionDictionary** out) {
  if (!isMovieActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMovieActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapHideActionDictionary(const SkPdfObject& in, SkPdfHideActionDictionary** out) {
  return mapHideActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapHideActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfHideActionDictionary** out) {
  if (!isHideActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfHideActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapNamedActionsDictionary(const SkPdfObject& in, SkPdfNamedActionsDictionary** out) {
  return mapNamedActionsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapNamedActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfNamedActionsDictionary** out) {
  if (!isNamedActionsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfNamedActionsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapInteractiveFormDictionary(const SkPdfObject& in, SkPdfInteractiveFormDictionary** out) {
  return mapInteractiveFormDictionary(*in.doc(), *in.podofo(), out);
}

bool mapInteractiveFormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInteractiveFormDictionary** out) {
  if (!isInteractiveFormDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfInteractiveFormDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFieldDictionary(const SkPdfObject& in, SkPdfFieldDictionary** out) {
  return mapFieldDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFieldDictionary** out) {
  if (!isFieldDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFieldDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapVariableTextFieldDictionary(const SkPdfObject& in, SkPdfVariableTextFieldDictionary** out) {
  return mapVariableTextFieldDictionary(*in.doc(), *in.podofo(), out);
}

bool mapVariableTextFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfVariableTextFieldDictionary** out) {
  if (!isVariableTextFieldDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfVariableTextFieldDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapAppearanceCharacteristicsDictionary(const SkPdfObject& in, SkPdfAppearanceCharacteristicsDictionary** out) {
  return mapAppearanceCharacteristicsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapAppearanceCharacteristicsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAppearanceCharacteristicsDictionary** out) {
  if (!isAppearanceCharacteristicsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfAppearanceCharacteristicsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapCheckboxFieldDictionary(const SkPdfObject& in, SkPdfCheckboxFieldDictionary** out) {
  return mapCheckboxFieldDictionary(*in.doc(), *in.podofo(), out);
}

bool mapCheckboxFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfCheckboxFieldDictionary** out) {
  if (!isCheckboxFieldDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfCheckboxFieldDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapRadioButtonFieldDictionary(const SkPdfObject& in, SkPdfRadioButtonFieldDictionary** out) {
  return mapRadioButtonFieldDictionary(*in.doc(), *in.podofo(), out);
}

bool mapRadioButtonFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfRadioButtonFieldDictionary** out) {
  if (!isRadioButtonFieldDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfRadioButtonFieldDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTextFieldDictionary(const SkPdfObject& in, SkPdfTextFieldDictionary** out) {
  return mapTextFieldDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTextFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTextFieldDictionary** out) {
  if (!isTextFieldDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTextFieldDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapChoiceFieldDictionary(const SkPdfObject& in, SkPdfChoiceFieldDictionary** out) {
  return mapChoiceFieldDictionary(*in.doc(), *in.podofo(), out);
}

bool mapChoiceFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfChoiceFieldDictionary** out) {
  if (!isChoiceFieldDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfChoiceFieldDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSignatureDictionary(const SkPdfObject& in, SkPdfSignatureDictionary** out) {
  return mapSignatureDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSignatureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSignatureDictionary** out) {
  if (!isSignatureDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSignatureDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSubmitFormActionDictionary(const SkPdfObject& in, SkPdfSubmitFormActionDictionary** out) {
  return mapSubmitFormActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSubmitFormActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSubmitFormActionDictionary** out) {
  if (!isSubmitFormActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSubmitFormActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapResetFormActionDictionary(const SkPdfObject& in, SkPdfResetFormActionDictionary** out) {
  return mapResetFormActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapResetFormActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfResetFormActionDictionary** out) {
  if (!isResetFormActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfResetFormActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapImportDataActionDictionary(const SkPdfObject& in, SkPdfImportDataActionDictionary** out) {
  return mapImportDataActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapImportDataActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfImportDataActionDictionary** out) {
  if (!isImportDataActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfImportDataActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapJavascriptActionDictionary(const SkPdfObject& in, SkPdfJavascriptActionDictionary** out) {
  return mapJavascriptActionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapJavascriptActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfJavascriptActionDictionary** out) {
  if (!isJavascriptActionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfJavascriptActionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFTrailerDictionary(const SkPdfObject& in, SkPdfFDFTrailerDictionary** out) {
  return mapFDFTrailerDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFTrailerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFTrailerDictionary** out) {
  if (!isFDFTrailerDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFTrailerDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFCatalogDictionary(const SkPdfObject& in, SkPdfFDFCatalogDictionary** out) {
  return mapFDFCatalogDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFCatalogDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFCatalogDictionary** out) {
  if (!isFDFCatalogDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFCatalogDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFDictionary(const SkPdfObject& in, SkPdfFDFDictionary** out) {
  return mapFDFDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFDictionary** out) {
  if (!isFDFDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapEncryptedEmbeddedFileStreamDictionary(const SkPdfObject& in, SkPdfEncryptedEmbeddedFileStreamDictionary** out) {
  return mapEncryptedEmbeddedFileStreamDictionary(*in.doc(), *in.podofo(), out);
}

bool mapEncryptedEmbeddedFileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfEncryptedEmbeddedFileStreamDictionary** out) {
  if (!isEncryptedEmbeddedFileStreamDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfEncryptedEmbeddedFileStreamDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapJavascriptDictionary(const SkPdfObject& in, SkPdfJavascriptDictionary** out) {
  return mapJavascriptDictionary(*in.doc(), *in.podofo(), out);
}

bool mapJavascriptDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfJavascriptDictionary** out) {
  if (!isJavascriptDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfJavascriptDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFFieldDictionary(const SkPdfObject& in, SkPdfFDFFieldDictionary** out) {
  return mapFDFFieldDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFFieldDictionary** out) {
  if (!isFDFFieldDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFFieldDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapIconFitDictionary(const SkPdfObject& in, SkPdfIconFitDictionary** out) {
  return mapIconFitDictionary(*in.doc(), *in.podofo(), out);
}

bool mapIconFitDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfIconFitDictionary** out) {
  if (!isIconFitDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfIconFitDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFPageDictionary(const SkPdfObject& in, SkPdfFDFPageDictionary** out) {
  return mapFDFPageDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFPageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFPageDictionary** out) {
  if (!isFDFPageDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFPageDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFTemplateDictionary(const SkPdfObject& in, SkPdfFDFTemplateDictionary** out) {
  return mapFDFTemplateDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFTemplateDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFTemplateDictionary** out) {
  if (!isFDFTemplateDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFTemplateDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFNamedPageReferenceDictionary(const SkPdfObject& in, SkPdfFDFNamedPageReferenceDictionary** out) {
  return mapFDFNamedPageReferenceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFNamedPageReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFNamedPageReferenceDictionary** out) {
  if (!isFDFNamedPageReferenceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFNamedPageReferenceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapFDFFileAnnotationDictionary(const SkPdfObject& in, SkPdfFDFFileAnnotationDictionary** out) {
  return mapFDFFileAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapFDFFileAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfFDFFileAnnotationDictionary** out) {
  if (!isFDFFileAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfFDFFileAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSoundObjectDictionary(const SkPdfObject& in, SkPdfSoundObjectDictionary** out) {
  return mapSoundObjectDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSoundObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSoundObjectDictionary** out) {
  if (!isSoundObjectDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSoundObjectDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMovieDictionary(const SkPdfObject& in, SkPdfMovieDictionary** out) {
  return mapMovieDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMovieDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieDictionary** out) {
  if (!isMovieDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMovieDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMovieActivationDictionary(const SkPdfObject& in, SkPdfMovieActivationDictionary** out) {
  return mapMovieActivationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMovieActivationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMovieActivationDictionary** out) {
  if (!isMovieActivationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMovieActivationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapDocumentInformationDictionary(const SkPdfObject& in, SkPdfDocumentInformationDictionary** out) {
  return mapDocumentInformationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapDocumentInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfDocumentInformationDictionary** out) {
  if (!isDocumentInformationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfDocumentInformationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMetadataStreamDictionary(const SkPdfObject& in, SkPdfMetadataStreamDictionary** out) {
  return mapMetadataStreamDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMetadataStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMetadataStreamDictionary** out) {
  if (!isMetadataStreamDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMetadataStreamDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapComponentsWithMetadataDictionary(const SkPdfObject& in, SkPdfComponentsWithMetadataDictionary** out) {
  return mapComponentsWithMetadataDictionary(*in.doc(), *in.podofo(), out);
}

bool mapComponentsWithMetadataDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfComponentsWithMetadataDictionary** out) {
  if (!isComponentsWithMetadataDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfComponentsWithMetadataDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPagePieceDictionary(const SkPdfObject& in, SkPdfPagePieceDictionary** out) {
  return mapPagePieceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPagePieceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPagePieceDictionary** out) {
  if (!isPagePieceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPagePieceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapApplicationDataDictionary(const SkPdfObject& in, SkPdfApplicationDataDictionary** out) {
  return mapApplicationDataDictionary(*in.doc(), *in.podofo(), out);
}

bool mapApplicationDataDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfApplicationDataDictionary** out) {
  if (!isApplicationDataDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfApplicationDataDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapStructureTreeRootDictionary(const SkPdfObject& in, SkPdfStructureTreeRootDictionary** out) {
  return mapStructureTreeRootDictionary(*in.doc(), *in.podofo(), out);
}

bool mapStructureTreeRootDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStructureTreeRootDictionary** out) {
  if (!isStructureTreeRootDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfStructureTreeRootDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapStructureElementDictionary(const SkPdfObject& in, SkPdfStructureElementDictionary** out) {
  return mapStructureElementDictionary(*in.doc(), *in.podofo(), out);
}

bool mapStructureElementDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStructureElementDictionary** out) {
  if (!isStructureElementDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfStructureElementDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMarkedContentReferenceDictionary(const SkPdfObject& in, SkPdfMarkedContentReferenceDictionary** out) {
  return mapMarkedContentReferenceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMarkedContentReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMarkedContentReferenceDictionary** out) {
  if (!isMarkedContentReferenceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMarkedContentReferenceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapObjectReferenceDictionary(const SkPdfObject& in, SkPdfObjectReferenceDictionary** out) {
  return mapObjectReferenceDictionary(*in.doc(), *in.podofo(), out);
}

bool mapObjectReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObjectReferenceDictionary** out) {
  if (!isObjectReferenceDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfObjectReferenceDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapStructureElementAccessDictionary(const SkPdfObject& in, SkPdfStructureElementAccessDictionary** out) {
  return mapStructureElementAccessDictionary(*in.doc(), *in.podofo(), out);
}

bool mapStructureElementAccessDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStructureElementAccessDictionary** out) {
  if (!isStructureElementAccessDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfStructureElementAccessDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapAttributeObjectDictionary(const SkPdfObject& in, SkPdfAttributeObjectDictionary** out) {
  return mapAttributeObjectDictionary(*in.doc(), *in.podofo(), out);
}

bool mapAttributeObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfAttributeObjectDictionary** out) {
  if (!isAttributeObjectDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfAttributeObjectDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMarkInformationDictionary(const SkPdfObject& in, SkPdfMarkInformationDictionary** out) {
  return mapMarkInformationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMarkInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMarkInformationDictionary** out) {
  if (!isMarkInformationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMarkInformationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapArtifactsDictionary(const SkPdfObject& in, SkPdfArtifactsDictionary** out) {
  return mapArtifactsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapArtifactsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfArtifactsDictionary** out) {
  if (!isArtifactsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfArtifactsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapStandardStructureDictionary(const SkPdfObject& in, SkPdfStandardStructureDictionary** out) {
  return mapStandardStructureDictionary(*in.doc(), *in.podofo(), out);
}

bool mapStandardStructureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfStandardStructureDictionary** out) {
  if (!isStandardStructureDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfStandardStructureDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapBlockLevelStructureElementsDictionary(const SkPdfObject& in, SkPdfBlockLevelStructureElementsDictionary** out) {
  return mapBlockLevelStructureElementsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapBlockLevelStructureElementsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBlockLevelStructureElementsDictionary** out) {
  if (!isBlockLevelStructureElementsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfBlockLevelStructureElementsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapInlineLevelStructureElementsDictionary(const SkPdfObject& in, SkPdfInlineLevelStructureElementsDictionary** out) {
  return mapInlineLevelStructureElementsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapInlineLevelStructureElementsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfInlineLevelStructureElementsDictionary** out) {
  if (!isInlineLevelStructureElementsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfInlineLevelStructureElementsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapListAttributeDictionary(const SkPdfObject& in, SkPdfListAttributeDictionary** out) {
  return mapListAttributeDictionary(*in.doc(), *in.podofo(), out);
}

bool mapListAttributeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfListAttributeDictionary** out) {
  if (!isListAttributeDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfListAttributeDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTableAttributesDictionary(const SkPdfObject& in, SkPdfTableAttributesDictionary** out) {
  return mapTableAttributesDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTableAttributesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTableAttributesDictionary** out) {
  if (!isTableAttributesDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTableAttributesDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWebCaptureInformationDictionary(const SkPdfObject& in, SkPdfWebCaptureInformationDictionary** out) {
  return mapWebCaptureInformationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWebCaptureInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureInformationDictionary** out) {
  if (!isWebCaptureInformationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWebCaptureInformationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWebCaptureDictionary(const SkPdfObject& in, SkPdfWebCaptureDictionary** out) {
  return mapWebCaptureDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWebCaptureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureDictionary** out) {
  if (!isWebCaptureDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWebCaptureDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWebCapturePageSetDictionary(const SkPdfObject& in, SkPdfWebCapturePageSetDictionary** out) {
  return mapWebCapturePageSetDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWebCapturePageSetDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCapturePageSetDictionary** out) {
  if (!isWebCapturePageSetDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWebCapturePageSetDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWebCaptureImageSetDictionary(const SkPdfObject& in, SkPdfWebCaptureImageSetDictionary** out) {
  return mapWebCaptureImageSetDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWebCaptureImageSetDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureImageSetDictionary** out) {
  if (!isWebCaptureImageSetDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWebCaptureImageSetDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSourceInformationDictionary(const SkPdfObject& in, SkPdfSourceInformationDictionary** out) {
  return mapSourceInformationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSourceInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSourceInformationDictionary** out) {
  if (!isSourceInformationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSourceInformationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapURLAliasDictionary(const SkPdfObject& in, SkPdfURLAliasDictionary** out) {
  return mapURLAliasDictionary(*in.doc(), *in.podofo(), out);
}

bool mapURLAliasDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfURLAliasDictionary** out) {
  if (!isURLAliasDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfURLAliasDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWebCaptureCommandDictionary(const SkPdfObject& in, SkPdfWebCaptureCommandDictionary** out) {
  return mapWebCaptureCommandDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWebCaptureCommandDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureCommandDictionary** out) {
  if (!isWebCaptureCommandDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWebCaptureCommandDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapWebCaptureCommandSettingsDictionary(const SkPdfObject& in, SkPdfWebCaptureCommandSettingsDictionary** out) {
  return mapWebCaptureCommandSettingsDictionary(*in.doc(), *in.podofo(), out);
}

bool mapWebCaptureCommandSettingsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfWebCaptureCommandSettingsDictionary** out) {
  if (!isWebCaptureCommandSettingsDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfWebCaptureCommandSettingsDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapBoxColorInformationDictionary(const SkPdfObject& in, SkPdfBoxColorInformationDictionary** out) {
  return mapBoxColorInformationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapBoxColorInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBoxColorInformationDictionary** out) {
  if (!isBoxColorInformationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfBoxColorInformationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapBoxStyleDictionary(const SkPdfObject& in, SkPdfBoxStyleDictionary** out) {
  return mapBoxStyleDictionary(*in.doc(), *in.podofo(), out);
}

bool mapBoxStyleDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfBoxStyleDictionary** out) {
  if (!isBoxStyleDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfBoxStyleDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPrinterMarkAnnotationDictionary(const SkPdfObject& in, SkPdfPrinterMarkAnnotationDictionary** out) {
  return mapPrinterMarkAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPrinterMarkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPrinterMarkAnnotationDictionary** out) {
  if (!isPrinterMarkAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPrinterMarkAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPrinterMarkFormDictionary(const SkPdfObject& in, SkPdfPrinterMarkFormDictionary** out) {
  return mapPrinterMarkFormDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPrinterMarkFormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPrinterMarkFormDictionary** out) {
  if (!isPrinterMarkFormDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPrinterMarkFormDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapSeparationDictionary(const SkPdfObject& in, SkPdfSeparationDictionary** out) {
  return mapSeparationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapSeparationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfSeparationDictionary** out) {
  if (!isSeparationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfSeparationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapPDF_XOutputIntentDictionary(const SkPdfObject& in, SkPdfPDF_XOutputIntentDictionary** out) {
  return mapPDF_XOutputIntentDictionary(*in.doc(), *in.podofo(), out);
}

bool mapPDF_XOutputIntentDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfPDF_XOutputIntentDictionary** out) {
  if (!isPDF_XOutputIntentDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfPDF_XOutputIntentDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTrapNetworkAnnotationDictionary(const SkPdfObject& in, SkPdfTrapNetworkAnnotationDictionary** out) {
  return mapTrapNetworkAnnotationDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTrapNetworkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTrapNetworkAnnotationDictionary** out) {
  if (!isTrapNetworkAnnotationDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTrapNetworkAnnotationDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapTrapNetworkAppearanceStreamDictionary(const SkPdfObject& in, SkPdfTrapNetworkAppearanceStreamDictionary** out) {
  return mapTrapNetworkAppearanceStreamDictionary(*in.doc(), *in.podofo(), out);
}

bool mapTrapNetworkAppearanceStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfTrapNetworkAppearanceStreamDictionary** out) {
  if (!isTrapNetworkAppearanceStreamDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfTrapNetworkAppearanceStreamDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapOpiVersionDictionary(const SkPdfObject& in, SkPdfOpiVersionDictionary** out) {
  return mapOpiVersionDictionary(*in.doc(), *in.podofo(), out);
}

bool mapOpiVersionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfOpiVersionDictionary** out) {
  if (!isOpiVersionDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfOpiVersionDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool mapMultiMasterFontDictionary(const SkPdfObject& in, SkPdfMultiMasterFontDictionary** out) {
  return mapMultiMasterFontDictionary(*in.doc(), *in.podofo(), out);
}

bool mapMultiMasterFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfMultiMasterFontDictionary** out) {
  if (!isMultiMasterFontDictionary(podofoDoc, podofoObj)) return false;


  *out = new SkPdfMultiMasterFontDictionary(&podofoDoc, &podofoObj);
  return true;
}

bool isObject(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ObjectFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfObject** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapObject(*pdfDoc, *value, (SkPdfObject**)data);
}

bool ObjectFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfObject** data) {
  if (ObjectFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ObjectFromDictionary(pdfDoc, dict, abr, data);
}

bool isNull(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Null;
}

bool NullFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfNull** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapNull(*pdfDoc, *value, (SkPdfNull**)data);
}

bool NullFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfNull** data) {
  if (NullFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return NullFromDictionary(pdfDoc, dict, abr, data);
}

bool isBoolean(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Bool;
}

bool BooleanFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfBoolean** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapBoolean(*pdfDoc, *value, (SkPdfBoolean**)data);
}

bool BooleanFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfBoolean** data) {
  if (BooleanFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return BooleanFromDictionary(pdfDoc, dict, abr, data);
}

bool isInteger(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Number || podofoObj.GetDataType() == ePdfDataType_Real;
}

bool IntegerFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfInteger** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapInteger(*pdfDoc, *value, (SkPdfInteger**)data);
}

bool IntegerFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfInteger** data) {
  if (IntegerFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return IntegerFromDictionary(pdfDoc, dict, abr, data);
}

bool isNumber(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Number || podofoObj.GetDataType() == ePdfDataType_Real;
}

bool NumberFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfNumber** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapNumber(*pdfDoc, *value, (SkPdfNumber**)data);
}

bool NumberFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfNumber** data) {
  if (NumberFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return NumberFromDictionary(pdfDoc, dict, abr, data);
}

bool isName(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Name;
}

bool NameFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfName** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapName(*pdfDoc, *value, (SkPdfName**)data);
}

bool NameFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfName** data) {
  if (NameFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return NameFromDictionary(pdfDoc, dict, abr, data);
}

bool isReference(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Reference;
}

bool ReferenceFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfReference** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapReference(*pdfDoc, *value, (SkPdfReference**)data);
}

bool ReferenceFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfReference** data) {
  if (ReferenceFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ReferenceFromDictionary(pdfDoc, dict, abr, data);
}

bool isArray(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Array;
}

bool ArrayFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfArray** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapArray(*pdfDoc, *value, (SkPdfArray**)data);
}

bool ArrayFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfArray** data) {
  if (ArrayFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ArrayFromDictionary(pdfDoc, dict, abr, data);
}

bool isString(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_String || podofoObj.GetDataType() == ePdfDataType_HexString;
}

bool StringFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfString** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapString(*pdfDoc, *value, (SkPdfString**)data);
}

bool StringFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfString** data) {
  if (StringFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StringFromDictionary(pdfDoc, dict, abr, data);
}

bool isHexString(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_HexString;
}

bool HexStringFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfHexString** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapHexString(*pdfDoc, *value, (SkPdfHexString**)data);
}

bool HexStringFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfHexString** data) {
  if (HexStringFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return HexStringFromDictionary(pdfDoc, dict, abr, data);
}

bool isDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return podofoObj.GetDataType() == ePdfDataType_Dictionary;
}

bool DictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapDictionary(*pdfDoc, *value, (SkPdfDictionary**)data);
}

bool DictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfDictionary** data) {
  if (DictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return DictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isStream(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool StreamFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfStream** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapStream(*pdfDoc, *value, (SkPdfStream**)data);
}

bool StreamFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfStream** data) {
  if (StreamFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StreamFromDictionary(pdfDoc, dict, abr, data);
}

bool isXObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool XObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfXObjectDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapXObjectDictionary(*pdfDoc, *value, (SkPdfXObjectDictionary**)data);
}

bool XObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfXObjectDictionary** data) {
  if (XObjectDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return XObjectDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFontDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFontDictionary(*pdfDoc, *value, (SkPdfFontDictionary**)data);
}

bool FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFontDictionary** data) {
  if (FontDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FontDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTrueTypeFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "TrueType")) return false;

  return true;
}

bool TrueTypeFontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTrueTypeFontDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTrueTypeFontDictionary(*pdfDoc, *value, (SkPdfTrueTypeFontDictionary**)data);
}

bool TrueTypeFontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTrueTypeFontDictionary** data) {
  if (TrueTypeFontDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TrueTypeFontDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isStreamCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool StreamCommonDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfStreamCommonDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapStreamCommonDictionary(*pdfDoc, *value, (SkPdfStreamCommonDictionary**)data);
}

bool StreamCommonDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfStreamCommonDictionary** data) {
  if (StreamCommonDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StreamCommonDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isLzwdecodeAndFlatedecodeFiltersDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool LzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapLzwdecodeAndFlatedecodeFiltersDictionary(*pdfDoc, *value, (SkPdfLzwdecodeAndFlatedecodeFiltersDictionary**)data);
}

bool LzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** data) {
  if (LzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return LzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCcittfaxdecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CcittfaxdecodeFilterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCcittfaxdecodeFilterDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCcittfaxdecodeFilterDictionary(*pdfDoc, *value, (SkPdfCcittfaxdecodeFilterDictionary**)data);
}

bool CcittfaxdecodeFilterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCcittfaxdecodeFilterDictionary** data) {
  if (CcittfaxdecodeFilterDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CcittfaxdecodeFilterDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isJbig2DecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Jbig2DecodeFilterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfJbig2DecodeFilterDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapJbig2DecodeFilterDictionary(*pdfDoc, *value, (SkPdfJbig2DecodeFilterDictionary**)data);
}

bool Jbig2DecodeFilterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfJbig2DecodeFilterDictionary** data) {
  if (Jbig2DecodeFilterDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Jbig2DecodeFilterDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isDctdecodeFilterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool DctdecodeFilterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfDctdecodeFilterDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapDctdecodeFilterDictionary(*pdfDoc, *value, (SkPdfDctdecodeFilterDictionary**)data);
}

bool DctdecodeFilterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfDctdecodeFilterDictionary** data) {
  if (DctdecodeFilterDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return DctdecodeFilterDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFileTrailerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FileTrailerDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFileTrailerDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFileTrailerDictionary(*pdfDoc, *value, (SkPdfFileTrailerDictionary**)data);
}

bool FileTrailerDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFileTrailerDictionary** data) {
  if (FileTrailerDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FileTrailerDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isEncryptionCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool EncryptionCommonDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfEncryptionCommonDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapEncryptionCommonDictionary(*pdfDoc, *value, (SkPdfEncryptionCommonDictionary**)data);
}

bool EncryptionCommonDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfEncryptionCommonDictionary** data) {
  if (EncryptionCommonDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return EncryptionCommonDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isStandardSecurityHandlerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool StandardSecurityHandlerDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfStandardSecurityHandlerDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapStandardSecurityHandlerDictionary(*pdfDoc, *value, (SkPdfStandardSecurityHandlerDictionary**)data);
}

bool StandardSecurityHandlerDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfStandardSecurityHandlerDictionary** data) {
  if (StandardSecurityHandlerDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StandardSecurityHandlerDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCatalogDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CatalogDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCatalogDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCatalogDictionary(*pdfDoc, *value, (SkPdfCatalogDictionary**)data);
}

bool CatalogDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCatalogDictionary** data) {
  if (CatalogDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CatalogDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPageTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PageTreeNodeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPageTreeNodeDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPageTreeNodeDictionary(*pdfDoc, *value, (SkPdfPageTreeNodeDictionary**)data);
}

bool PageTreeNodeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPageTreeNodeDictionary** data) {
  if (PageTreeNodeDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PageTreeNodeDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPageObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PageObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPageObjectDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPageObjectDictionary(*pdfDoc, *value, (SkPdfPageObjectDictionary**)data);
}

bool PageObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPageObjectDictionary** data) {
  if (PageObjectDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PageObjectDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isNameDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool NameDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfNameDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapNameDictionary(*pdfDoc, *value, (SkPdfNameDictionary**)data);
}

bool NameDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfNameDictionary** data) {
  if (NameDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return NameDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isResourceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ResourceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfResourceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapResourceDictionary(*pdfDoc, *value, (SkPdfResourceDictionary**)data);
}

bool ResourceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfResourceDictionary** data) {
  if (ResourceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ResourceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isNameTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool NameTreeNodeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfNameTreeNodeDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapNameTreeNodeDictionary(*pdfDoc, *value, (SkPdfNameTreeNodeDictionary**)data);
}

bool NameTreeNodeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfNameTreeNodeDictionary** data) {
  if (NameTreeNodeDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return NameTreeNodeDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isNumberTreeNodeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool NumberTreeNodeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfNumberTreeNodeDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapNumberTreeNodeDictionary(*pdfDoc, *value, (SkPdfNumberTreeNodeDictionary**)data);
}

bool NumberTreeNodeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfNumberTreeNodeDictionary** data) {
  if (NumberTreeNodeDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return NumberTreeNodeDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFunctionCommonDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FunctionCommonDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFunctionCommonDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFunctionCommonDictionary(*pdfDoc, *value, (SkPdfFunctionCommonDictionary**)data);
}

bool FunctionCommonDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFunctionCommonDictionary** data) {
  if (FunctionCommonDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FunctionCommonDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType0FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type0FunctionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType0FunctionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType0FunctionDictionary(*pdfDoc, *value, (SkPdfType0FunctionDictionary**)data);
}

bool Type0FunctionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType0FunctionDictionary** data) {
  if (Type0FunctionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type0FunctionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType2FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type2FunctionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType2FunctionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType2FunctionDictionary(*pdfDoc, *value, (SkPdfType2FunctionDictionary**)data);
}

bool Type2FunctionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType2FunctionDictionary** data) {
  if (Type2FunctionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type2FunctionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType3FunctionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type3FunctionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType3FunctionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType3FunctionDictionary(*pdfDoc, *value, (SkPdfType3FunctionDictionary**)data);
}

bool Type3FunctionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType3FunctionDictionary** data) {
  if (Type3FunctionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type3FunctionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFileSpecificationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FileSpecificationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFileSpecificationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFileSpecificationDictionary(*pdfDoc, *value, (SkPdfFileSpecificationDictionary**)data);
}

bool FileSpecificationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFileSpecificationDictionary** data) {
  if (FileSpecificationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FileSpecificationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isEmbeddedFileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool EmbeddedFileStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfEmbeddedFileStreamDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapEmbeddedFileStreamDictionary(*pdfDoc, *value, (SkPdfEmbeddedFileStreamDictionary**)data);
}

bool EmbeddedFileStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfEmbeddedFileStreamDictionary** data) {
  if (EmbeddedFileStreamDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return EmbeddedFileStreamDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isEmbeddedFileParameterDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool EmbeddedFileParameterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfEmbeddedFileParameterDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapEmbeddedFileParameterDictionary(*pdfDoc, *value, (SkPdfEmbeddedFileParameterDictionary**)data);
}

bool EmbeddedFileParameterDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfEmbeddedFileParameterDictionary** data) {
  if (EmbeddedFileParameterDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return EmbeddedFileParameterDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMacOsFileInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MacOsFileInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMacOsFileInformationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMacOsFileInformationDictionary(*pdfDoc, *value, (SkPdfMacOsFileInformationDictionary**)data);
}

bool MacOsFileInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMacOsFileInformationDictionary** data) {
  if (MacOsFileInformationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MacOsFileInformationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isGraphicsStateDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool GraphicsStateDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfGraphicsStateDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapGraphicsStateDictionary(*pdfDoc, *value, (SkPdfGraphicsStateDictionary**)data);
}

bool GraphicsStateDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfGraphicsStateDictionary** data) {
  if (GraphicsStateDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return GraphicsStateDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCalgrayColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CalgrayColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCalgrayColorSpaceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCalgrayColorSpaceDictionary(*pdfDoc, *value, (SkPdfCalgrayColorSpaceDictionary**)data);
}

bool CalgrayColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCalgrayColorSpaceDictionary** data) {
  if (CalgrayColorSpaceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CalgrayColorSpaceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCalrgbColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CalrgbColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCalrgbColorSpaceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCalrgbColorSpaceDictionary(*pdfDoc, *value, (SkPdfCalrgbColorSpaceDictionary**)data);
}

bool CalrgbColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCalrgbColorSpaceDictionary** data) {
  if (CalrgbColorSpaceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CalrgbColorSpaceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isLabColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool LabColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfLabColorSpaceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapLabColorSpaceDictionary(*pdfDoc, *value, (SkPdfLabColorSpaceDictionary**)data);
}

bool LabColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfLabColorSpaceDictionary** data) {
  if (LabColorSpaceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return LabColorSpaceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isIccProfileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool IccProfileStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfIccProfileStreamDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapIccProfileStreamDictionary(*pdfDoc, *value, (SkPdfIccProfileStreamDictionary**)data);
}

bool IccProfileStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfIccProfileStreamDictionary** data) {
  if (IccProfileStreamDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return IccProfileStreamDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isDeviceNColorSpaceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool DeviceNColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfDeviceNColorSpaceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapDeviceNColorSpaceDictionary(*pdfDoc, *value, (SkPdfDeviceNColorSpaceDictionary**)data);
}

bool DeviceNColorSpaceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfDeviceNColorSpaceDictionary** data) {
  if (DeviceNColorSpaceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return DeviceNColorSpaceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType1PatternDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type1PatternDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType1PatternDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType1PatternDictionary(*pdfDoc, *value, (SkPdfType1PatternDictionary**)data);
}

bool Type1PatternDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType1PatternDictionary** data) {
  if (Type1PatternDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type1PatternDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType2PatternDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type2PatternDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType2PatternDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType2PatternDictionary(*pdfDoc, *value, (SkPdfType2PatternDictionary**)data);
}

bool Type2PatternDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType2PatternDictionary** data) {
  if (Type2PatternDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type2PatternDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfShadingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapShadingDictionary(*pdfDoc, *value, (SkPdfShadingDictionary**)data);
}

bool ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfShadingDictionary** data) {
  if (ShadingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ShadingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType1ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type1ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType1ShadingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType1ShadingDictionary(*pdfDoc, *value, (SkPdfType1ShadingDictionary**)data);
}

bool Type1ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType1ShadingDictionary** data) {
  if (Type1ShadingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type1ShadingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType2ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type2ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType2ShadingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType2ShadingDictionary(*pdfDoc, *value, (SkPdfType2ShadingDictionary**)data);
}

bool Type2ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType2ShadingDictionary** data) {
  if (Type2ShadingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type2ShadingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType3ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type3ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType3ShadingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType3ShadingDictionary(*pdfDoc, *value, (SkPdfType3ShadingDictionary**)data);
}

bool Type3ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType3ShadingDictionary** data) {
  if (Type3ShadingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type3ShadingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType4ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type4ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType4ShadingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType4ShadingDictionary(*pdfDoc, *value, (SkPdfType4ShadingDictionary**)data);
}

bool Type4ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType4ShadingDictionary** data) {
  if (Type4ShadingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type4ShadingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType5ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type5ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType5ShadingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType5ShadingDictionary(*pdfDoc, *value, (SkPdfType5ShadingDictionary**)data);
}

bool Type5ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType5ShadingDictionary** data) {
  if (Type5ShadingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type5ShadingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType6ShadingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type6ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType6ShadingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType6ShadingDictionary(*pdfDoc, *value, (SkPdfType6ShadingDictionary**)data);
}

bool Type6ShadingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType6ShadingDictionary** data) {
  if (Type6ShadingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type6ShadingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "Image")) return false;

  return true;
}

bool ImageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfImageDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapImageDictionary(*pdfDoc, *value, (SkPdfImageDictionary**)data);
}

bool ImageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfImageDictionary** data) {
  if (ImageDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ImageDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isAlternateImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool AlternateImageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfAlternateImageDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapAlternateImageDictionary(*pdfDoc, *value, (SkPdfAlternateImageDictionary**)data);
}

bool AlternateImageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfAlternateImageDictionary** data) {
  if (AlternateImageDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return AlternateImageDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType1FormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "Form")) return false;

  return true;
}

bool Type1FormDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType1FormDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType1FormDictionary(*pdfDoc, *value, (SkPdfType1FormDictionary**)data);
}

bool Type1FormDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType1FormDictionary** data) {
  if (Type1FormDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type1FormDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isGroupAttributesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool GroupAttributesDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfGroupAttributesDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapGroupAttributesDictionary(*pdfDoc, *value, (SkPdfGroupAttributesDictionary**)data);
}

bool GroupAttributesDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfGroupAttributesDictionary** data) {
  if (GroupAttributesDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return GroupAttributesDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfReferenceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapReferenceDictionary(*pdfDoc, *value, (SkPdfReferenceDictionary**)data);
}

bool ReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfReferenceDictionary** data) {
  if (ReferenceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ReferenceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPSXobjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PSXobjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPSXobjectDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPSXobjectDictionary(*pdfDoc, *value, (SkPdfPSXobjectDictionary**)data);
}

bool PSXobjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPSXobjectDictionary** data) {
  if (PSXobjectDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PSXobjectDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType1FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "MMType1") && (Subtype != "TrueType") && (Subtype != "Type3") && (Subtype != "Type1")) return false;

  return true;
}

bool Type1FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType1FontDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType1FontDictionary(*pdfDoc, *value, (SkPdfType1FontDictionary**)data);
}

bool Type1FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType1FontDictionary** data) {
  if (Type1FontDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type1FontDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType3FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "Type3")) return false;

  return true;
}

bool Type3FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType3FontDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType3FontDictionary(*pdfDoc, *value, (SkPdfType3FontDictionary**)data);
}

bool Type3FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType3FontDictionary** data) {
  if (Type3FontDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type3FontDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isEncodingDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool EncodingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfEncodingDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapEncodingDictionary(*pdfDoc, *value, (SkPdfEncodingDictionary**)data);
}

bool EncodingDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfEncodingDictionary** data) {
  if (EncodingDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return EncodingDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCIDSystemInfoDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CIDSystemInfoDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCIDSystemInfoDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCIDSystemInfoDictionary(*pdfDoc, *value, (SkPdfCIDSystemInfoDictionary**)data);
}

bool CIDSystemInfoDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCIDSystemInfoDictionary** data) {
  if (CIDSystemInfoDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CIDSystemInfoDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCIDFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "CIDFontType0") && (Subtype != "CIDFontType2")) return false;

  return true;
}

bool CIDFontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCIDFontDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCIDFontDictionary(*pdfDoc, *value, (SkPdfCIDFontDictionary**)data);
}

bool CIDFontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCIDFontDictionary** data) {
  if (CIDFontDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CIDFontDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCMapDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CMapDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCMapDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCMapDictionary(*pdfDoc, *value, (SkPdfCMapDictionary**)data);
}

bool CMapDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCMapDictionary** data) {
  if (CMapDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CMapDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType0FontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "Type0")) return false;

  return true;
}

bool Type0FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType0FontDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType0FontDictionary(*pdfDoc, *value, (SkPdfType0FontDictionary**)data);
}

bool Type0FontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType0FontDictionary** data) {
  if (Type0FontDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type0FontDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFontDescriptorDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Type;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Type", "", &Type)) return false;
  if ((Type != "FontDescriptor")) return false;

  return true;
}

bool FontDescriptorDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFontDescriptorDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFontDescriptorDictionary(*pdfDoc, *value, (SkPdfFontDescriptorDictionary**)data);
}

bool FontDescriptorDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFontDescriptorDictionary** data) {
  if (FontDescriptorDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FontDescriptorDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCIDFontDescriptorDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CIDFontDescriptorDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCIDFontDescriptorDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCIDFontDescriptorDictionary(*pdfDoc, *value, (SkPdfCIDFontDescriptorDictionary**)data);
}

bool CIDFontDescriptorDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCIDFontDescriptorDictionary** data) {
  if (CIDFontDescriptorDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CIDFontDescriptorDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isEmbeddedFontStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool EmbeddedFontStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfEmbeddedFontStreamDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapEmbeddedFontStreamDictionary(*pdfDoc, *value, (SkPdfEmbeddedFontStreamDictionary**)data);
}

bool EmbeddedFontStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfEmbeddedFontStreamDictionary** data) {
  if (EmbeddedFontStreamDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return EmbeddedFontStreamDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType1HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type1HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType1HalftoneDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType1HalftoneDictionary(*pdfDoc, *value, (SkPdfType1HalftoneDictionary**)data);
}

bool Type1HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType1HalftoneDictionary** data) {
  if (Type1HalftoneDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type1HalftoneDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType6HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type6HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType6HalftoneDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType6HalftoneDictionary(*pdfDoc, *value, (SkPdfType6HalftoneDictionary**)data);
}

bool Type6HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType6HalftoneDictionary** data) {
  if (Type6HalftoneDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type6HalftoneDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType10HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type10HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType10HalftoneDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType10HalftoneDictionary(*pdfDoc, *value, (SkPdfType10HalftoneDictionary**)data);
}

bool Type10HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType10HalftoneDictionary** data) {
  if (Type10HalftoneDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type10HalftoneDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType16HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type16HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType16HalftoneDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType16HalftoneDictionary(*pdfDoc, *value, (SkPdfType16HalftoneDictionary**)data);
}

bool Type16HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType16HalftoneDictionary** data) {
  if (Type16HalftoneDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type16HalftoneDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isType5HalftoneDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool Type5HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfType5HalftoneDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapType5HalftoneDictionary(*pdfDoc, *value, (SkPdfType5HalftoneDictionary**)data);
}

bool Type5HalftoneDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfType5HalftoneDictionary** data) {
  if (Type5HalftoneDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return Type5HalftoneDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSoftMaskDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SoftMaskDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSoftMaskDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSoftMaskDictionary(*pdfDoc, *value, (SkPdfSoftMaskDictionary**)data);
}

bool SoftMaskDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSoftMaskDictionary** data) {
  if (SoftMaskDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SoftMaskDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSoftMaskImageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SoftMaskImageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSoftMaskImageDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSoftMaskImageDictionary(*pdfDoc, *value, (SkPdfSoftMaskImageDictionary**)data);
}

bool SoftMaskImageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSoftMaskImageDictionary** data) {
  if (SoftMaskImageDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SoftMaskImageDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTransparencyGroupDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool TransparencyGroupDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTransparencyGroupDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTransparencyGroupDictionary(*pdfDoc, *value, (SkPdfTransparencyGroupDictionary**)data);
}

bool TransparencyGroupDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTransparencyGroupDictionary** data) {
  if (TransparencyGroupDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TransparencyGroupDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isViewerPreferencesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ViewerPreferencesDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfViewerPreferencesDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapViewerPreferencesDictionary(*pdfDoc, *value, (SkPdfViewerPreferencesDictionary**)data);
}

bool ViewerPreferencesDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfViewerPreferencesDictionary** data) {
  if (ViewerPreferencesDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ViewerPreferencesDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isOutlineDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool OutlineDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfOutlineDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapOutlineDictionary(*pdfDoc, *value, (SkPdfOutlineDictionary**)data);
}

bool OutlineDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfOutlineDictionary** data) {
  if (OutlineDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return OutlineDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isOutlineItemDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool OutlineItemDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfOutlineItemDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapOutlineItemDictionary(*pdfDoc, *value, (SkPdfOutlineItemDictionary**)data);
}

bool OutlineItemDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfOutlineItemDictionary** data) {
  if (OutlineItemDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return OutlineItemDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPageLabelDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PageLabelDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPageLabelDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPageLabelDictionary(*pdfDoc, *value, (SkPdfPageLabelDictionary**)data);
}

bool PageLabelDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPageLabelDictionary** data) {
  if (PageLabelDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PageLabelDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isThreadDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ThreadDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfThreadDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapThreadDictionary(*pdfDoc, *value, (SkPdfThreadDictionary**)data);
}

bool ThreadDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfThreadDictionary** data) {
  if (ThreadDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ThreadDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isBeadDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool BeadDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfBeadDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapBeadDictionary(*pdfDoc, *value, (SkPdfBeadDictionary**)data);
}

bool BeadDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfBeadDictionary** data) {
  if (BeadDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return BeadDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTransitionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool TransitionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTransitionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTransitionDictionary(*pdfDoc, *value, (SkPdfTransitionDictionary**)data);
}

bool TransitionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTransitionDictionary** data) {
  if (TransitionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TransitionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool AnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapAnnotationDictionary(*pdfDoc, *value, (SkPdfAnnotationDictionary**)data);
}

bool AnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfAnnotationDictionary** data) {
  if (AnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return AnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isBorderStyleDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool BorderStyleDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfBorderStyleDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapBorderStyleDictionary(*pdfDoc, *value, (SkPdfBorderStyleDictionary**)data);
}

bool BorderStyleDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfBorderStyleDictionary** data) {
  if (BorderStyleDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return BorderStyleDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isAppearanceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool AppearanceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfAppearanceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapAppearanceDictionary(*pdfDoc, *value, (SkPdfAppearanceDictionary**)data);
}

bool AppearanceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfAppearanceDictionary** data) {
  if (AppearanceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return AppearanceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTextAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool TextAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTextAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTextAnnotationDictionary(*pdfDoc, *value, (SkPdfTextAnnotationDictionary**)data);
}

bool TextAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTextAnnotationDictionary** data) {
  if (TextAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TextAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isALinkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ALinkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfALinkAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapALinkAnnotationDictionary(*pdfDoc, *value, (SkPdfALinkAnnotationDictionary**)data);
}

bool ALinkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfALinkAnnotationDictionary** data) {
  if (ALinkAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ALinkAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFreeTextAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FreeTextAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFreeTextAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFreeTextAnnotationDictionary(*pdfDoc, *value, (SkPdfFreeTextAnnotationDictionary**)data);
}

bool FreeTextAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFreeTextAnnotationDictionary** data) {
  if (FreeTextAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FreeTextAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isLineAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool LineAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfLineAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapLineAnnotationDictionary(*pdfDoc, *value, (SkPdfLineAnnotationDictionary**)data);
}

bool LineAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfLineAnnotationDictionary** data) {
  if (LineAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return LineAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSquareOrCircleAnnotation(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SquareOrCircleAnnotationFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSquareOrCircleAnnotation** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSquareOrCircleAnnotation(*pdfDoc, *value, (SkPdfSquareOrCircleAnnotation**)data);
}

bool SquareOrCircleAnnotationFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSquareOrCircleAnnotation** data) {
  if (SquareOrCircleAnnotationFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SquareOrCircleAnnotationFromDictionary(pdfDoc, dict, abr, data);
}

bool isMarkupAnnotationsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MarkupAnnotationsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMarkupAnnotationsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMarkupAnnotationsDictionary(*pdfDoc, *value, (SkPdfMarkupAnnotationsDictionary**)data);
}

bool MarkupAnnotationsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMarkupAnnotationsDictionary** data) {
  if (MarkupAnnotationsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MarkupAnnotationsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isRubberStampAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool RubberStampAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfRubberStampAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapRubberStampAnnotationDictionary(*pdfDoc, *value, (SkPdfRubberStampAnnotationDictionary**)data);
}

bool RubberStampAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfRubberStampAnnotationDictionary** data) {
  if (RubberStampAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return RubberStampAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isInkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool InkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfInkAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapInkAnnotationDictionary(*pdfDoc, *value, (SkPdfInkAnnotationDictionary**)data);
}

bool InkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfInkAnnotationDictionary** data) {
  if (InkAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return InkAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPopUpAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PopUpAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPopUpAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPopUpAnnotationDictionary(*pdfDoc, *value, (SkPdfPopUpAnnotationDictionary**)data);
}

bool PopUpAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPopUpAnnotationDictionary** data) {
  if (PopUpAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PopUpAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFileAttachmentAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FileAttachmentAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFileAttachmentAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFileAttachmentAnnotationDictionary(*pdfDoc, *value, (SkPdfFileAttachmentAnnotationDictionary**)data);
}

bool FileAttachmentAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFileAttachmentAnnotationDictionary** data) {
  if (FileAttachmentAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FileAttachmentAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSoundAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SoundAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSoundAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSoundAnnotationDictionary(*pdfDoc, *value, (SkPdfSoundAnnotationDictionary**)data);
}

bool SoundAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSoundAnnotationDictionary** data) {
  if (SoundAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SoundAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMovieAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MovieAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMovieAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMovieAnnotationDictionary(*pdfDoc, *value, (SkPdfMovieAnnotationDictionary**)data);
}

bool MovieAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMovieAnnotationDictionary** data) {
  if (MovieAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MovieAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWidgetAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WidgetAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWidgetAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWidgetAnnotationDictionary(*pdfDoc, *value, (SkPdfWidgetAnnotationDictionary**)data);
}

bool WidgetAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWidgetAnnotationDictionary** data) {
  if (WidgetAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WidgetAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapActionDictionary(*pdfDoc, *value, (SkPdfActionDictionary**)data);
}

bool ActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfActionDictionary** data) {
  if (ActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isAnnotationActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool AnnotationActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfAnnotationActionsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapAnnotationActionsDictionary(*pdfDoc, *value, (SkPdfAnnotationActionsDictionary**)data);
}

bool AnnotationActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfAnnotationActionsDictionary** data) {
  if (AnnotationActionsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return AnnotationActionsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPageObjectActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PageObjectActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPageObjectActionsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPageObjectActionsDictionary(*pdfDoc, *value, (SkPdfPageObjectActionsDictionary**)data);
}

bool PageObjectActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPageObjectActionsDictionary** data) {
  if (PageObjectActionsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PageObjectActionsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFormFieldActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FormFieldActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFormFieldActionsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFormFieldActionsDictionary(*pdfDoc, *value, (SkPdfFormFieldActionsDictionary**)data);
}

bool FormFieldActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFormFieldActionsDictionary** data) {
  if (FormFieldActionsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FormFieldActionsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isDocumentCatalogActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool DocumentCatalogActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfDocumentCatalogActionsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapDocumentCatalogActionsDictionary(*pdfDoc, *value, (SkPdfDocumentCatalogActionsDictionary**)data);
}

bool DocumentCatalogActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfDocumentCatalogActionsDictionary** data) {
  if (DocumentCatalogActionsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return DocumentCatalogActionsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isGoToActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool GoToActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfGoToActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapGoToActionDictionary(*pdfDoc, *value, (SkPdfGoToActionDictionary**)data);
}

bool GoToActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfGoToActionDictionary** data) {
  if (GoToActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return GoToActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isRemoteGoToActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool RemoteGoToActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfRemoteGoToActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapRemoteGoToActionDictionary(*pdfDoc, *value, (SkPdfRemoteGoToActionDictionary**)data);
}

bool RemoteGoToActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfRemoteGoToActionDictionary** data) {
  if (RemoteGoToActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return RemoteGoToActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isLaunchActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool LaunchActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfLaunchActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapLaunchActionDictionary(*pdfDoc, *value, (SkPdfLaunchActionDictionary**)data);
}

bool LaunchActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfLaunchActionDictionary** data) {
  if (LaunchActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return LaunchActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWindowsLaunchActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WindowsLaunchActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWindowsLaunchActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWindowsLaunchActionDictionary(*pdfDoc, *value, (SkPdfWindowsLaunchActionDictionary**)data);
}

bool WindowsLaunchActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWindowsLaunchActionDictionary** data) {
  if (WindowsLaunchActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WindowsLaunchActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isThreadActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ThreadActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfThreadActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapThreadActionDictionary(*pdfDoc, *value, (SkPdfThreadActionDictionary**)data);
}

bool ThreadActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfThreadActionDictionary** data) {
  if (ThreadActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ThreadActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isURIActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool URIActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfURIActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapURIActionDictionary(*pdfDoc, *value, (SkPdfURIActionDictionary**)data);
}

bool URIActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfURIActionDictionary** data) {
  if (URIActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return URIActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isURIDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool URIDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfURIDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapURIDictionary(*pdfDoc, *value, (SkPdfURIDictionary**)data);
}

bool URIDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfURIDictionary** data) {
  if (URIDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return URIDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSoundActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SoundActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSoundActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSoundActionDictionary(*pdfDoc, *value, (SkPdfSoundActionDictionary**)data);
}

bool SoundActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSoundActionDictionary** data) {
  if (SoundActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SoundActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMovieActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MovieActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMovieActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMovieActionDictionary(*pdfDoc, *value, (SkPdfMovieActionDictionary**)data);
}

bool MovieActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMovieActionDictionary** data) {
  if (MovieActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MovieActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isHideActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool HideActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfHideActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapHideActionDictionary(*pdfDoc, *value, (SkPdfHideActionDictionary**)data);
}

bool HideActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfHideActionDictionary** data) {
  if (HideActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return HideActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isNamedActionsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool NamedActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfNamedActionsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapNamedActionsDictionary(*pdfDoc, *value, (SkPdfNamedActionsDictionary**)data);
}

bool NamedActionsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfNamedActionsDictionary** data) {
  if (NamedActionsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return NamedActionsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isInteractiveFormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool InteractiveFormDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfInteractiveFormDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapInteractiveFormDictionary(*pdfDoc, *value, (SkPdfInteractiveFormDictionary**)data);
}

bool InteractiveFormDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfInteractiveFormDictionary** data) {
  if (InteractiveFormDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return InteractiveFormDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFieldDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFieldDictionary(*pdfDoc, *value, (SkPdfFieldDictionary**)data);
}

bool FieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFieldDictionary** data) {
  if (FieldDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FieldDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isVariableTextFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool VariableTextFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfVariableTextFieldDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapVariableTextFieldDictionary(*pdfDoc, *value, (SkPdfVariableTextFieldDictionary**)data);
}

bool VariableTextFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfVariableTextFieldDictionary** data) {
  if (VariableTextFieldDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return VariableTextFieldDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isAppearanceCharacteristicsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool AppearanceCharacteristicsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfAppearanceCharacteristicsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapAppearanceCharacteristicsDictionary(*pdfDoc, *value, (SkPdfAppearanceCharacteristicsDictionary**)data);
}

bool AppearanceCharacteristicsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfAppearanceCharacteristicsDictionary** data) {
  if (AppearanceCharacteristicsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return AppearanceCharacteristicsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isCheckboxFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool CheckboxFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfCheckboxFieldDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapCheckboxFieldDictionary(*pdfDoc, *value, (SkPdfCheckboxFieldDictionary**)data);
}

bool CheckboxFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfCheckboxFieldDictionary** data) {
  if (CheckboxFieldDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return CheckboxFieldDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isRadioButtonFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool RadioButtonFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfRadioButtonFieldDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapRadioButtonFieldDictionary(*pdfDoc, *value, (SkPdfRadioButtonFieldDictionary**)data);
}

bool RadioButtonFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfRadioButtonFieldDictionary** data) {
  if (RadioButtonFieldDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return RadioButtonFieldDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTextFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool TextFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTextFieldDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTextFieldDictionary(*pdfDoc, *value, (SkPdfTextFieldDictionary**)data);
}

bool TextFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTextFieldDictionary** data) {
  if (TextFieldDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TextFieldDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isChoiceFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ChoiceFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfChoiceFieldDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapChoiceFieldDictionary(*pdfDoc, *value, (SkPdfChoiceFieldDictionary**)data);
}

bool ChoiceFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfChoiceFieldDictionary** data) {
  if (ChoiceFieldDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ChoiceFieldDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSignatureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SignatureDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSignatureDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSignatureDictionary(*pdfDoc, *value, (SkPdfSignatureDictionary**)data);
}

bool SignatureDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSignatureDictionary** data) {
  if (SignatureDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SignatureDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSubmitFormActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SubmitFormActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSubmitFormActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSubmitFormActionDictionary(*pdfDoc, *value, (SkPdfSubmitFormActionDictionary**)data);
}

bool SubmitFormActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSubmitFormActionDictionary** data) {
  if (SubmitFormActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SubmitFormActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isResetFormActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ResetFormActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfResetFormActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapResetFormActionDictionary(*pdfDoc, *value, (SkPdfResetFormActionDictionary**)data);
}

bool ResetFormActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfResetFormActionDictionary** data) {
  if (ResetFormActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ResetFormActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isImportDataActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ImportDataActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfImportDataActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapImportDataActionDictionary(*pdfDoc, *value, (SkPdfImportDataActionDictionary**)data);
}

bool ImportDataActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfImportDataActionDictionary** data) {
  if (ImportDataActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ImportDataActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isJavascriptActionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool JavascriptActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfJavascriptActionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapJavascriptActionDictionary(*pdfDoc, *value, (SkPdfJavascriptActionDictionary**)data);
}

bool JavascriptActionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfJavascriptActionDictionary** data) {
  if (JavascriptActionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return JavascriptActionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFTrailerDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFTrailerDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFTrailerDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFTrailerDictionary(*pdfDoc, *value, (SkPdfFDFTrailerDictionary**)data);
}

bool FDFTrailerDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFTrailerDictionary** data) {
  if (FDFTrailerDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFTrailerDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFCatalogDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFCatalogDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFCatalogDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFCatalogDictionary(*pdfDoc, *value, (SkPdfFDFCatalogDictionary**)data);
}

bool FDFCatalogDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFCatalogDictionary** data) {
  if (FDFCatalogDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFCatalogDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFDictionary(*pdfDoc, *value, (SkPdfFDFDictionary**)data);
}

bool FDFDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFDictionary** data) {
  if (FDFDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isEncryptedEmbeddedFileStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool EncryptedEmbeddedFileStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfEncryptedEmbeddedFileStreamDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapEncryptedEmbeddedFileStreamDictionary(*pdfDoc, *value, (SkPdfEncryptedEmbeddedFileStreamDictionary**)data);
}

bool EncryptedEmbeddedFileStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfEncryptedEmbeddedFileStreamDictionary** data) {
  if (EncryptedEmbeddedFileStreamDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return EncryptedEmbeddedFileStreamDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isJavascriptDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool JavascriptDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfJavascriptDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapJavascriptDictionary(*pdfDoc, *value, (SkPdfJavascriptDictionary**)data);
}

bool JavascriptDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfJavascriptDictionary** data) {
  if (JavascriptDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return JavascriptDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFFieldDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFFieldDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFFieldDictionary(*pdfDoc, *value, (SkPdfFDFFieldDictionary**)data);
}

bool FDFFieldDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFFieldDictionary** data) {
  if (FDFFieldDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFFieldDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isIconFitDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool IconFitDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfIconFitDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapIconFitDictionary(*pdfDoc, *value, (SkPdfIconFitDictionary**)data);
}

bool IconFitDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfIconFitDictionary** data) {
  if (IconFitDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return IconFitDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFPageDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFPageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFPageDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFPageDictionary(*pdfDoc, *value, (SkPdfFDFPageDictionary**)data);
}

bool FDFPageDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFPageDictionary** data) {
  if (FDFPageDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFPageDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFTemplateDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFTemplateDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFTemplateDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFTemplateDictionary(*pdfDoc, *value, (SkPdfFDFTemplateDictionary**)data);
}

bool FDFTemplateDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFTemplateDictionary** data) {
  if (FDFTemplateDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFTemplateDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFNamedPageReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFNamedPageReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFNamedPageReferenceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFNamedPageReferenceDictionary(*pdfDoc, *value, (SkPdfFDFNamedPageReferenceDictionary**)data);
}

bool FDFNamedPageReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFNamedPageReferenceDictionary** data) {
  if (FDFNamedPageReferenceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFNamedPageReferenceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isFDFFileAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool FDFFileAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfFDFFileAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapFDFFileAnnotationDictionary(*pdfDoc, *value, (SkPdfFDFFileAnnotationDictionary**)data);
}

bool FDFFileAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfFDFFileAnnotationDictionary** data) {
  if (FDFFileAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return FDFFileAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSoundObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SoundObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSoundObjectDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSoundObjectDictionary(*pdfDoc, *value, (SkPdfSoundObjectDictionary**)data);
}

bool SoundObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSoundObjectDictionary** data) {
  if (SoundObjectDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SoundObjectDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMovieDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MovieDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMovieDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMovieDictionary(*pdfDoc, *value, (SkPdfMovieDictionary**)data);
}

bool MovieDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMovieDictionary** data) {
  if (MovieDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MovieDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMovieActivationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MovieActivationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMovieActivationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMovieActivationDictionary(*pdfDoc, *value, (SkPdfMovieActivationDictionary**)data);
}

bool MovieActivationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMovieActivationDictionary** data) {
  if (MovieActivationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MovieActivationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isDocumentInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool DocumentInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfDocumentInformationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapDocumentInformationDictionary(*pdfDoc, *value, (SkPdfDocumentInformationDictionary**)data);
}

bool DocumentInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfDocumentInformationDictionary** data) {
  if (DocumentInformationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return DocumentInformationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMetadataStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MetadataStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMetadataStreamDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMetadataStreamDictionary(*pdfDoc, *value, (SkPdfMetadataStreamDictionary**)data);
}

bool MetadataStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMetadataStreamDictionary** data) {
  if (MetadataStreamDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MetadataStreamDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isComponentsWithMetadataDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ComponentsWithMetadataDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfComponentsWithMetadataDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapComponentsWithMetadataDictionary(*pdfDoc, *value, (SkPdfComponentsWithMetadataDictionary**)data);
}

bool ComponentsWithMetadataDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfComponentsWithMetadataDictionary** data) {
  if (ComponentsWithMetadataDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ComponentsWithMetadataDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPagePieceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PagePieceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPagePieceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPagePieceDictionary(*pdfDoc, *value, (SkPdfPagePieceDictionary**)data);
}

bool PagePieceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPagePieceDictionary** data) {
  if (PagePieceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PagePieceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isApplicationDataDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ApplicationDataDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfApplicationDataDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapApplicationDataDictionary(*pdfDoc, *value, (SkPdfApplicationDataDictionary**)data);
}

bool ApplicationDataDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfApplicationDataDictionary** data) {
  if (ApplicationDataDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ApplicationDataDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isStructureTreeRootDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool StructureTreeRootDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfStructureTreeRootDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapStructureTreeRootDictionary(*pdfDoc, *value, (SkPdfStructureTreeRootDictionary**)data);
}

bool StructureTreeRootDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfStructureTreeRootDictionary** data) {
  if (StructureTreeRootDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StructureTreeRootDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isStructureElementDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool StructureElementDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfStructureElementDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapStructureElementDictionary(*pdfDoc, *value, (SkPdfStructureElementDictionary**)data);
}

bool StructureElementDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfStructureElementDictionary** data) {
  if (StructureElementDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StructureElementDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMarkedContentReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MarkedContentReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMarkedContentReferenceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMarkedContentReferenceDictionary(*pdfDoc, *value, (SkPdfMarkedContentReferenceDictionary**)data);
}

bool MarkedContentReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMarkedContentReferenceDictionary** data) {
  if (MarkedContentReferenceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MarkedContentReferenceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isObjectReferenceDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ObjectReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfObjectReferenceDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapObjectReferenceDictionary(*pdfDoc, *value, (SkPdfObjectReferenceDictionary**)data);
}

bool ObjectReferenceDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfObjectReferenceDictionary** data) {
  if (ObjectReferenceDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ObjectReferenceDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isStructureElementAccessDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool StructureElementAccessDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfStructureElementAccessDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapStructureElementAccessDictionary(*pdfDoc, *value, (SkPdfStructureElementAccessDictionary**)data);
}

bool StructureElementAccessDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfStructureElementAccessDictionary** data) {
  if (StructureElementAccessDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StructureElementAccessDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isAttributeObjectDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool AttributeObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfAttributeObjectDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapAttributeObjectDictionary(*pdfDoc, *value, (SkPdfAttributeObjectDictionary**)data);
}

bool AttributeObjectDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfAttributeObjectDictionary** data) {
  if (AttributeObjectDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return AttributeObjectDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMarkInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool MarkInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMarkInformationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMarkInformationDictionary(*pdfDoc, *value, (SkPdfMarkInformationDictionary**)data);
}

bool MarkInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMarkInformationDictionary** data) {
  if (MarkInformationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MarkInformationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isArtifactsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ArtifactsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfArtifactsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapArtifactsDictionary(*pdfDoc, *value, (SkPdfArtifactsDictionary**)data);
}

bool ArtifactsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfArtifactsDictionary** data) {
  if (ArtifactsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ArtifactsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isStandardStructureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool StandardStructureDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfStandardStructureDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapStandardStructureDictionary(*pdfDoc, *value, (SkPdfStandardStructureDictionary**)data);
}

bool StandardStructureDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfStandardStructureDictionary** data) {
  if (StandardStructureDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return StandardStructureDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isBlockLevelStructureElementsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool BlockLevelStructureElementsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfBlockLevelStructureElementsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapBlockLevelStructureElementsDictionary(*pdfDoc, *value, (SkPdfBlockLevelStructureElementsDictionary**)data);
}

bool BlockLevelStructureElementsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfBlockLevelStructureElementsDictionary** data) {
  if (BlockLevelStructureElementsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return BlockLevelStructureElementsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isInlineLevelStructureElementsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool InlineLevelStructureElementsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfInlineLevelStructureElementsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapInlineLevelStructureElementsDictionary(*pdfDoc, *value, (SkPdfInlineLevelStructureElementsDictionary**)data);
}

bool InlineLevelStructureElementsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfInlineLevelStructureElementsDictionary** data) {
  if (InlineLevelStructureElementsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return InlineLevelStructureElementsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isListAttributeDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool ListAttributeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfListAttributeDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapListAttributeDictionary(*pdfDoc, *value, (SkPdfListAttributeDictionary**)data);
}

bool ListAttributeDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfListAttributeDictionary** data) {
  if (ListAttributeDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return ListAttributeDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTableAttributesDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool TableAttributesDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTableAttributesDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTableAttributesDictionary(*pdfDoc, *value, (SkPdfTableAttributesDictionary**)data);
}

bool TableAttributesDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTableAttributesDictionary** data) {
  if (TableAttributesDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TableAttributesDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWebCaptureInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WebCaptureInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWebCaptureInformationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWebCaptureInformationDictionary(*pdfDoc, *value, (SkPdfWebCaptureInformationDictionary**)data);
}

bool WebCaptureInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWebCaptureInformationDictionary** data) {
  if (WebCaptureInformationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WebCaptureInformationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWebCaptureDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WebCaptureDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWebCaptureDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWebCaptureDictionary(*pdfDoc, *value, (SkPdfWebCaptureDictionary**)data);
}

bool WebCaptureDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWebCaptureDictionary** data) {
  if (WebCaptureDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WebCaptureDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWebCapturePageSetDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WebCapturePageSetDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWebCapturePageSetDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWebCapturePageSetDictionary(*pdfDoc, *value, (SkPdfWebCapturePageSetDictionary**)data);
}

bool WebCapturePageSetDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWebCapturePageSetDictionary** data) {
  if (WebCapturePageSetDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WebCapturePageSetDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWebCaptureImageSetDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WebCaptureImageSetDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWebCaptureImageSetDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWebCaptureImageSetDictionary(*pdfDoc, *value, (SkPdfWebCaptureImageSetDictionary**)data);
}

bool WebCaptureImageSetDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWebCaptureImageSetDictionary** data) {
  if (WebCaptureImageSetDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WebCaptureImageSetDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSourceInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SourceInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSourceInformationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSourceInformationDictionary(*pdfDoc, *value, (SkPdfSourceInformationDictionary**)data);
}

bool SourceInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSourceInformationDictionary** data) {
  if (SourceInformationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SourceInformationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isURLAliasDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool URLAliasDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfURLAliasDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapURLAliasDictionary(*pdfDoc, *value, (SkPdfURLAliasDictionary**)data);
}

bool URLAliasDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfURLAliasDictionary** data) {
  if (URLAliasDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return URLAliasDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWebCaptureCommandDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WebCaptureCommandDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWebCaptureCommandDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWebCaptureCommandDictionary(*pdfDoc, *value, (SkPdfWebCaptureCommandDictionary**)data);
}

bool WebCaptureCommandDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWebCaptureCommandDictionary** data) {
  if (WebCaptureCommandDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WebCaptureCommandDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isWebCaptureCommandSettingsDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool WebCaptureCommandSettingsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfWebCaptureCommandSettingsDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapWebCaptureCommandSettingsDictionary(*pdfDoc, *value, (SkPdfWebCaptureCommandSettingsDictionary**)data);
}

bool WebCaptureCommandSettingsDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfWebCaptureCommandSettingsDictionary** data) {
  if (WebCaptureCommandSettingsDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return WebCaptureCommandSettingsDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isBoxColorInformationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool BoxColorInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfBoxColorInformationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapBoxColorInformationDictionary(*pdfDoc, *value, (SkPdfBoxColorInformationDictionary**)data);
}

bool BoxColorInformationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfBoxColorInformationDictionary** data) {
  if (BoxColorInformationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return BoxColorInformationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isBoxStyleDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool BoxStyleDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfBoxStyleDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapBoxStyleDictionary(*pdfDoc, *value, (SkPdfBoxStyleDictionary**)data);
}

bool BoxStyleDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfBoxStyleDictionary** data) {
  if (BoxStyleDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return BoxStyleDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPrinterMarkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PrinterMarkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPrinterMarkAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPrinterMarkAnnotationDictionary(*pdfDoc, *value, (SkPdfPrinterMarkAnnotationDictionary**)data);
}

bool PrinterMarkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPrinterMarkAnnotationDictionary** data) {
  if (PrinterMarkAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PrinterMarkAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPrinterMarkFormDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PrinterMarkFormDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPrinterMarkFormDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPrinterMarkFormDictionary(*pdfDoc, *value, (SkPdfPrinterMarkFormDictionary**)data);
}

bool PrinterMarkFormDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPrinterMarkFormDictionary** data) {
  if (PrinterMarkFormDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PrinterMarkFormDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isSeparationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool SeparationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfSeparationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapSeparationDictionary(*pdfDoc, *value, (SkPdfSeparationDictionary**)data);
}

bool SeparationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfSeparationDictionary** data) {
  if (SeparationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SeparationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isPDF_XOutputIntentDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool PDF_XOutputIntentDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfPDF_XOutputIntentDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapPDF_XOutputIntentDictionary(*pdfDoc, *value, (SkPdfPDF_XOutputIntentDictionary**)data);
}

bool PDF_XOutputIntentDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfPDF_XOutputIntentDictionary** data) {
  if (PDF_XOutputIntentDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return PDF_XOutputIntentDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTrapNetworkAnnotationDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool TrapNetworkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTrapNetworkAnnotationDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTrapNetworkAnnotationDictionary(*pdfDoc, *value, (SkPdfTrapNetworkAnnotationDictionary**)data);
}

bool TrapNetworkAnnotationDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTrapNetworkAnnotationDictionary** data) {
  if (TrapNetworkAnnotationDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TrapNetworkAnnotationDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isTrapNetworkAppearanceStreamDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool TrapNetworkAppearanceStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfTrapNetworkAppearanceStreamDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapTrapNetworkAppearanceStreamDictionary(*pdfDoc, *value, (SkPdfTrapNetworkAppearanceStreamDictionary**)data);
}

bool TrapNetworkAppearanceStreamDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfTrapNetworkAppearanceStreamDictionary** data) {
  if (TrapNetworkAppearanceStreamDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return TrapNetworkAppearanceStreamDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isOpiVersionDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  return true;
}

bool OpiVersionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfOpiVersionDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapOpiVersionDictionary(*pdfDoc, *value, (SkPdfOpiVersionDictionary**)data);
}

bool OpiVersionDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfOpiVersionDictionary** data) {
  if (OpiVersionDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return OpiVersionDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool isMultiMasterFontDictionary(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {
  std::string Subtype;
  if (!NameFromDictionary(&podofoDoc, podofoObj.GetDictionary(), "Subtype", "", &Subtype)) return false;
  if ((Subtype != "MMType1")) return false;

  return true;
}

bool MultiMasterFontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, SkPdfMultiMasterFontDictionary** data) {
  const PdfObject* value = resolveReferenceObject(pdfDoc, dict.GetKey(PdfName(key)), true);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  return mapMultiMasterFontDictionary(*pdfDoc, *value, (SkPdfMultiMasterFontDictionary**)data);
}

bool MultiMasterFontDictionaryFromDictionary(const PdfMemDocument* pdfDoc, const PdfDictionary& dict, const char* key, const char* abr, SkPdfMultiMasterFontDictionary** data) {
  if (MultiMasterFontDictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return MultiMasterFontDictionaryFromDictionary(pdfDoc, dict, abr, data);
}

/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMapper_autogen.h"
#include "SkPdfUtils.h"
#include "SkPdfNativeObject.h"

SkPdfNativeObjectType SkPdfMapper::mapDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isDictionary(in)) return kNone_SkPdfNativeObjectType;

  SkPdfNativeObjectType ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapALinkAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapAlternateImageDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapAnnotationActionsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapAppearanceCharacteristicsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapAppearanceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapApplicationDataDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapArtifactsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapAttributeObjectDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapBeadDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapBlockLevelStructureElementsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapBorderStyleDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapBoxColorInformationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapBoxStyleDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCIDFontDescriptorDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCIDFontDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCIDSystemInfoDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCMapDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCalgrayColorSpaceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCalrgbColorSpaceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCatalogDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCcittfaxdecodeFilterDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapCheckboxFieldDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapChoiceFieldDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapComponentsWithMetadataDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapDctdecodeFilterDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapDeviceNColorSpaceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapDocumentCatalogActionsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapDocumentInformationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapEmbeddedFileParameterDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapEmbeddedFileStreamDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapEmbeddedFontStreamDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapEncodingDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapEncryptedEmbeddedFileStreamDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapEncryptionCommonDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFCatalogDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFFieldDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFFileAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFNamedPageReferenceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFPageDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFTemplateDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFDFTrailerDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFieldDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFileAttachmentAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFileSpecificationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFileTrailerDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFontDescriptorDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFontDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFormFieldActionsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFreeTextAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapFunctionCommonDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapGoToActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapGraphicsStateDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapGroupAttributesDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapHideActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapIccProfileStreamDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapIconFitDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapImportDataActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapInkAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapInlineLevelStructureElementsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapInteractiveFormDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapJavascriptActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapJavascriptDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapJbig2DecodeFilterDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapLabColorSpaceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapLaunchActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapLineAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapListAttributeDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapLzwdecodeAndFlatedecodeFiltersDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMacOsFileInformationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMarkInformationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMarkedContentReferenceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMarkupAnnotationsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMetadataStreamDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMovieActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMovieActivationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMovieAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMovieDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapNameDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapNameTreeNodeDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapNamedActionsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapNumberTreeNodeDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapObjectReferenceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapOpiVersionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapOutlineDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapOutlineItemDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPDF_XOutputIntentDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPSXobjectDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPageLabelDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPageObjectActionsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPageObjectDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPagePieceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPageTreeNodeDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPopUpAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPrinterMarkAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapPrinterMarkFormDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapRadioButtonFieldDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapReferenceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapRemoteGoToActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapResetFormActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapResourceDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapRubberStampAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSeparationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapShadingDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSignatureDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSoftMaskDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSoundActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSoundAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSoundObjectDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSourceInformationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSquareOrCircleAnnotation(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapStandardSecurityHandlerDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapStandardStructureDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapStreamCommonDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapStructureElementAccessDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapStructureElementDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapStructureTreeRootDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSubmitFormActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTableAttributesDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTextAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTextFieldDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapThreadActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapThreadDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTransitionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTransparencyGroupDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTrapNetworkAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTrapNetworkAppearanceStreamDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType0FunctionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType10HalftoneDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType16HalftoneDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType1HalftoneDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType1PatternDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType2FunctionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType2PatternDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType3FunctionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType5HalftoneDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType6HalftoneDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapURIActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapURIDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapURLAliasDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapVariableTextFieldDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapViewerPreferencesDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWebCaptureCommandDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWebCaptureCommandSettingsDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWebCaptureDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWebCaptureImageSetDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWebCaptureInformationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWebCapturePageSetDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWidgetAnnotationDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapWindowsLaunchActionDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapXObjectDictionary(in))) return ret;

  return kDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapXObjectDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isXObjectDictionary(in)) return kNone_SkPdfNativeObjectType;

  SkPdfNativeObjectType ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapImageDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType1FormDictionary(in))) return ret;

  return kXObjectDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFontDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFontDictionary(in)) return kNone_SkPdfNativeObjectType;

  SkPdfNativeObjectType ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType0FontDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType1FontDictionary(in))) return ret;

  return kFontDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTrueTypeFontDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTrueTypeFontDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTrueTypeFontDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapStreamCommonDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isStreamCommonDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kStreamCommonDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapLzwdecodeAndFlatedecodeFiltersDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isLzwdecodeAndFlatedecodeFiltersDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kLzwdecodeAndFlatedecodeFiltersDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCcittfaxdecodeFilterDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCcittfaxdecodeFilterDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCcittfaxdecodeFilterDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapJbig2DecodeFilterDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isJbig2DecodeFilterDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kJbig2DecodeFilterDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapDctdecodeFilterDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isDctdecodeFilterDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kDctdecodeFilterDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFileTrailerDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFileTrailerDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFileTrailerDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapEncryptionCommonDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isEncryptionCommonDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kEncryptionCommonDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapStandardSecurityHandlerDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isStandardSecurityHandlerDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kStandardSecurityHandlerDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCatalogDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCatalogDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCatalogDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPageTreeNodeDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPageTreeNodeDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPageTreeNodeDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPageObjectDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPageObjectDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPageObjectDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapNameDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isNameDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kNameDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapResourceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isResourceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kResourceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapNameTreeNodeDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isNameTreeNodeDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kNameTreeNodeDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapNumberTreeNodeDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isNumberTreeNodeDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kNumberTreeNodeDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFunctionCommonDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFunctionCommonDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFunctionCommonDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType0FunctionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType0FunctionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType0FunctionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType2FunctionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType2FunctionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType2FunctionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType3FunctionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType3FunctionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType3FunctionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFileSpecificationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFileSpecificationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFileSpecificationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapEmbeddedFileStreamDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isEmbeddedFileStreamDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kEmbeddedFileStreamDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapEmbeddedFileParameterDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isEmbeddedFileParameterDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kEmbeddedFileParameterDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMacOsFileInformationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMacOsFileInformationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMacOsFileInformationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapGraphicsStateDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isGraphicsStateDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kGraphicsStateDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCalgrayColorSpaceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCalgrayColorSpaceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCalgrayColorSpaceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCalrgbColorSpaceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCalrgbColorSpaceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCalrgbColorSpaceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapLabColorSpaceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isLabColorSpaceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kLabColorSpaceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapIccProfileStreamDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isIccProfileStreamDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kIccProfileStreamDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapDeviceNColorSpaceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isDeviceNColorSpaceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kDeviceNColorSpaceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType1PatternDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType1PatternDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType1PatternDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType2PatternDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType2PatternDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType2PatternDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapShadingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isShadingDictionary(in)) return kNone_SkPdfNativeObjectType;

  SkPdfNativeObjectType ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType1ShadingDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType2ShadingDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType3ShadingDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType4ShadingDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType5ShadingDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType6ShadingDictionary(in))) return ret;

  return kShadingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType1ShadingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType1ShadingDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType1ShadingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType2ShadingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType2ShadingDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType2ShadingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType3ShadingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType3ShadingDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType3ShadingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType4ShadingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType4ShadingDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType4ShadingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType5ShadingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType5ShadingDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType5ShadingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType6ShadingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType6ShadingDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType6ShadingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapImageDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isImageDictionary(in)) return kNone_SkPdfNativeObjectType;

  SkPdfNativeObjectType ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapSoftMaskImageDictionary(in))) return ret;

  return kImageDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapAlternateImageDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isAlternateImageDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kAlternateImageDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType1FormDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType1FormDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType1FormDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapGroupAttributesDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isGroupAttributesDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kGroupAttributesDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapReferenceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isReferenceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kReferenceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPSXobjectDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPSXobjectDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPSXobjectDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType1FontDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType1FontDictionary(in)) return kNone_SkPdfNativeObjectType;

  SkPdfNativeObjectType ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapMultiMasterFontDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapTrueTypeFontDictionary(in))) return ret;
  if (kNone_SkPdfNativeObjectType != (ret = mapType3FontDictionary(in))) return ret;

  return kType1FontDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType3FontDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType3FontDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType3FontDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapEncodingDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isEncodingDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kEncodingDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCIDSystemInfoDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCIDSystemInfoDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCIDSystemInfoDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCIDFontDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCIDFontDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCIDFontDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCMapDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCMapDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCMapDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType0FontDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType0FontDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType0FontDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFontDescriptorDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFontDescriptorDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFontDescriptorDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCIDFontDescriptorDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCIDFontDescriptorDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCIDFontDescriptorDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapEmbeddedFontStreamDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isEmbeddedFontStreamDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kEmbeddedFontStreamDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType1HalftoneDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType1HalftoneDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType1HalftoneDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType6HalftoneDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType6HalftoneDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType6HalftoneDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType10HalftoneDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType10HalftoneDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType10HalftoneDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType16HalftoneDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType16HalftoneDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType16HalftoneDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapType5HalftoneDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isType5HalftoneDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kType5HalftoneDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSoftMaskDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSoftMaskDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSoftMaskDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSoftMaskImageDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSoftMaskImageDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSoftMaskImageDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTransparencyGroupDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTransparencyGroupDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTransparencyGroupDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapViewerPreferencesDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isViewerPreferencesDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kViewerPreferencesDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapOutlineDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isOutlineDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kOutlineDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapOutlineItemDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isOutlineItemDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kOutlineItemDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPageLabelDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPageLabelDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPageLabelDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapThreadDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isThreadDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kThreadDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapBeadDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isBeadDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kBeadDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTransitionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTransitionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTransitionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapBorderStyleDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isBorderStyleDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kBorderStyleDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapAppearanceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isAppearanceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kAppearanceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTextAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTextAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTextAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapALinkAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isALinkAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kALinkAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFreeTextAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFreeTextAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFreeTextAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapLineAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isLineAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kLineAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSquareOrCircleAnnotation(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSquareOrCircleAnnotation(in)) return kNone_SkPdfNativeObjectType;


  return kSquareOrCircleAnnotation_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMarkupAnnotationsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMarkupAnnotationsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMarkupAnnotationsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapRubberStampAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isRubberStampAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kRubberStampAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapInkAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isInkAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kInkAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPopUpAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPopUpAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPopUpAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFileAttachmentAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFileAttachmentAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFileAttachmentAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSoundAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSoundAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSoundAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMovieAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMovieAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMovieAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWidgetAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWidgetAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWidgetAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapAnnotationActionsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isAnnotationActionsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kAnnotationActionsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPageObjectActionsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPageObjectActionsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPageObjectActionsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFormFieldActionsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFormFieldActionsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFormFieldActionsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapDocumentCatalogActionsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isDocumentCatalogActionsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kDocumentCatalogActionsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapGoToActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isGoToActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kGoToActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapRemoteGoToActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isRemoteGoToActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kRemoteGoToActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapLaunchActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isLaunchActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kLaunchActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWindowsLaunchActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWindowsLaunchActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWindowsLaunchActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapThreadActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isThreadActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kThreadActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapURIActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isURIActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kURIActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapURIDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isURIDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kURIDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSoundActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSoundActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSoundActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMovieActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMovieActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMovieActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapHideActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isHideActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kHideActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapNamedActionsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isNamedActionsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kNamedActionsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapInteractiveFormDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isInteractiveFormDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kInteractiveFormDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFieldDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFieldDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFieldDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapVariableTextFieldDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isVariableTextFieldDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kVariableTextFieldDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapAppearanceCharacteristicsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isAppearanceCharacteristicsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kAppearanceCharacteristicsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapCheckboxFieldDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isCheckboxFieldDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kCheckboxFieldDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapRadioButtonFieldDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isRadioButtonFieldDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kRadioButtonFieldDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTextFieldDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTextFieldDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTextFieldDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapChoiceFieldDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isChoiceFieldDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kChoiceFieldDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSignatureDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSignatureDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSignatureDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSubmitFormActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSubmitFormActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSubmitFormActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapResetFormActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isResetFormActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kResetFormActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapImportDataActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isImportDataActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kImportDataActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapJavascriptActionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isJavascriptActionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kJavascriptActionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFTrailerDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFTrailerDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFTrailerDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFCatalogDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFCatalogDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFCatalogDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapEncryptedEmbeddedFileStreamDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isEncryptedEmbeddedFileStreamDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kEncryptedEmbeddedFileStreamDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapJavascriptDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isJavascriptDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kJavascriptDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFFieldDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFFieldDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFFieldDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapIconFitDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isIconFitDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kIconFitDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFPageDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFPageDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFPageDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFTemplateDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFTemplateDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFTemplateDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFNamedPageReferenceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFNamedPageReferenceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFNamedPageReferenceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapFDFFileAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isFDFFileAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kFDFFileAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSoundObjectDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSoundObjectDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSoundObjectDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMovieDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMovieDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMovieDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMovieActivationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMovieActivationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMovieActivationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapDocumentInformationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isDocumentInformationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kDocumentInformationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMetadataStreamDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMetadataStreamDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMetadataStreamDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapComponentsWithMetadataDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isComponentsWithMetadataDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kComponentsWithMetadataDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPagePieceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPagePieceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPagePieceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapApplicationDataDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isApplicationDataDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kApplicationDataDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapStructureTreeRootDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isStructureTreeRootDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kStructureTreeRootDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapStructureElementDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isStructureElementDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kStructureElementDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMarkedContentReferenceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMarkedContentReferenceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMarkedContentReferenceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapObjectReferenceDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isObjectReferenceDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kObjectReferenceDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapStructureElementAccessDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isStructureElementAccessDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kStructureElementAccessDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapAttributeObjectDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isAttributeObjectDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kAttributeObjectDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMarkInformationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMarkInformationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMarkInformationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapArtifactsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isArtifactsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kArtifactsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapStandardStructureDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isStandardStructureDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kStandardStructureDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapBlockLevelStructureElementsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isBlockLevelStructureElementsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kBlockLevelStructureElementsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapInlineLevelStructureElementsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isInlineLevelStructureElementsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kInlineLevelStructureElementsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapListAttributeDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isListAttributeDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kListAttributeDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTableAttributesDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTableAttributesDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTableAttributesDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWebCaptureInformationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWebCaptureInformationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWebCaptureInformationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWebCaptureDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWebCaptureDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWebCaptureDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWebCapturePageSetDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWebCapturePageSetDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWebCapturePageSetDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWebCaptureImageSetDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWebCaptureImageSetDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWebCaptureImageSetDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSourceInformationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSourceInformationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSourceInformationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapURLAliasDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isURLAliasDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kURLAliasDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWebCaptureCommandDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWebCaptureCommandDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWebCaptureCommandDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapWebCaptureCommandSettingsDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isWebCaptureCommandSettingsDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kWebCaptureCommandSettingsDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapBoxColorInformationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isBoxColorInformationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kBoxColorInformationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapBoxStyleDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isBoxStyleDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kBoxStyleDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPrinterMarkAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPrinterMarkAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPrinterMarkAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPrinterMarkFormDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPrinterMarkFormDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPrinterMarkFormDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapSeparationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isSeparationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kSeparationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapPDF_XOutputIntentDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isPDF_XOutputIntentDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kPDF_XOutputIntentDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTrapNetworkAnnotationDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTrapNetworkAnnotationDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTrapNetworkAnnotationDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapTrapNetworkAppearanceStreamDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isTrapNetworkAppearanceStreamDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kTrapNetworkAppearanceStreamDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapOpiVersionDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isOpiVersionDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kOpiVersionDictionary_SkPdfNativeObjectType;
}

SkPdfNativeObjectType SkPdfMapper::mapMultiMasterFontDictionary(const SkPdfNativeObject* in) const {
  if (in == NULL || !isMultiMasterFontDictionary(in)) return kNone_SkPdfNativeObjectType;


  return kMultiMasterFontDictionary_SkPdfNativeObjectType;
}

bool SkPdfMapper::isDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapDictionary(value)) return false;
  *data = (SkPdfDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfDictionary** data) const {
  if (SkPdfDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isXObjectDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfXObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfXObjectDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapXObjectDictionary(value)) return false;
  *data = (SkPdfXObjectDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfXObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfXObjectDictionary** data) const {
  if (SkPdfXObjectDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfXObjectDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFontDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFontDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFontDictionary(value)) return false;
  *data = (SkPdfFontDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFontDictionary** data) const {
  if (SkPdfFontDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFontDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTrueTypeFontDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "TrueType")) return false;

  return true;
}

bool SkPdfMapper::SkPdfTrueTypeFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTrueTypeFontDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTrueTypeFontDictionary(value)) return false;
  *data = (SkPdfTrueTypeFontDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTrueTypeFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTrueTypeFontDictionary** data) const {
  if (SkPdfTrueTypeFontDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTrueTypeFontDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isStreamCommonDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfStreamCommonDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfStreamCommonDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapStreamCommonDictionary(value)) return false;
  *data = (SkPdfStreamCommonDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfStreamCommonDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfStreamCommonDictionary** data) const {
  if (SkPdfStreamCommonDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfStreamCommonDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isLzwdecodeAndFlatedecodeFiltersDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfLzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapLzwdecodeAndFlatedecodeFiltersDictionary(value)) return false;
  *data = (SkPdfLzwdecodeAndFlatedecodeFiltersDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfLzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfLzwdecodeAndFlatedecodeFiltersDictionary** data) const {
  if (SkPdfLzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfLzwdecodeAndFlatedecodeFiltersDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCcittfaxdecodeFilterDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCcittfaxdecodeFilterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCcittfaxdecodeFilterDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCcittfaxdecodeFilterDictionary(value)) return false;
  *data = (SkPdfCcittfaxdecodeFilterDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCcittfaxdecodeFilterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCcittfaxdecodeFilterDictionary** data) const {
  if (SkPdfCcittfaxdecodeFilterDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCcittfaxdecodeFilterDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isJbig2DecodeFilterDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfJbig2DecodeFilterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfJbig2DecodeFilterDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapJbig2DecodeFilterDictionary(value)) return false;
  *data = (SkPdfJbig2DecodeFilterDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfJbig2DecodeFilterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfJbig2DecodeFilterDictionary** data) const {
  if (SkPdfJbig2DecodeFilterDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfJbig2DecodeFilterDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isDctdecodeFilterDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfDctdecodeFilterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfDctdecodeFilterDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapDctdecodeFilterDictionary(value)) return false;
  *data = (SkPdfDctdecodeFilterDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfDctdecodeFilterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfDctdecodeFilterDictionary** data) const {
  if (SkPdfDctdecodeFilterDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfDctdecodeFilterDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFileTrailerDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFileTrailerDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFileTrailerDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFileTrailerDictionary(value)) return false;
  *data = (SkPdfFileTrailerDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFileTrailerDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFileTrailerDictionary** data) const {
  if (SkPdfFileTrailerDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFileTrailerDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isEncryptionCommonDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfEncryptionCommonDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfEncryptionCommonDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapEncryptionCommonDictionary(value)) return false;
  *data = (SkPdfEncryptionCommonDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfEncryptionCommonDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfEncryptionCommonDictionary** data) const {
  if (SkPdfEncryptionCommonDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfEncryptionCommonDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isStandardSecurityHandlerDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfStandardSecurityHandlerDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfStandardSecurityHandlerDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapStandardSecurityHandlerDictionary(value)) return false;
  *data = (SkPdfStandardSecurityHandlerDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfStandardSecurityHandlerDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfStandardSecurityHandlerDictionary** data) const {
  if (SkPdfStandardSecurityHandlerDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfStandardSecurityHandlerDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCatalogDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCatalogDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCatalogDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCatalogDictionary(value)) return false;
  *data = (SkPdfCatalogDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCatalogDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCatalogDictionary** data) const {
  if (SkPdfCatalogDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCatalogDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPageTreeNodeDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPageTreeNodeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPageTreeNodeDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPageTreeNodeDictionary(value)) return false;
  *data = (SkPdfPageTreeNodeDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPageTreeNodeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPageTreeNodeDictionary** data) const {
  if (SkPdfPageTreeNodeDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPageTreeNodeDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPageObjectDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPageObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPageObjectDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPageObjectDictionary(value)) return false;
  *data = (SkPdfPageObjectDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPageObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPageObjectDictionary** data) const {
  if (SkPdfPageObjectDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPageObjectDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isNameDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfNameDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfNameDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapNameDictionary(value)) return false;
  *data = (SkPdfNameDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfNameDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfNameDictionary** data) const {
  if (SkPdfNameDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfNameDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isResourceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfResourceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfResourceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapResourceDictionary(value)) return false;
  *data = (SkPdfResourceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfResourceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfResourceDictionary** data) const {
  if (SkPdfResourceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfResourceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isNameTreeNodeDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfNameTreeNodeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfNameTreeNodeDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapNameTreeNodeDictionary(value)) return false;
  *data = (SkPdfNameTreeNodeDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfNameTreeNodeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfNameTreeNodeDictionary** data) const {
  if (SkPdfNameTreeNodeDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfNameTreeNodeDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isNumberTreeNodeDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfNumberTreeNodeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfNumberTreeNodeDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapNumberTreeNodeDictionary(value)) return false;
  *data = (SkPdfNumberTreeNodeDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfNumberTreeNodeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfNumberTreeNodeDictionary** data) const {
  if (SkPdfNumberTreeNodeDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfNumberTreeNodeDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFunctionCommonDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFunctionCommonDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFunctionCommonDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFunctionCommonDictionary(value)) return false;
  *data = (SkPdfFunctionCommonDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFunctionCommonDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFunctionCommonDictionary** data) const {
  if (SkPdfFunctionCommonDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFunctionCommonDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType0FunctionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType0FunctionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType0FunctionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType0FunctionDictionary(value)) return false;
  *data = (SkPdfType0FunctionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType0FunctionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType0FunctionDictionary** data) const {
  if (SkPdfType0FunctionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType0FunctionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType2FunctionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType2FunctionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType2FunctionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType2FunctionDictionary(value)) return false;
  *data = (SkPdfType2FunctionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType2FunctionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType2FunctionDictionary** data) const {
  if (SkPdfType2FunctionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType2FunctionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType3FunctionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType3FunctionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType3FunctionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType3FunctionDictionary(value)) return false;
  *data = (SkPdfType3FunctionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType3FunctionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType3FunctionDictionary** data) const {
  if (SkPdfType3FunctionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType3FunctionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFileSpecificationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFileSpecificationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFileSpecificationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFileSpecificationDictionary(value)) return false;
  *data = (SkPdfFileSpecificationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFileSpecificationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFileSpecificationDictionary** data) const {
  if (SkPdfFileSpecificationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFileSpecificationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isEmbeddedFileStreamDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfEmbeddedFileStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfEmbeddedFileStreamDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapEmbeddedFileStreamDictionary(value)) return false;
  *data = (SkPdfEmbeddedFileStreamDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfEmbeddedFileStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfEmbeddedFileStreamDictionary** data) const {
  if (SkPdfEmbeddedFileStreamDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfEmbeddedFileStreamDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isEmbeddedFileParameterDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfEmbeddedFileParameterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfEmbeddedFileParameterDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapEmbeddedFileParameterDictionary(value)) return false;
  *data = (SkPdfEmbeddedFileParameterDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfEmbeddedFileParameterDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfEmbeddedFileParameterDictionary** data) const {
  if (SkPdfEmbeddedFileParameterDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfEmbeddedFileParameterDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMacOsFileInformationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMacOsFileInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMacOsFileInformationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMacOsFileInformationDictionary(value)) return false;
  *data = (SkPdfMacOsFileInformationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMacOsFileInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMacOsFileInformationDictionary** data) const {
  if (SkPdfMacOsFileInformationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMacOsFileInformationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isGraphicsStateDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfGraphicsStateDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfGraphicsStateDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapGraphicsStateDictionary(value)) return false;
  *data = (SkPdfGraphicsStateDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfGraphicsStateDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfGraphicsStateDictionary** data) const {
  if (SkPdfGraphicsStateDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfGraphicsStateDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCalgrayColorSpaceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCalgrayColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCalgrayColorSpaceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCalgrayColorSpaceDictionary(value)) return false;
  *data = (SkPdfCalgrayColorSpaceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCalgrayColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCalgrayColorSpaceDictionary** data) const {
  if (SkPdfCalgrayColorSpaceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCalgrayColorSpaceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCalrgbColorSpaceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCalrgbColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCalrgbColorSpaceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCalrgbColorSpaceDictionary(value)) return false;
  *data = (SkPdfCalrgbColorSpaceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCalrgbColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCalrgbColorSpaceDictionary** data) const {
  if (SkPdfCalrgbColorSpaceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCalrgbColorSpaceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isLabColorSpaceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfLabColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfLabColorSpaceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapLabColorSpaceDictionary(value)) return false;
  *data = (SkPdfLabColorSpaceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfLabColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfLabColorSpaceDictionary** data) const {
  if (SkPdfLabColorSpaceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfLabColorSpaceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isIccProfileStreamDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfIccProfileStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfIccProfileStreamDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapIccProfileStreamDictionary(value)) return false;
  *data = (SkPdfIccProfileStreamDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfIccProfileStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfIccProfileStreamDictionary** data) const {
  if (SkPdfIccProfileStreamDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfIccProfileStreamDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isDeviceNColorSpaceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfDeviceNColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfDeviceNColorSpaceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapDeviceNColorSpaceDictionary(value)) return false;
  *data = (SkPdfDeviceNColorSpaceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfDeviceNColorSpaceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfDeviceNColorSpaceDictionary** data) const {
  if (SkPdfDeviceNColorSpaceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfDeviceNColorSpaceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType1PatternDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("PatternType", "");
  if (ret == NULL || !ret->isInteger()) return false;
  if ((ret->intValue() != 1)) return false;

  return true;
}

bool SkPdfMapper::SkPdfType1PatternDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType1PatternDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType1PatternDictionary(value)) return false;
  *data = (SkPdfType1PatternDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType1PatternDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType1PatternDictionary** data) const {
  if (SkPdfType1PatternDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType1PatternDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType2PatternDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType2PatternDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType2PatternDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType2PatternDictionary(value)) return false;
  *data = (SkPdfType2PatternDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType2PatternDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType2PatternDictionary** data) const {
  if (SkPdfType2PatternDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType2PatternDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isShadingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfShadingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapShadingDictionary(value)) return false;
  *data = (SkPdfShadingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfShadingDictionary** data) const {
  if (SkPdfShadingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfShadingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType1ShadingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType1ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType1ShadingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType1ShadingDictionary(value)) return false;
  *data = (SkPdfType1ShadingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType1ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType1ShadingDictionary** data) const {
  if (SkPdfType1ShadingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType1ShadingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType2ShadingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType2ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType2ShadingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType2ShadingDictionary(value)) return false;
  *data = (SkPdfType2ShadingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType2ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType2ShadingDictionary** data) const {
  if (SkPdfType2ShadingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType2ShadingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType3ShadingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType3ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType3ShadingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType3ShadingDictionary(value)) return false;
  *data = (SkPdfType3ShadingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType3ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType3ShadingDictionary** data) const {
  if (SkPdfType3ShadingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType3ShadingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType4ShadingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType4ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType4ShadingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType4ShadingDictionary(value)) return false;
  *data = (SkPdfType4ShadingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType4ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType4ShadingDictionary** data) const {
  if (SkPdfType4ShadingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType4ShadingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType5ShadingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType5ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType5ShadingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType5ShadingDictionary(value)) return false;
  *data = (SkPdfType5ShadingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType5ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType5ShadingDictionary** data) const {
  if (SkPdfType5ShadingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType5ShadingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType6ShadingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType6ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType6ShadingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType6ShadingDictionary(value)) return false;
  *data = (SkPdfType6ShadingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType6ShadingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType6ShadingDictionary** data) const {
  if (SkPdfType6ShadingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType6ShadingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isImageDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "Image") && (ret->nameValue2() != "Image")) return false;

  return true;
}

bool SkPdfMapper::SkPdfImageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfImageDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapImageDictionary(value)) return false;
  *data = (SkPdfImageDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfImageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfImageDictionary** data) const {
  if (SkPdfImageDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfImageDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isAlternateImageDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfAlternateImageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfAlternateImageDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapAlternateImageDictionary(value)) return false;
  *data = (SkPdfAlternateImageDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfAlternateImageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfAlternateImageDictionary** data) const {
  if (SkPdfAlternateImageDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfAlternateImageDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType1FormDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "Form")) return false;

  return true;
}

bool SkPdfMapper::SkPdfType1FormDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType1FormDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType1FormDictionary(value)) return false;
  *data = (SkPdfType1FormDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType1FormDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType1FormDictionary** data) const {
  if (SkPdfType1FormDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType1FormDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isGroupAttributesDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfGroupAttributesDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfGroupAttributesDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapGroupAttributesDictionary(value)) return false;
  *data = (SkPdfGroupAttributesDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfGroupAttributesDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfGroupAttributesDictionary** data) const {
  if (SkPdfGroupAttributesDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfGroupAttributesDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isReferenceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfReferenceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapReferenceDictionary(value)) return false;
  *data = (SkPdfReferenceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfReferenceDictionary** data) const {
  if (SkPdfReferenceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfReferenceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPSXobjectDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPSXobjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPSXobjectDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPSXobjectDictionary(value)) return false;
  *data = (SkPdfPSXobjectDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPSXobjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPSXobjectDictionary** data) const {
  if (SkPdfPSXobjectDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPSXobjectDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType1FontDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "MMType1") && (ret->nameValue2() != "TrueType") && (ret->nameValue2() != "Type3") && (ret->nameValue2() != "Type1")) return false;

  return true;
}

bool SkPdfMapper::SkPdfType1FontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType1FontDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType1FontDictionary(value)) return false;
  *data = (SkPdfType1FontDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType1FontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType1FontDictionary** data) const {
  if (SkPdfType1FontDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType1FontDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType3FontDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "Type3")) return false;

  return true;
}

bool SkPdfMapper::SkPdfType3FontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType3FontDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType3FontDictionary(value)) return false;
  *data = (SkPdfType3FontDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType3FontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType3FontDictionary** data) const {
  if (SkPdfType3FontDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType3FontDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isEncodingDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfEncodingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfEncodingDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapEncodingDictionary(value)) return false;
  *data = (SkPdfEncodingDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfEncodingDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfEncodingDictionary** data) const {
  if (SkPdfEncodingDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfEncodingDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCIDSystemInfoDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCIDSystemInfoDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCIDSystemInfoDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCIDSystemInfoDictionary(value)) return false;
  *data = (SkPdfCIDSystemInfoDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCIDSystemInfoDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCIDSystemInfoDictionary** data) const {
  if (SkPdfCIDSystemInfoDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCIDSystemInfoDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCIDFontDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "CIDFontType0") && (ret->nameValue2() != "CIDFontType2")) return false;

  return true;
}

bool SkPdfMapper::SkPdfCIDFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCIDFontDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCIDFontDictionary(value)) return false;
  *data = (SkPdfCIDFontDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCIDFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCIDFontDictionary** data) const {
  if (SkPdfCIDFontDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCIDFontDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCMapDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCMapDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCMapDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCMapDictionary(value)) return false;
  *data = (SkPdfCMapDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCMapDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCMapDictionary** data) const {
  if (SkPdfCMapDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCMapDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType0FontDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "Type0")) return false;

  return true;
}

bool SkPdfMapper::SkPdfType0FontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType0FontDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType0FontDictionary(value)) return false;
  *data = (SkPdfType0FontDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType0FontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType0FontDictionary** data) const {
  if (SkPdfType0FontDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType0FontDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFontDescriptorDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Type", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "FontDescriptor")) return false;

  return true;
}

bool SkPdfMapper::SkPdfFontDescriptorDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFontDescriptorDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFontDescriptorDictionary(value)) return false;
  *data = (SkPdfFontDescriptorDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFontDescriptorDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFontDescriptorDictionary** data) const {
  if (SkPdfFontDescriptorDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFontDescriptorDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCIDFontDescriptorDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCIDFontDescriptorDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCIDFontDescriptorDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCIDFontDescriptorDictionary(value)) return false;
  *data = (SkPdfCIDFontDescriptorDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCIDFontDescriptorDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCIDFontDescriptorDictionary** data) const {
  if (SkPdfCIDFontDescriptorDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCIDFontDescriptorDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isEmbeddedFontStreamDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfEmbeddedFontStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfEmbeddedFontStreamDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapEmbeddedFontStreamDictionary(value)) return false;
  *data = (SkPdfEmbeddedFontStreamDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfEmbeddedFontStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfEmbeddedFontStreamDictionary** data) const {
  if (SkPdfEmbeddedFontStreamDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfEmbeddedFontStreamDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType1HalftoneDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType1HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType1HalftoneDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType1HalftoneDictionary(value)) return false;
  *data = (SkPdfType1HalftoneDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType1HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType1HalftoneDictionary** data) const {
  if (SkPdfType1HalftoneDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType1HalftoneDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType6HalftoneDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType6HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType6HalftoneDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType6HalftoneDictionary(value)) return false;
  *data = (SkPdfType6HalftoneDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType6HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType6HalftoneDictionary** data) const {
  if (SkPdfType6HalftoneDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType6HalftoneDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType10HalftoneDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType10HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType10HalftoneDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType10HalftoneDictionary(value)) return false;
  *data = (SkPdfType10HalftoneDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType10HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType10HalftoneDictionary** data) const {
  if (SkPdfType10HalftoneDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType10HalftoneDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType16HalftoneDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType16HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType16HalftoneDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType16HalftoneDictionary(value)) return false;
  *data = (SkPdfType16HalftoneDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType16HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType16HalftoneDictionary** data) const {
  if (SkPdfType16HalftoneDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType16HalftoneDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isType5HalftoneDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfType5HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfType5HalftoneDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapType5HalftoneDictionary(value)) return false;
  *data = (SkPdfType5HalftoneDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfType5HalftoneDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfType5HalftoneDictionary** data) const {
  if (SkPdfType5HalftoneDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfType5HalftoneDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSoftMaskDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("S", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "Alpha") && (ret->nameValue2() != "Luminosity")) return false;

  return true;
}

bool SkPdfMapper::SkPdfSoftMaskDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSoftMaskDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSoftMaskDictionary(value)) return false;
  *data = (SkPdfSoftMaskDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSoftMaskDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSoftMaskDictionary** data) const {
  if (SkPdfSoftMaskDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSoftMaskDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSoftMaskImageDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "Image")) return false;

  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("ColorSpace", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "DeviceGray") && (ret->nameValue2() != "Gray")) return false;

  return true;
}

bool SkPdfMapper::SkPdfSoftMaskImageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSoftMaskImageDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSoftMaskImageDictionary(value)) return false;
  *data = (SkPdfSoftMaskImageDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSoftMaskImageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSoftMaskImageDictionary** data) const {
  if (SkPdfSoftMaskImageDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSoftMaskImageDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTransparencyGroupDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("S", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "Transparency")) return false;

  return true;
}

bool SkPdfMapper::SkPdfTransparencyGroupDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTransparencyGroupDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTransparencyGroupDictionary(value)) return false;
  *data = (SkPdfTransparencyGroupDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTransparencyGroupDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTransparencyGroupDictionary** data) const {
  if (SkPdfTransparencyGroupDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTransparencyGroupDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isViewerPreferencesDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfViewerPreferencesDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfViewerPreferencesDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapViewerPreferencesDictionary(value)) return false;
  *data = (SkPdfViewerPreferencesDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfViewerPreferencesDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfViewerPreferencesDictionary** data) const {
  if (SkPdfViewerPreferencesDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfViewerPreferencesDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isOutlineDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfOutlineDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfOutlineDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapOutlineDictionary(value)) return false;
  *data = (SkPdfOutlineDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfOutlineDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfOutlineDictionary** data) const {
  if (SkPdfOutlineDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfOutlineDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isOutlineItemDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfOutlineItemDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfOutlineItemDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapOutlineItemDictionary(value)) return false;
  *data = (SkPdfOutlineItemDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfOutlineItemDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfOutlineItemDictionary** data) const {
  if (SkPdfOutlineItemDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfOutlineItemDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPageLabelDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPageLabelDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPageLabelDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPageLabelDictionary(value)) return false;
  *data = (SkPdfPageLabelDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPageLabelDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPageLabelDictionary** data) const {
  if (SkPdfPageLabelDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPageLabelDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isThreadDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfThreadDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfThreadDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapThreadDictionary(value)) return false;
  *data = (SkPdfThreadDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfThreadDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfThreadDictionary** data) const {
  if (SkPdfThreadDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfThreadDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isBeadDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfBeadDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfBeadDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapBeadDictionary(value)) return false;
  *data = (SkPdfBeadDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfBeadDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfBeadDictionary** data) const {
  if (SkPdfBeadDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfBeadDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTransitionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfTransitionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTransitionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTransitionDictionary(value)) return false;
  *data = (SkPdfTransitionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTransitionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTransitionDictionary** data) const {
  if (SkPdfTransitionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTransitionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapAnnotationDictionary(value)) return false;
  *data = (SkPdfAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfAnnotationDictionary** data) const {
  if (SkPdfAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isBorderStyleDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfBorderStyleDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfBorderStyleDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapBorderStyleDictionary(value)) return false;
  *data = (SkPdfBorderStyleDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfBorderStyleDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfBorderStyleDictionary** data) const {
  if (SkPdfBorderStyleDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfBorderStyleDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isAppearanceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfAppearanceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfAppearanceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapAppearanceDictionary(value)) return false;
  *data = (SkPdfAppearanceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfAppearanceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfAppearanceDictionary** data) const {
  if (SkPdfAppearanceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfAppearanceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTextAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfTextAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTextAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTextAnnotationDictionary(value)) return false;
  *data = (SkPdfTextAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTextAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTextAnnotationDictionary** data) const {
  if (SkPdfTextAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTextAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isALinkAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfALinkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfALinkAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapALinkAnnotationDictionary(value)) return false;
  *data = (SkPdfALinkAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfALinkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfALinkAnnotationDictionary** data) const {
  if (SkPdfALinkAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfALinkAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFreeTextAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFreeTextAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFreeTextAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFreeTextAnnotationDictionary(value)) return false;
  *data = (SkPdfFreeTextAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFreeTextAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFreeTextAnnotationDictionary** data) const {
  if (SkPdfFreeTextAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFreeTextAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isLineAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfLineAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfLineAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapLineAnnotationDictionary(value)) return false;
  *data = (SkPdfLineAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfLineAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfLineAnnotationDictionary** data) const {
  if (SkPdfLineAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfLineAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSquareOrCircleAnnotation(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSquareOrCircleAnnotationFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSquareOrCircleAnnotation** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSquareOrCircleAnnotation(value)) return false;
  *data = (SkPdfSquareOrCircleAnnotation*)value;
  return true;
}

bool SkPdfMapper::SkPdfSquareOrCircleAnnotationFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSquareOrCircleAnnotation** data) const {
  if (SkPdfSquareOrCircleAnnotationFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSquareOrCircleAnnotationFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMarkupAnnotationsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMarkupAnnotationsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMarkupAnnotationsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMarkupAnnotationsDictionary(value)) return false;
  *data = (SkPdfMarkupAnnotationsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMarkupAnnotationsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMarkupAnnotationsDictionary** data) const {
  if (SkPdfMarkupAnnotationsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMarkupAnnotationsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isRubberStampAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfRubberStampAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfRubberStampAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapRubberStampAnnotationDictionary(value)) return false;
  *data = (SkPdfRubberStampAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfRubberStampAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfRubberStampAnnotationDictionary** data) const {
  if (SkPdfRubberStampAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfRubberStampAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isInkAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfInkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfInkAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapInkAnnotationDictionary(value)) return false;
  *data = (SkPdfInkAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfInkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfInkAnnotationDictionary** data) const {
  if (SkPdfInkAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfInkAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPopUpAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPopUpAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPopUpAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPopUpAnnotationDictionary(value)) return false;
  *data = (SkPdfPopUpAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPopUpAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPopUpAnnotationDictionary** data) const {
  if (SkPdfPopUpAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPopUpAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFileAttachmentAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFileAttachmentAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFileAttachmentAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFileAttachmentAnnotationDictionary(value)) return false;
  *data = (SkPdfFileAttachmentAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFileAttachmentAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFileAttachmentAnnotationDictionary** data) const {
  if (SkPdfFileAttachmentAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFileAttachmentAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSoundAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSoundAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSoundAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSoundAnnotationDictionary(value)) return false;
  *data = (SkPdfSoundAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSoundAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSoundAnnotationDictionary** data) const {
  if (SkPdfSoundAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSoundAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMovieAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMovieAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMovieAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMovieAnnotationDictionary(value)) return false;
  *data = (SkPdfMovieAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMovieAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMovieAnnotationDictionary** data) const {
  if (SkPdfMovieAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMovieAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWidgetAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWidgetAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWidgetAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWidgetAnnotationDictionary(value)) return false;
  *data = (SkPdfWidgetAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWidgetAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWidgetAnnotationDictionary** data) const {
  if (SkPdfWidgetAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWidgetAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapActionDictionary(value)) return false;
  *data = (SkPdfActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfActionDictionary** data) const {
  if (SkPdfActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isAnnotationActionsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfAnnotationActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfAnnotationActionsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapAnnotationActionsDictionary(value)) return false;
  *data = (SkPdfAnnotationActionsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfAnnotationActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfAnnotationActionsDictionary** data) const {
  if (SkPdfAnnotationActionsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfAnnotationActionsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPageObjectActionsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPageObjectActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPageObjectActionsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPageObjectActionsDictionary(value)) return false;
  *data = (SkPdfPageObjectActionsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPageObjectActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPageObjectActionsDictionary** data) const {
  if (SkPdfPageObjectActionsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPageObjectActionsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFormFieldActionsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFormFieldActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFormFieldActionsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFormFieldActionsDictionary(value)) return false;
  *data = (SkPdfFormFieldActionsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFormFieldActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFormFieldActionsDictionary** data) const {
  if (SkPdfFormFieldActionsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFormFieldActionsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isDocumentCatalogActionsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfDocumentCatalogActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfDocumentCatalogActionsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapDocumentCatalogActionsDictionary(value)) return false;
  *data = (SkPdfDocumentCatalogActionsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfDocumentCatalogActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfDocumentCatalogActionsDictionary** data) const {
  if (SkPdfDocumentCatalogActionsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfDocumentCatalogActionsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isGoToActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfGoToActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfGoToActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapGoToActionDictionary(value)) return false;
  *data = (SkPdfGoToActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfGoToActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfGoToActionDictionary** data) const {
  if (SkPdfGoToActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfGoToActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isRemoteGoToActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfRemoteGoToActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfRemoteGoToActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapRemoteGoToActionDictionary(value)) return false;
  *data = (SkPdfRemoteGoToActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfRemoteGoToActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfRemoteGoToActionDictionary** data) const {
  if (SkPdfRemoteGoToActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfRemoteGoToActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isLaunchActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfLaunchActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfLaunchActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapLaunchActionDictionary(value)) return false;
  *data = (SkPdfLaunchActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfLaunchActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfLaunchActionDictionary** data) const {
  if (SkPdfLaunchActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfLaunchActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWindowsLaunchActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWindowsLaunchActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWindowsLaunchActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWindowsLaunchActionDictionary(value)) return false;
  *data = (SkPdfWindowsLaunchActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWindowsLaunchActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWindowsLaunchActionDictionary** data) const {
  if (SkPdfWindowsLaunchActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWindowsLaunchActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isThreadActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfThreadActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfThreadActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapThreadActionDictionary(value)) return false;
  *data = (SkPdfThreadActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfThreadActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfThreadActionDictionary** data) const {
  if (SkPdfThreadActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfThreadActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isURIActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfURIActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfURIActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapURIActionDictionary(value)) return false;
  *data = (SkPdfURIActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfURIActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfURIActionDictionary** data) const {
  if (SkPdfURIActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfURIActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isURIDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfURIDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfURIDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapURIDictionary(value)) return false;
  *data = (SkPdfURIDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfURIDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfURIDictionary** data) const {
  if (SkPdfURIDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfURIDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSoundActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSoundActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSoundActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSoundActionDictionary(value)) return false;
  *data = (SkPdfSoundActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSoundActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSoundActionDictionary** data) const {
  if (SkPdfSoundActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSoundActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMovieActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMovieActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMovieActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMovieActionDictionary(value)) return false;
  *data = (SkPdfMovieActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMovieActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMovieActionDictionary** data) const {
  if (SkPdfMovieActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMovieActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isHideActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfHideActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfHideActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapHideActionDictionary(value)) return false;
  *data = (SkPdfHideActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfHideActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfHideActionDictionary** data) const {
  if (SkPdfHideActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfHideActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isNamedActionsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfNamedActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfNamedActionsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapNamedActionsDictionary(value)) return false;
  *data = (SkPdfNamedActionsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfNamedActionsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfNamedActionsDictionary** data) const {
  if (SkPdfNamedActionsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfNamedActionsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isInteractiveFormDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfInteractiveFormDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfInteractiveFormDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapInteractiveFormDictionary(value)) return false;
  *data = (SkPdfInteractiveFormDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfInteractiveFormDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfInteractiveFormDictionary** data) const {
  if (SkPdfInteractiveFormDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfInteractiveFormDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFieldDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFieldDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFieldDictionary(value)) return false;
  *data = (SkPdfFieldDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFieldDictionary** data) const {
  if (SkPdfFieldDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFieldDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isVariableTextFieldDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfVariableTextFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfVariableTextFieldDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapVariableTextFieldDictionary(value)) return false;
  *data = (SkPdfVariableTextFieldDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfVariableTextFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfVariableTextFieldDictionary** data) const {
  if (SkPdfVariableTextFieldDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfVariableTextFieldDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isAppearanceCharacteristicsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfAppearanceCharacteristicsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfAppearanceCharacteristicsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapAppearanceCharacteristicsDictionary(value)) return false;
  *data = (SkPdfAppearanceCharacteristicsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfAppearanceCharacteristicsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfAppearanceCharacteristicsDictionary** data) const {
  if (SkPdfAppearanceCharacteristicsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfAppearanceCharacteristicsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isCheckboxFieldDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfCheckboxFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfCheckboxFieldDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapCheckboxFieldDictionary(value)) return false;
  *data = (SkPdfCheckboxFieldDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfCheckboxFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfCheckboxFieldDictionary** data) const {
  if (SkPdfCheckboxFieldDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfCheckboxFieldDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isRadioButtonFieldDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfRadioButtonFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfRadioButtonFieldDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapRadioButtonFieldDictionary(value)) return false;
  *data = (SkPdfRadioButtonFieldDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfRadioButtonFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfRadioButtonFieldDictionary** data) const {
  if (SkPdfRadioButtonFieldDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfRadioButtonFieldDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTextFieldDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfTextFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTextFieldDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTextFieldDictionary(value)) return false;
  *data = (SkPdfTextFieldDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTextFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTextFieldDictionary** data) const {
  if (SkPdfTextFieldDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTextFieldDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isChoiceFieldDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfChoiceFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfChoiceFieldDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapChoiceFieldDictionary(value)) return false;
  *data = (SkPdfChoiceFieldDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfChoiceFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfChoiceFieldDictionary** data) const {
  if (SkPdfChoiceFieldDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfChoiceFieldDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSignatureDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSignatureDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSignatureDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSignatureDictionary(value)) return false;
  *data = (SkPdfSignatureDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSignatureDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSignatureDictionary** data) const {
  if (SkPdfSignatureDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSignatureDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSubmitFormActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSubmitFormActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSubmitFormActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSubmitFormActionDictionary(value)) return false;
  *data = (SkPdfSubmitFormActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSubmitFormActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSubmitFormActionDictionary** data) const {
  if (SkPdfSubmitFormActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSubmitFormActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isResetFormActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfResetFormActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfResetFormActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapResetFormActionDictionary(value)) return false;
  *data = (SkPdfResetFormActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfResetFormActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfResetFormActionDictionary** data) const {
  if (SkPdfResetFormActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfResetFormActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isImportDataActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfImportDataActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfImportDataActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapImportDataActionDictionary(value)) return false;
  *data = (SkPdfImportDataActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfImportDataActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfImportDataActionDictionary** data) const {
  if (SkPdfImportDataActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfImportDataActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isJavascriptActionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfJavascriptActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfJavascriptActionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapJavascriptActionDictionary(value)) return false;
  *data = (SkPdfJavascriptActionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfJavascriptActionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfJavascriptActionDictionary** data) const {
  if (SkPdfJavascriptActionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfJavascriptActionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFTrailerDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFTrailerDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFTrailerDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFTrailerDictionary(value)) return false;
  *data = (SkPdfFDFTrailerDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFTrailerDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFTrailerDictionary** data) const {
  if (SkPdfFDFTrailerDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFTrailerDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFCatalogDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFCatalogDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFCatalogDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFCatalogDictionary(value)) return false;
  *data = (SkPdfFDFCatalogDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFCatalogDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFCatalogDictionary** data) const {
  if (SkPdfFDFCatalogDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFCatalogDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFDictionary(value)) return false;
  *data = (SkPdfFDFDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFDictionary** data) const {
  if (SkPdfFDFDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isEncryptedEmbeddedFileStreamDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfEncryptedEmbeddedFileStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfEncryptedEmbeddedFileStreamDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapEncryptedEmbeddedFileStreamDictionary(value)) return false;
  *data = (SkPdfEncryptedEmbeddedFileStreamDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfEncryptedEmbeddedFileStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfEncryptedEmbeddedFileStreamDictionary** data) const {
  if (SkPdfEncryptedEmbeddedFileStreamDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfEncryptedEmbeddedFileStreamDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isJavascriptDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfJavascriptDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfJavascriptDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapJavascriptDictionary(value)) return false;
  *data = (SkPdfJavascriptDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfJavascriptDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfJavascriptDictionary** data) const {
  if (SkPdfJavascriptDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfJavascriptDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFFieldDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFFieldDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFFieldDictionary(value)) return false;
  *data = (SkPdfFDFFieldDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFFieldDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFFieldDictionary** data) const {
  if (SkPdfFDFFieldDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFFieldDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isIconFitDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfIconFitDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfIconFitDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapIconFitDictionary(value)) return false;
  *data = (SkPdfIconFitDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfIconFitDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfIconFitDictionary** data) const {
  if (SkPdfIconFitDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfIconFitDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFPageDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFPageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFPageDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFPageDictionary(value)) return false;
  *data = (SkPdfFDFPageDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFPageDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFPageDictionary** data) const {
  if (SkPdfFDFPageDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFPageDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFTemplateDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFTemplateDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFTemplateDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFTemplateDictionary(value)) return false;
  *data = (SkPdfFDFTemplateDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFTemplateDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFTemplateDictionary** data) const {
  if (SkPdfFDFTemplateDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFTemplateDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFNamedPageReferenceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFNamedPageReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFNamedPageReferenceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFNamedPageReferenceDictionary(value)) return false;
  *data = (SkPdfFDFNamedPageReferenceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFNamedPageReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFNamedPageReferenceDictionary** data) const {
  if (SkPdfFDFNamedPageReferenceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFNamedPageReferenceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isFDFFileAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfFDFFileAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfFDFFileAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapFDFFileAnnotationDictionary(value)) return false;
  *data = (SkPdfFDFFileAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfFDFFileAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfFDFFileAnnotationDictionary** data) const {
  if (SkPdfFDFFileAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfFDFFileAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSoundObjectDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSoundObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSoundObjectDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSoundObjectDictionary(value)) return false;
  *data = (SkPdfSoundObjectDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSoundObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSoundObjectDictionary** data) const {
  if (SkPdfSoundObjectDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSoundObjectDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMovieDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMovieDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMovieDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMovieDictionary(value)) return false;
  *data = (SkPdfMovieDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMovieDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMovieDictionary** data) const {
  if (SkPdfMovieDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMovieDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMovieActivationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMovieActivationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMovieActivationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMovieActivationDictionary(value)) return false;
  *data = (SkPdfMovieActivationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMovieActivationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMovieActivationDictionary** data) const {
  if (SkPdfMovieActivationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMovieActivationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isDocumentInformationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfDocumentInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfDocumentInformationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapDocumentInformationDictionary(value)) return false;
  *data = (SkPdfDocumentInformationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfDocumentInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfDocumentInformationDictionary** data) const {
  if (SkPdfDocumentInformationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfDocumentInformationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMetadataStreamDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMetadataStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMetadataStreamDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMetadataStreamDictionary(value)) return false;
  *data = (SkPdfMetadataStreamDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMetadataStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMetadataStreamDictionary** data) const {
  if (SkPdfMetadataStreamDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMetadataStreamDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isComponentsWithMetadataDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfComponentsWithMetadataDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfComponentsWithMetadataDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapComponentsWithMetadataDictionary(value)) return false;
  *data = (SkPdfComponentsWithMetadataDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfComponentsWithMetadataDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfComponentsWithMetadataDictionary** data) const {
  if (SkPdfComponentsWithMetadataDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfComponentsWithMetadataDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPagePieceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPagePieceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPagePieceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPagePieceDictionary(value)) return false;
  *data = (SkPdfPagePieceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPagePieceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPagePieceDictionary** data) const {
  if (SkPdfPagePieceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPagePieceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isApplicationDataDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfApplicationDataDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfApplicationDataDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapApplicationDataDictionary(value)) return false;
  *data = (SkPdfApplicationDataDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfApplicationDataDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfApplicationDataDictionary** data) const {
  if (SkPdfApplicationDataDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfApplicationDataDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isStructureTreeRootDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfStructureTreeRootDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfStructureTreeRootDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapStructureTreeRootDictionary(value)) return false;
  *data = (SkPdfStructureTreeRootDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfStructureTreeRootDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfStructureTreeRootDictionary** data) const {
  if (SkPdfStructureTreeRootDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfStructureTreeRootDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isStructureElementDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfStructureElementDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfStructureElementDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapStructureElementDictionary(value)) return false;
  *data = (SkPdfStructureElementDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfStructureElementDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfStructureElementDictionary** data) const {
  if (SkPdfStructureElementDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfStructureElementDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMarkedContentReferenceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMarkedContentReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMarkedContentReferenceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMarkedContentReferenceDictionary(value)) return false;
  *data = (SkPdfMarkedContentReferenceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMarkedContentReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMarkedContentReferenceDictionary** data) const {
  if (SkPdfMarkedContentReferenceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMarkedContentReferenceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isObjectReferenceDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfObjectReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfObjectReferenceDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapObjectReferenceDictionary(value)) return false;
  *data = (SkPdfObjectReferenceDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfObjectReferenceDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfObjectReferenceDictionary** data) const {
  if (SkPdfObjectReferenceDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfObjectReferenceDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isStructureElementAccessDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfStructureElementAccessDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfStructureElementAccessDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapStructureElementAccessDictionary(value)) return false;
  *data = (SkPdfStructureElementAccessDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfStructureElementAccessDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfStructureElementAccessDictionary** data) const {
  if (SkPdfStructureElementAccessDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfStructureElementAccessDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isAttributeObjectDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfAttributeObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfAttributeObjectDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapAttributeObjectDictionary(value)) return false;
  *data = (SkPdfAttributeObjectDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfAttributeObjectDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfAttributeObjectDictionary** data) const {
  if (SkPdfAttributeObjectDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfAttributeObjectDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMarkInformationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfMarkInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMarkInformationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMarkInformationDictionary(value)) return false;
  *data = (SkPdfMarkInformationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMarkInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMarkInformationDictionary** data) const {
  if (SkPdfMarkInformationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMarkInformationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isArtifactsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfArtifactsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfArtifactsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapArtifactsDictionary(value)) return false;
  *data = (SkPdfArtifactsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfArtifactsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfArtifactsDictionary** data) const {
  if (SkPdfArtifactsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfArtifactsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isStandardStructureDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfStandardStructureDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfStandardStructureDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapStandardStructureDictionary(value)) return false;
  *data = (SkPdfStandardStructureDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfStandardStructureDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfStandardStructureDictionary** data) const {
  if (SkPdfStandardStructureDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfStandardStructureDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isBlockLevelStructureElementsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfBlockLevelStructureElementsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfBlockLevelStructureElementsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapBlockLevelStructureElementsDictionary(value)) return false;
  *data = (SkPdfBlockLevelStructureElementsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfBlockLevelStructureElementsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfBlockLevelStructureElementsDictionary** data) const {
  if (SkPdfBlockLevelStructureElementsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfBlockLevelStructureElementsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isInlineLevelStructureElementsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfInlineLevelStructureElementsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfInlineLevelStructureElementsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapInlineLevelStructureElementsDictionary(value)) return false;
  *data = (SkPdfInlineLevelStructureElementsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfInlineLevelStructureElementsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfInlineLevelStructureElementsDictionary** data) const {
  if (SkPdfInlineLevelStructureElementsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfInlineLevelStructureElementsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isListAttributeDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfListAttributeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfListAttributeDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapListAttributeDictionary(value)) return false;
  *data = (SkPdfListAttributeDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfListAttributeDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfListAttributeDictionary** data) const {
  if (SkPdfListAttributeDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfListAttributeDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTableAttributesDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfTableAttributesDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTableAttributesDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTableAttributesDictionary(value)) return false;
  *data = (SkPdfTableAttributesDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTableAttributesDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTableAttributesDictionary** data) const {
  if (SkPdfTableAttributesDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTableAttributesDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWebCaptureInformationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWebCaptureInformationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWebCaptureInformationDictionary(value)) return false;
  *data = (SkPdfWebCaptureInformationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWebCaptureInformationDictionary** data) const {
  if (SkPdfWebCaptureInformationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWebCaptureInformationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWebCaptureDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWebCaptureDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWebCaptureDictionary(value)) return false;
  *data = (SkPdfWebCaptureDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWebCaptureDictionary** data) const {
  if (SkPdfWebCaptureDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWebCaptureDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWebCapturePageSetDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWebCapturePageSetDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWebCapturePageSetDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWebCapturePageSetDictionary(value)) return false;
  *data = (SkPdfWebCapturePageSetDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWebCapturePageSetDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWebCapturePageSetDictionary** data) const {
  if (SkPdfWebCapturePageSetDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWebCapturePageSetDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWebCaptureImageSetDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureImageSetDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWebCaptureImageSetDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWebCaptureImageSetDictionary(value)) return false;
  *data = (SkPdfWebCaptureImageSetDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureImageSetDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWebCaptureImageSetDictionary** data) const {
  if (SkPdfWebCaptureImageSetDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWebCaptureImageSetDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSourceInformationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSourceInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSourceInformationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSourceInformationDictionary(value)) return false;
  *data = (SkPdfSourceInformationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSourceInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSourceInformationDictionary** data) const {
  if (SkPdfSourceInformationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSourceInformationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isURLAliasDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfURLAliasDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfURLAliasDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapURLAliasDictionary(value)) return false;
  *data = (SkPdfURLAliasDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfURLAliasDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfURLAliasDictionary** data) const {
  if (SkPdfURLAliasDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfURLAliasDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWebCaptureCommandDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureCommandDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWebCaptureCommandDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWebCaptureCommandDictionary(value)) return false;
  *data = (SkPdfWebCaptureCommandDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureCommandDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWebCaptureCommandDictionary** data) const {
  if (SkPdfWebCaptureCommandDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWebCaptureCommandDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isWebCaptureCommandSettingsDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureCommandSettingsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfWebCaptureCommandSettingsDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapWebCaptureCommandSettingsDictionary(value)) return false;
  *data = (SkPdfWebCaptureCommandSettingsDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfWebCaptureCommandSettingsDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfWebCaptureCommandSettingsDictionary** data) const {
  if (SkPdfWebCaptureCommandSettingsDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfWebCaptureCommandSettingsDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isBoxColorInformationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfBoxColorInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfBoxColorInformationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapBoxColorInformationDictionary(value)) return false;
  *data = (SkPdfBoxColorInformationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfBoxColorInformationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfBoxColorInformationDictionary** data) const {
  if (SkPdfBoxColorInformationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfBoxColorInformationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isBoxStyleDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfBoxStyleDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfBoxStyleDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapBoxStyleDictionary(value)) return false;
  *data = (SkPdfBoxStyleDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfBoxStyleDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfBoxStyleDictionary** data) const {
  if (SkPdfBoxStyleDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfBoxStyleDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPrinterMarkAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPrinterMarkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPrinterMarkAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPrinterMarkAnnotationDictionary(value)) return false;
  *data = (SkPdfPrinterMarkAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPrinterMarkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPrinterMarkAnnotationDictionary** data) const {
  if (SkPdfPrinterMarkAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPrinterMarkAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPrinterMarkFormDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPrinterMarkFormDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPrinterMarkFormDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPrinterMarkFormDictionary(value)) return false;
  *data = (SkPdfPrinterMarkFormDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPrinterMarkFormDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPrinterMarkFormDictionary** data) const {
  if (SkPdfPrinterMarkFormDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPrinterMarkFormDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isSeparationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfSeparationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfSeparationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapSeparationDictionary(value)) return false;
  *data = (SkPdfSeparationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfSeparationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfSeparationDictionary** data) const {
  if (SkPdfSeparationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfSeparationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isPDF_XOutputIntentDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfPDF_XOutputIntentDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfPDF_XOutputIntentDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapPDF_XOutputIntentDictionary(value)) return false;
  *data = (SkPdfPDF_XOutputIntentDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfPDF_XOutputIntentDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfPDF_XOutputIntentDictionary** data) const {
  if (SkPdfPDF_XOutputIntentDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfPDF_XOutputIntentDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTrapNetworkAnnotationDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfTrapNetworkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTrapNetworkAnnotationDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTrapNetworkAnnotationDictionary(value)) return false;
  *data = (SkPdfTrapNetworkAnnotationDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTrapNetworkAnnotationDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTrapNetworkAnnotationDictionary** data) const {
  if (SkPdfTrapNetworkAnnotationDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTrapNetworkAnnotationDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isTrapNetworkAppearanceStreamDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfTrapNetworkAppearanceStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfTrapNetworkAppearanceStreamDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapTrapNetworkAppearanceStreamDictionary(value)) return false;
  *data = (SkPdfTrapNetworkAppearanceStreamDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfTrapNetworkAppearanceStreamDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfTrapNetworkAppearanceStreamDictionary** data) const {
  if (SkPdfTrapNetworkAppearanceStreamDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfTrapNetworkAppearanceStreamDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isOpiVersionDictionary(const SkPdfNativeObject* nativeObj) const {
  return true;
}

bool SkPdfMapper::SkPdfOpiVersionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfOpiVersionDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapOpiVersionDictionary(value)) return false;
  *data = (SkPdfOpiVersionDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfOpiVersionDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfOpiVersionDictionary** data) const {
  if (SkPdfOpiVersionDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfOpiVersionDictionaryFromDictionary(dict, abr, data);
}

bool SkPdfMapper::isMultiMasterFontDictionary(const SkPdfNativeObject* nativeObj) const {
  const SkPdfNativeObject* ret = NULL;
  if (!nativeObj->isDictionary()) return false;
  ret = nativeObj->get("Subtype", "");
  if (ret == NULL || !ret->isName()) return false;
  if ((ret->nameValue2() != "MMType1")) return false;

  return true;
}

bool SkPdfMapper::SkPdfMultiMasterFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, SkPdfMultiMasterFontDictionary** data) const {
  const SkPdfNativeObject* value = dict->get(key);
  if (value == NULL) { return false; }
  if (data == NULL) { return true; }
  if (kNone_SkPdfNativeObjectType == mapMultiMasterFontDictionary(value)) return false;
  *data = (SkPdfMultiMasterFontDictionary*)value;
  return true;
}

bool SkPdfMapper::SkPdfMultiMasterFontDictionaryFromDictionary(const SkPdfNativeObject* dict, const char* key, const char* abr, SkPdfMultiMasterFontDictionary** data) const {
  if (SkPdfMultiMasterFontDictionaryFromDictionary(dict, key, data)) return true;
  if (abr == NULL || *abr == '\0') return false;
  return SkPdfMultiMasterFontDictionaryFromDictionary(dict, abr, data);
}

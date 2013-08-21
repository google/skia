/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfFDFDictionary_DEFINED
#define SkPdfFDFDictionary_DEFINED

#include "SkPdfDictionary_autogen.h"

// Entries in the FDF dictionary
class SkPdfFDFDictionary : public SkPdfDictionary {
public:
public:
   SkPdfFDFDictionary* asFDFDictionary() {return this;}
   const SkPdfFDFDictionary* asFDFDictionary() const {return this;}

private:
   SkPdfALinkAnnotationDictionary* asALinkAnnotationDictionary() {return (SkPdfALinkAnnotationDictionary*)this;}
   const SkPdfALinkAnnotationDictionary* asALinkAnnotationDictionary() const {return (const SkPdfALinkAnnotationDictionary*)this;}

   SkPdfActionDictionary* asActionDictionary() {return (SkPdfActionDictionary*)this;}
   const SkPdfActionDictionary* asActionDictionary() const {return (const SkPdfActionDictionary*)this;}

   SkPdfAlternateImageDictionary* asAlternateImageDictionary() {return (SkPdfAlternateImageDictionary*)this;}
   const SkPdfAlternateImageDictionary* asAlternateImageDictionary() const {return (const SkPdfAlternateImageDictionary*)this;}

   SkPdfAnnotationActionsDictionary* asAnnotationActionsDictionary() {return (SkPdfAnnotationActionsDictionary*)this;}
   const SkPdfAnnotationActionsDictionary* asAnnotationActionsDictionary() const {return (const SkPdfAnnotationActionsDictionary*)this;}

   SkPdfAnnotationDictionary* asAnnotationDictionary() {return (SkPdfAnnotationDictionary*)this;}
   const SkPdfAnnotationDictionary* asAnnotationDictionary() const {return (const SkPdfAnnotationDictionary*)this;}

   SkPdfAppearanceCharacteristicsDictionary* asAppearanceCharacteristicsDictionary() {return (SkPdfAppearanceCharacteristicsDictionary*)this;}
   const SkPdfAppearanceCharacteristicsDictionary* asAppearanceCharacteristicsDictionary() const {return (const SkPdfAppearanceCharacteristicsDictionary*)this;}

   SkPdfAppearanceDictionary* asAppearanceDictionary() {return (SkPdfAppearanceDictionary*)this;}
   const SkPdfAppearanceDictionary* asAppearanceDictionary() const {return (const SkPdfAppearanceDictionary*)this;}

   SkPdfApplicationDataDictionary* asApplicationDataDictionary() {return (SkPdfApplicationDataDictionary*)this;}
   const SkPdfApplicationDataDictionary* asApplicationDataDictionary() const {return (const SkPdfApplicationDataDictionary*)this;}

   SkPdfArtifactsDictionary* asArtifactsDictionary() {return (SkPdfArtifactsDictionary*)this;}
   const SkPdfArtifactsDictionary* asArtifactsDictionary() const {return (const SkPdfArtifactsDictionary*)this;}

   SkPdfAttributeObjectDictionary* asAttributeObjectDictionary() {return (SkPdfAttributeObjectDictionary*)this;}
   const SkPdfAttributeObjectDictionary* asAttributeObjectDictionary() const {return (const SkPdfAttributeObjectDictionary*)this;}

   SkPdfBeadDictionary* asBeadDictionary() {return (SkPdfBeadDictionary*)this;}
   const SkPdfBeadDictionary* asBeadDictionary() const {return (const SkPdfBeadDictionary*)this;}

   SkPdfBlockLevelStructureElementsDictionary* asBlockLevelStructureElementsDictionary() {return (SkPdfBlockLevelStructureElementsDictionary*)this;}
   const SkPdfBlockLevelStructureElementsDictionary* asBlockLevelStructureElementsDictionary() const {return (const SkPdfBlockLevelStructureElementsDictionary*)this;}

   SkPdfBorderStyleDictionary* asBorderStyleDictionary() {return (SkPdfBorderStyleDictionary*)this;}
   const SkPdfBorderStyleDictionary* asBorderStyleDictionary() const {return (const SkPdfBorderStyleDictionary*)this;}

   SkPdfBoxColorInformationDictionary* asBoxColorInformationDictionary() {return (SkPdfBoxColorInformationDictionary*)this;}
   const SkPdfBoxColorInformationDictionary* asBoxColorInformationDictionary() const {return (const SkPdfBoxColorInformationDictionary*)this;}

   SkPdfBoxStyleDictionary* asBoxStyleDictionary() {return (SkPdfBoxStyleDictionary*)this;}
   const SkPdfBoxStyleDictionary* asBoxStyleDictionary() const {return (const SkPdfBoxStyleDictionary*)this;}

   SkPdfCIDFontDescriptorDictionary* asCIDFontDescriptorDictionary() {return (SkPdfCIDFontDescriptorDictionary*)this;}
   const SkPdfCIDFontDescriptorDictionary* asCIDFontDescriptorDictionary() const {return (const SkPdfCIDFontDescriptorDictionary*)this;}

   SkPdfCIDFontDictionary* asCIDFontDictionary() {return (SkPdfCIDFontDictionary*)this;}
   const SkPdfCIDFontDictionary* asCIDFontDictionary() const {return (const SkPdfCIDFontDictionary*)this;}

   SkPdfCIDSystemInfoDictionary* asCIDSystemInfoDictionary() {return (SkPdfCIDSystemInfoDictionary*)this;}
   const SkPdfCIDSystemInfoDictionary* asCIDSystemInfoDictionary() const {return (const SkPdfCIDSystemInfoDictionary*)this;}

   SkPdfCMapDictionary* asCMapDictionary() {return (SkPdfCMapDictionary*)this;}
   const SkPdfCMapDictionary* asCMapDictionary() const {return (const SkPdfCMapDictionary*)this;}

   SkPdfCalgrayColorSpaceDictionary* asCalgrayColorSpaceDictionary() {return (SkPdfCalgrayColorSpaceDictionary*)this;}
   const SkPdfCalgrayColorSpaceDictionary* asCalgrayColorSpaceDictionary() const {return (const SkPdfCalgrayColorSpaceDictionary*)this;}

   SkPdfCalrgbColorSpaceDictionary* asCalrgbColorSpaceDictionary() {return (SkPdfCalrgbColorSpaceDictionary*)this;}
   const SkPdfCalrgbColorSpaceDictionary* asCalrgbColorSpaceDictionary() const {return (const SkPdfCalrgbColorSpaceDictionary*)this;}

   SkPdfCatalogDictionary* asCatalogDictionary() {return (SkPdfCatalogDictionary*)this;}
   const SkPdfCatalogDictionary* asCatalogDictionary() const {return (const SkPdfCatalogDictionary*)this;}

   SkPdfCcittfaxdecodeFilterDictionary* asCcittfaxdecodeFilterDictionary() {return (SkPdfCcittfaxdecodeFilterDictionary*)this;}
   const SkPdfCcittfaxdecodeFilterDictionary* asCcittfaxdecodeFilterDictionary() const {return (const SkPdfCcittfaxdecodeFilterDictionary*)this;}

   SkPdfCheckboxFieldDictionary* asCheckboxFieldDictionary() {return (SkPdfCheckboxFieldDictionary*)this;}
   const SkPdfCheckboxFieldDictionary* asCheckboxFieldDictionary() const {return (const SkPdfCheckboxFieldDictionary*)this;}

   SkPdfChoiceFieldDictionary* asChoiceFieldDictionary() {return (SkPdfChoiceFieldDictionary*)this;}
   const SkPdfChoiceFieldDictionary* asChoiceFieldDictionary() const {return (const SkPdfChoiceFieldDictionary*)this;}

   SkPdfComponentsWithMetadataDictionary* asComponentsWithMetadataDictionary() {return (SkPdfComponentsWithMetadataDictionary*)this;}
   const SkPdfComponentsWithMetadataDictionary* asComponentsWithMetadataDictionary() const {return (const SkPdfComponentsWithMetadataDictionary*)this;}

   SkPdfDctdecodeFilterDictionary* asDctdecodeFilterDictionary() {return (SkPdfDctdecodeFilterDictionary*)this;}
   const SkPdfDctdecodeFilterDictionary* asDctdecodeFilterDictionary() const {return (const SkPdfDctdecodeFilterDictionary*)this;}

   SkPdfDeviceNColorSpaceDictionary* asDeviceNColorSpaceDictionary() {return (SkPdfDeviceNColorSpaceDictionary*)this;}
   const SkPdfDeviceNColorSpaceDictionary* asDeviceNColorSpaceDictionary() const {return (const SkPdfDeviceNColorSpaceDictionary*)this;}

   SkPdfDocumentCatalogActionsDictionary* asDocumentCatalogActionsDictionary() {return (SkPdfDocumentCatalogActionsDictionary*)this;}
   const SkPdfDocumentCatalogActionsDictionary* asDocumentCatalogActionsDictionary() const {return (const SkPdfDocumentCatalogActionsDictionary*)this;}

   SkPdfDocumentInformationDictionary* asDocumentInformationDictionary() {return (SkPdfDocumentInformationDictionary*)this;}
   const SkPdfDocumentInformationDictionary* asDocumentInformationDictionary() const {return (const SkPdfDocumentInformationDictionary*)this;}

   SkPdfEmbeddedFileParameterDictionary* asEmbeddedFileParameterDictionary() {return (SkPdfEmbeddedFileParameterDictionary*)this;}
   const SkPdfEmbeddedFileParameterDictionary* asEmbeddedFileParameterDictionary() const {return (const SkPdfEmbeddedFileParameterDictionary*)this;}

   SkPdfEmbeddedFileStreamDictionary* asEmbeddedFileStreamDictionary() {return (SkPdfEmbeddedFileStreamDictionary*)this;}
   const SkPdfEmbeddedFileStreamDictionary* asEmbeddedFileStreamDictionary() const {return (const SkPdfEmbeddedFileStreamDictionary*)this;}

   SkPdfEmbeddedFontStreamDictionary* asEmbeddedFontStreamDictionary() {return (SkPdfEmbeddedFontStreamDictionary*)this;}
   const SkPdfEmbeddedFontStreamDictionary* asEmbeddedFontStreamDictionary() const {return (const SkPdfEmbeddedFontStreamDictionary*)this;}

   SkPdfEncodingDictionary* asEncodingDictionary() {return (SkPdfEncodingDictionary*)this;}
   const SkPdfEncodingDictionary* asEncodingDictionary() const {return (const SkPdfEncodingDictionary*)this;}

   SkPdfEncryptedEmbeddedFileStreamDictionary* asEncryptedEmbeddedFileStreamDictionary() {return (SkPdfEncryptedEmbeddedFileStreamDictionary*)this;}
   const SkPdfEncryptedEmbeddedFileStreamDictionary* asEncryptedEmbeddedFileStreamDictionary() const {return (const SkPdfEncryptedEmbeddedFileStreamDictionary*)this;}

   SkPdfEncryptionCommonDictionary* asEncryptionCommonDictionary() {return (SkPdfEncryptionCommonDictionary*)this;}
   const SkPdfEncryptionCommonDictionary* asEncryptionCommonDictionary() const {return (const SkPdfEncryptionCommonDictionary*)this;}

   SkPdfFDFCatalogDictionary* asFDFCatalogDictionary() {return (SkPdfFDFCatalogDictionary*)this;}
   const SkPdfFDFCatalogDictionary* asFDFCatalogDictionary() const {return (const SkPdfFDFCatalogDictionary*)this;}

   SkPdfFDFFieldDictionary* asFDFFieldDictionary() {return (SkPdfFDFFieldDictionary*)this;}
   const SkPdfFDFFieldDictionary* asFDFFieldDictionary() const {return (const SkPdfFDFFieldDictionary*)this;}

   SkPdfFDFFileAnnotationDictionary* asFDFFileAnnotationDictionary() {return (SkPdfFDFFileAnnotationDictionary*)this;}
   const SkPdfFDFFileAnnotationDictionary* asFDFFileAnnotationDictionary() const {return (const SkPdfFDFFileAnnotationDictionary*)this;}

   SkPdfFDFNamedPageReferenceDictionary* asFDFNamedPageReferenceDictionary() {return (SkPdfFDFNamedPageReferenceDictionary*)this;}
   const SkPdfFDFNamedPageReferenceDictionary* asFDFNamedPageReferenceDictionary() const {return (const SkPdfFDFNamedPageReferenceDictionary*)this;}

   SkPdfFDFPageDictionary* asFDFPageDictionary() {return (SkPdfFDFPageDictionary*)this;}
   const SkPdfFDFPageDictionary* asFDFPageDictionary() const {return (const SkPdfFDFPageDictionary*)this;}

   SkPdfFDFTemplateDictionary* asFDFTemplateDictionary() {return (SkPdfFDFTemplateDictionary*)this;}
   const SkPdfFDFTemplateDictionary* asFDFTemplateDictionary() const {return (const SkPdfFDFTemplateDictionary*)this;}

   SkPdfFDFTrailerDictionary* asFDFTrailerDictionary() {return (SkPdfFDFTrailerDictionary*)this;}
   const SkPdfFDFTrailerDictionary* asFDFTrailerDictionary() const {return (const SkPdfFDFTrailerDictionary*)this;}

   SkPdfFieldDictionary* asFieldDictionary() {return (SkPdfFieldDictionary*)this;}
   const SkPdfFieldDictionary* asFieldDictionary() const {return (const SkPdfFieldDictionary*)this;}

   SkPdfFileAttachmentAnnotationDictionary* asFileAttachmentAnnotationDictionary() {return (SkPdfFileAttachmentAnnotationDictionary*)this;}
   const SkPdfFileAttachmentAnnotationDictionary* asFileAttachmentAnnotationDictionary() const {return (const SkPdfFileAttachmentAnnotationDictionary*)this;}

   SkPdfFileSpecificationDictionary* asFileSpecificationDictionary() {return (SkPdfFileSpecificationDictionary*)this;}
   const SkPdfFileSpecificationDictionary* asFileSpecificationDictionary() const {return (const SkPdfFileSpecificationDictionary*)this;}

   SkPdfFileTrailerDictionary* asFileTrailerDictionary() {return (SkPdfFileTrailerDictionary*)this;}
   const SkPdfFileTrailerDictionary* asFileTrailerDictionary() const {return (const SkPdfFileTrailerDictionary*)this;}

   SkPdfFontDescriptorDictionary* asFontDescriptorDictionary() {return (SkPdfFontDescriptorDictionary*)this;}
   const SkPdfFontDescriptorDictionary* asFontDescriptorDictionary() const {return (const SkPdfFontDescriptorDictionary*)this;}

   SkPdfFontDictionary* asFontDictionary() {return (SkPdfFontDictionary*)this;}
   const SkPdfFontDictionary* asFontDictionary() const {return (const SkPdfFontDictionary*)this;}

   SkPdfType0FontDictionary* asType0FontDictionary() {return (SkPdfType0FontDictionary*)this;}
   const SkPdfType0FontDictionary* asType0FontDictionary() const {return (const SkPdfType0FontDictionary*)this;}

   SkPdfType1FontDictionary* asType1FontDictionary() {return (SkPdfType1FontDictionary*)this;}
   const SkPdfType1FontDictionary* asType1FontDictionary() const {return (const SkPdfType1FontDictionary*)this;}

   SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return (SkPdfMultiMasterFontDictionary*)this;}
   const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return (const SkPdfMultiMasterFontDictionary*)this;}

   SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return (SkPdfTrueTypeFontDictionary*)this;}
   const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return (const SkPdfTrueTypeFontDictionary*)this;}

   SkPdfType3FontDictionary* asType3FontDictionary() {return (SkPdfType3FontDictionary*)this;}
   const SkPdfType3FontDictionary* asType3FontDictionary() const {return (const SkPdfType3FontDictionary*)this;}

   SkPdfFormFieldActionsDictionary* asFormFieldActionsDictionary() {return (SkPdfFormFieldActionsDictionary*)this;}
   const SkPdfFormFieldActionsDictionary* asFormFieldActionsDictionary() const {return (const SkPdfFormFieldActionsDictionary*)this;}

   SkPdfFreeTextAnnotationDictionary* asFreeTextAnnotationDictionary() {return (SkPdfFreeTextAnnotationDictionary*)this;}
   const SkPdfFreeTextAnnotationDictionary* asFreeTextAnnotationDictionary() const {return (const SkPdfFreeTextAnnotationDictionary*)this;}

   SkPdfFunctionCommonDictionary* asFunctionCommonDictionary() {return (SkPdfFunctionCommonDictionary*)this;}
   const SkPdfFunctionCommonDictionary* asFunctionCommonDictionary() const {return (const SkPdfFunctionCommonDictionary*)this;}

   SkPdfGoToActionDictionary* asGoToActionDictionary() {return (SkPdfGoToActionDictionary*)this;}
   const SkPdfGoToActionDictionary* asGoToActionDictionary() const {return (const SkPdfGoToActionDictionary*)this;}

   SkPdfGraphicsStateDictionary* asGraphicsStateDictionary() {return (SkPdfGraphicsStateDictionary*)this;}
   const SkPdfGraphicsStateDictionary* asGraphicsStateDictionary() const {return (const SkPdfGraphicsStateDictionary*)this;}

   SkPdfGroupAttributesDictionary* asGroupAttributesDictionary() {return (SkPdfGroupAttributesDictionary*)this;}
   const SkPdfGroupAttributesDictionary* asGroupAttributesDictionary() const {return (const SkPdfGroupAttributesDictionary*)this;}

   SkPdfHideActionDictionary* asHideActionDictionary() {return (SkPdfHideActionDictionary*)this;}
   const SkPdfHideActionDictionary* asHideActionDictionary() const {return (const SkPdfHideActionDictionary*)this;}

   SkPdfIccProfileStreamDictionary* asIccProfileStreamDictionary() {return (SkPdfIccProfileStreamDictionary*)this;}
   const SkPdfIccProfileStreamDictionary* asIccProfileStreamDictionary() const {return (const SkPdfIccProfileStreamDictionary*)this;}

   SkPdfIconFitDictionary* asIconFitDictionary() {return (SkPdfIconFitDictionary*)this;}
   const SkPdfIconFitDictionary* asIconFitDictionary() const {return (const SkPdfIconFitDictionary*)this;}

   SkPdfImportDataActionDictionary* asImportDataActionDictionary() {return (SkPdfImportDataActionDictionary*)this;}
   const SkPdfImportDataActionDictionary* asImportDataActionDictionary() const {return (const SkPdfImportDataActionDictionary*)this;}

   SkPdfInkAnnotationDictionary* asInkAnnotationDictionary() {return (SkPdfInkAnnotationDictionary*)this;}
   const SkPdfInkAnnotationDictionary* asInkAnnotationDictionary() const {return (const SkPdfInkAnnotationDictionary*)this;}

   SkPdfInlineLevelStructureElementsDictionary* asInlineLevelStructureElementsDictionary() {return (SkPdfInlineLevelStructureElementsDictionary*)this;}
   const SkPdfInlineLevelStructureElementsDictionary* asInlineLevelStructureElementsDictionary() const {return (const SkPdfInlineLevelStructureElementsDictionary*)this;}

   SkPdfInteractiveFormDictionary* asInteractiveFormDictionary() {return (SkPdfInteractiveFormDictionary*)this;}
   const SkPdfInteractiveFormDictionary* asInteractiveFormDictionary() const {return (const SkPdfInteractiveFormDictionary*)this;}

   SkPdfJavascriptActionDictionary* asJavascriptActionDictionary() {return (SkPdfJavascriptActionDictionary*)this;}
   const SkPdfJavascriptActionDictionary* asJavascriptActionDictionary() const {return (const SkPdfJavascriptActionDictionary*)this;}

   SkPdfJavascriptDictionary* asJavascriptDictionary() {return (SkPdfJavascriptDictionary*)this;}
   const SkPdfJavascriptDictionary* asJavascriptDictionary() const {return (const SkPdfJavascriptDictionary*)this;}

   SkPdfJbig2DecodeFilterDictionary* asJbig2DecodeFilterDictionary() {return (SkPdfJbig2DecodeFilterDictionary*)this;}
   const SkPdfJbig2DecodeFilterDictionary* asJbig2DecodeFilterDictionary() const {return (const SkPdfJbig2DecodeFilterDictionary*)this;}

   SkPdfLabColorSpaceDictionary* asLabColorSpaceDictionary() {return (SkPdfLabColorSpaceDictionary*)this;}
   const SkPdfLabColorSpaceDictionary* asLabColorSpaceDictionary() const {return (const SkPdfLabColorSpaceDictionary*)this;}

   SkPdfLaunchActionDictionary* asLaunchActionDictionary() {return (SkPdfLaunchActionDictionary*)this;}
   const SkPdfLaunchActionDictionary* asLaunchActionDictionary() const {return (const SkPdfLaunchActionDictionary*)this;}

   SkPdfLineAnnotationDictionary* asLineAnnotationDictionary() {return (SkPdfLineAnnotationDictionary*)this;}
   const SkPdfLineAnnotationDictionary* asLineAnnotationDictionary() const {return (const SkPdfLineAnnotationDictionary*)this;}

   SkPdfListAttributeDictionary* asListAttributeDictionary() {return (SkPdfListAttributeDictionary*)this;}
   const SkPdfListAttributeDictionary* asListAttributeDictionary() const {return (const SkPdfListAttributeDictionary*)this;}

   SkPdfLzwdecodeAndFlatedecodeFiltersDictionary* asLzwdecodeAndFlatedecodeFiltersDictionary() {return (SkPdfLzwdecodeAndFlatedecodeFiltersDictionary*)this;}
   const SkPdfLzwdecodeAndFlatedecodeFiltersDictionary* asLzwdecodeAndFlatedecodeFiltersDictionary() const {return (const SkPdfLzwdecodeAndFlatedecodeFiltersDictionary*)this;}

   SkPdfMacOsFileInformationDictionary* asMacOsFileInformationDictionary() {return (SkPdfMacOsFileInformationDictionary*)this;}
   const SkPdfMacOsFileInformationDictionary* asMacOsFileInformationDictionary() const {return (const SkPdfMacOsFileInformationDictionary*)this;}

   SkPdfMarkInformationDictionary* asMarkInformationDictionary() {return (SkPdfMarkInformationDictionary*)this;}
   const SkPdfMarkInformationDictionary* asMarkInformationDictionary() const {return (const SkPdfMarkInformationDictionary*)this;}

   SkPdfMarkedContentReferenceDictionary* asMarkedContentReferenceDictionary() {return (SkPdfMarkedContentReferenceDictionary*)this;}
   const SkPdfMarkedContentReferenceDictionary* asMarkedContentReferenceDictionary() const {return (const SkPdfMarkedContentReferenceDictionary*)this;}

   SkPdfMarkupAnnotationsDictionary* asMarkupAnnotationsDictionary() {return (SkPdfMarkupAnnotationsDictionary*)this;}
   const SkPdfMarkupAnnotationsDictionary* asMarkupAnnotationsDictionary() const {return (const SkPdfMarkupAnnotationsDictionary*)this;}

   SkPdfMetadataStreamDictionary* asMetadataStreamDictionary() {return (SkPdfMetadataStreamDictionary*)this;}
   const SkPdfMetadataStreamDictionary* asMetadataStreamDictionary() const {return (const SkPdfMetadataStreamDictionary*)this;}

   SkPdfMovieActionDictionary* asMovieActionDictionary() {return (SkPdfMovieActionDictionary*)this;}
   const SkPdfMovieActionDictionary* asMovieActionDictionary() const {return (const SkPdfMovieActionDictionary*)this;}

   SkPdfMovieActivationDictionary* asMovieActivationDictionary() {return (SkPdfMovieActivationDictionary*)this;}
   const SkPdfMovieActivationDictionary* asMovieActivationDictionary() const {return (const SkPdfMovieActivationDictionary*)this;}

   SkPdfMovieAnnotationDictionary* asMovieAnnotationDictionary() {return (SkPdfMovieAnnotationDictionary*)this;}
   const SkPdfMovieAnnotationDictionary* asMovieAnnotationDictionary() const {return (const SkPdfMovieAnnotationDictionary*)this;}

   SkPdfMovieDictionary* asMovieDictionary() {return (SkPdfMovieDictionary*)this;}
   const SkPdfMovieDictionary* asMovieDictionary() const {return (const SkPdfMovieDictionary*)this;}

   SkPdfNameDictionary* asNameDictionary() {return (SkPdfNameDictionary*)this;}
   const SkPdfNameDictionary* asNameDictionary() const {return (const SkPdfNameDictionary*)this;}

   SkPdfNameTreeNodeDictionary* asNameTreeNodeDictionary() {return (SkPdfNameTreeNodeDictionary*)this;}
   const SkPdfNameTreeNodeDictionary* asNameTreeNodeDictionary() const {return (const SkPdfNameTreeNodeDictionary*)this;}

   SkPdfNamedActionsDictionary* asNamedActionsDictionary() {return (SkPdfNamedActionsDictionary*)this;}
   const SkPdfNamedActionsDictionary* asNamedActionsDictionary() const {return (const SkPdfNamedActionsDictionary*)this;}

   SkPdfNumberTreeNodeDictionary* asNumberTreeNodeDictionary() {return (SkPdfNumberTreeNodeDictionary*)this;}
   const SkPdfNumberTreeNodeDictionary* asNumberTreeNodeDictionary() const {return (const SkPdfNumberTreeNodeDictionary*)this;}

   SkPdfObjectReferenceDictionary* asObjectReferenceDictionary() {return (SkPdfObjectReferenceDictionary*)this;}
   const SkPdfObjectReferenceDictionary* asObjectReferenceDictionary() const {return (const SkPdfObjectReferenceDictionary*)this;}

   SkPdfOpiVersionDictionary* asOpiVersionDictionary() {return (SkPdfOpiVersionDictionary*)this;}
   const SkPdfOpiVersionDictionary* asOpiVersionDictionary() const {return (const SkPdfOpiVersionDictionary*)this;}

   SkPdfOutlineDictionary* asOutlineDictionary() {return (SkPdfOutlineDictionary*)this;}
   const SkPdfOutlineDictionary* asOutlineDictionary() const {return (const SkPdfOutlineDictionary*)this;}

   SkPdfOutlineItemDictionary* asOutlineItemDictionary() {return (SkPdfOutlineItemDictionary*)this;}
   const SkPdfOutlineItemDictionary* asOutlineItemDictionary() const {return (const SkPdfOutlineItemDictionary*)this;}

   SkPdfPDF_XOutputIntentDictionary* asPDF_XOutputIntentDictionary() {return (SkPdfPDF_XOutputIntentDictionary*)this;}
   const SkPdfPDF_XOutputIntentDictionary* asPDF_XOutputIntentDictionary() const {return (const SkPdfPDF_XOutputIntentDictionary*)this;}

   SkPdfPSXobjectDictionary* asPSXobjectDictionary() {return (SkPdfPSXobjectDictionary*)this;}
   const SkPdfPSXobjectDictionary* asPSXobjectDictionary() const {return (const SkPdfPSXobjectDictionary*)this;}

   SkPdfPageLabelDictionary* asPageLabelDictionary() {return (SkPdfPageLabelDictionary*)this;}
   const SkPdfPageLabelDictionary* asPageLabelDictionary() const {return (const SkPdfPageLabelDictionary*)this;}

   SkPdfPageObjectActionsDictionary* asPageObjectActionsDictionary() {return (SkPdfPageObjectActionsDictionary*)this;}
   const SkPdfPageObjectActionsDictionary* asPageObjectActionsDictionary() const {return (const SkPdfPageObjectActionsDictionary*)this;}

   SkPdfPageObjectDictionary* asPageObjectDictionary() {return (SkPdfPageObjectDictionary*)this;}
   const SkPdfPageObjectDictionary* asPageObjectDictionary() const {return (const SkPdfPageObjectDictionary*)this;}

   SkPdfPagePieceDictionary* asPagePieceDictionary() {return (SkPdfPagePieceDictionary*)this;}
   const SkPdfPagePieceDictionary* asPagePieceDictionary() const {return (const SkPdfPagePieceDictionary*)this;}

   SkPdfPageTreeNodeDictionary* asPageTreeNodeDictionary() {return (SkPdfPageTreeNodeDictionary*)this;}
   const SkPdfPageTreeNodeDictionary* asPageTreeNodeDictionary() const {return (const SkPdfPageTreeNodeDictionary*)this;}

   SkPdfPopUpAnnotationDictionary* asPopUpAnnotationDictionary() {return (SkPdfPopUpAnnotationDictionary*)this;}
   const SkPdfPopUpAnnotationDictionary* asPopUpAnnotationDictionary() const {return (const SkPdfPopUpAnnotationDictionary*)this;}

   SkPdfPrinterMarkAnnotationDictionary* asPrinterMarkAnnotationDictionary() {return (SkPdfPrinterMarkAnnotationDictionary*)this;}
   const SkPdfPrinterMarkAnnotationDictionary* asPrinterMarkAnnotationDictionary() const {return (const SkPdfPrinterMarkAnnotationDictionary*)this;}

   SkPdfPrinterMarkFormDictionary* asPrinterMarkFormDictionary() {return (SkPdfPrinterMarkFormDictionary*)this;}
   const SkPdfPrinterMarkFormDictionary* asPrinterMarkFormDictionary() const {return (const SkPdfPrinterMarkFormDictionary*)this;}

   SkPdfRadioButtonFieldDictionary* asRadioButtonFieldDictionary() {return (SkPdfRadioButtonFieldDictionary*)this;}
   const SkPdfRadioButtonFieldDictionary* asRadioButtonFieldDictionary() const {return (const SkPdfRadioButtonFieldDictionary*)this;}

   SkPdfReferenceDictionary* asReferenceDictionary() {return (SkPdfReferenceDictionary*)this;}
   const SkPdfReferenceDictionary* asReferenceDictionary() const {return (const SkPdfReferenceDictionary*)this;}

   SkPdfRemoteGoToActionDictionary* asRemoteGoToActionDictionary() {return (SkPdfRemoteGoToActionDictionary*)this;}
   const SkPdfRemoteGoToActionDictionary* asRemoteGoToActionDictionary() const {return (const SkPdfRemoteGoToActionDictionary*)this;}

   SkPdfResetFormActionDictionary* asResetFormActionDictionary() {return (SkPdfResetFormActionDictionary*)this;}
   const SkPdfResetFormActionDictionary* asResetFormActionDictionary() const {return (const SkPdfResetFormActionDictionary*)this;}

   SkPdfResourceDictionary* asResourceDictionary() {return (SkPdfResourceDictionary*)this;}
   const SkPdfResourceDictionary* asResourceDictionary() const {return (const SkPdfResourceDictionary*)this;}

   SkPdfRubberStampAnnotationDictionary* asRubberStampAnnotationDictionary() {return (SkPdfRubberStampAnnotationDictionary*)this;}
   const SkPdfRubberStampAnnotationDictionary* asRubberStampAnnotationDictionary() const {return (const SkPdfRubberStampAnnotationDictionary*)this;}

   SkPdfSeparationDictionary* asSeparationDictionary() {return (SkPdfSeparationDictionary*)this;}
   const SkPdfSeparationDictionary* asSeparationDictionary() const {return (const SkPdfSeparationDictionary*)this;}

   SkPdfShadingDictionary* asShadingDictionary() {return (SkPdfShadingDictionary*)this;}
   const SkPdfShadingDictionary* asShadingDictionary() const {return (const SkPdfShadingDictionary*)this;}

   SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return (SkPdfType1ShadingDictionary*)this;}
   const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return (const SkPdfType1ShadingDictionary*)this;}

   SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return (SkPdfType2ShadingDictionary*)this;}
   const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return (const SkPdfType2ShadingDictionary*)this;}

   SkPdfType3ShadingDictionary* asType3ShadingDictionary() {return (SkPdfType3ShadingDictionary*)this;}
   const SkPdfType3ShadingDictionary* asType3ShadingDictionary() const {return (const SkPdfType3ShadingDictionary*)this;}

   SkPdfType4ShadingDictionary* asType4ShadingDictionary() {return (SkPdfType4ShadingDictionary*)this;}
   const SkPdfType4ShadingDictionary* asType4ShadingDictionary() const {return (const SkPdfType4ShadingDictionary*)this;}

   SkPdfType5ShadingDictionary* asType5ShadingDictionary() {return (SkPdfType5ShadingDictionary*)this;}
   const SkPdfType5ShadingDictionary* asType5ShadingDictionary() const {return (const SkPdfType5ShadingDictionary*)this;}

   SkPdfType6ShadingDictionary* asType6ShadingDictionary() {return (SkPdfType6ShadingDictionary*)this;}
   const SkPdfType6ShadingDictionary* asType6ShadingDictionary() const {return (const SkPdfType6ShadingDictionary*)this;}

   SkPdfSignatureDictionary* asSignatureDictionary() {return (SkPdfSignatureDictionary*)this;}
   const SkPdfSignatureDictionary* asSignatureDictionary() const {return (const SkPdfSignatureDictionary*)this;}

   SkPdfSoftMaskDictionary* asSoftMaskDictionary() {return (SkPdfSoftMaskDictionary*)this;}
   const SkPdfSoftMaskDictionary* asSoftMaskDictionary() const {return (const SkPdfSoftMaskDictionary*)this;}

   SkPdfSoundActionDictionary* asSoundActionDictionary() {return (SkPdfSoundActionDictionary*)this;}
   const SkPdfSoundActionDictionary* asSoundActionDictionary() const {return (const SkPdfSoundActionDictionary*)this;}

   SkPdfSoundAnnotationDictionary* asSoundAnnotationDictionary() {return (SkPdfSoundAnnotationDictionary*)this;}
   const SkPdfSoundAnnotationDictionary* asSoundAnnotationDictionary() const {return (const SkPdfSoundAnnotationDictionary*)this;}

   SkPdfSoundObjectDictionary* asSoundObjectDictionary() {return (SkPdfSoundObjectDictionary*)this;}
   const SkPdfSoundObjectDictionary* asSoundObjectDictionary() const {return (const SkPdfSoundObjectDictionary*)this;}

   SkPdfSourceInformationDictionary* asSourceInformationDictionary() {return (SkPdfSourceInformationDictionary*)this;}
   const SkPdfSourceInformationDictionary* asSourceInformationDictionary() const {return (const SkPdfSourceInformationDictionary*)this;}

   SkPdfSquareOrCircleAnnotation* asSquareOrCircleAnnotation() {return (SkPdfSquareOrCircleAnnotation*)this;}
   const SkPdfSquareOrCircleAnnotation* asSquareOrCircleAnnotation() const {return (const SkPdfSquareOrCircleAnnotation*)this;}

   SkPdfStandardSecurityHandlerDictionary* asStandardSecurityHandlerDictionary() {return (SkPdfStandardSecurityHandlerDictionary*)this;}
   const SkPdfStandardSecurityHandlerDictionary* asStandardSecurityHandlerDictionary() const {return (const SkPdfStandardSecurityHandlerDictionary*)this;}

   SkPdfStandardStructureDictionary* asStandardStructureDictionary() {return (SkPdfStandardStructureDictionary*)this;}
   const SkPdfStandardStructureDictionary* asStandardStructureDictionary() const {return (const SkPdfStandardStructureDictionary*)this;}

   SkPdfStreamCommonDictionary* asStreamCommonDictionary() {return (SkPdfStreamCommonDictionary*)this;}
   const SkPdfStreamCommonDictionary* asStreamCommonDictionary() const {return (const SkPdfStreamCommonDictionary*)this;}

   SkPdfStructureElementAccessDictionary* asStructureElementAccessDictionary() {return (SkPdfStructureElementAccessDictionary*)this;}
   const SkPdfStructureElementAccessDictionary* asStructureElementAccessDictionary() const {return (const SkPdfStructureElementAccessDictionary*)this;}

   SkPdfStructureElementDictionary* asStructureElementDictionary() {return (SkPdfStructureElementDictionary*)this;}
   const SkPdfStructureElementDictionary* asStructureElementDictionary() const {return (const SkPdfStructureElementDictionary*)this;}

   SkPdfStructureTreeRootDictionary* asStructureTreeRootDictionary() {return (SkPdfStructureTreeRootDictionary*)this;}
   const SkPdfStructureTreeRootDictionary* asStructureTreeRootDictionary() const {return (const SkPdfStructureTreeRootDictionary*)this;}

   SkPdfSubmitFormActionDictionary* asSubmitFormActionDictionary() {return (SkPdfSubmitFormActionDictionary*)this;}
   const SkPdfSubmitFormActionDictionary* asSubmitFormActionDictionary() const {return (const SkPdfSubmitFormActionDictionary*)this;}

   SkPdfTableAttributesDictionary* asTableAttributesDictionary() {return (SkPdfTableAttributesDictionary*)this;}
   const SkPdfTableAttributesDictionary* asTableAttributesDictionary() const {return (const SkPdfTableAttributesDictionary*)this;}

   SkPdfTextAnnotationDictionary* asTextAnnotationDictionary() {return (SkPdfTextAnnotationDictionary*)this;}
   const SkPdfTextAnnotationDictionary* asTextAnnotationDictionary() const {return (const SkPdfTextAnnotationDictionary*)this;}

   SkPdfTextFieldDictionary* asTextFieldDictionary() {return (SkPdfTextFieldDictionary*)this;}
   const SkPdfTextFieldDictionary* asTextFieldDictionary() const {return (const SkPdfTextFieldDictionary*)this;}

   SkPdfThreadActionDictionary* asThreadActionDictionary() {return (SkPdfThreadActionDictionary*)this;}
   const SkPdfThreadActionDictionary* asThreadActionDictionary() const {return (const SkPdfThreadActionDictionary*)this;}

   SkPdfThreadDictionary* asThreadDictionary() {return (SkPdfThreadDictionary*)this;}
   const SkPdfThreadDictionary* asThreadDictionary() const {return (const SkPdfThreadDictionary*)this;}

   SkPdfTransitionDictionary* asTransitionDictionary() {return (SkPdfTransitionDictionary*)this;}
   const SkPdfTransitionDictionary* asTransitionDictionary() const {return (const SkPdfTransitionDictionary*)this;}

   SkPdfTransparencyGroupDictionary* asTransparencyGroupDictionary() {return (SkPdfTransparencyGroupDictionary*)this;}
   const SkPdfTransparencyGroupDictionary* asTransparencyGroupDictionary() const {return (const SkPdfTransparencyGroupDictionary*)this;}

   SkPdfTrapNetworkAnnotationDictionary* asTrapNetworkAnnotationDictionary() {return (SkPdfTrapNetworkAnnotationDictionary*)this;}
   const SkPdfTrapNetworkAnnotationDictionary* asTrapNetworkAnnotationDictionary() const {return (const SkPdfTrapNetworkAnnotationDictionary*)this;}

   SkPdfTrapNetworkAppearanceStreamDictionary* asTrapNetworkAppearanceStreamDictionary() {return (SkPdfTrapNetworkAppearanceStreamDictionary*)this;}
   const SkPdfTrapNetworkAppearanceStreamDictionary* asTrapNetworkAppearanceStreamDictionary() const {return (const SkPdfTrapNetworkAppearanceStreamDictionary*)this;}

   SkPdfType0FunctionDictionary* asType0FunctionDictionary() {return (SkPdfType0FunctionDictionary*)this;}
   const SkPdfType0FunctionDictionary* asType0FunctionDictionary() const {return (const SkPdfType0FunctionDictionary*)this;}

   SkPdfType10HalftoneDictionary* asType10HalftoneDictionary() {return (SkPdfType10HalftoneDictionary*)this;}
   const SkPdfType10HalftoneDictionary* asType10HalftoneDictionary() const {return (const SkPdfType10HalftoneDictionary*)this;}

   SkPdfType16HalftoneDictionary* asType16HalftoneDictionary() {return (SkPdfType16HalftoneDictionary*)this;}
   const SkPdfType16HalftoneDictionary* asType16HalftoneDictionary() const {return (const SkPdfType16HalftoneDictionary*)this;}

   SkPdfType1HalftoneDictionary* asType1HalftoneDictionary() {return (SkPdfType1HalftoneDictionary*)this;}
   const SkPdfType1HalftoneDictionary* asType1HalftoneDictionary() const {return (const SkPdfType1HalftoneDictionary*)this;}

   SkPdfType1PatternDictionary* asType1PatternDictionary() {return (SkPdfType1PatternDictionary*)this;}
   const SkPdfType1PatternDictionary* asType1PatternDictionary() const {return (const SkPdfType1PatternDictionary*)this;}

   SkPdfType2FunctionDictionary* asType2FunctionDictionary() {return (SkPdfType2FunctionDictionary*)this;}
   const SkPdfType2FunctionDictionary* asType2FunctionDictionary() const {return (const SkPdfType2FunctionDictionary*)this;}

   SkPdfType2PatternDictionary* asType2PatternDictionary() {return (SkPdfType2PatternDictionary*)this;}
   const SkPdfType2PatternDictionary* asType2PatternDictionary() const {return (const SkPdfType2PatternDictionary*)this;}

   SkPdfType3FunctionDictionary* asType3FunctionDictionary() {return (SkPdfType3FunctionDictionary*)this;}
   const SkPdfType3FunctionDictionary* asType3FunctionDictionary() const {return (const SkPdfType3FunctionDictionary*)this;}

   SkPdfType5HalftoneDictionary* asType5HalftoneDictionary() {return (SkPdfType5HalftoneDictionary*)this;}
   const SkPdfType5HalftoneDictionary* asType5HalftoneDictionary() const {return (const SkPdfType5HalftoneDictionary*)this;}

   SkPdfType6HalftoneDictionary* asType6HalftoneDictionary() {return (SkPdfType6HalftoneDictionary*)this;}
   const SkPdfType6HalftoneDictionary* asType6HalftoneDictionary() const {return (const SkPdfType6HalftoneDictionary*)this;}

   SkPdfURIActionDictionary* asURIActionDictionary() {return (SkPdfURIActionDictionary*)this;}
   const SkPdfURIActionDictionary* asURIActionDictionary() const {return (const SkPdfURIActionDictionary*)this;}

   SkPdfURIDictionary* asURIDictionary() {return (SkPdfURIDictionary*)this;}
   const SkPdfURIDictionary* asURIDictionary() const {return (const SkPdfURIDictionary*)this;}

   SkPdfURLAliasDictionary* asURLAliasDictionary() {return (SkPdfURLAliasDictionary*)this;}
   const SkPdfURLAliasDictionary* asURLAliasDictionary() const {return (const SkPdfURLAliasDictionary*)this;}

   SkPdfVariableTextFieldDictionary* asVariableTextFieldDictionary() {return (SkPdfVariableTextFieldDictionary*)this;}
   const SkPdfVariableTextFieldDictionary* asVariableTextFieldDictionary() const {return (const SkPdfVariableTextFieldDictionary*)this;}

   SkPdfViewerPreferencesDictionary* asViewerPreferencesDictionary() {return (SkPdfViewerPreferencesDictionary*)this;}
   const SkPdfViewerPreferencesDictionary* asViewerPreferencesDictionary() const {return (const SkPdfViewerPreferencesDictionary*)this;}

   SkPdfWebCaptureCommandDictionary* asWebCaptureCommandDictionary() {return (SkPdfWebCaptureCommandDictionary*)this;}
   const SkPdfWebCaptureCommandDictionary* asWebCaptureCommandDictionary() const {return (const SkPdfWebCaptureCommandDictionary*)this;}

   SkPdfWebCaptureCommandSettingsDictionary* asWebCaptureCommandSettingsDictionary() {return (SkPdfWebCaptureCommandSettingsDictionary*)this;}
   const SkPdfWebCaptureCommandSettingsDictionary* asWebCaptureCommandSettingsDictionary() const {return (const SkPdfWebCaptureCommandSettingsDictionary*)this;}

   SkPdfWebCaptureDictionary* asWebCaptureDictionary() {return (SkPdfWebCaptureDictionary*)this;}
   const SkPdfWebCaptureDictionary* asWebCaptureDictionary() const {return (const SkPdfWebCaptureDictionary*)this;}

   SkPdfWebCaptureImageSetDictionary* asWebCaptureImageSetDictionary() {return (SkPdfWebCaptureImageSetDictionary*)this;}
   const SkPdfWebCaptureImageSetDictionary* asWebCaptureImageSetDictionary() const {return (const SkPdfWebCaptureImageSetDictionary*)this;}

   SkPdfWebCaptureInformationDictionary* asWebCaptureInformationDictionary() {return (SkPdfWebCaptureInformationDictionary*)this;}
   const SkPdfWebCaptureInformationDictionary* asWebCaptureInformationDictionary() const {return (const SkPdfWebCaptureInformationDictionary*)this;}

   SkPdfWebCapturePageSetDictionary* asWebCapturePageSetDictionary() {return (SkPdfWebCapturePageSetDictionary*)this;}
   const SkPdfWebCapturePageSetDictionary* asWebCapturePageSetDictionary() const {return (const SkPdfWebCapturePageSetDictionary*)this;}

   SkPdfWidgetAnnotationDictionary* asWidgetAnnotationDictionary() {return (SkPdfWidgetAnnotationDictionary*)this;}
   const SkPdfWidgetAnnotationDictionary* asWidgetAnnotationDictionary() const {return (const SkPdfWidgetAnnotationDictionary*)this;}

   SkPdfWindowsLaunchActionDictionary* asWindowsLaunchActionDictionary() {return (SkPdfWindowsLaunchActionDictionary*)this;}
   const SkPdfWindowsLaunchActionDictionary* asWindowsLaunchActionDictionary() const {return (const SkPdfWindowsLaunchActionDictionary*)this;}

   SkPdfXObjectDictionary* asXObjectDictionary() {return (SkPdfXObjectDictionary*)this;}
   const SkPdfXObjectDictionary* asXObjectDictionary() const {return (const SkPdfXObjectDictionary*)this;}

   SkPdfImageDictionary* asImageDictionary() {return (SkPdfImageDictionary*)this;}
   const SkPdfImageDictionary* asImageDictionary() const {return (const SkPdfImageDictionary*)this;}

   SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() {return (SkPdfSoftMaskImageDictionary*)this;}
   const SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() const {return (const SkPdfSoftMaskImageDictionary*)this;}

   SkPdfType1FormDictionary* asType1FormDictionary() {return (SkPdfType1FormDictionary*)this;}
   const SkPdfType1FormDictionary* asType1FormDictionary() const {return (const SkPdfType1FormDictionary*)this;}

public:
   bool valid() const {return true;}
  SkPdfFileSpec F(SkPdfNativeDoc* doc);
  bool has_F() const;
  SkPdfArray* ID(SkPdfNativeDoc* doc);
  bool has_ID() const;
  SkPdfArray* Fields(SkPdfNativeDoc* doc);
  bool has_Fields() const;
  SkString Status(SkPdfNativeDoc* doc);
  bool has_Status() const;
  SkPdfArray* Pages(SkPdfNativeDoc* doc);
  bool has_Pages() const;
  SkString Encoding(SkPdfNativeDoc* doc);
  bool has_Encoding() const;
  SkPdfArray* Annots(SkPdfNativeDoc* doc);
  bool has_Annots() const;
  SkPdfStream* Differences(SkPdfNativeDoc* doc);
  bool has_Differences() const;
  SkString Target(SkPdfNativeDoc* doc);
  bool has_Target() const;
  SkPdfArray* EmbeddedFDFs(SkPdfNativeDoc* doc);
  bool has_EmbeddedFDFs() const;
  SkPdfDictionary* JavaScript(SkPdfNativeDoc* doc);
  bool has_JavaScript() const;
};

#endif  // SkPdfFDFDictionary_DEFINED

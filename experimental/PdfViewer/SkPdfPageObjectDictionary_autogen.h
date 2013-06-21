#ifndef __DEFINED__SkPdfPageObjectDictionary
#define __DEFINED__SkPdfPageObjectDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfDictionary_autogen.h"

// Entries in a page object
class SkPdfPageObjectDictionary : public SkPdfDictionary {
public:
  virtual SkPdfObjectType getType() const { return kPageObjectDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kPageObjectDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfPageObjectDictionary* asPageObjectDictionary() {return this;}
  virtual const SkPdfPageObjectDictionary* asPageObjectDictionary() const {return this;}

private:
  virtual SkPdfALinkAnnotationDictionary* asALinkAnnotationDictionary() {return NULL;}
  virtual const SkPdfALinkAnnotationDictionary* asALinkAnnotationDictionary() const {return NULL;}

  virtual SkPdfActionDictionary* asActionDictionary() {return NULL;}
  virtual const SkPdfActionDictionary* asActionDictionary() const {return NULL;}

  virtual SkPdfAlternateImageDictionary* asAlternateImageDictionary() {return NULL;}
  virtual const SkPdfAlternateImageDictionary* asAlternateImageDictionary() const {return NULL;}

  virtual SkPdfAnnotationActionsDictionary* asAnnotationActionsDictionary() {return NULL;}
  virtual const SkPdfAnnotationActionsDictionary* asAnnotationActionsDictionary() const {return NULL;}

  virtual SkPdfAnnotationDictionary* asAnnotationDictionary() {return NULL;}
  virtual const SkPdfAnnotationDictionary* asAnnotationDictionary() const {return NULL;}

  virtual SkPdfAppearanceCharacteristicsDictionary* asAppearanceCharacteristicsDictionary() {return NULL;}
  virtual const SkPdfAppearanceCharacteristicsDictionary* asAppearanceCharacteristicsDictionary() const {return NULL;}

  virtual SkPdfAppearanceDictionary* asAppearanceDictionary() {return NULL;}
  virtual const SkPdfAppearanceDictionary* asAppearanceDictionary() const {return NULL;}

  virtual SkPdfApplicationDataDictionary* asApplicationDataDictionary() {return NULL;}
  virtual const SkPdfApplicationDataDictionary* asApplicationDataDictionary() const {return NULL;}

  virtual SkPdfArtifactsDictionary* asArtifactsDictionary() {return NULL;}
  virtual const SkPdfArtifactsDictionary* asArtifactsDictionary() const {return NULL;}

  virtual SkPdfAttributeObjectDictionary* asAttributeObjectDictionary() {return NULL;}
  virtual const SkPdfAttributeObjectDictionary* asAttributeObjectDictionary() const {return NULL;}

  virtual SkPdfBeadDictionary* asBeadDictionary() {return NULL;}
  virtual const SkPdfBeadDictionary* asBeadDictionary() const {return NULL;}

  virtual SkPdfBlockLevelStructureElementsDictionary* asBlockLevelStructureElementsDictionary() {return NULL;}
  virtual const SkPdfBlockLevelStructureElementsDictionary* asBlockLevelStructureElementsDictionary() const {return NULL;}

  virtual SkPdfBorderStyleDictionary* asBorderStyleDictionary() {return NULL;}
  virtual const SkPdfBorderStyleDictionary* asBorderStyleDictionary() const {return NULL;}

  virtual SkPdfBoxColorInformationDictionary* asBoxColorInformationDictionary() {return NULL;}
  virtual const SkPdfBoxColorInformationDictionary* asBoxColorInformationDictionary() const {return NULL;}

  virtual SkPdfBoxStyleDictionary* asBoxStyleDictionary() {return NULL;}
  virtual const SkPdfBoxStyleDictionary* asBoxStyleDictionary() const {return NULL;}

  virtual SkPdfCIDFontDescriptorDictionary* asCIDFontDescriptorDictionary() {return NULL;}
  virtual const SkPdfCIDFontDescriptorDictionary* asCIDFontDescriptorDictionary() const {return NULL;}

  virtual SkPdfCIDSystemInfoDictionary* asCIDSystemInfoDictionary() {return NULL;}
  virtual const SkPdfCIDSystemInfoDictionary* asCIDSystemInfoDictionary() const {return NULL;}

  virtual SkPdfCMapDictionary* asCMapDictionary() {return NULL;}
  virtual const SkPdfCMapDictionary* asCMapDictionary() const {return NULL;}

  virtual SkPdfCalgrayColorSpaceDictionary* asCalgrayColorSpaceDictionary() {return NULL;}
  virtual const SkPdfCalgrayColorSpaceDictionary* asCalgrayColorSpaceDictionary() const {return NULL;}

  virtual SkPdfCalrgbColorSpaceDictionary* asCalrgbColorSpaceDictionary() {return NULL;}
  virtual const SkPdfCalrgbColorSpaceDictionary* asCalrgbColorSpaceDictionary() const {return NULL;}

  virtual SkPdfCatalogDictionary* asCatalogDictionary() {return NULL;}
  virtual const SkPdfCatalogDictionary* asCatalogDictionary() const {return NULL;}

  virtual SkPdfCcittfaxdecodeFilterDictionary* asCcittfaxdecodeFilterDictionary() {return NULL;}
  virtual const SkPdfCcittfaxdecodeFilterDictionary* asCcittfaxdecodeFilterDictionary() const {return NULL;}

  virtual SkPdfCheckboxFieldDictionary* asCheckboxFieldDictionary() {return NULL;}
  virtual const SkPdfCheckboxFieldDictionary* asCheckboxFieldDictionary() const {return NULL;}

  virtual SkPdfChoiceFieldDictionary* asChoiceFieldDictionary() {return NULL;}
  virtual const SkPdfChoiceFieldDictionary* asChoiceFieldDictionary() const {return NULL;}

  virtual SkPdfComponentsWithMetadataDictionary* asComponentsWithMetadataDictionary() {return NULL;}
  virtual const SkPdfComponentsWithMetadataDictionary* asComponentsWithMetadataDictionary() const {return NULL;}

  virtual SkPdfDctdecodeFilterDictionary* asDctdecodeFilterDictionary() {return NULL;}
  virtual const SkPdfDctdecodeFilterDictionary* asDctdecodeFilterDictionary() const {return NULL;}

  virtual SkPdfDeviceNColorSpaceDictionary* asDeviceNColorSpaceDictionary() {return NULL;}
  virtual const SkPdfDeviceNColorSpaceDictionary* asDeviceNColorSpaceDictionary() const {return NULL;}

  virtual SkPdfDocumentCatalogActionsDictionary* asDocumentCatalogActionsDictionary() {return NULL;}
  virtual const SkPdfDocumentCatalogActionsDictionary* asDocumentCatalogActionsDictionary() const {return NULL;}

  virtual SkPdfDocumentInformationDictionary* asDocumentInformationDictionary() {return NULL;}
  virtual const SkPdfDocumentInformationDictionary* asDocumentInformationDictionary() const {return NULL;}

  virtual SkPdfEmbeddedFileParameterDictionary* asEmbeddedFileParameterDictionary() {return NULL;}
  virtual const SkPdfEmbeddedFileParameterDictionary* asEmbeddedFileParameterDictionary() const {return NULL;}

  virtual SkPdfEmbeddedFileStreamDictionary* asEmbeddedFileStreamDictionary() {return NULL;}
  virtual const SkPdfEmbeddedFileStreamDictionary* asEmbeddedFileStreamDictionary() const {return NULL;}

  virtual SkPdfEmbeddedFontStreamDictionary* asEmbeddedFontStreamDictionary() {return NULL;}
  virtual const SkPdfEmbeddedFontStreamDictionary* asEmbeddedFontStreamDictionary() const {return NULL;}

  virtual SkPdfEncodingDictionary* asEncodingDictionary() {return NULL;}
  virtual const SkPdfEncodingDictionary* asEncodingDictionary() const {return NULL;}

  virtual SkPdfEncryptedEmbeddedFileStreamDictionary* asEncryptedEmbeddedFileStreamDictionary() {return NULL;}
  virtual const SkPdfEncryptedEmbeddedFileStreamDictionary* asEncryptedEmbeddedFileStreamDictionary() const {return NULL;}

  virtual SkPdfEncryptionCommonDictionary* asEncryptionCommonDictionary() {return NULL;}
  virtual const SkPdfEncryptionCommonDictionary* asEncryptionCommonDictionary() const {return NULL;}

  virtual SkPdfFDFCatalogDictionary* asFDFCatalogDictionary() {return NULL;}
  virtual const SkPdfFDFCatalogDictionary* asFDFCatalogDictionary() const {return NULL;}

  virtual SkPdfFDFDictionary* asFDFDictionary() {return NULL;}
  virtual const SkPdfFDFDictionary* asFDFDictionary() const {return NULL;}

  virtual SkPdfFDFFieldDictionary* asFDFFieldDictionary() {return NULL;}
  virtual const SkPdfFDFFieldDictionary* asFDFFieldDictionary() const {return NULL;}

  virtual SkPdfFDFFileAnnotationDictionary* asFDFFileAnnotationDictionary() {return NULL;}
  virtual const SkPdfFDFFileAnnotationDictionary* asFDFFileAnnotationDictionary() const {return NULL;}

  virtual SkPdfFDFNamedPageReferenceDictionary* asFDFNamedPageReferenceDictionary() {return NULL;}
  virtual const SkPdfFDFNamedPageReferenceDictionary* asFDFNamedPageReferenceDictionary() const {return NULL;}

  virtual SkPdfFDFPageDictionary* asFDFPageDictionary() {return NULL;}
  virtual const SkPdfFDFPageDictionary* asFDFPageDictionary() const {return NULL;}

  virtual SkPdfFDFTemplateDictionary* asFDFTemplateDictionary() {return NULL;}
  virtual const SkPdfFDFTemplateDictionary* asFDFTemplateDictionary() const {return NULL;}

  virtual SkPdfFDFTrailerDictionary* asFDFTrailerDictionary() {return NULL;}
  virtual const SkPdfFDFTrailerDictionary* asFDFTrailerDictionary() const {return NULL;}

  virtual SkPdfFieldDictionary* asFieldDictionary() {return NULL;}
  virtual const SkPdfFieldDictionary* asFieldDictionary() const {return NULL;}

  virtual SkPdfFileAttachmentAnnotationDictionary* asFileAttachmentAnnotationDictionary() {return NULL;}
  virtual const SkPdfFileAttachmentAnnotationDictionary* asFileAttachmentAnnotationDictionary() const {return NULL;}

  virtual SkPdfFileSpecificationDictionary* asFileSpecificationDictionary() {return NULL;}
  virtual const SkPdfFileSpecificationDictionary* asFileSpecificationDictionary() const {return NULL;}

  virtual SkPdfFileTrailerDictionary* asFileTrailerDictionary() {return NULL;}
  virtual const SkPdfFileTrailerDictionary* asFileTrailerDictionary() const {return NULL;}

  virtual SkPdfFontDescriptorDictionary* asFontDescriptorDictionary() {return NULL;}
  virtual const SkPdfFontDescriptorDictionary* asFontDescriptorDictionary() const {return NULL;}

  virtual SkPdfFontDictionary* asFontDictionary() {return NULL;}
  virtual const SkPdfFontDictionary* asFontDictionary() const {return NULL;}

  virtual SkPdfCIDFontDictionary* asCIDFontDictionary() {return NULL;}
  virtual const SkPdfCIDFontDictionary* asCIDFontDictionary() const {return NULL;}

  virtual SkPdfType0FontDictionary* asType0FontDictionary() {return NULL;}
  virtual const SkPdfType0FontDictionary* asType0FontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

  virtual SkPdfType1FontDictionary* asType1FontDictionary() {return NULL;}
  virtual const SkPdfType1FontDictionary* asType1FontDictionary() const {return NULL;}

  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

  virtual SkPdfFormFieldActionsDictionary* asFormFieldActionsDictionary() {return NULL;}
  virtual const SkPdfFormFieldActionsDictionary* asFormFieldActionsDictionary() const {return NULL;}

  virtual SkPdfFreeTextAnnotationDictionary* asFreeTextAnnotationDictionary() {return NULL;}
  virtual const SkPdfFreeTextAnnotationDictionary* asFreeTextAnnotationDictionary() const {return NULL;}

  virtual SkPdfFunctionCommonDictionary* asFunctionCommonDictionary() {return NULL;}
  virtual const SkPdfFunctionCommonDictionary* asFunctionCommonDictionary() const {return NULL;}

  virtual SkPdfGoToActionDictionary* asGoToActionDictionary() {return NULL;}
  virtual const SkPdfGoToActionDictionary* asGoToActionDictionary() const {return NULL;}

  virtual SkPdfGraphicsStateDictionary* asGraphicsStateDictionary() {return NULL;}
  virtual const SkPdfGraphicsStateDictionary* asGraphicsStateDictionary() const {return NULL;}

  virtual SkPdfGroupAttributesDictionary* asGroupAttributesDictionary() {return NULL;}
  virtual const SkPdfGroupAttributesDictionary* asGroupAttributesDictionary() const {return NULL;}

  virtual SkPdfHideActionDictionary* asHideActionDictionary() {return NULL;}
  virtual const SkPdfHideActionDictionary* asHideActionDictionary() const {return NULL;}

  virtual SkPdfIccProfileStreamDictionary* asIccProfileStreamDictionary() {return NULL;}
  virtual const SkPdfIccProfileStreamDictionary* asIccProfileStreamDictionary() const {return NULL;}

  virtual SkPdfIconFitDictionary* asIconFitDictionary() {return NULL;}
  virtual const SkPdfIconFitDictionary* asIconFitDictionary() const {return NULL;}

  virtual SkPdfImportDataActionDictionary* asImportDataActionDictionary() {return NULL;}
  virtual const SkPdfImportDataActionDictionary* asImportDataActionDictionary() const {return NULL;}

  virtual SkPdfInkAnnotationDictionary* asInkAnnotationDictionary() {return NULL;}
  virtual const SkPdfInkAnnotationDictionary* asInkAnnotationDictionary() const {return NULL;}

  virtual SkPdfInlineLevelStructureElementsDictionary* asInlineLevelStructureElementsDictionary() {return NULL;}
  virtual const SkPdfInlineLevelStructureElementsDictionary* asInlineLevelStructureElementsDictionary() const {return NULL;}

  virtual SkPdfInteractiveFormDictionary* asInteractiveFormDictionary() {return NULL;}
  virtual const SkPdfInteractiveFormDictionary* asInteractiveFormDictionary() const {return NULL;}

  virtual SkPdfJavascriptActionDictionary* asJavascriptActionDictionary() {return NULL;}
  virtual const SkPdfJavascriptActionDictionary* asJavascriptActionDictionary() const {return NULL;}

  virtual SkPdfJavascriptDictionary* asJavascriptDictionary() {return NULL;}
  virtual const SkPdfJavascriptDictionary* asJavascriptDictionary() const {return NULL;}

  virtual SkPdfJbig2DecodeFilterDictionary* asJbig2DecodeFilterDictionary() {return NULL;}
  virtual const SkPdfJbig2DecodeFilterDictionary* asJbig2DecodeFilterDictionary() const {return NULL;}

  virtual SkPdfLabColorSpaceDictionary* asLabColorSpaceDictionary() {return NULL;}
  virtual const SkPdfLabColorSpaceDictionary* asLabColorSpaceDictionary() const {return NULL;}

  virtual SkPdfLaunchActionDictionary* asLaunchActionDictionary() {return NULL;}
  virtual const SkPdfLaunchActionDictionary* asLaunchActionDictionary() const {return NULL;}

  virtual SkPdfLineAnnotationDictionary* asLineAnnotationDictionary() {return NULL;}
  virtual const SkPdfLineAnnotationDictionary* asLineAnnotationDictionary() const {return NULL;}

  virtual SkPdfListAttributeDictionary* asListAttributeDictionary() {return NULL;}
  virtual const SkPdfListAttributeDictionary* asListAttributeDictionary() const {return NULL;}

  virtual SkPdfLzwdecodeAndFlatedecodeFiltersDictionary* asLzwdecodeAndFlatedecodeFiltersDictionary() {return NULL;}
  virtual const SkPdfLzwdecodeAndFlatedecodeFiltersDictionary* asLzwdecodeAndFlatedecodeFiltersDictionary() const {return NULL;}

  virtual SkPdfMacOsFileInformationDictionary* asMacOsFileInformationDictionary() {return NULL;}
  virtual const SkPdfMacOsFileInformationDictionary* asMacOsFileInformationDictionary() const {return NULL;}

  virtual SkPdfMarkInformationDictionary* asMarkInformationDictionary() {return NULL;}
  virtual const SkPdfMarkInformationDictionary* asMarkInformationDictionary() const {return NULL;}

  virtual SkPdfMarkedContentReferenceDictionary* asMarkedContentReferenceDictionary() {return NULL;}
  virtual const SkPdfMarkedContentReferenceDictionary* asMarkedContentReferenceDictionary() const {return NULL;}

  virtual SkPdfMarkupAnnotationsDictionary* asMarkupAnnotationsDictionary() {return NULL;}
  virtual const SkPdfMarkupAnnotationsDictionary* asMarkupAnnotationsDictionary() const {return NULL;}

  virtual SkPdfMetadataStreamDictionary* asMetadataStreamDictionary() {return NULL;}
  virtual const SkPdfMetadataStreamDictionary* asMetadataStreamDictionary() const {return NULL;}

  virtual SkPdfMovieActionDictionary* asMovieActionDictionary() {return NULL;}
  virtual const SkPdfMovieActionDictionary* asMovieActionDictionary() const {return NULL;}

  virtual SkPdfMovieActivationDictionary* asMovieActivationDictionary() {return NULL;}
  virtual const SkPdfMovieActivationDictionary* asMovieActivationDictionary() const {return NULL;}

  virtual SkPdfMovieAnnotationDictionary* asMovieAnnotationDictionary() {return NULL;}
  virtual const SkPdfMovieAnnotationDictionary* asMovieAnnotationDictionary() const {return NULL;}

  virtual SkPdfMovieDictionary* asMovieDictionary() {return NULL;}
  virtual const SkPdfMovieDictionary* asMovieDictionary() const {return NULL;}

  virtual SkPdfNameDictionary* asNameDictionary() {return NULL;}
  virtual const SkPdfNameDictionary* asNameDictionary() const {return NULL;}

  virtual SkPdfNameTreeNodeDictionary* asNameTreeNodeDictionary() {return NULL;}
  virtual const SkPdfNameTreeNodeDictionary* asNameTreeNodeDictionary() const {return NULL;}

  virtual SkPdfNamedActionsDictionary* asNamedActionsDictionary() {return NULL;}
  virtual const SkPdfNamedActionsDictionary* asNamedActionsDictionary() const {return NULL;}

  virtual SkPdfNumberTreeNodeDictionary* asNumberTreeNodeDictionary() {return NULL;}
  virtual const SkPdfNumberTreeNodeDictionary* asNumberTreeNodeDictionary() const {return NULL;}

  virtual SkPdfObjectReferenceDictionary* asObjectReferenceDictionary() {return NULL;}
  virtual const SkPdfObjectReferenceDictionary* asObjectReferenceDictionary() const {return NULL;}

  virtual SkPdfOpiVersionDictionary* asOpiVersionDictionary() {return NULL;}
  virtual const SkPdfOpiVersionDictionary* asOpiVersionDictionary() const {return NULL;}

  virtual SkPdfOutlineDictionary* asOutlineDictionary() {return NULL;}
  virtual const SkPdfOutlineDictionary* asOutlineDictionary() const {return NULL;}

  virtual SkPdfOutlineItemDictionary* asOutlineItemDictionary() {return NULL;}
  virtual const SkPdfOutlineItemDictionary* asOutlineItemDictionary() const {return NULL;}

  virtual SkPdfPDF_XOutputIntentDictionary* asPDF_XOutputIntentDictionary() {return NULL;}
  virtual const SkPdfPDF_XOutputIntentDictionary* asPDF_XOutputIntentDictionary() const {return NULL;}

  virtual SkPdfPSXobjectDictionary* asPSXobjectDictionary() {return NULL;}
  virtual const SkPdfPSXobjectDictionary* asPSXobjectDictionary() const {return NULL;}

  virtual SkPdfPageLabelDictionary* asPageLabelDictionary() {return NULL;}
  virtual const SkPdfPageLabelDictionary* asPageLabelDictionary() const {return NULL;}

  virtual SkPdfPageObjectActionsDictionary* asPageObjectActionsDictionary() {return NULL;}
  virtual const SkPdfPageObjectActionsDictionary* asPageObjectActionsDictionary() const {return NULL;}

  virtual SkPdfPagePieceDictionary* asPagePieceDictionary() {return NULL;}
  virtual const SkPdfPagePieceDictionary* asPagePieceDictionary() const {return NULL;}

  virtual SkPdfPageTreeNodeDictionary* asPageTreeNodeDictionary() {return NULL;}
  virtual const SkPdfPageTreeNodeDictionary* asPageTreeNodeDictionary() const {return NULL;}

  virtual SkPdfPopUpAnnotationDictionary* asPopUpAnnotationDictionary() {return NULL;}
  virtual const SkPdfPopUpAnnotationDictionary* asPopUpAnnotationDictionary() const {return NULL;}

  virtual SkPdfPrinterMarkAnnotationDictionary* asPrinterMarkAnnotationDictionary() {return NULL;}
  virtual const SkPdfPrinterMarkAnnotationDictionary* asPrinterMarkAnnotationDictionary() const {return NULL;}

  virtual SkPdfPrinterMarkFormDictionary* asPrinterMarkFormDictionary() {return NULL;}
  virtual const SkPdfPrinterMarkFormDictionary* asPrinterMarkFormDictionary() const {return NULL;}

  virtual SkPdfRadioButtonFieldDictionary* asRadioButtonFieldDictionary() {return NULL;}
  virtual const SkPdfRadioButtonFieldDictionary* asRadioButtonFieldDictionary() const {return NULL;}

  virtual SkPdfReferenceDictionary* asReferenceDictionary() {return NULL;}
  virtual const SkPdfReferenceDictionary* asReferenceDictionary() const {return NULL;}

  virtual SkPdfRemoteGoToActionDictionary* asRemoteGoToActionDictionary() {return NULL;}
  virtual const SkPdfRemoteGoToActionDictionary* asRemoteGoToActionDictionary() const {return NULL;}

  virtual SkPdfResetFormActionDictionary* asResetFormActionDictionary() {return NULL;}
  virtual const SkPdfResetFormActionDictionary* asResetFormActionDictionary() const {return NULL;}

  virtual SkPdfResourceDictionary* asResourceDictionary() {return NULL;}
  virtual const SkPdfResourceDictionary* asResourceDictionary() const {return NULL;}

  virtual SkPdfRubberStampAnnotationDictionary* asRubberStampAnnotationDictionary() {return NULL;}
  virtual const SkPdfRubberStampAnnotationDictionary* asRubberStampAnnotationDictionary() const {return NULL;}

  virtual SkPdfSeparationDictionary* asSeparationDictionary() {return NULL;}
  virtual const SkPdfSeparationDictionary* asSeparationDictionary() const {return NULL;}

  virtual SkPdfShadingDictionary* asShadingDictionary() {return NULL;}
  virtual const SkPdfShadingDictionary* asShadingDictionary() const {return NULL;}

  virtual SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return NULL;}
  virtual const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return NULL;}

  virtual SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return NULL;}
  virtual const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return NULL;}

  virtual SkPdfType3ShadingDictionary* asType3ShadingDictionary() {return NULL;}
  virtual const SkPdfType3ShadingDictionary* asType3ShadingDictionary() const {return NULL;}

  virtual SkPdfType4ShadingDictionary* asType4ShadingDictionary() {return NULL;}
  virtual const SkPdfType4ShadingDictionary* asType4ShadingDictionary() const {return NULL;}

  virtual SkPdfType5ShadingDictionary* asType5ShadingDictionary() {return NULL;}
  virtual const SkPdfType5ShadingDictionary* asType5ShadingDictionary() const {return NULL;}

  virtual SkPdfType6ShadingDictionary* asType6ShadingDictionary() {return NULL;}
  virtual const SkPdfType6ShadingDictionary* asType6ShadingDictionary() const {return NULL;}

  virtual SkPdfSignatureDictionary* asSignatureDictionary() {return NULL;}
  virtual const SkPdfSignatureDictionary* asSignatureDictionary() const {return NULL;}

  virtual SkPdfSoftMaskDictionary* asSoftMaskDictionary() {return NULL;}
  virtual const SkPdfSoftMaskDictionary* asSoftMaskDictionary() const {return NULL;}

  virtual SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() {return NULL;}
  virtual const SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() const {return NULL;}

  virtual SkPdfSoundActionDictionary* asSoundActionDictionary() {return NULL;}
  virtual const SkPdfSoundActionDictionary* asSoundActionDictionary() const {return NULL;}

  virtual SkPdfSoundAnnotationDictionary* asSoundAnnotationDictionary() {return NULL;}
  virtual const SkPdfSoundAnnotationDictionary* asSoundAnnotationDictionary() const {return NULL;}

  virtual SkPdfSoundObjectDictionary* asSoundObjectDictionary() {return NULL;}
  virtual const SkPdfSoundObjectDictionary* asSoundObjectDictionary() const {return NULL;}

  virtual SkPdfSourceInformationDictionary* asSourceInformationDictionary() {return NULL;}
  virtual const SkPdfSourceInformationDictionary* asSourceInformationDictionary() const {return NULL;}

  virtual SkPdfSquareOrCircleAnnotation* asSquareOrCircleAnnotation() {return NULL;}
  virtual const SkPdfSquareOrCircleAnnotation* asSquareOrCircleAnnotation() const {return NULL;}

  virtual SkPdfStandardSecurityHandlerDictionary* asStandardSecurityHandlerDictionary() {return NULL;}
  virtual const SkPdfStandardSecurityHandlerDictionary* asStandardSecurityHandlerDictionary() const {return NULL;}

  virtual SkPdfStandardStructureDictionary* asStandardStructureDictionary() {return NULL;}
  virtual const SkPdfStandardStructureDictionary* asStandardStructureDictionary() const {return NULL;}

  virtual SkPdfStreamCommonDictionary* asStreamCommonDictionary() {return NULL;}
  virtual const SkPdfStreamCommonDictionary* asStreamCommonDictionary() const {return NULL;}

  virtual SkPdfStructureElementAccessDictionary* asStructureElementAccessDictionary() {return NULL;}
  virtual const SkPdfStructureElementAccessDictionary* asStructureElementAccessDictionary() const {return NULL;}

  virtual SkPdfStructureElementDictionary* asStructureElementDictionary() {return NULL;}
  virtual const SkPdfStructureElementDictionary* asStructureElementDictionary() const {return NULL;}

  virtual SkPdfStructureTreeRootDictionary* asStructureTreeRootDictionary() {return NULL;}
  virtual const SkPdfStructureTreeRootDictionary* asStructureTreeRootDictionary() const {return NULL;}

  virtual SkPdfSubmitFormActionDictionary* asSubmitFormActionDictionary() {return NULL;}
  virtual const SkPdfSubmitFormActionDictionary* asSubmitFormActionDictionary() const {return NULL;}

  virtual SkPdfTableAttributesDictionary* asTableAttributesDictionary() {return NULL;}
  virtual const SkPdfTableAttributesDictionary* asTableAttributesDictionary() const {return NULL;}

  virtual SkPdfTextAnnotationDictionary* asTextAnnotationDictionary() {return NULL;}
  virtual const SkPdfTextAnnotationDictionary* asTextAnnotationDictionary() const {return NULL;}

  virtual SkPdfTextFieldDictionary* asTextFieldDictionary() {return NULL;}
  virtual const SkPdfTextFieldDictionary* asTextFieldDictionary() const {return NULL;}

  virtual SkPdfThreadActionDictionary* asThreadActionDictionary() {return NULL;}
  virtual const SkPdfThreadActionDictionary* asThreadActionDictionary() const {return NULL;}

  virtual SkPdfThreadDictionary* asThreadDictionary() {return NULL;}
  virtual const SkPdfThreadDictionary* asThreadDictionary() const {return NULL;}

  virtual SkPdfTransitionDictionary* asTransitionDictionary() {return NULL;}
  virtual const SkPdfTransitionDictionary* asTransitionDictionary() const {return NULL;}

  virtual SkPdfTransparencyGroupDictionary* asTransparencyGroupDictionary() {return NULL;}
  virtual const SkPdfTransparencyGroupDictionary* asTransparencyGroupDictionary() const {return NULL;}

  virtual SkPdfTrapNetworkAnnotationDictionary* asTrapNetworkAnnotationDictionary() {return NULL;}
  virtual const SkPdfTrapNetworkAnnotationDictionary* asTrapNetworkAnnotationDictionary() const {return NULL;}

  virtual SkPdfTrapNetworkAppearanceStreamDictionary* asTrapNetworkAppearanceStreamDictionary() {return NULL;}
  virtual const SkPdfTrapNetworkAppearanceStreamDictionary* asTrapNetworkAppearanceStreamDictionary() const {return NULL;}

  virtual SkPdfType0FunctionDictionary* asType0FunctionDictionary() {return NULL;}
  virtual const SkPdfType0FunctionDictionary* asType0FunctionDictionary() const {return NULL;}

  virtual SkPdfType10HalftoneDictionary* asType10HalftoneDictionary() {return NULL;}
  virtual const SkPdfType10HalftoneDictionary* asType10HalftoneDictionary() const {return NULL;}

  virtual SkPdfType16HalftoneDictionary* asType16HalftoneDictionary() {return NULL;}
  virtual const SkPdfType16HalftoneDictionary* asType16HalftoneDictionary() const {return NULL;}

  virtual SkPdfType1HalftoneDictionary* asType1HalftoneDictionary() {return NULL;}
  virtual const SkPdfType1HalftoneDictionary* asType1HalftoneDictionary() const {return NULL;}

  virtual SkPdfType1PatternDictionary* asType1PatternDictionary() {return NULL;}
  virtual const SkPdfType1PatternDictionary* asType1PatternDictionary() const {return NULL;}

  virtual SkPdfType2FunctionDictionary* asType2FunctionDictionary() {return NULL;}
  virtual const SkPdfType2FunctionDictionary* asType2FunctionDictionary() const {return NULL;}

  virtual SkPdfType2PatternDictionary* asType2PatternDictionary() {return NULL;}
  virtual const SkPdfType2PatternDictionary* asType2PatternDictionary() const {return NULL;}

  virtual SkPdfType3FunctionDictionary* asType3FunctionDictionary() {return NULL;}
  virtual const SkPdfType3FunctionDictionary* asType3FunctionDictionary() const {return NULL;}

  virtual SkPdfType5HalftoneDictionary* asType5HalftoneDictionary() {return NULL;}
  virtual const SkPdfType5HalftoneDictionary* asType5HalftoneDictionary() const {return NULL;}

  virtual SkPdfType6HalftoneDictionary* asType6HalftoneDictionary() {return NULL;}
  virtual const SkPdfType6HalftoneDictionary* asType6HalftoneDictionary() const {return NULL;}

  virtual SkPdfURIActionDictionary* asURIActionDictionary() {return NULL;}
  virtual const SkPdfURIActionDictionary* asURIActionDictionary() const {return NULL;}

  virtual SkPdfURIDictionary* asURIDictionary() {return NULL;}
  virtual const SkPdfURIDictionary* asURIDictionary() const {return NULL;}

  virtual SkPdfURLAliasDictionary* asURLAliasDictionary() {return NULL;}
  virtual const SkPdfURLAliasDictionary* asURLAliasDictionary() const {return NULL;}

  virtual SkPdfVariableTextFieldDictionary* asVariableTextFieldDictionary() {return NULL;}
  virtual const SkPdfVariableTextFieldDictionary* asVariableTextFieldDictionary() const {return NULL;}

  virtual SkPdfViewerPreferencesDictionary* asViewerPreferencesDictionary() {return NULL;}
  virtual const SkPdfViewerPreferencesDictionary* asViewerPreferencesDictionary() const {return NULL;}

  virtual SkPdfWebCaptureCommandDictionary* asWebCaptureCommandDictionary() {return NULL;}
  virtual const SkPdfWebCaptureCommandDictionary* asWebCaptureCommandDictionary() const {return NULL;}

  virtual SkPdfWebCaptureCommandSettingsDictionary* asWebCaptureCommandSettingsDictionary() {return NULL;}
  virtual const SkPdfWebCaptureCommandSettingsDictionary* asWebCaptureCommandSettingsDictionary() const {return NULL;}

  virtual SkPdfWebCaptureDictionary* asWebCaptureDictionary() {return NULL;}
  virtual const SkPdfWebCaptureDictionary* asWebCaptureDictionary() const {return NULL;}

  virtual SkPdfWebCaptureImageSetDictionary* asWebCaptureImageSetDictionary() {return NULL;}
  virtual const SkPdfWebCaptureImageSetDictionary* asWebCaptureImageSetDictionary() const {return NULL;}

  virtual SkPdfWebCaptureInformationDictionary* asWebCaptureInformationDictionary() {return NULL;}
  virtual const SkPdfWebCaptureInformationDictionary* asWebCaptureInformationDictionary() const {return NULL;}

  virtual SkPdfWebCapturePageSetDictionary* asWebCapturePageSetDictionary() {return NULL;}
  virtual const SkPdfWebCapturePageSetDictionary* asWebCapturePageSetDictionary() const {return NULL;}

  virtual SkPdfWidgetAnnotationDictionary* asWidgetAnnotationDictionary() {return NULL;}
  virtual const SkPdfWidgetAnnotationDictionary* asWidgetAnnotationDictionary() const {return NULL;}

  virtual SkPdfWindowsLaunchActionDictionary* asWindowsLaunchActionDictionary() {return NULL;}
  virtual const SkPdfWindowsLaunchActionDictionary* asWindowsLaunchActionDictionary() const {return NULL;}

  virtual SkPdfXObjectDictionary* asXObjectDictionary() {return NULL;}
  virtual const SkPdfXObjectDictionary* asXObjectDictionary() const {return NULL;}

  virtual SkPdfImageDictionary* asImageDictionary() {return NULL;}
  virtual const SkPdfImageDictionary* asImageDictionary() const {return NULL;}

  virtual SkPdfType1FormDictionary* asType1FormDictionary() {return NULL;}
  virtual const SkPdfType1FormDictionary* asType1FormDictionary() const {return NULL;}

public:
private:
public:
  SkPdfPageObjectDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfDictionary(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfPageObjectDictionary& operator=(const SkPdfPageObjectDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Required) The type of PDF object that this dictionary describes; must be
 *  Page for a page object.
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

/** (Required; must be an indirect reference) The page tree node that is the im-
 *  mediate parent of this page object.
**/
  bool has_Parent() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Parent", "", NULL));
  }

  SkPdfDictionary* Parent() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Parent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required if PieceInfo is present; optional otherwise; PDF 1.3) The date and
 *  time (see Section 3.8.2, "Dates") when the page's contents were most re-
 *  cently modified. If a page-piece dictionary (PieceInfo) is present, the
 *  modification date is used to ascertain which of the application data dic-
 *  tionaries that it contains correspond to the current content of the page
 *  (see Section 9.4, "Page-Piece Dictionaries").
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

/** (Required; inheritable) A dictionary containing any resources required by
 *  the page (see Section 3.7.2, "Resource Dictionaries"). If the page requires
 *  no resources, the value of this entry should be an empty dictionary; omit-
 *  ting the entry entirely indicates that the resources are to be inherited from
 *  an ancestor node in the page tree.
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

/** (Required; inheritable) A rectangle (see Section 3.8.3, "Rectangles"), ex-
 *  pressed in default user space units, defining the boundaries of the physical
 *  medium on which the page is intended to be displayed or printed (see
 *  Section 9.10.1, "Page Boundaries").
**/
  bool has_MediaBox() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "MediaBox", "", NULL));
  }

  SkRect MediaBox() const {
    SkRect ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "MediaBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkRect();
  }

/** (Optional; inheritable) A rectangle, expressed in default user space units,
 *  defining the visible region of default user space. When the page is dis-
 *  played or printed, its contents are to be clipped (cropped) to this rectangle
 *  and then imposed on the output medium in some implementation-
 *  defined manner (see Section 9.10.1, "Page Boundaries"). Default value:
 *  the value of MediaBox.
**/
  bool has_CropBox() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CropBox", "", NULL));
  }

  SkRect CropBox() const {
    SkRect ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CropBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkRect();
  }

/** (Optional; PDF 1.3) A rectangle, expressed in default user space units, de-
 *  fining the region to which the contents of the page should be clipped
 *  when output in a production environment (see Section 9.10.1, "Page
 *  Boundaries"). Default value: the value of CropBox.
**/
  bool has_BleedBox() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BleedBox", "", NULL));
  }

  SkRect BleedBox() const {
    SkRect ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BleedBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkRect();
  }

/** (Optional; PDF 1.3) A rectangle, expressed in default user space units, de-
 *  fining the intended dimensions of the finished page after trimming (see
 *  Section 9.10.1, "Page Boundaries"). Default value: the value of CropBox.
**/
  bool has_TrimBox() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TrimBox", "", NULL));
  }

  SkRect TrimBox() const {
    SkRect ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TrimBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkRect();
  }

/** (Optional; PDF 1.3) A rectangle, expressed in default user space units, de-
 *  fining the extent of the page's meaningful content (including potential
 *  white space) as intended by the page's creator (see Section 9.10.1, "Page
 *  Boundaries"). Default value: the value of CropBox.
**/
  bool has_ArtBox() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ArtBox", "", NULL));
  }

  SkRect ArtBox() const {
    SkRect ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ArtBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkRect();
  }

/** (Optional) A box color information dictionary specifying the colors and
 *  other visual characteristics to be used in displaying guidelines on the
 *  screen for the various page boundaries (see "Display of Page Boundaries"
 *  on page 679). If this entry is absent, the viewer application should use its
 *  own current default settings.
**/
  bool has_BoxColorInfo() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BoxColorInfo", "", NULL));
  }

  SkPdfDictionary* BoxColorInfo() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BoxColorInfo", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional) A content stream (see Section 3.7.1, "Content Streams") de-
 *  scribing the contents of this page. If this entry is absent, the page is empty.
 *  The value may be either a single stream or an array of streams. If it is an
 *  array, the effect is as if all of the streams in the array were concatenated, in
 *  order, to form a single stream. This allows a program generating a PDF
 *  file to create image objects and other resources as they occur, even though
 *  they interrupt the content stream. The division between streams may
 *  occur only at the boundaries between lexical tokens (see Section 3.1, "Lex-
 *  ical Conventions"), but is unrelated to the page's logical content or orga-
 *  nization. Applications that consume or produce PDF files are not required
 *  to preserve the existing structure of the Contents array. (See implementa-
 *  tion note 22 in Appendix H.)
**/
  bool has_Contents() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Contents", "", NULL));
  }

  bool isContentsAStream() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Contents", "", &ret)) return false;
    return ret->podofo()->HasStream();
  }

  SkPdfStream* getContentsAsStream() const {
    SkPdfStream* ret = NULL;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Contents", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

  bool isContentsAArray() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Contents", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Array;
  }

  SkPdfArray getContentsAsArray() const {
    SkPdfArray ret = SkPdfArray();
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Contents", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

/** (Optional; inheritable) The number of degrees by which the page should
 *  be rotated clockwise when displayed or printed. The value must be a mul-
 *  tiple of 90. Default value: 0.
**/
  bool has_Rotate() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Rotate", "", NULL));
  }

  long Rotate() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Rotate", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Optional; PDF 1.4) A group attributes dictionary specifying the attributes
 *  of the page's page group for use in the transparent imaging model (see
 *  Sections 7.3.6, "Page Group," and 7.5.5, "Transparency Group XObjects").
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

/** (Optional) A stream object defining the page's thumbnail image (see Sec-
 *  tion 8.2.3, "Thumbnail Images").
**/
  bool has_Thumb() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Thumb", "", NULL));
  }

  SkPdfStream* Thumb() const {
    SkPdfStream* ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Thumb", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.1; recommended if the page contains article beads) An ar-
 *  ray of indirect references to article beads appearing on the page (see Sec-
 *  tion 8.3.2, "Articles"; see also implementation note 23 in Appendix H).
 *  The beads are listed in the array in natural reading order.
**/
  bool has_B() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "B", "", NULL));
  }

  SkPdfArray B() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "B", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

/** (Optional; PDF 1.1) The page's display duration (also called its advance
 *  timing): the maximum length of time, in seconds, that the page will be
 *  displayed during presentations before the viewer application automati-
 *  cally advances to the next page (see Section 8.3.3, "Presentations"). By
 *  default, the viewer does not advance automatically.
**/
  bool has_Dur() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Dur", "", NULL));
  }

  double Dur() const {
    double ret;
    if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Dur", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Optional; PDF 1.1) A transition dictionary describing the transition effect
 *  to be used when displaying the page during presentations (see Section
 *  8.3.3, "Presentations").
**/
  bool has_Trans() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Trans", "", NULL));
  }

  SkPdfDictionary* Trans() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Trans", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional) An array of annotation dictionaries representing annotations
 *  associated with the page (see Section 8.4, "Annotations").
**/
  bool has_Annots() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Annots", "", NULL));
  }

  SkPdfArray Annots() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Annots", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

/** (Optional; PDF 1.2) An additional-actions dictionary defining actions to
 *  be performed when the page is opened or closed (see Section 8.5.2, "Trig-
 *  ger Events"; see also implementation note 24 in Appendix H).
**/
  bool has_AA() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "AA", "", NULL));
  }

  SkPdfDictionary* AA() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "AA", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.4) A metadata stream containing metadata for the page
 *  (see Section 9.2.2, "Metadata Streams").
**/
  bool has_Metadata() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Metadata", "", NULL));
  }

  SkPdfStream* Metadata() const {
    SkPdfStream* ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Metadata", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.3) A page-piece dictionary associated with the page (see
 *  Section 9.4, "Page-Piece Dictionaries").
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

/** (Required if the page contains structural content items; PDF 1.3) The inte-
 *  ger key of the page's entry in the structural parent tree (see "Finding Struc-
 *  ture Elements from Content Items" on page 600).
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

/** (Optional; PDF 1.3; indirect reference preferred) The digital identifier of the
 *  page's parent Web Capture content set (see Section 9.9.5, "Object At-
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

/** (Optional; PDF 1.3) The page's preferred zoom (magnification) factor: the
 *  factor by which it should be scaled to achieve the "natural" display magni-
 *  fication (see Section 9.9.5, "Object Attributes Related to Web Capture").
**/
  bool has_PZ() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "PZ", "", NULL));
  }

  double PZ() const {
    double ret;
    if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "PZ", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Optional; PDF 1.3) A separation dictionary containing information need-
 *  ed to generate color separations for the page (see Section 9.10.3, "Separa-
 *  tion Dictionaries").
**/
  bool has_SeparationInfo() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SeparationInfo", "", NULL));
  }

  SkPdfDictionary* SeparationInfo() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SeparationInfo", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

};

#endif  // __DEFINED__SkPdfPageObjectDictionary

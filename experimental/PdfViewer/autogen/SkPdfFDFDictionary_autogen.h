#ifndef __DEFINED__SkPdfFDFDictionary
#define __DEFINED__SkPdfFDFDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfDictionary_autogen.h"

// Entries in the FDF dictionary
class SkPdfFDFDictionary : public SkPdfDictionary {
public:
  virtual SkPdfObjectType getType() const { return kFDFDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kFDFDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfFDFDictionary* asFDFDictionary() {return this;}
  virtual const SkPdfFDFDictionary* asFDFDictionary() const {return this;}

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

  virtual SkPdfType1FontDictionary* asType1FontDictionary() {return NULL;}
  virtual const SkPdfType1FontDictionary* asType1FontDictionary() const {return NULL;}

  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

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

  virtual SkPdfPageObjectDictionary* asPageObjectDictionary() {return NULL;}
  virtual const SkPdfPageObjectDictionary* asPageObjectDictionary() const {return NULL;}

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
  SkPdfFDFDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfDictionary(podofoDoc, podofoObj) {}

  SkPdfFDFDictionary(const SkPdfFDFDictionary& from) : SkPdfDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfFDFDictionary& operator=(const SkPdfFDFDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Optional) The source file or target file: the PDF document file that
 *  this FDF file was exported from or is intended to be imported into.
**/
  bool has_F() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "F", "", NULL));
  }

  SkPdfFileSpec F() const {
    SkPdfFileSpec ret;
    if (FileSpecFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "F", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfFileSpec();
  }

/** (Optional) An array of two strings constituting a file identifier (see
 *  Section 9.3, "File Identifiers") for the source or target file designated
 *  by F, taken from the ID entry in the file's trailer dictionary (see Sec-
 *  tion 3.4.4, "File Trailer").
**/
  bool has_ID() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ID", "", NULL));
  }

  SkPdfArray* ID() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ID", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional) An array of FDF field dictionaries (see "FDF Fields" on
 *  page 564) describing the root fields (those with no ancestors in
 *  the field hierarchy) to be exported or imported. This entry and
 *  the Pages entry may not both be present.
**/
  bool has_Fields() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Fields", "", NULL));
  }

  SkPdfArray* Fields() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Fields", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional) A status string to be displayed indicating the result of an
 *  action, typically a submit-form action (see "Submit-Form Actions"
 *  on page 550). The string is encoded with PDFDocEncoding. (See
 *  implementation note 91 in Appendix H.) This entry and the Pages
 *  entry may not both be present.
**/
  bool has_Status() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Status", "", NULL));
  }

  std::string Status() const {
    std::string ret;
    if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Status", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Optional; PDF 1.3) An array of FDF page dictionaries (see "FDF
 *  Pages" on page 566) describing new pages to be added to a PDF
 *  target document. The Fields and Status entries may not be present
 *  together with this entry.
**/
  bool has_Pages() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Pages", "", NULL));
  }

  SkPdfArray* Pages() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Pages", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.3) The encoding to be used for any FDF field
 *  value or option (V or Opt in the field dictionary; see Table 8.72 on
 *  page 564) that is a string and does not begin with the Unicode pre-
 *  fix U+FEFF. (See implementation note 92 in Appendix H.) Default
 *  value: PDFDocEncoding.
**/
  bool has_Encoding() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", NULL));
  }

  std::string Encoding() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Optional; PDF 1.3) An array of FDF annotation dictionaries (see
 *  "FDF Annotation Dictionaries" on page 568). The array can in-
 *  clude annotations of any of the standard types listed in Table 8.14
 *  on page 499 except Link, Movie, Widget, PrinterMark, and TrapNet.
**/
  bool has_Annots() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Annots", "", NULL));
  }

  SkPdfArray* Annots() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Annots", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.4) A stream containing all the bytes in all incre-
 *  mental updates made to the underlying PDF document since it was
 *  opened (see Section 3.4.5, "Incremental Updates"). If a submit-
 *  form action submitting the document to a remote server in FDF
 *  format has its IncludeAppendSaves flag set (see "Submit-Form
 *  Actions" on page 550), the contents of this stream are included in
 *  the submission. This allows any digital signatures (see "Signature
 *  Fields" on page 547) to be transmitted to the server. An incremental
 *  update is automatically performed just before the submission takes
 *  place, in order to capture all changes made to the document. Note
 *  that the submission always includes the full set of incremental up-
 *  dates back to the time the document was first opened, even if some
 *  of them may already have been included in intervening submissions.
 *  Note: Although a Fields or Annots entry (or both) may be present
 *  along with Differences, there is no guarantee that their contents will be
 *  consistent with it. In particular, if Differences contains a digital signa-
 *  ture, only the values of the form fields given in the Differences stream
 *  can be considered trustworthy under that signature.
**/
  bool has_Differences() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Differences", "", NULL));
  }

  SkPdfStream* Differences() const {
    SkPdfStream* ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Differences", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.4) The name of a browser frame in which the un-
 *  derlying PDF document is to be opened. This mimics the behavior
 *  of the target attribute in HTML <href> tags.
**/
  bool has_Target() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Target", "", NULL));
  }

  std::string Target() const {
    std::string ret;
    if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Target", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Optional; PDF 1.4) An array of file specifications (see Section 3.10,
 *  "File Specifications") representing other FDF files embedded with-
 *  in this one (Section 3.10.3, "Embedded File Streams").
**/
  bool has_EmbeddedFDFs() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EmbeddedFDFs", "", NULL));
  }

  SkPdfArray* EmbeddedFDFs() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EmbeddedFDFs", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.4) A JavaScript dictionary (see Table 8.71) defin-
 *  ing document-level JavaScript scripts.
**/
  bool has_JavaScript() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "JavaScript", "", NULL));
  }

  SkPdfDictionary* JavaScript() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "JavaScript", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

};

#endif  // __DEFINED__SkPdfFDFDictionary

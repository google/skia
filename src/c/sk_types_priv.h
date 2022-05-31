/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_types_priv_DEFINED
#define sk_types_priv_DEFINED

#include "include/c/sk_types.h"

#include "include/core/SkTypes.h" // required to make sure SK_SUPPORT_GPU is defined

#define SK_SKIP_ARG__(keep, skip, ...) skip
#define SK_SKIP_ARG_(args) SK_SKIP_ARG__ args
#define SK_SKIP_ARG(...) SK_SKIP_ARG_((__VA_ARGS__, ))

#define SK_FIRST_ARG__(keep, skip, ...) keep
#define SK_FIRST_ARG_(args) SK_FIRST_ARG__ args
#define SK_FIRST_ARG(...) SK_FIRST_ARG_((__VA_ARGS__, ))

#if SK_SUPPORT_GPU
#    include "include/gpu/GrDirectContext.h"
#    include "include/gpu/GrBackendSurface.h"
#    include "include/gpu/gl/GrGLInterface.h"
#    include "include/gpu/gl/GrGLAssembleInterface.h"
#    define SK_ONLY_GPU(...) SK_FIRST_ARG(__VA_ARGS__)
#    if SK_VULKAN
#        include "include/gpu/vk/GrVkTypes.h"
#        include "include/gpu/vk/GrVkBackendContext.h"
#        include "include/gpu/vk/GrVkExtensions.h"
#        define SK_ONLY_VULKAN(...) SK_FIRST_ARG(__VA_ARGS__)
#    else
#        define SK_ONLY_VULKAN(...) SK_SKIP_ARG(__VA_ARGS__)
#    endif
#    if SK_METAL
#        define SK_ONLY_METAL(...) SK_FIRST_ARG(__VA_ARGS__)
#    else
#        define SK_ONLY_METAL(...) SK_SKIP_ARG(__VA_ARGS__)
#    endif
#else // !SK_SUPPORT_GPU
#    define SK_ONLY_GPU(...) SK_SKIP_ARG(__VA_ARGS__)
#    define SK_ONLY_VULKAN(...) SK_SKIP_ARG(__VA_ARGS__)
#    define SK_ONLY_METAL(...) SK_SKIP_ARG(__VA_ARGS__)
#endif // SK_SUPPORT_GPU


// Define a mapping between a C++ type and the C type.
//
// Usual Values:
//  - C++  |  SkType   |  SkSomeType
//  - C    |  sk_type  |  sk_some_type_t
//  - Map  |  Name     |  ToSomeType / AsSomeType
//
#define DEF_MAP_DECL(SkType, sk_type, Name, Declaration, Ns)        \
    Declaration;                                                    \
    static inline const Ns::SkType& As##Name(const sk_type& t) {    \
        return reinterpret_cast<const Ns::SkType&>(t);              \
    }                                                               \
    static inline const Ns::SkType* As##Name(const sk_type* t) {    \
        return reinterpret_cast<const Ns::SkType*>(t);              \
    }                                                               \
    static inline Ns::SkType& As##Name(sk_type& t) {                \
        return reinterpret_cast<Ns::SkType&>(t);                    \
    }                                                               \
    static inline Ns::SkType* As##Name(sk_type* t) {                \
        return reinterpret_cast<Ns::SkType*>(t);                    \
    }                                                               \
    static inline const sk_type& To##Name(const Ns::SkType& t) {    \
        return reinterpret_cast<const sk_type&>(t);                 \
    }                                                               \
    static inline const sk_type* To##Name(const Ns::SkType* t) {    \
        return reinterpret_cast<const sk_type*>(t);                 \
    }                                                               \
    static inline sk_type& To##Name(Ns::SkType& t) {                \
        return reinterpret_cast<sk_type&>(t);                       \
    }                                                               \
    static inline sk_type* To##Name(Ns::SkType* t) {                \
        return reinterpret_cast<sk_type*>(t);                       \
    }

#define DEF_CLASS_MAP(SkType, sk_type, Name)                   \
    DEF_MAP_DECL(SkType, sk_type, Name, class SkType, )

#define DEF_CLASS_MAP_WITH_NS(Ns, SkType, sk_type, Name)        \
    DEF_MAP_DECL(SkType, sk_type, Name, class SkType, Ns)

#define DEF_STRUCT_MAP(SkType, sk_type, Name)                  \
    DEF_MAP_DECL(SkType, sk_type, Name, struct SkType, )

#define DEF_MAP(SkType, sk_type, Name)                         \
    DEF_MAP_DECL(SkType, sk_type, Name, ,)

DEF_CLASS_MAP(Sk3DView, sk_3dview_t, 3DView)
DEF_CLASS_MAP(SkBitmap, sk_bitmap_t, Bitmap)
DEF_CLASS_MAP(SkCanvas, sk_canvas_t, Canvas)
DEF_CLASS_MAP(SkCodec, sk_codec_t, Codec)
DEF_CLASS_MAP(SkColorFilter, sk_colorfilter_t, ColorFilter)
DEF_CLASS_MAP(SkColorSpace, sk_colorspace_t, ColorSpace)
DEF_CLASS_MAP(SkColorTable, sk_colortable_t, ColorTable)
DEF_CLASS_MAP(SkData, sk_data_t, Data)
DEF_CLASS_MAP(SkDocument, sk_document_t, Document)
DEF_CLASS_MAP(SkDrawable, sk_drawable_t, Drawable)
DEF_CLASS_MAP(SkDynamicMemoryWStream, sk_wstream_dynamicmemorystream_t, DynamicMemoryWStream)
DEF_CLASS_MAP(SkFILEStream, sk_stream_filestream_t, FileStream)
DEF_CLASS_MAP(SkFILEWStream, sk_wstream_filestream_t, FileWStream)
DEF_CLASS_MAP(SkFont, sk_font_t, Font)
DEF_CLASS_MAP(SkFontMgr, sk_fontmgr_t, FontMgr)
DEF_CLASS_MAP(SkFontStyle, sk_fontstyle_t, FontStyle)
DEF_CLASS_MAP(SkFontStyleSet, sk_fontstyleset_t, FontStyleSet)
DEF_CLASS_MAP(SkImage, sk_image_t, Image)
DEF_CLASS_MAP(SkImageFilter, sk_imagefilter_t, ImageFilter)
DEF_CLASS_MAP(SkMaskFilter, sk_maskfilter_t, MaskFilter)
DEF_CLASS_MAP(SkMatrix44, sk_matrix44_t, Matrix44)
DEF_CLASS_MAP(SkMemoryStream, sk_stream_memorystream_t, MemoryStream)
DEF_CLASS_MAP(SkNWayCanvas, sk_nway_canvas_t, NWayCanvas)
DEF_CLASS_MAP(SkNoDrawCanvas, sk_nodraw_canvas_t, NoDrawCanvas)
DEF_CLASS_MAP(SkOverdrawCanvas, sk_overdraw_canvas_t, OverdrawCanvas)
DEF_CLASS_MAP(SkOpBuilder, sk_opbuilder_t, OpBuilder)
DEF_CLASS_MAP(SkPaint, sk_paint_t, Paint)
DEF_CLASS_MAP(SkPath, sk_path_t, Path)
DEF_CLASS_MAP(SkPathEffect, sk_path_effect_t, PathEffect)
DEF_CLASS_MAP(SkPathMeasure, sk_pathmeasure_t, PathMeasure)
DEF_CLASS_MAP(SkPicture, sk_picture_t, Picture)
DEF_CLASS_MAP(SkPictureRecorder, sk_picture_recorder_t, PictureRecorder)
DEF_CLASS_MAP(SkPixmap, sk_pixmap_t, Pixmap)
DEF_CLASS_MAP(SkRegion, sk_region_t, Region)
DEF_CLASS_MAP(SkRRect, sk_rrect_t, RRect)
DEF_CLASS_MAP(SkRuntimeEffect, sk_runtimeeffect_t, RuntimeEffect)
DEF_CLASS_MAP(SkShader, sk_shader_t, Shader)
DEF_CLASS_MAP(SkStream, sk_stream_t, Stream)
DEF_CLASS_MAP(SkStreamAsset, sk_stream_asset_t, StreamAsset)
DEF_CLASS_MAP(SkStreamRewindable, sk_stream_streamrewindable_t, StreamRewindable)
DEF_CLASS_MAP(SkString, sk_string_t, String)
DEF_CLASS_MAP(SkSurface, sk_surface_t, Surface)
DEF_CLASS_MAP(SkSurfaceProps, sk_surfaceprops_t, SurfaceProps)
DEF_CLASS_MAP(SkTextBlob, sk_textblob_t, TextBlob)
DEF_CLASS_MAP(SkTextBlobBuilder, sk_textblob_builder_t, TextBlobBuilder)
DEF_CLASS_MAP(SkTraceMemoryDump, sk_tracememorydump_t, TraceMemoryDump)
DEF_CLASS_MAP(SkTypeface, sk_typeface_t, Typeface)
DEF_CLASS_MAP(SkVertices, sk_vertices_t, Vertices)
DEF_CLASS_MAP(SkWStream, sk_wstream_t, WStream)
DEF_CLASS_MAP(SkXMLStreamWriter, sk_xmlstreamwriter_t, XMLStreamWriter)
DEF_CLASS_MAP(SkXMLWriter, sk_xmlwriter_t, XMLWriter)

DEF_CLASS_MAP(GrDirectContext, gr_direct_context_t, GrDirectContext)
DEF_CLASS_MAP(GrRecordingContext, gr_recording_context_t, GrRecordingContext)
DEF_CLASS_MAP(GrBackendTexture, gr_backendtexture_t, GrBackendTexture)
DEF_CLASS_MAP(GrBackendRenderTarget, gr_backendrendertarget_t, GrBackendRenderTarget)

DEF_CLASS_MAP(GrVkExtensions, gr_vk_extensions_t, GrVkExtensions)

DEF_STRUCT_MAP(skcms_ICCProfile, sk_colorspace_icc_profile_t, ColorSpaceIccProfile)
DEF_STRUCT_MAP(SkColorSpacePrimaries, sk_colorspace_primaries_t, ColorSpacePrimaries)
DEF_STRUCT_MAP(skcms_TransferFunction, sk_colorspace_transfer_fn_t, ColorSpaceTransferFn)
DEF_STRUCT_MAP(skcms_Matrix3x3, sk_colorspace_xyz_t, ColorSpaceXyz)
DEF_STRUCT_MAP(SkHighContrastConfig, sk_highcontrastconfig_t, HighContrastConfig)
DEF_STRUCT_MAP(SkIPoint, sk_ipoint_t, IPoint)
DEF_STRUCT_MAP(SkIRect, sk_irect_t, IRect)
DEF_STRUCT_MAP(SkISize, sk_isize_t, ISize)
DEF_STRUCT_MAP(SkMask, sk_mask_t, Mask)
DEF_STRUCT_MAP(SkPoint, sk_point_t, Point)
DEF_STRUCT_MAP(SkPoint3, sk_point3_t, Point3)
DEF_STRUCT_MAP(SkRect, sk_rect_t, Rect)
DEF_STRUCT_MAP(SkRSXform, sk_rsxform_t, RSXform)
DEF_STRUCT_MAP(SkSize, sk_size_t, Size)

DEF_STRUCT_MAP(GrGLTextureInfo, gr_gl_textureinfo_t, GrGLTextureInfo)
DEF_STRUCT_MAP(GrGLFramebufferInfo, gr_gl_framebufferinfo_t, GrGLFramebufferInfo)
DEF_STRUCT_MAP(GrGLInterface, gr_glinterface_t, GrGLInterface)

DEF_STRUCT_MAP(GrVkYcbcrConversionInfo, gr_vk_ycbcrconversioninfo_t, GrVkYcbcrConversionInfo)
DEF_STRUCT_MAP(GrVkImageInfo, gr_vk_imageinfo_t, GrVkImageInfo)

#include "include/effects/SkRuntimeEffect.h"
DEF_MAP(SkRuntimeEffect::Uniform, sk_runtimeeffect_uniform_t, RuntimeEffectUniform)

#include "include/core/SkCanvas.h"
DEF_MAP(SkCanvas::Lattice, sk_lattice_t, Lattice)

#include "include/codec/SkCodec.h"
DEF_MAP(SkCodec::FrameInfo, sk_codec_frameinfo_t, FrameInfo)
DEF_MAP(SkCodec::Options, sk_codec_options_t, CodecOptions)

#include "include/core/SkColor.h"
DEF_MAP(SkColor4f, sk_color4f_t, Color4f)

#include "include/core/SkImageFilter.h"
DEF_MAP(SkImageFilter::CropRect, sk_imagefilter_croprect_t, ImageFilterCropRect)

#include "include/encode/SkJpegEncoder.h"
DEF_MAP(SkJpegEncoder::Options, sk_jpegencoder_options_t, JpegEncoderOptions)

#include "include/core/SkFontMetrics.h"
DEF_MAP(SkFontMetrics, sk_fontmetrics_t, FontMetrics)

#include "include/core/SkPath.h"
DEF_MAP(SkPath::Iter, sk_path_iterator_t, PathIter)
DEF_MAP(SkPath::RawIter, sk_path_rawiterator_t, PathRawIter)

#include "include/encode/SkPngEncoder.h"
DEF_MAP(SkPngEncoder::Options, sk_pngencoder_options_t, PngEncoderOptions)

#include "include/core/SkRegion.h"
DEF_MAP(SkRegion::Iterator, sk_region_iterator_t, RegionIterator)
DEF_MAP(SkRegion::Cliperator, sk_region_cliperator_t, RegionCliperator)
DEF_MAP(SkRegion::Spanerator, sk_region_spanerator_t, RegionSpanerator)

#include "include/core/SkTime.h"
DEF_MAP(SkTime::DateTime, sk_time_datetime_t, TimeDateTime)

#include "include/encode/SkWebpEncoder.h"
DEF_MAP(SkWebpEncoder::Options, sk_webpencoder_options_t, WebpEncoderOptions)

#include "include/core/SkMatrix.h"
static inline SkMatrix AsMatrix(const sk_matrix_t* matrix) {
    return SkMatrix::MakeAll(
        matrix->scaleX, matrix->skewX,  matrix->transX,
        matrix->skewY,  matrix->scaleY, matrix->transY,
        matrix->persp0, matrix->persp1, matrix->persp2);
}
static inline sk_matrix_t ToMatrix(const SkMatrix* matrix) {
    sk_matrix_t m;
    m.scaleX = matrix->get(SkMatrix::kMScaleX);
    m.skewX  = matrix->get(SkMatrix::kMSkewX);
    m.transX = matrix->get(SkMatrix::kMTransX);
    m.skewY  = matrix->get(SkMatrix::kMSkewY);
    m.scaleY = matrix->get(SkMatrix::kMScaleY);
    m.transY = matrix->get(SkMatrix::kMTransY);
    m.persp0 = matrix->get(SkMatrix::kMPersp0);
    m.persp1 = matrix->get(SkMatrix::kMPersp1);
    m.persp2 = matrix->get(SkMatrix::kMPersp2);
    return m;
}
static inline sk_matrix_t ToMatrix(const SkMatrix& matrix) {
    sk_matrix_t m;
    m.scaleX = matrix.get(SkMatrix::kMScaleX);
    m.skewX  = matrix.get(SkMatrix::kMSkewX);
    m.transX = matrix.get(SkMatrix::kMTransX);
    m.skewY  = matrix.get(SkMatrix::kMSkewY);
    m.scaleY = matrix.get(SkMatrix::kMScaleY);
    m.transY = matrix.get(SkMatrix::kMTransY);
    m.persp0 = matrix.get(SkMatrix::kMPersp0);
    m.persp1 = matrix.get(SkMatrix::kMPersp1);
    m.persp2 = matrix.get(SkMatrix::kMPersp2);
    return m;
}

#include "include/core/SkImageInfo.h"
static inline SkImageInfo AsImageInfo(const sk_imageinfo_t* info) {
    return SkImageInfo::Make(
        info->width,
        info->height,
        (SkColorType)info->colorType,
        (SkAlphaType)info->alphaType,
        sk_ref_sp(AsColorSpace(info->colorspace))); 
}
static inline sk_imageinfo_t ToImageInfo(const SkImageInfo info) {
    return {
        ToColorSpace(info.refColorSpace().release()),
        info.width(),
        info.height(),
        (sk_colortype_t)info.colorType(),
        (sk_alphatype_t)info.alphaType(),
    };
}
static inline const SkImageInfo* ToImageInfo(const sk_imageinfo_t* cinfo) {
    return reinterpret_cast<const SkImageInfo*>(cinfo);
}
static inline sk_imageinfo_t* AsImageInfo(SkImageInfo* info) {
    return reinterpret_cast<sk_imageinfo_t*>(info);
}

#include "include/core/SkTextBlob.h"
static inline SkTextBlobBuilder::RunBuffer AsTextBlobBuilderRunBuffer(const sk_textblob_builder_runbuffer_t* runbuffer) {
    return {
        (SkGlyphID*)runbuffer->glyphs,
        (SkScalar*)runbuffer->pos,
        (char*)runbuffer->utf8text,
        (uint32_t*)runbuffer->clusters,
    };
}
static inline sk_textblob_builder_runbuffer_t ToTextBlobBuilderRunBuffer(const SkTextBlobBuilder::RunBuffer runbuffer) {
    return {
        runbuffer.glyphs,
        runbuffer.pos,
        runbuffer.utf8text,
        runbuffer.clusters,
    };
}

#include "include/docs/SkPDFDocument.h"
static inline SkTime::DateTime AsDocumentOptionalTimestamp(const sk_time_datetime_t* datetime) {
    if (datetime) {
        return *AsTimeDateTime(datetime);
    } else {
        return SkTime::DateTime();
    }
}
static inline SkString AsDocumentOptionalString(const sk_string_t* skstring) {
    if (skstring) {
        return *AsString(skstring);
    } else {
        return SkString();
    }
}
static inline SkPDF::Metadata AsDocumentPDFMetadata(const sk_document_pdf_metadata_t* metadata) {
    SkPDF::Metadata md;
    md.fTitle = AsDocumentOptionalString(metadata->fTitle);
    md.fAuthor = AsDocumentOptionalString(metadata->fAuthor);
    md.fSubject = AsDocumentOptionalString(metadata->fSubject);
    md.fKeywords = AsDocumentOptionalString(metadata->fKeywords);
    md.fCreator = AsDocumentOptionalString(metadata->fCreator);
    md.fProducer = AsDocumentOptionalString(metadata->fProducer);
    md.fCreation = AsDocumentOptionalTimestamp(metadata->fCreation);
    md.fModified = AsDocumentOptionalTimestamp(metadata->fModified);
    md.fRasterDPI = metadata->fRasterDPI;
    md.fPDFA = metadata->fPDFA;
    md.fEncodingQuality = metadata->fEncodingQuality;
    return md;
}

#include "modules/skottie/include/Skottie.h"
DEF_CLASS_MAP_WITH_NS(skottie, Animation, skottie_animation_t, SkottieAnimation)

DEF_CLASS_MAP_WITH_NS(skottie, ResourceProvider, skottie_resource_provider_t, SkottieResourceProvider)
DEF_CLASS_MAP_WITH_NS(skottie, PropertyObserver, skottie_property_observer_t, SkottiePropertyObserver)
DEF_CLASS_MAP_WITH_NS(skottie, Logger, skottie_logger_t, SkottieLogger)
DEF_CLASS_MAP_WITH_NS(skottie, MarkerObserver, skottie_marker_observer_t, SkottieMarkerObserver)

#include "modules/sksg/include/SkSGInvalidationController.h"
DEF_CLASS_MAP_WITH_NS(sksg, InvalidationController, sksg_invalidation_controller_t, SksgInvalidationController)

#if SK_SUPPORT_GPU
// GPU specific

static inline GrContextOptions AsGrContextOptions(const gr_context_options_t* options) {
    GrContextOptions opts;
    opts.fAllowPathMaskCaching = options->fAllowPathMaskCaching;
    opts.fAvoidStencilBuffers = options->fAvoidStencilBuffers;
    opts.fBufferMapThreshold = options->fBufferMapThreshold;
    opts.fDoManualMipmapping = options->fDoManualMipmapping;
    opts.fGlyphCacheTextureMaximumBytes = options->fGlyphCacheTextureMaximumBytes;
    opts.fRuntimeProgramCacheSize = options->fRuntimeProgramCacheSize;
    return opts;
}

#if SK_VULKAN
#define DEF_MAP_VK(VkType, vk_type)                 \
    static inline VkType As##VkType(vk_type* t) {   \
        return reinterpret_cast<VkType>(t);         \
    }                                               \
    static inline vk_type* To##VkType(VkType t) {   \
        return reinterpret_cast<vk_type*>(t);       \
    }

DEF_MAP_VK(VkInstance, vk_instance_t);
DEF_MAP_VK(VkDevice, vk_device_t);
DEF_MAP_VK(VkPhysicalDevice, vk_physical_device_t);
DEF_MAP_VK(VkQueue, vk_queue_t);
DEF_MAP(VkPhysicalDeviceFeatures, vk_physical_device_features_t, VkPhysicalDeviceFeatures);
DEF_MAP(VkPhysicalDeviceFeatures2, vk_physical_device_features_2_t, VkPhysicalDeviceFeatures2);
DEF_MAP(GrVkMemoryAllocator, gr_vk_memory_allocator_t, GrVkMemoryAllocator);

static inline GrVkBackendContext AsGrVkBackendContext(const gr_vk_backendcontext_t* context) {
    GrVkBackendContext ctx;
    ctx.fInstance = AsVkInstance(context->fInstance);
    ctx.fPhysicalDevice = AsVkPhysicalDevice(context->fPhysicalDevice);
    ctx.fDevice = AsVkDevice(context->fDevice);
    ctx.fQueue = AsVkQueue(context->fQueue);
    ctx.fGraphicsQueueIndex = context->fGraphicsQueueIndex;
    ctx.fMinAPIVersion = context->fMinAPIVersion;
    ctx.fInstanceVersion = context->fInstanceVersion;
    ctx.fMaxAPIVersion = context->fMaxAPIVersion;
    ctx.fExtensions = context->fExtensions;
    ctx.fVkExtensions = AsGrVkExtensions(context->fVkExtensions);
    ctx.fFeatures = context->fFeatures;
    ctx.fDeviceFeatures = AsVkPhysicalDeviceFeatures(context->fDeviceFeatures);
    ctx.fDeviceFeatures2 = AsVkPhysicalDeviceFeatures2(context->fDeviceFeatures2);
    ctx.fMemoryAllocator = sk_ref_sp(AsGrVkMemoryAllocator(context->fMemoryAllocator));
    if (context->fGetProc != nullptr) {
        ctx.fGetProc = [context](const char* name, VkInstance instance, VkDevice device) -> PFN_vkVoidFunction {
            return context->fGetProc(context->fGetProcUserData, name, ToVkInstance(instance), ToVkDevice(device));
        };
    }
    ctx.fOwnsInstanceAndDevice = context->fOwnsInstanceAndDevice;
    ctx.fProtectedContext = context->fProtectedContext ? GrProtected::kYes : GrProtected::kNo;
    return ctx;
}

#endif // SK_VULKAN

#if SK_METAL

static inline GrMtlTextureInfo AsGrMtlTextureInfo(const gr_mtl_textureinfo_t* mtlInfo) {
    GrMtlTextureInfo info;
    info.fTexture.retain(mtlInfo->fTexture);
    return info;
}

#endif // SK_METAL

#endif // SK_SUPPORT_GPU

#endif

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES


#include "GrAHardwareBufferImageGenerator.h"

#include <android/hardware_buffer.h>

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrResourceProviderPriv.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "SkMessageBus.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLTypes.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#ifdef SK_VULKAN
#include "vk/GrVkExtensions.h"
#include "vk/GrVkGpu.h"
#endif

#define PROT_CONTENT_EXT_STR "EGL_EXT_protected_content"
#define EGL_PROTECTED_CONTENT_EXT 0x32C0

static bool can_import_protected_content_eglimpl() {
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    const char* exts = eglQueryString(dpy, EGL_EXTENSIONS);
    size_t cropExtLen = strlen(PROT_CONTENT_EXT_STR);
    size_t extsLen = strlen(exts);
    bool equal = !strcmp(PROT_CONTENT_EXT_STR, exts);
    bool atStart = !strncmp(PROT_CONTENT_EXT_STR " ", exts, cropExtLen+1);
    bool atEnd = (cropExtLen+1) < extsLen
                  && !strcmp(" " PROT_CONTENT_EXT_STR,
                  exts + extsLen - (cropExtLen+1));
    bool inMiddle = strstr(exts, " " PROT_CONTENT_EXT_STR " ");
    return equal || atStart || atEnd || inMiddle;
}

static bool can_import_protected_content(GrContext* context) {
    if (GrBackendApi::kOpenGL == context->backend()) {
        // Only compute whether the extension is present once the first time this
        // function is called.
        static bool hasIt = can_import_protected_content_eglimpl();
        return hasIt;
    }
    return false;
}

std::unique_ptr<SkImageGenerator> GrAHardwareBufferImageGenerator::Make(
        AHardwareBuffer* graphicBuffer, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace,
        GrSurfaceOrigin surfaceOrigin) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(graphicBuffer, &bufferDesc);
    SkColorType colorType;
    switch (bufferDesc.format) {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
    case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
        colorType = kRGBA_8888_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
        colorType = kRGBA_F16_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
        colorType = kRGB_565_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
        colorType = kRGB_888x_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
        colorType = kRGBA_1010102_SkColorType;
        break;
    default:
        // Given that we only use this texture as a source, colorType will not impact how Skia uses
        // the texture.  The only potential affect this is anticipated to have is that for some
        // format types if we are not bound as an OES texture we may get invalid results for SKP
        // capture if we read back the texture.
        colorType = kRGBA_8888_SkColorType;
        break;
    }
    SkImageInfo info = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType,
                                         alphaType, std::move(colorSpace));
    bool createProtectedImage = 0 != (bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);
    return std::unique_ptr<SkImageGenerator>(new GrAHardwareBufferImageGenerator(
            info, graphicBuffer, alphaType, createProtectedImage,
            bufferDesc.format, surfaceOrigin));
}

GrAHardwareBufferImageGenerator::GrAHardwareBufferImageGenerator(const SkImageInfo& info,
        AHardwareBuffer* hardwareBuffer, SkAlphaType alphaType, bool isProtectedContent,
        uint32_t bufferFormat, GrSurfaceOrigin surfaceOrigin)
    : INHERITED(info)
    , fHardwareBuffer(hardwareBuffer)
    , fBufferFormat(bufferFormat)
    , fIsProtectedContent(isProtectedContent)
    , fSurfaceOrigin(surfaceOrigin) {
    AHardwareBuffer_acquire(fHardwareBuffer);
}

GrAHardwareBufferImageGenerator::~GrAHardwareBufferImageGenerator() {
    AHardwareBuffer_release(fHardwareBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_VULKAN

class VulkanCleanupHelper {
public:
    VulkanCleanupHelper(GrVkGpu* gpu, VkImage image, VkDeviceMemory memory)
        : fDevice(gpu->device())
        , fImage(image)
        , fMemory(memory)
        , fDestroyImage(gpu->vkInterface()->fFunctions.fDestroyImage)
        , fFreeMemory(gpu->vkInterface()->fFunctions.fFreeMemory) {}
    ~VulkanCleanupHelper() {
        fDestroyImage(fDevice, fImage, nullptr);
        fFreeMemory(fDevice, fMemory, nullptr);
    }
private:
    VkDevice           fDevice;
    VkImage            fImage;
    VkDeviceMemory     fMemory;
    PFN_vkDestroyImage fDestroyImage;
    PFN_vkFreeMemory   fFreeMemory;
};

void GrAHardwareBufferImageGenerator::DeleteVkImage(void* context) {
    VulkanCleanupHelper* cleanupHelper = static_cast<VulkanCleanupHelper*>(context);
    delete cleanupHelper;
}

#define VK_CALL(X) gpu->vkInterface()->fFunctions.f##X;

static GrBackendTexture make_vk_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height, GrPixelConfig config,
        GrAHardwareBufferImageGenerator::DeleteImageProc* deleteProc,
        GrAHardwareBufferImageGenerator::DeleteImageCtx* deleteCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat) {
    SkASSERT(context->backend() == GrBackendApi::kVulkan);
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->contextPriv().getGpu());

    VkPhysicalDevice physicalDevice = gpu->physicalDevice();
    VkDevice device = gpu->device();

    SkASSERT(gpu);

    if (!gpu->vkCaps().supportsAndroidHWBExternalMemory()) {
        return GrBackendTexture();
    }

    SkASSERT(backendFormat.getVkFormat());
    VkFormat format = *backendFormat.getVkFormat();

    VkResult err;

    VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
    hwbFormatProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    hwbFormatProps.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    hwbProps.pNext = &hwbFormatProps;

    err = VK_CALL(GetAndroidHardwareBufferProperties(device, hardwareBuffer, &hwbProps));
    if (VK_SUCCESS != err) {
        return GrBackendTexture();
    }

    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = 0;  // If this is zero it is as if we aren't using this struct.

    const GrVkYcbcrConversionInfo* ycbcrConversion = backendFormat.getVkYcbcrConversionInfo();
    if (!ycbcrConversion) {
        return GrBackendTexture();
    }

    if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
        // TODO: We should not assume the transfer features here and instead should have a way for
        // Ganesh's tracking of intenral images to report whether or not they support transfers.
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(!ycbcrConversion->isValid());
    } else {
        SkASSERT(ycbcrConversion->isValid());
        // We have an external only format
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(format == VK_FORMAT_UNDEFINED);
        SkASSERT(hwbFormatProps.externalFormat == ycbcrConversion->fExternalFormat);
        externalFormat.externalFormat = hwbFormatProps.externalFormat;
    }
    SkASSERT(format == hwbFormatProps.format);

    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo{
            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,                 // sType
            &externalFormat,                                                     // pNext
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,  // handleTypes
    };
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (format != VK_FORMAT_UNDEFINED) {
        usageFlags = usageFlags |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // TODO: Check the supported tilings vkGetPhysicalDeviceImageFormatProperties2 to see if we have
    // to use linear. Add better linear support throughout Ganesh.
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        &externalMemoryImageInfo,                    // pNext
        0,                                           // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        format,                                      // VkFormat
        { (uint32_t)width, (uint32_t)height, 1 },    // VkExtent3D
        1,                                           // mipLevels
        1,                                           // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                       // samples
        tiling,                                      // VkImageTiling
        usageFlags,                                  // VkImageUsageFlags
        VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode
        0,                                           // queueFamilyCount
        0,                                           // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,                   // initialLayout
    };

    VkImage image;
    err = VK_CALL(CreateImage(device, &imageCreateInfo, nullptr, &image));
    if (VK_SUCCESS != err) {
        return GrBackendTexture();
    }

    VkPhysicalDeviceMemoryProperties2 phyDevMemProps;
    phyDevMemProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    phyDevMemProps.pNext = nullptr;

    uint32_t typeIndex = 0;
    uint32_t heapIndex = 0;
    bool foundHeap = false;
    VK_CALL(GetPhysicalDeviceMemoryProperties2(physicalDevice, &phyDevMemProps));
    uint32_t memTypeCnt = phyDevMemProps.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < memTypeCnt && !foundHeap; ++i) {
        if (hwbProps.memoryTypeBits & (1 << i)) {
            const VkPhysicalDeviceMemoryProperties& pdmp = phyDevMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                typeIndex = i;
                heapIndex = pdmp.memoryTypes[i].heapIndex;
                foundHeap = true;
            }
        }
    }
    if (!foundHeap) {
        VK_CALL(DestroyImage(device, image, nullptr));
        return GrBackendTexture();
    }

    VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
    hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    hwbImportInfo.pNext = nullptr;
    hwbImportInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &hwbImportInfo;
    dedicatedAllocInfo.image = image;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        &dedicatedAllocInfo,                         // pNext
        hwbProps.allocationSize,                     // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    VkDeviceMemory memory;

    err = VK_CALL(AllocateMemory(device, &allocInfo, nullptr, &memory));
    if (VK_SUCCESS != err) {
        VK_CALL(DestroyImage(device, image, nullptr));
        return GrBackendTexture();
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = memory;
    bindImageInfo.memoryOffset = 0;

    err = VK_CALL(BindImageMemory2(device, 1, &bindImageInfo));
    if (VK_SUCCESS != err) {
        VK_CALL(DestroyImage(device, image, nullptr));
        VK_CALL(FreeMemory(device, memory, nullptr));
        return GrBackendTexture();
    }

    GrVkImageInfo imageInfo;

    imageInfo.fImage = image;
    imageInfo.fAlloc = GrVkAlloc(memory, 0, hwbProps.allocationSize, 0);
    imageInfo.fImageTiling = tiling;
    imageInfo.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.fFormat = format;
    imageInfo.fLevelCount = 1;
    // TODO: This should possibly be VK_QUEUE_FAMILY_FOREIGN_EXT but current Adreno devices do not
    // support that extension. Or if we know the source of the AHardwareBuffer is not from a
    // "foreign" device we can leave them as external.
    imageInfo.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    imageInfo.fYcbcrConversionInfo = *ycbcrConversion;

    *deleteProc = GrAHardwareBufferImageGenerator::DeleteVkImage;
    *deleteCtx = new VulkanCleanupHelper(gpu, image, memory);

    return GrBackendTexture(width, height, imageInfo);
}
#endif

class GLCleanupHelper {
public:
    GLCleanupHelper(GrGLuint texID, EGLImageKHR image, EGLDisplay display)
        : fTexID(texID)
        , fImage(image)
        , fDisplay(display) { }
    ~GLCleanupHelper() {
        glDeleteTextures(1, &fTexID);
        // eglDestroyImageKHR will remove a ref from the AHardwareBuffer
        eglDestroyImageKHR(fDisplay, fImage);
    }
private:
    GrGLuint    fTexID;
    EGLImageKHR fImage;
    EGLDisplay  fDisplay;
};

void GrAHardwareBufferImageGenerator::DeleteGLTexture(void* context) {
    GLCleanupHelper* cleanupHelper = static_cast<GLCleanupHelper*>(context);
    delete cleanupHelper;
}

static GrBackendTexture make_gl_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height, GrPixelConfig config,
        GrAHardwareBufferImageGenerator::DeleteImageProc* deleteProc,
        GrAHardwareBufferImageGenerator::DeleteImageCtx* deleteCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat) {
    while (GL_NO_ERROR != glGetError()) {} //clear GL errors

    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(hardwareBuffer);
    EGLint attribs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                         isProtectedContent ? EGL_PROTECTED_CONTENT_EXT : EGL_NONE,
                         isProtectedContent ? EGL_TRUE : EGL_NONE,
                         EGL_NONE };
    EGLDisplay display = eglGetCurrentDisplay();
    // eglCreateImageKHR will add a ref to the AHardwareBuffer
    EGLImageKHR image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                          clientBuffer, attribs);
    if (EGL_NO_IMAGE_KHR == image) {
        SkDebugf("Could not create EGL image, err = (%#x)", (int) eglGetError() );
        return GrBackendTexture();
    }

    GrGLuint texID;
    glGenTextures(1, &texID);
    if (!texID) {
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    context->resetContext(kTextureBinding_GrGLBackendState);

    GrGLTextureInfo textureInfo;
    textureInfo.fID = texID;
    SkASSERT(backendFormat.isValid());
    textureInfo.fTarget = *backendFormat.getGLTarget();
    textureInfo.fFormat = *backendFormat.getGLFormat();

    *deleteProc = GrAHardwareBufferImageGenerator::DeleteGLTexture;
    *deleteCtx = new GLCleanupHelper(texID, image, display);

    return GrBackendTexture(width, height, GrMipMapped::kNo, textureInfo);
}

static GrBackendTexture make_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height, GrPixelConfig config,
        GrAHardwareBufferImageGenerator::DeleteImageProc* deleteProc,
        GrAHardwareBufferImageGenerator::DeleteImageCtx* deleteCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat) {
    if (context->abandoned()) {
        return GrBackendTexture();
    }
    bool createProtectedImage = isProtectedContent && can_import_protected_content(context);

    if (GrBackendApi::kOpenGL == context->backend()) {
        return make_gl_backend_texture(context, hardwareBuffer, width, height, config, deleteProc,
                                       deleteCtx, createProtectedImage, backendFormat);
    } else {
        SkASSERT(GrBackendApi::kVulkan == context->backend());
#ifdef SK_VULKAN
        // Currently we don't support protected images on vulkan
        SkASSERT(!createProtectedImage);
        return make_vk_backend_texture(context, hardwareBuffer, width, height, config, deleteProc,
                                       deleteCtx, createProtectedImage, backendFormat);
#else
        return GrBackendTexture();
#endif
    }
}

GrBackendFormat get_backend_format(GrContext* context, AHardwareBuffer* hardwareBuffer,
                                   GrBackendApi backend, uint32_t bufferFormat) {
    if (backend == GrBackendApi::kOpenGL) {
        switch (bufferFormat) {
            //TODO: find out if we can detect, which graphic buffers support GR_GL_TEXTURE_2D
            case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
                return GrBackendFormat::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGB565, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGB10_A2, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_EXTERNAL);
            default:
                return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
        }
    } else if (backend == GrBackendApi::kVulkan) {
#ifdef SK_VULKAN
        switch (bufferFormat) {
            case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
                return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
            case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
                return GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT);
            case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
                return GrBackendFormat::MakeVk(VK_FORMAT_R5G6B5_UNORM_PACK16);
            case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
                return GrBackendFormat::MakeVk(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
            case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
                return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
            case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
                return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8_UNORM);
            default: {
                GrVkGpu* gpu = static_cast<GrVkGpu*>(context->contextPriv().getGpu());
                SkASSERT(gpu);
                VkDevice device = gpu->device();

                if (!gpu->vkCaps().supportsAndroidHWBExternalMemory()) {
                    return GrBackendFormat();
                }
                VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
                hwbFormatProps.sType =
                        VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
                hwbFormatProps.pNext = nullptr;

                VkAndroidHardwareBufferPropertiesANDROID hwbProps;
                hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
                hwbProps.pNext = &hwbFormatProps;

                VkResult err = VK_CALL(GetAndroidHardwareBufferProperties(device, hardwareBuffer,
                                                                          &hwbProps));
                if (VK_SUCCESS != err) {
                    return GrBackendFormat();
                }

                if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
                    return GrBackendFormat();
                }

                GrVkYcbcrConversionInfo ycbcrConversion;
                ycbcrConversion.fYcbcrModel = hwbFormatProps.suggestedYcbcrModel;
                ycbcrConversion.fYcbcrRange = hwbFormatProps.suggestedYcbcrRange;
                ycbcrConversion.fXChromaOffset = hwbFormatProps.suggestedXChromaOffset;
                ycbcrConversion.fYChromaOffset = hwbFormatProps.suggestedYChromaOffset;
                ycbcrConversion.fForceExplicitReconstruction = VK_FALSE;
                ycbcrConversion.fExternalFormat = hwbFormatProps.externalFormat;
                ycbcrConversion.fExternalFormatFeatures = hwbFormatProps.formatFeatures;
                if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT &
                    hwbFormatProps.formatFeatures) {
                    ycbcrConversion.fChromaFilter = VK_FILTER_LINEAR;
                } else {
                    ycbcrConversion.fChromaFilter = VK_FILTER_NEAREST;
                }

                return GrBackendFormat::MakeVk(ycbcrConversion);
            }
        }
#else
        return GrBackendFormat();
#endif
    }
    return GrBackendFormat();
}

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::makeProxy(GrContext* context) {
    if (context->abandoned()) {
        return nullptr;
    }

    GrBackendFormat backendFormat = get_backend_format(context, fHardwareBuffer,
                                                       context->backend(),
                                                       fBufferFormat);
    GrPixelConfig pixelConfig = context->contextPriv().caps()->getConfigFromBackendFormat(
            backendFormat, this->getInfo().colorType());

    if (pixelConfig == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    int width = this->getInfo().width();
    int height = this->getInfo().height();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = pixelConfig;

    GrTextureType textureType = GrTextureType::k2D;
    if (context->backend() == GrBackendApi::kOpenGL) {
        textureType = GrTextureType::kExternal;
    } else if (context->backend() == GrBackendApi::kVulkan) {
        const VkFormat* format = backendFormat.getVkFormat();
        SkASSERT(format);
        if (*format == VK_FORMAT_UNDEFINED) {
            textureType = GrTextureType::kExternal;
        }
    }

    auto proxyProvider = context->contextPriv().proxyProvider();

    AHardwareBuffer* hardwareBuffer = fHardwareBuffer;
    AHardwareBuffer_acquire(hardwareBuffer);

    const bool isProtectedContent = fIsProtectedContent;

    sk_sp<GrTextureProxy> texProxy = proxyProvider->createLazyProxy(
            [context, hardwareBuffer, width, height, pixelConfig, isProtectedContent,
             backendFormat](GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    AHardwareBuffer_release(hardwareBuffer);
                    return sk_sp<GrTexture>();
                }

                DeleteImageProc deleteImageProc = nullptr;
                DeleteImageCtx deleteImageCtx = nullptr;

                GrBackendTexture backendTex = make_backend_texture(context, hardwareBuffer,
                                                                   width, height, pixelConfig,
                                                                   &deleteImageProc,
                                                                   &deleteImageCtx,
                                                                   isProtectedContent,
                                                                   backendFormat);
                if (!backendTex.isValid()) {
                    return sk_sp<GrTexture>();
                }
                SkASSERT(deleteImageProc && deleteImageCtx);

                backendTex.fConfig = pixelConfig;
                // We make this texture cacheable to avoid recreating a GrTexture every time this
                // is invoked. We know the owning SkIamge will send an invalidation message when the
                // image is destroyed, so the texture will be removed at that time.
                sk_sp<GrTexture> tex = resourceProvider->wrapBackendTexture(
                        backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kYes, kRead_GrIOType);
                if (!tex) {
                    deleteImageProc(deleteImageCtx);
                    return sk_sp<GrTexture>();
                }

                if (deleteImageProc) {
                    sk_sp<GrReleaseProcHelper> releaseProcHelper(
                            new GrReleaseProcHelper(deleteImageProc, deleteImageCtx));
                    tex->setRelease(releaseProcHelper);
                }

                return tex;
            },
            backendFormat, desc, fSurfaceOrigin, GrMipMapped::kNo,
            GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact, SkBudgeted::kNo);

    if (!texProxy) {
        AHardwareBuffer_release(hardwareBuffer);
    }
    return texProxy;
}

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin, bool willNeedMipMaps) {
    sk_sp<GrTextureProxy> texProxy = this->makeProxy(context);
    if (!texProxy) {
        return nullptr;
    }

    if (0 == origin.fX && 0 == origin.fY &&
        info.width() == this->getInfo().width() && info.height() == this->getInfo().height()) {
        // If the caller wants the full texture we're done. The caller will handle making a copy for
        // mip maps if that is required.
        return texProxy;
    }
    // Otherwise, make a copy for the requested subset.
    SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());

    GrMipMapped mipMapped = willNeedMipMaps ? GrMipMapped::kYes : GrMipMapped::kNo;

    return GrSurfaceProxy::Copy(context, texProxy.get(), mipMapped, subset, SkBackingFit::kExact,
                                SkBudgeted::kYes);
}

bool GrAHardwareBufferImageGenerator::onIsValid(GrContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    return GrBackendApi::kOpenGL == context->backend() ||
           GrBackendApi::kVulkan == context->backend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK

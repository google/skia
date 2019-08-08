/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#include "src/gpu/GrAHardwareBufferUtils.h"

#include <android/hardware_buffer.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#include "include/gpu/GrContext.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkCaps.h"
#include "src/gpu/vk/GrVkGpu.h"
#endif

#define PROT_CONTENT_EXT_STR "EGL_EXT_protected_content"
#define EGL_PROTECTED_CONTENT_EXT 0x32C0

#define VK_CALL(X) gpu->vkInterface()->fFunctions.f##X;

namespace GrAHardwareBufferUtils {

SkColorType GetSkColorTypeFromBufferFormat(uint32_t bufferFormat) {
    switch (bufferFormat) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            return kRGBA_8888_SkColorType;
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            return kRGB_888x_SkColorType;
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            return kRGBA_F16_SkColorType;
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            return kRGB_565_SkColorType;
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            return kRGB_888x_SkColorType;
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            return kRGBA_1010102_SkColorType;
        default:
            // Given that we only use this texture as a source, colorType will not impact how Skia
            // uses the texture.  The only potential affect this is anticipated to have is that for
            // some format types if we are not bound as an OES texture we may get invalid results
            // for SKP capture if we read back the texture.
            return kRGBA_8888_SkColorType;
    }
}

GrBackendFormat GetBackendFormat(GrContext* context, AHardwareBuffer* hardwareBuffer,
                                 uint32_t bufferFormat, bool requireKnownFormat) {
    GrBackendApi backend = context->backend();

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
                if (requireKnownFormat) {
                    return GrBackendFormat();
                } else {
                    return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
                }
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
                if (requireKnownFormat) {
                    return GrBackendFormat();
                } else {
                    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->priv().getGpu());
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

                    VkResult err = VK_CALL(GetAndroidHardwareBufferProperties(device,
                                                                              hardwareBuffer,
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
        }
#else
        return GrBackendFormat();
#endif
    }
    return GrBackendFormat();
}

class GLTextureHelper {
public:
    GLTextureHelper(GrGLuint texID, EGLImageKHR image, EGLDisplay display, GrGLuint texTarget)
        : fTexID(texID)
        , fImage(image)
        , fDisplay(display)
        , fTexTarget(texTarget) { }
    ~GLTextureHelper() {
        glDeleteTextures(1, &fTexID);
        // eglDestroyImageKHR will remove a ref from the AHardwareBuffer
        eglDestroyImageKHR(fDisplay, fImage);
    }
    void rebind(GrContext* grContext);

private:
    GrGLuint    fTexID;
    EGLImageKHR fImage;
    EGLDisplay  fDisplay;
    GrGLuint    fTexTarget;
};

void GLTextureHelper::rebind(GrContext* grContext) {
    glBindTexture(fTexTarget, fTexID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture(%#x, %d) failed (%#x)", (int) fTexTarget,
            (int) fTexID, (int) status);
        return;
    }
    glEGLImageTargetTexture2DOES(fTexTarget, fImage);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", (int) status);
        return;
    }
    grContext->resetContext(kTextureBinding_GrGLBackendState);
}

void delete_gl_texture(void* context) {
    GLTextureHelper* cleanupHelper = static_cast<GLTextureHelper*>(context);
    delete cleanupHelper;
}

void update_gl_texture(void* context, GrContext* grContext) {
    GLTextureHelper* cleanupHelper = static_cast<GLTextureHelper*>(context);
    cleanupHelper->rebind(grContext);
}

static GrBackendTexture make_gl_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height,
        DeleteImageProc* deleteProc,
        UpdateImageProc* updateProc,
        TexImageCtx* imageCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat,
        bool isRenderable) {
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

    GrGLuint target = isRenderable ? GR_GL_TEXTURE_2D : GR_GL_TEXTURE_EXTERNAL;

    glBindTexture(target, texID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    glEGLImageTargetTexture2DOES(target, image);
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
    textureInfo.fTarget = target;
    textureInfo.fFormat = GrGLFormatToEnum(backendFormat.asGLFormat());

    *deleteProc = delete_gl_texture;
    *updateProc = update_gl_texture;
    *imageCtx = new GLTextureHelper(texID, image, display, target);

    return GrBackendTexture(width, height, GrMipMapped::kNo, textureInfo);
}

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

void delete_vk_image(void* context) {
    VulkanCleanupHelper* cleanupHelper = static_cast<VulkanCleanupHelper*>(context);
    delete cleanupHelper;
}

void update_vk_image(void* context, GrContext* grContext) {
    // no op
}

static GrBackendTexture make_vk_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height,
        DeleteImageProc* deleteProc,
        UpdateImageProc* updateProc,
        TexImageCtx* imageCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat,
        bool isRenderable) {
    SkASSERT(context->backend() == GrBackendApi::kVulkan);
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->priv().getGpu());

    VkPhysicalDevice physicalDevice = gpu->physicalDevice();
    VkDevice device = gpu->device();

    SkASSERT(gpu);

    if (!gpu->vkCaps().supportsAndroidHWBExternalMemory()) {
        return GrBackendTexture();
    }

    VkFormat format;
    SkAssertResult(backendFormat.asVkFormat(&format));

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
        if (isRenderable) {
            usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
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

    *deleteProc = delete_vk_image;
    *updateProc = update_vk_image;
    *imageCtx = new VulkanCleanupHelper(gpu, image, memory);

    return GrBackendTexture(width, height, imageInfo);
}
#endif

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

GrBackendTexture MakeBackendTexture(GrContext* context, AHardwareBuffer* hardwareBuffer,
                                    int width, int height,
                                    DeleteImageProc* deleteProc,
                                    UpdateImageProc* updateProc,
                                    TexImageCtx* imageCtx,
                                    bool isProtectedContent,
                                    const GrBackendFormat& backendFormat,
                                    bool isRenderable) {
    if (context->abandoned()) {
        return GrBackendTexture();
    }
    bool createProtectedImage = isProtectedContent && can_import_protected_content(context);

    if (GrBackendApi::kOpenGL == context->backend()) {
        return make_gl_backend_texture(context, hardwareBuffer, width, height, deleteProc,
                                       updateProc, imageCtx, createProtectedImage, backendFormat,
                                       isRenderable);
    } else {
        SkASSERT(GrBackendApi::kVulkan == context->backend());
#ifdef SK_VULKAN
        // Currently we don't support protected images on vulkan
        SkASSERT(!createProtectedImage);
        return make_vk_backend_texture(context, hardwareBuffer, width, height, deleteProc,
                                       updateProc, imageCtx, createProtectedImage, backendFormat,
                                       isRenderable);
#else
        return GrBackendTexture();
#endif
    }
}

} // GrAHardwareBufferUtils

#endif


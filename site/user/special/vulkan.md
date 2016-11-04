Vulkan
======

Skis has a Vulkan implementation of its GPU backend. The Vulkan backend can be built alongside the OpenGL backend. The client can select between the OpenGL and Vulkan implementation at runtime. The Vulkan backend has reached feature parity with the OpenGL backend. At this time we find that many Vulkan drivers have bugs that Skia triggers for which we have no workaround. We are reporting bugs to vendors as we find them.

Build for Windows and Linux
---------------------------
To build the Vulkan backend add skia_vulkan=1 to your GYP_DEFINES and rerun gyp_skia. For example:

<!--?prettify lang=sh?-->
     export GYP_DEFINES="$GYP_DEFINES skia_vulkan=1"
     python ./gyp_skia

The Vulkan SDK must be installed and the VULKAN_SDK environment variable must point to the installation location. The Windows installer will set the environment variable. However, on Linux it must be set after installation.

Build as usual for your platform.


Build for Android
-----------------
The Vulkan backend will run on a device running the N release with Vulkan drivers. To build the Vulkan backend simply add --vulkan to the flags passed to ./platform_tools/android/bin/android_ninja


Using the Vulkan Backend
------------------------

To create a GrContext that is backed by Vulkan the client creates a Vulkan device and queue, initializes a GrVkBackendContext to describe the context, and then calls GrContext::Create:

<!--?prettify lang=c++?-->
    sk_sp<GrVkBackendContext> vkContext = new GrVkBackendContext;
    vkBackendContext.fInstance = vkInstance;
    vkBackendContext.fPhysicalDevice = vkPhysDevice;
    ...
    vkBackendContext.fInterface.reset(GrVkCreateInterface(instance, vkPhysDevice, extensionFlags);
    ...
  
    sk_sp<GrContext> context = GrContext::Create(kVulkan_GrBackend, (GrBackendContext) vkBackendContext);

When using the Vulkan backend the GrBackendObject field in GrBackendRenderTargetDesc and GrBackendTextureDesc is interpeted as a pointer to a GrVkImageInfo object. GrVkImageInfo specifies a VkImage and associated state (tiling, layout, format, etc). This allows the client to import externally created Vulkan images as destinations for Skia rendering via SkSurface factory functions or for to composite Skia rendered content using SkImage::getTextureHandle().

After getting a GrVkImageInfo* via getTextureHandle() or getRenderTargetHandle(), the client should check the fImageLayout field to know what layout Skia left the VkImage in before using the VkImage. If the client changes the layout of the VkImage, GrVkImageInfo::updateImageLayout(VkImageLayout layout) should be called before resuming Skia rendering.

The client is responsible for any synchronization or barriers needed before Skia performs I/O on a VkImage imported into Skia via GrVkImageInfo.
Skia will assume it can start issuing commands referencing the VkImage without the need for additional synchronization.

Vulkan
======

Skis has a Vulkan implementation of its GPU backend. The Vulkan backend can be
built alongside the OpenGL backend. The client can select between the OpenGL
and Vulkan implementation at runtime. The Vulkan backend has reached feature
parity with the OpenGL backend. At this time we find that many Vulkan drivers
have bugs that Skia triggers for which we have no workaround. We are reporting
bugs to vendors as we find them.

Windows and Linux
-----------------
To build the Vulkan backend, set `skia_vulkan_sdk` to the path to your Vulkan SDK in `args.gn`.
This defaults to the environment variable `VULKAN_SDK`.

Android
-------
The Vulkan backend can run on any device with Vulkan drivers, including all Android N+ devices.
To build the Vulkan backend, set `ndk_api = 24` in `args.gn` to target Android N.

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

When using the Vulkan backend the GrBackendObject field in
GrBackendRenderTargetDesc and GrBackendTextureDesc is interpeted as a pointer
to a GrVkImageInfo object. GrVkImageInfo specifies a VkImage and associated
state (tiling, layout, format, etc). This allows the client to import
externally created Vulkan images as destinations for Skia rendering via
SkSurface factory functions or for to composite Skia rendered content using
SkImage::getTextureHandle().

After getting a GrVkImageInfo* via getTextureHandle() or
getRenderTargetHandle(), the client should check the fImageLayout field to know
what layout Skia left the VkImage in before using the VkImage. If the client
changes the layout of the VkImage,
GrVkImageInfo::updateImageLayout(VkImageLayout layout) should be called before
resuming Skia rendering.

The client is responsible for any synchronization or barriers needed before
Skia performs I/O on a VkImage imported into Skia via GrVkImageInfo.  Skia will
assume it can start issuing commands referencing the VkImage without the need
for additional synchronization.

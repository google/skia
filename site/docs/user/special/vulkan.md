
---
title: "Vulkan"
linkTitle: "Vulkan"

---


Skia has a Vulkan implementation of its GPU backend. The Vulkan backend can be
built alongside the OpenGL backend. The client can select between the OpenGL
and Vulkan implementation at runtime. The Vulkan backend has reached feature
parity with the OpenGL backend. At this time we find that many Vulkan drivers
have bugs that Skia triggers for which we have no workaround. We are reporting
bugs to vendors as we find them.

Windows and Linux
-----------------
To build the Vulkan backend, set `skia_use_vulkan=true` in `args.gn`.

Android
-------
The Vulkan backend can run on any device with Vulkan drivers, including all Android N+ devices.
To build the Vulkan backend, set `ndk_api = 24` in `args.gn` to target Android N.

Mac
---
The Vulkan backend can be run in software emulation using SwiftShader. This will allow for
testing and debugging via `dm`. (Vulkan is not supported in interactive apps like `viewer`.)

Skia already includes the SwiftShader library as an external dependency. To build it, you
will first need to install [CMake](https://cmake.org/download/). Set up CMake for command
line use by opening the app and following the instructions in _Tools > How to Install For
Command Line Use_. Once CMake has been prepared, SwiftShader needs to be compiled. Follow
these steps, substituting your actual Skia directory instead of `$(SKIA_DIR)` below:

<!--?prettify lang=bash-->
    $ cd $(SKIA_DIR)/third_party/externals/swiftshader/build
    $ cmake ..
    $ cmake --build . --parallel

Once its build completes, SwiftShader's `build` directory should include a `Darwin`
subdirectory containing `libvk_swiftshader.dylib`. To allow Skia to see this library,
we need to reference it in `args.gn` like so:

```
skia_use_vulkan = true
extra_cflags = [ "-D", "SK_GPU_TOOLS_VK_LIBRARY_NAME=$(SKIA_DIR)/third_party/externals/swiftshader/build/Darwin/libvk_swiftshader.dylib" ]
```

Using the Vulkan Backend
------------------------

To create a GrContext that is backed by Vulkan the client creates a Vulkan device and queue, initializes a GrVkBackendContext to describe the context, and then calls GrContext::MakeVulkan:

<!--?prettify lang=c++?-->
    sk_sp<GrVkBackendContext> vkContext = new GrVkBackendContext;
    vkBackendContext.fInstance = vkInstance;
    vkBackendContext.fPhysicalDevice = vkPhysDevice;
    ...
    vkBackendContext.fInterface.reset(GrVkCreateInterface(instance, vkPhysDevice, extensionFlags);
    ...

    sk_sp<GrContext> context = GrContext::MakeVulkan(vkBackendContext);

When using the Vulkan backend, GrVkImageInfo is used to construct GrBackendTexture
and GrBackendRenderTarget objects that in turn are used to create SkSurface and SkImage
objects that refer to VkImages created by the Skia client.

The GrBackendObject returned by SkImage::getTextureHandle(),
SkSurface::getTextureHandle(), and SkSurface::getRenderTargetHandle() should be
interpreted as a GrVkImageInfo*. This allows a client to get the backing VkImage
of a SkImage or SkSurface.

GrVkImageInfo specifies a VkImage and associated state (tiling, layout, format, etc).
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


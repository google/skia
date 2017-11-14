/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfacePriv_DEFINED
#define SkSurfacePriv_DEFINED

#include "SkSurface.h"

struct SkImageInfo;

class SkSurfacePriv {
public:
    /**
     *  Returns a surface that stores no pixels. It can be drawn to via its canvas, but that
     *  canvas does not draw anything. Calling makeImageSnapshot() will return nullptr.
     */
    static sk_sp<SkSurface> MakeNull(int width, int height);

    /**
     * This creates a characterization of this SkSurface's properties that can
     * be used to perform gpu-backend preprocessing in a separate thread (via
     * the SkDeferredDisplayListRecorder).
     * It will return false on failure (e.g., if the SkSurface is cpu-backed).
     */
    static bool Characterize(const SkSurface* surface,
                             SkSurfaceCharacterization* characterization);

    /**
     * Draw a deferred display list (created via SkDeferredDisplayListRecorder).
     * The draw will be skipped if the characterization stored in the display list
     * isn't compatible with this surface.
     */
    static void Draw(SkSurface* surface, SkDeferredDisplayList* deferredDisplayList);

    /**
     * Issue any pending surface IO to the current backend 3D API. After issuing all commands,
     * numSemaphore semaphores will be signaled by the gpu. The client passes in an array of
     * numSemaphores GrBackendSemaphores. In general these GrBackendSemaphore's can be either
     * initialized or not. If they are initialized, the backend uses the passed in semaphore.
     * If it is not initialized, a new semaphore is created and the GrBackendSemaphore object
     * is initialized with that semaphore.
     *
     * The client will own and be responsible for deleting the underlying semaphores that are stored
     * and returned in initialized GrBackendSemaphore objects. The GrBackendSemaphore objects
     * themselves can be deleted as soon as this function returns.
     *
     * If the backend API is OpenGL only uninitialized GrBackendSemaphores are supported.
     * If the backend API is Vulkan either initialized or unitialized semaphores are supported.
     * If unitialized, the semaphores which are created will be valid for use only with the VkDevice
     * with which they were created.
     *
     * If this call returns GrSemaphoresSubmited::kNo, the GPU backend will not have created or
     * added any semaphores to signal on the GPU. Thus the client should not have the GPU wait on
     * any of the semaphores. However, any pending surface IO will still be flushed.
     */
    static GrSemaphoresSubmitted FlushAndSignalSemaphores(SkSurface* surface, int numSemaphores,
                                                          GrBackendSemaphore signalSemaphores[]);

    static const SkSurfaceProps& Props(SkSurface* surface) { return surface->fProps; }

    /**
     * Inserts a list of GPU semaphores that the current backend 3D API must wait on before
     * executing any more commands on the GPU for this surface. Skia will take ownership of the
     * underlying semaphores and delete them once they have been signaled and waited on.
     *
     * If this call returns false, then the GPU backend will not wait on any passed in semaphores,
     * and the client will still own the semaphores.
     */
    static bool Wait(SkSurface* surface, int numSemaphores,
                     const GrBackendSemaphore* waitSemaphores);

};

static inline SkSurfaceProps SkSurfacePropsCopyOrDefault(const SkSurfaceProps* props) {
    if (props) {
        return *props;
    } else {
        return SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    }
}

static inline SkPixelGeometry SkSurfacePropsDefaultPixelGeometry() {
    return SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType).pixelGeometry();
}

constexpr size_t kIgnoreRowBytesValue = static_cast<size_t>(~0);

bool SkSurfaceValidateRasterInfo(const SkImageInfo&, size_t rb = kIgnoreRowBytesValue);

#endif

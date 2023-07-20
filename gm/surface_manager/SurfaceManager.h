/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceManager_DEFINED
#define SurfaceManager_DEFINED

#include "include/core/SkSurface.h"

#include <string>

// Abstract class to create and manage surfaces.
class SurfaceManager {
public:
    // Constructs a SurfaceManager for the given config name (e.g. "8888", "565", "gles"). It
    // returns nullptr if the config is unknown, and it aborts execution if the config is known but
    // we weren't able to construct the surface for any reason.
    static std::unique_ptr<SurfaceManager> FromConfig(std::string config, int width, int height);

    // Returns the surface created from the given config. All calls return the same surface.
    virtual sk_sp<SkSurface> getSurface() = 0;

    // Flushes the surface. This method should be called after the GM is done drawing. This ensures
    // that all commands are flushed to the GPU in the case of Ganesh-backed surfaces. Failing to
    // do so may lead to blank pixmaps.
    virtual void flush() = 0;

    virtual ~SurfaceManager() = default;
};

#endif  // SurfaceManager_DEFINED

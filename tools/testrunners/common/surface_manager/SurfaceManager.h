/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceManager_DEFINED
#define SurfaceManager_DEFINED

#include "include/core/SkSurface.h"

#include <map>
#include <memory>
#include <string>

struct GrContextOptions;

namespace skgpu::graphite {
struct ContextOptions;
}

namespace sk_gpu_test {
class ContextInfo;
};

// Gathers the parameters needed by SurfaceManager::FromConfig to construct a surface for a given
// configuration.
struct SurfaceOptions {
    int width;
    int height;

    std::function<void(GrContextOptions*)> modifyGrContextOptions = nullptr;
    std::function<void(skgpu::graphite::ContextOptions*)> modifyGraphiteContextOptions = nullptr;
};

// Abstract class to create and manage surfaces used by various kinds of tests (GMs, Benchmarks,
// etc.).
class SurfaceManager {
public:
    enum class CpuOrGpu { kCPU, kGPU };

    // Constructs a SurfaceManager for the given config name (e.g. "8888", "565", "gles"). It
    // returns nullptr if the config is unknown, and it aborts execution if the config is known but
    // we weren't able to construct the surface for any reason.
    static std::unique_ptr<SurfaceManager> FromConfig(std::string config,
                                                      SurfaceOptions surfaceOptions);

    // Returns the surface created from the given config. All calls return the same surface.
    virtual sk_sp<SkSurface> getSurface() = 0;

    // Flushes the surface. This method should be called after the test is done drawing. This
    // ensures that all commands are flushed to the GPU in the case of Ganesh-backed surfaces.
    // Failing to do so may lead to blank pixmaps.
    virtual void flush() {}

    // Returns the subset of Gold key/value pairs that are determined by the surface config. The
    // returned map includes keys "cpu_or_gpu" and "cpu_or_gpu_value", which are populated based
    // on the cpuName and gpuName arguments, and whether the surface config is CPU or GPU bound.
    // The returned map also includes various keys pertaining to color, which are generated from
    // the SkColorInfo passed to this class' constructor.
    std::map<std::string, std::string> getGoldKeyValuePairs(std::string cpuName,
                                                            std::string gpuName) const;

    // Returns the subset of Perf key/value pairs that are determined by the surface config. The
    // returned map includes keys "cpu_or_gpu" and "cpu_or_gpu_value", which are populated based
    // on the cpuName and gpuName arguments, and whether the surface config is CPU or GPU bound.
    std::map<std::string, std::string> getPerfKeyValuePairs(std::string cpuName,
                                                            std::string gpuName) const;

    // Returns an enum indicating whether the surface is CPU or GPU bound.
    CpuOrGpu isCpuOrGpuBound() const;

    // Returns the Ganesh ContextInfo on SurfaceManager implementations that support it.
    virtual sk_gpu_test::ContextInfo* getGaneshContextInfo() {
        SK_ABORT("This SurfaceManager implementation does not support the requested operation.");
    }

    virtual ~SurfaceManager() = default;

protected:
    // Takes the config name passed to FromConfig(), and the SkColorInfo used by FromConfig() to
    // create the surface.
    SurfaceManager(std::string config, SkColorInfo colorInfo, CpuOrGpu cpuOrGpu)
            : fConfig(config), fColorInfo(colorInfo), fCpuOrGpu(cpuOrGpu) {}

private:
    std::string fConfig;
    SkColorInfo fColorInfo;
    CpuOrGpu fCpuOrGpu;

    std::map<std::string, std::string> getCpuOrGpuKeyValuePairs(std::string cpuName,
                                                                std::string gpuName) const;
};

#endif  // SurfaceManager_DEFINED

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Recorder_DEFINED
#define skgpu_Recorder_DEFINED

#include "experimental/graphite/src/TaskGraph.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class Context;
class Device;
class DrawBufferManager;
class Recording;
class UniformCache;

class Recorder final {
public:
    ~Recorder();

    void add(sk_sp<Task>);

    Context* context() const;
    UniformCache* uniformCache();
    DrawBufferManager* drawBufferManager();

    std::unique_ptr<Recording> snap();

#if GR_TEST_UTILS
    bool deviceIsRegistered(Device*);
#endif

private:
    friend class Context; // For ctor
    friend class Device; // For registering and deregistering Devices;

    Recorder(sk_sp<Context>);

    // We keep track of all Devices that are connected to a Recorder. This allows the client to
    // safely delete an SkSurface or a Recorder in any order. If the client deletes the Recorder
    // we need to notify all Devices that the Recorder is no longer valid. If we delete the
    // SkSurface/Device first we will flush all the Device's into the Recorder before deregistering
    // it from the Recorder.
    //
    // We do not need to take a ref on the Device since the Device will flush and deregister itself
    // in its dtor. There is no other need for the Recorder to know about the Device after this
    // point.
    //
    // Note: We could probably get by with only registering Devices directly connected to
    // SkSurfaces. All other one off Devices will be created in a controlled scope where the
    // Recorder should still be valid by the time they need to flush their work when the Device is
    // deleted. We would have to make sure we safely handle cases where a client calls saveLayer
    // then either deletes the SkSurface or Recorder before calling restore. For simplicity we just
    // register every device for now, but if we see extra overhead in pushing back the extra
    // pointers, we can look into only registering SkSurface Devices.
    void registerDevice(Device*);
    void deregisterDevice(const Device*);

    sk_sp<Context> fContext;
    TaskGraph fGraph;
    std::unique_ptr<UniformCache> fUniformCache;
    std::unique_ptr<DrawBufferManager> fDrawBufferManager;
    std::vector<Device*> fTrackedDevices;
};

} // namespace skgpu

#endif // skgpu_Recorder_DEFINED

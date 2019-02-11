/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a simple OpenCL Hello World that tests you have a functioning OpenCL setup.

#include "cl.hpp"
#include <initializer_list>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

static inline void assert_cl(cl_int rc, const char* file, int line) {
    if (rc != CL_SUCCESS) {
        fprintf(stderr, "%s:%d, got OpenCL error code %d\n", file,line,rc);
        exit(1);
    }
}
#define cl_ok(err) assert_cl(err, __FILE__, __LINE__)

int main(int, char**) {
    std::vector<cl::Platform> platforms;
    cl_ok(cl::Platform::get(&platforms));

    std::vector<cl::Device> devices;
    for (cl::Platform platform : platforms) {
        std::vector<cl::Device> platform_devices;
        cl_ok(platform.getDevices(CL_DEVICE_TYPE_ALL, &platform_devices));
        devices.insert(devices.end(), platform_devices.begin(), platform_devices.end());
    }

    if (devices.empty()) {
        fprintf(stderr, "No OpenCL devices available. :(\n");
        return 1;
    }

    // To keep things simple we'll only create single-device cl::Contexts.
    for (cl::Device device : devices) {
        std::string name,
                    version,
                    driver,
                    vendor,
                    extensions;
        cl_ok(device.getInfo(CL_DEVICE_NAME,       &name));
        cl_ok(device.getInfo(CL_DEVICE_VERSION,    &version));
        cl_ok(device.getInfo(CL_DEVICE_VENDOR,     &vendor));
        cl_ok(device.getInfo(CL_DEVICE_EXTENSIONS, &extensions));
        cl_ok(device.getInfo(CL_DRIVER_VERSION,    &driver));

        fprintf(stdout, "Using %s%s, vendor %s, version %s, extensions:\n%s\n",
                version.c_str(), name.c_str(), vendor.c_str(), driver.c_str(), extensions.c_str());

        std::vector<cl::Device> devices = { device };

        // Some APIs can't return their cl_int error but might still fail,
        // so they take a pointer.  cl_ok() is really handy here too.
        cl_int ok;
        cl::Context ctx(devices,
                        nullptr/*optional cl_context_properties*/,
                        nullptr/*optional error reporting callback*/,
                        nullptr/*context argument for error reporting callback*/,
                        &ok);
        cl_ok(ok);

        cl::Program program(ctx,
                            "__kernel void mul(__global const float* a,    "
                            "                  __global const float* b,    "
                            "                  __global       float* dst) {"
                            "    int i = get_global_id(0);                 "
                            "    dst[i] = a[i] * b[i];                     "
                            "}                                             ",
                            /*and build now*/true,
                            &ok);
        cl_ok(ok);

        std::vector<float> a,b,p;
        for (int i = 0; i < 1000; i++) {
            a.push_back(+i);
            b.push_back(-i);
            p.push_back( 0);
        }

        cl::Buffer
            A(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(float)*a.size(), a.data()),
            B(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(float)*b.size(), b.data()),
            P(ctx, CL_MEM_WRITE_ONLY| CL_MEM_HOST_READ_ONLY, sizeof(float)*p.size());

        cl::Kernel mul(program, "mul", &ok);
        cl_ok(ok);
        cl_ok(mul.setArg(0, A));
        cl_ok(mul.setArg(1, B));
        cl_ok(mul.setArg(2, P));

        cl::CommandQueue queue(ctx, device);

        cl_ok(queue.enqueueNDRangeKernel(mul, cl::NDRange(0)  /*offset*/
                                            , cl::NDRange(1000) /*size*/));

        cl_ok(queue.enqueueReadBuffer(P, true/*block until read is done*/
                                       , 0                     /*offset in bytes*/
                                       , sizeof(float)*p.size() /*size in bytes*/
                                       , p.data()));

        fprintf(stdout, "OpenCL sez: %g x %g = %g\n", a[42], b[42], p[42]);
        for (int i = 0; i < 1000; i++) {
            if (p[i] != a[i]*b[i]) {
                return 1;
            }
        }
    }

    return 0;
}

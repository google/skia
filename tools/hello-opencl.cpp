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

// cl(Foo(...)) calls clFoo(...) and asserts that it succeeds.  Handy for calling OpenCL C APIs.
#define cl(...)    assert_cl((cl##__VA_ARGS__), __FILE__, __LINE__)

// Just assert the result of an OpenCL call.  Handy for the OpenCL C++ wrapper APIs.
#define cl_ok(err) assert_cl(err,               __FILE__, __LINE__)

int main(int argc, char** argv) {
    // Find any OpenCL platform+device with these substrings.
    const char* platform_match = argc > 1 ? argv[1] : "";
    const char* device_match   = argc > 2 ? argv[2] : "";

    // TODO: port these platform and device searches to the C++ wrapper APIs.

    cl_platform_id platform_id{};
    {
        cl_uint n;
        cl(GetPlatformIDs(0, nullptr, &n));

        std::vector<cl_platform_id> ids(n);
        cl(GetPlatformIDs(n, ids.data(), nullptr));

        bool found = false;
        for (cl_platform_id id : ids) {
            size_t len;
            cl(GetPlatformInfo(id, CL_PLATFORM_NAME,
                               0, nullptr, &len));

            std::string name(len, ' ');
            cl(GetPlatformInfo(id, CL_PLATFORM_NAME,
                               len, &name.front(), nullptr));

            fprintf(stdout, "Available platform: %s\n", name.c_str());

            if (name.find(platform_match) != std::string::npos) {
                platform_id = id;
                found = true;
            }
        }
        if (!found) {
            fprintf(stderr, "No platform containing '%s' found.\n", platform_match);
            exit(1);
        }
    }

    cl_device_id device_id{};
    {
        cl_uint n;
        cl(GetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL,
                        0, nullptr, &n));

        std::vector<cl_device_id> ids(n);
        cl(GetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL,
                        n, ids.data(), nullptr));

        bool found = false;
        for (cl_device_id id : ids) {
            size_t len;
            cl(GetDeviceInfo(id, CL_DEVICE_NAME,
                             0, nullptr, &len));

            std::string name(len, ' ');
            cl(GetDeviceInfo(id, CL_DEVICE_NAME,
                             len, &name.front(), nullptr));

            cl(GetDeviceInfo(id, CL_DRIVER_VERSION,
                             0, nullptr, &len));

            std::string version(len, ' ');
            cl(GetDeviceInfo(id, CL_DRIVER_VERSION,
                             len, &version.front(), nullptr));

            fprintf(stdout, "Available device %s, version %s\n", name.c_str(), version.c_str());

            if (name.find(device_match) != std::string::npos) {
                device_id = id;
                found = true;
            }
        }
        if (!found) {
            fprintf(stderr, "No device containing '%s' found.\n", device_match);
            exit(2);
        }
    }

    cl::Device device(device_id);

    std::string name,
                vendor,
                extensions;
    cl_ok(device.getInfo(CL_DEVICE_NAME,       &name));
    cl_ok(device.getInfo(CL_DEVICE_VENDOR,     &vendor));
    cl_ok(device.getInfo(CL_DEVICE_EXTENSIONS, &extensions));

    fprintf(stdout, "Picked name %s, vendor %s, extensions:\n%s\n",
            name.c_str(), vendor.c_str(), extensions.c_str());

    std::vector<cl::Device> devices = { device };

    // Some APIs can't return their cl_int error but might still fail,
    // so they take a pointer.  cl_ok() is really handy here too.
    cl_int ok;
    cl::Context ctx(devices,
                    nullptr/*optional cl_context_properties*/,
                    nullptr/*optional error reporting callback*/,
                    nullptr/*context arguement for error reporting callback*/,
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

    cl::Buffer A(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(float)*a.size(), a.data()),
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

    for (int i = 0; i < 1000; i++) {
        if (p[i] != a[i]*b[i]) {
            return 1;
        }
    }

    fprintf(stdout, "OpenCL sez: %g x %g = %g\n", a[42], b[42], p[42]);
    return 0;
}

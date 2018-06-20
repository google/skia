/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a simple OpenCL Hello World that tests you have a functioning OpenCL setup.

#include <CL/cl.hpp>
#include <initializer_list>

extern "C" {
    #include "cl/assert_cl.h"   // for cl(), cl_ok() macros
    #include "cl/find_cl.h"     // for clFindIdsByName
}

int main(int argc, char** argv) {
    // Find any OpenCL platform+device with these substrings.
    const char* platform_match = argc > 1 ? argv[1] : "";
    const char* device_match   = argc > 2 ? argv[2] : "";

    cl_platform_id platform_id;
    cl_device_id   device_id;

    char device_name[256];
    size_t device_name_len;

    // clFindIdsByName will narrate what it's doing when this is set.
    bool verbose = true;

    // The cl() macro prepends cl to its argument, calls it, and asserts that it succeeded,
    // printing out the file, line, and somewhat readable version of the error code on failure.
    //
    // It's generally used to call OpenCL APIs, but here we've written clFindIdsByName to match
    // the convention, as its error conditions are just going to be passed along from OpenCL.
    cl(FindIdsByName(platform_match,  device_match,
                     &platform_id,    &device_id,
                     sizeof(device_name), device_name, &device_name_len,
                     verbose));

    printf("picked %.*s\n", (int)device_name_len, device_name);

    // Allan's code is all C using OpenCL's C API,
    // but we can mix that freely with the C++ API found in cl.hpp.
    // cl_ok() comes in handy here, which is cl() without the extra cl- prefix.

    cl::Device device(device_id);

    std::string name,
                vendor,
                extensions;
    cl_ok(device.getInfo(CL_DEVICE_NAME,       &name));
    cl_ok(device.getInfo(CL_DEVICE_VENDOR,     &vendor));
    cl_ok(device.getInfo(CL_DEVICE_EXTENSIONS, &extensions));

    printf("name %s, vendor %s, extensions:\n%s\n",
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

    printf("OpenCL sez: %g x %g = %g\n", a[42], b[42], p[42]);
    return 0;
}

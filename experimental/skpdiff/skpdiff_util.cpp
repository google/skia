/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <time.h>
#include <dirent.h>
#include <glob.h>
#include "SkOSFile.h"
#include "skpdiff_util.h"

#if SK_SUPPORT_OPENCL
const char* cl_error_to_string(cl_int err) {
    switch (err) {
        case CL_SUCCESS:                         return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND:                return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:            return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE:          return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:   return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES:                return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY:              return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE:    return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP:                return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH:           return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:      return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE:           return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE:                     return "CL_MAP_FAILURE";
        case CL_INVALID_VALUE:                   return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE:             return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM:                return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE:                  return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT:                 return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES:        return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE:           return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR:                return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT:              return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE:              return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER:                 return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY:                  return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS:           return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM:                 return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE:      return "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME:             return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION:       return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL:                  return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX:               return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE:               return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE:                return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS:             return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION:          return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE:         return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE:          return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET:           return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST:         return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT:                   return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION:               return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT:               return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE:             return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL:               return "CL_INVALID_MIP_LEVEL";
        default:                                 return "UNKNOWN";
    }
    return "UNKNOWN";
}
#endif


double get_seconds() {
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    return currentTime.tv_sec + (double)currentTime.tv_nsec / 1e9;
}

bool get_directory(const char path[], SkTArray<SkString>* entries) {
    // Open the directory and check for success
    DIR* dir = opendir(path);
    if (NULL == dir) {
        return false;
    }

    // Loop through dir entries until there are none left (i.e. readdir returns NULL)
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        // dirent only gives relative paths, we need to join them to the base path to check if they
        // are directories.
        SkString joinedPath = SkOSPath::SkPathJoin(path, entry->d_name);

        // We only care about files
        if (!sk_isdir(joinedPath.c_str())) {
            entries->push_back(SkString(entry->d_name));
        }
    }

    closedir(dir);

    return true;
}

bool glob_files(const char globPattern[], SkTArray<SkString>* entries) {
    // TODO Make sure this works on windows. This may require use of FindNextFile windows function.
    glob_t globBuffer;
    if (glob(globPattern, 0, NULL, &globBuffer) != 0) {
        return false;
    }

    // Note these paths are in sorted order by default according to http://linux.die.net/man/3/glob
    // Check under the flag GLOB_NOSORT
    char** paths = globBuffer.gl_pathv;
    while(NULL != *paths) {
        entries->push_back(SkString(*paths));
        paths++;
    }

    globfree(&globBuffer);

    return true;
}

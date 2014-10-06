/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)
#   include <unistd.h>
#   include <sys/time.h>
#   include <dirent.h>
#endif

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
#   include <glob.h>
#endif

#if defined(SK_BUILD_FOR_MAC)
#   include <sys/syslimits.h> // PATH_MAX is here for Macs
#endif

#if defined(SK_BUILD_FOR_WIN32)
#   include <windows.h>
#endif

#include <stdlib.h>
#include <time.h>
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

// TODO refactor Timer to be used here
double get_seconds() {
#if defined(SK_BUILD_FOR_WIN32)
    LARGE_INTEGER currentTime;
    LARGE_INTEGER frequency;
    QueryPerformanceCounter(&currentTime);
    QueryPerformanceFrequency(&frequency);
    return (double)currentTime.QuadPart / (double)frequency.QuadPart;
#elif _POSIX_TIMERS > 0 && defined(CLOCK_REALTIME)
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    return currentTime.tv_sec + (double)currentTime.tv_nsec / 1e9;
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec + (double)currentTime.tv_usec / 1e6;
#else
    return clock() / (double)CLOCKS_PER_SEC;
#endif
}

bool get_directory(const char path[], SkTArray<SkString>* entries) {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)
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
        SkString joinedPath = SkOSPath::Join(path, entry->d_name);

        // We only care about files
        if (!sk_isdir(joinedPath.c_str())) {
            entries->push_back(SkString(entry->d_name));
        }
    }

    closedir(dir);

    return true;
#elif defined(SK_BUILD_FOR_WIN32)
    char pathDirGlob[MAX_PATH];
    size_t pathLength = strlen(path);
    strncpy(pathDirGlob, path, pathLength);

    if (path[pathLength - 1] == '/' || path[pathLength - 1] == '\\') {
        SkASSERT(pathLength + 2 <= MAX_PATH);
        pathDirGlob[pathLength] = '*';
        pathDirGlob[pathLength + 1] = '\0';
    } else {
        SkASSERT(pathLength + 3 <= MAX_PATH);
        pathDirGlob[pathLength] = '\\';
        pathDirGlob[pathLength + 1] = '*';
        pathDirGlob[pathLength + 2] = '\0';
    }

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(pathDirGlob, &findFileData);
    if (INVALID_HANDLE_VALUE == hFind) {
        return false;
    }

    do {
        if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            entries->push_back(SkString(findFileData.cFileName));
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return true;
#else
    return false;
#endif
}

bool glob_files(const char globPattern[], SkTArray<SkString>* entries) {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
    // TODO Make sure this works on windows. This may require use of FindNextFile windows function.
    glob_t globBuffer;
    if (glob(globPattern, 0, NULL, &globBuffer) != 0) {
        return false;
    }

    // Note these paths are in sorted order by default according to http://linux.die.net/man/3/glob
    // Check under the flag GLOB_NOSORT
    char** paths = globBuffer.gl_pathv;
    while(*paths) {
        entries->push_back(SkString(*paths));
        paths++;
    }

    globfree(&globBuffer);

    return true;
#else
    return false;
#endif
}

SkString get_absolute_path(const SkString& path) {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)
    SkString fullPath(PATH_MAX + 1);
    if (realpath(path.c_str(), fullPath.writable_str()) == NULL) {
        fullPath.reset();
    }
    return fullPath;
#elif defined(SK_BUILD_FOR_WIN32)
    SkString fullPath(MAX_PATH);
    if (_fullpath(fullPath.writable_str(), path.c_str(), MAX_PATH) == NULL) {
        fullPath.reset();
    }
    return fullPath;
#else
    return SkString();
#endif
}

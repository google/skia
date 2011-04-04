/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrTypes.h"
#include "GrGLInterface.h"
#include "GrGLDefines.h"

#include <stdio.h>

GrGLInterface* gGLInterface = NULL;

void gl_version_from_string(int* major, int* minor,
                            const char* versionString) {
    if (NULL == versionString) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }

    int n = sscanf(versionString, "%d.%d", major, minor);
    if (2 == n) {
      return;
    }

    char profile[2];
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile+1,
               major, minor);
    bool ok = 4 == n;
    if (!ok) {
        n = sscanf(versionString, "OpenGL ES %d.%d", major, minor);
        ok = 2 == n;
    }

    if (!ok) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
}

bool has_gl_extension_from_string(const char* ext,
                                  const char* extensionString) {
    int extLength = strlen(ext);

    while (true) {
        int n = strcspn(extensionString, " ");
        if (n == extLength && 0 == strncmp(ext, extensionString, n)) {
            return true;
        }
        if (0 == extensionString[n]) {
            return false;
        }
        extensionString += n+1;
    }

    return false;
}


GR_API void GrGLSetGLInterface(GrGLInterface* gl_interface) {
    gGLInterface = gl_interface;
}

GR_API GrGLInterface* GrGLGetGLInterface() {
    return gGLInterface;
}

bool has_gl_extension(const char* ext) {
    const char* glstr = reinterpret_cast<const char*>(
                GrGLGetGLInterface()->fGetString(GR_GL_EXTENSIONS));

    return has_gl_extension_from_string(ext, glstr);
}

void gl_version(int* major, int* minor) {
    const char* v = reinterpret_cast<const char*>(
                GrGLGetGLInterface()->fGetString(GR_GL_VERSION));
    gl_version_from_string(major, minor, v);
}

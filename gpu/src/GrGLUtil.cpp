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

#include "GrGLConfig.h"
#include "GrGLInterface.h"

void GrGLClearErr() {
    while (GR_GL_NO_ERROR != GrGLGetGLInterface()->fGetError()) {}
}

void GrGLCheckErr(const char* location, const char* call) {
    uint32_t err =  GrGLGetGLInterface()->fGetError();
    if (GR_GL_NO_ERROR != err) {
        GrPrintf("---- glGetError %x", err);
        if (NULL != location) {
            GrPrintf(" at\n\t%s", location);
        }
        if (NULL != call) {
            GrPrintf("\n\t\t%s", call);
        }
        GrPrintf("\n");
    }
}

void GrGLRestoreResetRowLength() {
    if (GR_GL_SUPPORT_DESKTOP) {
        GR_GL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
}

///////////////////////////////////////////////////////////////////////////////

#if GR_GL_LOG_CALLS
    bool gLogCallsGL = !!(GR_GL_LOG_CALLS_START);
#endif

#if GR_GL_CHECK_ERROR
    bool gCheckErrorGL = !!(GR_GL_CHECK_ERROR_START);
#endif


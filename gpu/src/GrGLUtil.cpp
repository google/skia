
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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


/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkGetExecutablePath.h"

#include <windows.h>

std::string SkGetExecutablePath() {
    char executableFileBuf[MAX_PATH];
    DWORD executablePathLen = GetModuleFileNameA(nullptr, executableFileBuf, MAX_PATH);
    return (executablePathLen > 0) ? std::string(executableFileBuf) : std::string();
}

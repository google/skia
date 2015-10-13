//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils.h: declaration of OS-specific utility functions

#ifndef SAMPLE_UTIL_PATH_UTILS_H
#define SAMPLE_UTIL_PATH_UTILS_H

#include <string>

namespace angle
{

std::string GetExecutablePath();
std::string GetExecutableDirectory();

// Cross platform equivalent of the Windows Sleep function
void Sleep(unsigned int milliseconds);

void SetLowPriorityProcess();

} // namespace angle

#endif // SAMPLE_UTIL_PATH_UTILS_H

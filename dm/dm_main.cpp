/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CrashHandler.h"
#include "SkCommandLineFlags.h"
#include "dm.h"

int main(int argc, char * const argv[]) {
    SetupCrashHandler();
    SkCommandLineFlags::Parse(argc, const_cast<char**>(argv));
    return dm_main();
}

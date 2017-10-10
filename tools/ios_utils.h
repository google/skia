/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ios_util_DEFINED
#define ios_util_DEFINED

#if defined(__cplusplus)
extern "C" {
#endif

    // cd into the current app's Documents/ directory.
    // (This is the only directory we can easily read and write.)
    void cd_Documents(void);

#if defined(__cplusplus)
}
#endif

#endif//ios_util_DEFINED

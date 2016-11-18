/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * We don't want any of SDL's prebaked SDL_config.h files.
 * Instead we set all the defines we need in GN (third_party/libsdl/BUILD.gn).
 * This header is just the barest basics of an SDL_config.h we can get away with.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "picture_utils.h"
#include "SkBitmap.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkStream.h"

namespace sk_tools {

    void make_filepath(SkString* path, const char* dir, const SkString& name) {
        size_t len = strlen(dir);
        path->set(dir);
        if (0 < len  && '/' != dir[len - 1]) {
            path->append("/");
        }
        path->append(name);
    }

    void setup_bitmap(SkBitmap* bitmap, int width, int height) {
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bitmap->allocPixels();
        bitmap->eraseColor(0);
    }

}

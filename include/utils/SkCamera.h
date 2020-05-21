/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "client_utils/android/View3D.h"

// Temporary, until Android updates to the new API
class Sk3DView : public android::skia::View3D {
public:
    ~Sk3DView() override {}
};

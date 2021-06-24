/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public enum ClipOp {
    DIFFERENCE(0),
    INTERSECT(1);

    ClipOp(int nativeInt) {
        this.mNativeInt = nativeInt;
    }

    int mNativeInt;
}

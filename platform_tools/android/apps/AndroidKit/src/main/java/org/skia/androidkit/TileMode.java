/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public enum TileMode {
    /**
     *  Replicate the edge color if the shader draws outside of its
     *  original bounds.
     */
    CLAMP,

    /**
     *  Repeat the shader's image horizontally and vertically.
     */
    REPEAT,

    /**
     *  Repeat the shader's image horizontally and vertically, alternating
     *  mirror images so that adjacent images always seam.
     */
    MIRROR,


    /**
     *  Only draw within the original domain, return transparent-black everywhere else.
     */
    DECAL,
}

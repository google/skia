/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1.samples;

import org.skia.androidkit.Canvas;

public interface Sample {
    /**
     * Renders a Sample frame t (ms) into the destination rect [L T R B].
     */
    public void render(Canvas canvas, long t, float left, float top, float right, float bottom);
}

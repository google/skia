/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class ComposeColorFilter extends ColorFilter {
    public ComposeColorFilter(ColorFilter outer, ColorFilter inner) {
        super(nMakeCompose(outer.getNativeInstance(), inner.getNativeInstance()));
    }

    private static native long nMakeCompose(long outer, long inner);
};
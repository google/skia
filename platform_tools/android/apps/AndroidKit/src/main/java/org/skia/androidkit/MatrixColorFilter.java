/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import java.lang.IllegalArgumentException;

public class MatrixColorFilter extends ColorFilter {
    public MatrixColorFilter(float[] m) throws IllegalArgumentException {
        super(makeNative(m));
    }

    private static long makeNative(float[] m) throws IllegalArgumentException {
        if (m.length != 20) {
            throw new IllegalArgumentException("Expecting an array of 20 floats.");
        }

        return nMakeMatrix(m);
    }

    private static native long nMakeMatrix(float[] m);
};

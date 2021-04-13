/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class Color {
    private float mR, mG, mB, mA;

    public Color(float r, float g, float b, float a) {
        mR = r;
        mG = g;
        mB = b;
        mA = a;
    }

    public Color() {
        this(0, 0, 0, 1);
    }

    public float r() { return mR; }
    public float g() { return mG; }
    public float b() { return mB; }
    public float a() { return mA; }

    public void setR(float r) { mR = r; }
    public void setG(float g) { mG = g; }
    public void setB(float b) { mB = b; }
    public void setA(float a) { mA = a; }
}

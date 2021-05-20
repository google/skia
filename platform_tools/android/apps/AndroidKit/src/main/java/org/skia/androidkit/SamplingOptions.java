/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class SamplingOptions {
    private FilterMode mFilter;
    private MipmapMode mMipmap;
    private float      mCubicCoeffB,
                       mCubicCoeffC;
    private boolean    mUseCubic;

    public enum FilterMode {
        NEAREST,
        LINEAR,
    };

    public enum MipmapMode {
        NONE,
        NEAREST,
        LINEAR,
    }

    public SamplingOptions() {
        this(FilterMode.NEAREST);
    }

    public SamplingOptions(FilterMode f) {
        this(f, MipmapMode.NONE);
    }

    public SamplingOptions(FilterMode f, MipmapMode m) {
        mFilter = f;
        mMipmap = m;
        mUseCubic = false;
    }

    public SamplingOptions(float cubicCoeffB, float cubicCoeffC) {
        mFilter = FilterMode.NEAREST;
        mMipmap = MipmapMode.NONE;
        mCubicCoeffB = cubicCoeffB;
        mCubicCoeffC = cubicCoeffC;
        mUseCubic = true;
    }

    public static SamplingOptions MITCHELL() {
        return new SamplingOptions(1/3.0f, 1/3.0f);
    }

    public static SamplingOptions CATMULLROM() {
        return new SamplingOptions(0.0f, 1/2.0f);
    }

    // package private
    int getNativeDesc() {
        // Encode all options except coefficients in a bit field:
        //
        //   b0   -> useCubic
        //   b1   -> filter
        //   b2,3 -> mipmap

        int desc = mUseCubic ? 0x01 : 0x00;

        switch (mFilter) {
        case NEAREST:
            break;
        case LINEAR:
            desc |= 0x02;
            break;
        }

        switch (mMipmap) {
        case NONE:
            break;
        case NEAREST:
            desc |= 0x04;
            break;
        case LINEAR:
            desc |= 0x08;
            break;
        }

        return desc;
    }

    float getCubicCoeffB() {
        return mCubicCoeffB;
    }

    float getCubicCoeffC() {
        return mCubicCoeffC;
    }
}
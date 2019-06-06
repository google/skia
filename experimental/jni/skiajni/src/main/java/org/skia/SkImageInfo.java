// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkColorSpace;
import org.skia.SkAlphaType;
import org.skia.SkColorType;

public class SkImageInfo {
    private native void nativeInit(int width, int height, SkColorType ct, SkAlphaType at, SkColorSpace cs);

    public SkImageInfo(int width, int height, SkColorType ct, SkAlphaType at, SkColorSpace cs) {
        nativeInit( width,  height,  ct,  at,  cs);
    }

    public native int getHeight();

    public native int getWidth();

    public native SkColorSpace getColorSpace();

    public native SkColorType getColorType();

    public native SkAlphaType getAlphaType();

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkColorSpace.referenced();
        SkAlphaType.referenced();
        SkColorType.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkImageInfo(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}

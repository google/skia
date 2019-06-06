// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkColorSpace;

public class SkColorSpace {
    public static native SkColorSpace newSRGB();

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
        staticInit();
    }

    static void referenced() {}

    private SkColorSpace(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}

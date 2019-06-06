// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkData;

public class SkData {
    public native int getSize();

    public native byte[] getData();

    private native void nativeInit(byte[] src);

    public SkData(byte[] src) {
        nativeInit( src);
    }

    public static native SkData newSubset(SkData src, int offset, int length);

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkData.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkData(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}

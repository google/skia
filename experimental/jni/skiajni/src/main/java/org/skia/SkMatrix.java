// This is an automatically generated file. Please do not edit.

package org.skia;


public class SkMatrix {
    private native void nativeInit();

    public SkMatrix() {
        nativeInit();
    }

    public SkMatrix(float[] mat) {
        this();
        this.mat = mat;
    }

    public float[] mat;

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        staticInit();
    }

    static void referenced() {}

    private SkMatrix(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}

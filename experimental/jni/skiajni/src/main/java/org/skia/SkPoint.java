// This is an automatically generated file. Please do not edit.

package org.skia;


public class SkPoint {
    private native void nativeInit();

    public SkPoint() {
        nativeInit();
    }

    public SkPoint(float x, float y) {
        this();
        this.x = x;
        this.y = y;
    }

    public float x;

    public float y;

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

    private SkPoint(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}

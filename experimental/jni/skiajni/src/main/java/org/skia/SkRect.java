// This is an automatically generated file. Please do not edit.

package org.skia;


public class SkRect {
    private native void nativeInit();

    public SkRect() {
        nativeInit();
    }

    public SkRect(float left, float top, float right, float bottom) {
        this();
        this.left = left;
        this.top = top;
        this.right = right;
        this.bottom = bottom;
    }

    public float left;

    public float top;

    public float right;

    public float bottom;

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

    private SkRect(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
